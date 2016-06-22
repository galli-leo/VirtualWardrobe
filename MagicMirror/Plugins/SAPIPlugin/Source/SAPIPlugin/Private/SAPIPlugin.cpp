// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SAPIPluginPrivatePCH.h"
#include "SAPIActor.h"
#include "ISpAudioPlug.h"
#include "AllowWindowsPlatformTypes.h"
#include "atlsafe.h"
#include "xml2json.hpp"
#include <regex>


#pragma warning(push)
#pragma warning(disable: 4191) // warning C4191: 'type cast' : unsafe conversion
#pragma warning(disable: 4996) // error C4996: 'GetVersionEx': was declared deprecated

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// atltransactionmanager.h doesn't use the W equivalent functions, use this workaround
#ifndef DeleteFile
#define DeleteFile DeleteFileW
#endif
#ifndef MoveFile
#define MoveFile MoveFileW
#endif

#include <atlbase.h>

#undef DeleteFile
#undef MoveFile

#include <sphelper.h>

#pragma warning(pop)

#define _ATL_APARTMENT_THREADED


// Code based on tutorial at 
// http://msdn.microsoft.com/en-us/library/ms720163(VS.85).aspx

#include <sapi.h>
#include <sphelper.h>
#include "comdef.h"
#include "ComPointer.h"


class FSAPIPlugin : public ISAPIPlugin
{

  DEFINE_LOG_CATEGORY(LogSAPI);
  /** IModuleInterface implementation */
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
	

  /** Initialize COM and set up SAPI for TTS and SR */
  void Init(ELanguage srLanguage = ELanguage::Lang_EnglishUS);

  // SR

  /** Add a new command to the SR grammar
      @param command A word or phrase to be recognized
      @return The id assigned to the new rule, or -1 if operation failed
  */
  int AddCommand(const FString& command);

  /** A callback used to inform the client when a word or
      phrase has been recognized. The first parameter is the ID of the
      recognized phrase (returned when it was added) and the second is
      the recognized phrase itself.
  */
  typedef void(*FSAPICallback)(unsigned, const FString&);

  /** Set the function to call when speech is recognized */
  void SetCallback(FSAPICallback pCallback) { m_pCallback = pCallback; }

  /** Set the voice to the one specified (must exist) */
  void SetVoice(unsigned idx);

  // Speak the specified text with the current voice (set in Control Panel or though SetVoice()).
  // Optionally, consider XML tags
  // (see http://www.tapiex.com/ES_Help/WP_XML_TTS_Tutorial.htm)
  void DoSpeak(const FString& text, bool bUseXML = false, bool bAsync = true, bool bPurgeQueue = false);
public:
  virtual void Speak(const FString &SpeechMarkup) override;

  /** Update - checks to see if any speech has been recognized */
  virtual void Update(ASAPIActor *Actor) override;

  virtual void SetLanguage(ELanguage Language) override;
  virtual void SetGrammar(const FString &GrammarFile, bool Enabled) override;
  virtual void SetRule(const FString &RuleUrl, bool Enabled) override;
  virtual void PushGrammar(const FString &GrammarFile) override;
  virtual void PopGrammar() override;
  ISpRecoGrammar* LoadGrammar(const FString &GrammarFile, ISpRecoGrammar *Current);

  virtual void SendAudio(const TArray<uint8> &Samples, int Channels, int SamplesPerSecond) override;

  virtual void Pause() override;
  virtual void Resume() override;
  virtual void AddWordTransition(const FString &Url, const TArray<FString> &Words, bool Overwrite = false, const FString &Separator = TEXT(";")) override;

 
private:

  void LogSAPIError(const FString &context, int hr) {
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    UE_LOG(LogSAPI, Error, TEXT("%s: %d: %s"), *context, hr, errMsg);
  }

  /** Uninitialize COM and destroy the SAPI voice */
  void Shutdown();

  void OnRecognized(const FString& text);

  FSAPICallback		m_pCallback;

  // Not in class to avoid having to include template header in header
  CComPtr<ISpAudioPlug> m_audioPlug;
  CComPtr<ISpVoice>		m_cpVoice;			// The TTS voice
  CComPtr<ISpRecognizer> m_cpRecognizer;
  CComPtr<ISpAudio> m_cpAudioInput;
  CComPtr<ISpRecoContext>	m_cpRecoContext;	// The SR context
  CComPtr<ISpRecoGrammar>	m_cpGrammar;		// The SR grammar
  CComPtr<ISpRecoGrammar> m_cpDictationGrammar;
  TComPtr<ISpRecoGrammar> m_currentGrammar;
  ASAPIActor *m_currentActor;
  ELanguage m_Language;
  int32 m_GrammarId;
  TMap<FString, TComPtr<ISpRecoGrammar>> m_Grammars;
  TArray<FString> m_EnabledGrammars;
  TArray<FString> m_GrammarStack;
};

IMPLEMENT_MODULE( FSAPIPlugin, SAPIPlugin )


void FSAPIPlugin::SendAudio(const TArray<uint8> &Samples, int Channels, int SamplesPerSecond)
{
	if (!m_audioPlug) {
		class SpAudioPlugImpl : public SpAudioPlug 
		{
			virtual ULONG Release() override 
			{
				return InternalRelease();
			}
			virtual ULONG AddRef() override
			{
				return InternalAddRef();
			}
			virtual HRESULT QueryInterface(REFIID RIID, void **ppObject)
			{
				return InternalQueryInterface(this, _GetEntries(), RIID, ppObject);
			}
		};
		m_audioPlug = new SpAudioPlugImpl();
		SpeechAudioFormatType Format = SpeechAudioFormatType::SAFTDefault;
		if (SamplesPerSecond == 44100)
		{
			Format = Channels == 2 ? SpeechAudioFormatType::SAFT44kHz16BitStereo : SpeechAudioFormatType::SAFT44kHz16BitMono;
		}
		else if (SamplesPerSecond == 48000)
		{
			Format = Channels == 2 ? SpeechAudioFormatType::SAFT48kHz16BitStereo : SpeechAudioFormatType::SAFT48kHz16BitMono;
		}
		else if (SamplesPerSecond == 32000)
		{
			Format = Channels == 2 ? SpeechAudioFormatType::SAFT32kHz16BitStereo : SpeechAudioFormatType::SAFT32kHz16BitMono;
		}
		else if (SamplesPerSecond == 16000)
		{
			Format = Channels == 2 ? SpeechAudioFormatType::SAFT16kHz16BitStereo : SpeechAudioFormatType::SAFT16kHz16BitMono;
		}
		else
		{
			// format not supported
			UE_LOG(LogSAPI, Error, TEXT("Unsupported audio format Channels: %d, SamplesPerSecond, %d"), Channels, SamplesPerSecond);
			return;
		}
		HRESULT hr = m_audioPlug->Init(VARIANT_FALSE, Format);
		if (FAILED(hr)) 
		{
			LogSAPIError("AudioPlug::Init", hr);
			return;
		}
		hr = m_cpRecognizer->SetInput(m_audioPlug, true);
		if (FAILED(hr))
		{
			LogSAPIError("Recognizer::SetInput", hr);
			return;
		}
	}
	long Written;
	VARIANT vData;
	vData.vt = VT_ARRAY | VT_UI1;
	SAFEARRAY Arr;
	Arr.cDims = 1;
	Arr.cLocks = 0;
	Arr.cbElements = Samples.Num();
	Arr.pvData = (void*)Samples.GetData();
	vData.parray = &Arr;
	m_audioPlug->SetData(vData, &Written);
}

void FSAPIPlugin::StartupModule()
{
  // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
  // Uninit COM
  int hr = ::CoInitialize(NULL);
  if (!SUCCEEDED(hr)) {
    LogSAPIError("CoInitialize", hr);
  }
}


void FSAPIPlugin::ShutdownModule()
{
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.
  Shutdown();
  // Uninit COM
  ::CoUninitialize();
}

void FSAPIPlugin::Pause() {
  if (!m_cpRecoContext.p)
    return;
  m_cpRecognizer.p->SetRecoState(SPRST_INACTIVE_WITH_PURGE);
}

void FSAPIPlugin::Resume() {
  if (!m_cpRecoContext.p)
    return;
  m_cpRecognizer.p->SetRecoState(SPRST_ACTIVE);
}

static LANGID convertLang(ELanguage srLanguage) {
  switch (srLanguage) {
  case ELanguage::Lang_EnglishUS:
    return 0x0409;
  case ELanguage::Lang_EnglishUK:
    return 0x0809;
  }
  return 0;
}

void FSAPIPlugin::SetLanguage(ELanguage srLanguage) {
  m_Language = srLanguage;
  if (m_cpRecognizer) {
    Shutdown();
  }
  Init(srLanguage);
}

void FSAPIPlugin::PushGrammar(const FString &FileName)
{
  SetGrammar(FileName, true);
  m_GrammarStack.Push(FileName);
  for (int i = 0; i < m_EnabledGrammars.Num(); i++) {
    FString Grammar = m_EnabledGrammars[i];
	TComPtr<ISpRecoGrammar> CurrentGrammar = m_Grammars[Grammar];
    if (CurrentGrammar) {
      bool Enabled = m_GrammarStack.Contains(Grammar);
      int hr = CurrentGrammar->SetRuleState(0, 0, Enabled ? SPRS_ACTIVE : SPRS_INACTIVE);
      if (!SUCCEEDED(hr)) {
        LogSAPIError("SetRuleState", hr);
      }
    }
  }
}

void FSAPIPlugin::PopGrammar()
{
  FString Grammar = m_GrammarStack.Pop();
  SetGrammar(Grammar, false);
  for (int i = 0; i < m_EnabledGrammars.Num(); i++) {
    Grammar = m_EnabledGrammars[i];
	TComPtr<ISpRecoGrammar> CurrentGrammar = m_Grammars[Grammar];
    if (CurrentGrammar) {
      int hr = CurrentGrammar->SetRuleState(0, 0, SPRS_ACTIVE);
      if (!SUCCEEDED(hr)) {
        LogSAPIError("SetRuleState", hr);
      }
    }
  }
}

void FSAPIPlugin::SetRule(const FString &RuleUrl, bool Enabled)
{
	int32 Sharp = RuleUrl.Find(TEXT("#"));
	if (Sharp > 0)
	{
		FString Grammar = RuleUrl.Left(Sharp);
		FString Rule = RuleUrl.RightChop(Sharp + 1);
		if (m_Grammars.Contains(Grammar))
		{
			auto Gr = m_Grammars[Grammar];
			HRESULT hr;
			if (!SUCCEEDED(hr = Gr->SetRuleState(Rule.GetCharArray().GetData(), 0, Enabled ? SPRS_ACTIVE : SPRS_INACTIVE)))
			{
				LogSAPIError(TEXT("Set Rule State"), hr);
			}
			return;
		}
	}
	UE_LOG(LogSAPI, Error, TEXT("SetRule: Rule not found %s"), *RuleUrl);
}

void FSAPIPlugin::SetGrammar(const FString &FileName, bool Enabled) {
  if (!m_cpRecognizer) {
    UE_LOG(LogSAPI, Error, TEXT("You must call SetLanguage before SetGrammar"));
    return;
  }
  m_EnabledGrammars.Remove(FileName);
  if (Enabled) {
    m_EnabledGrammars.Add(FileName);
  }
  HRESULT hr;
  hr = m_cpDictationGrammar->SetDictationState(SPRS_INACTIVE);
  if (!SUCCEEDED(hr)) {
    LogSAPIError("SetDictationState", hr);
  }
  if (m_Grammars.Contains(FileName)) {
    // For now: load it every time
    if (Enabled) {
      LoadGrammar(FileName, m_Grammars[FileName]);
    }
  }
  else {
    if (!Enabled) return;
    if (ISpRecoGrammar *grammar = LoadGrammar(FileName, nullptr)) {
      TComPtr<ISpRecoGrammar> p(grammar);
      m_Grammars.Add(FileName, p);
    }
  }
  m_currentGrammar = m_Grammars[FileName];
  if (m_currentGrammar) {
    hr = m_currentGrammar->SetRuleState(0, 0, Enabled ? SPRS_ACTIVE : SPRS_INACTIVE);
    if (!SUCCEEDED(hr)) {
      LogSAPIError("SetRuleState", hr);
    }
  }
	

}


ISpRecoGrammar* FSAPIPlugin::LoadGrammar(const FString &GrammarFile, ISpRecoGrammar *Current) {
  FString Path = FPaths::Combine(FPaths::GameUserDir().GetCharArray().GetData(), GrammarFile.GetCharArray().GetData());
  HRESULT hr;
  ISpRecoGrammar *pGrammar = Current;
  if (!pGrammar) {
    hr = m_cpRecoContext->CreateGrammar(m_GrammarId++, &pGrammar);
    if (!SUCCEEDED(hr)) {
      LogSAPIError("CreateGrammar", hr);
    }
  }
  {
    //WORD langId = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    hr = pGrammar->ResetGrammar(convertLang(m_Language));
    if (!SUCCEEDED(hr)) {
      LogSAPIError(TEXT("ResetGrammar"), hr);
    }
  }
  //// Command & control mode from a file
  if (pGrammar) {
    hr = pGrammar->LoadCmdFromFile(Path.GetCharArray().GetData(), SPLO_DYNAMIC);

    if (!SUCCEEDED(hr))
      {
        LogSAPIError(TEXT("LoadCmdFromFile"), hr);
      }
  }
  if (0 && SUCCEEDED(hr)) {
    hr = pGrammar->Commit(0);
    if (!SUCCEEDED(hr))
      {
        LogSAPIError(TEXT("Commit"), hr);
      }
  }
  return pGrammar;
}

// Return a token enumerator containing all tokens that match the primary language
// of a particular language id. pszRequiredAttributes can be used to specify additional attributes all tokens must have.
static HRESULT SpEnumTokensMatchingPrimaryLangIDLocal(const LPCWSTR pszCategoryId, LANGID priLangID, LPCWSTR pszRequiredAtts,
                                                 IEnumSpObjectTokens **ppEnum)
{
  HRESULT hr = S_OK;

  // First enumerate the tokens using pszRequiredAtts.
  CComPtr<ISpObjectTokenCategory> cpCategory;
  hr = SpGetCategoryFromId(pszCategoryId, &cpCategory);

  CComPtr<IEnumSpObjectTokens> cpEnum;
  if (SUCCEEDED(hr))
    {
      hr = cpCategory->EnumTokens(pszRequiredAtts, NULL, &cpEnum);
    }

  ULONG ulTokens(0);
  if (SUCCEEDED(hr))
    {
      hr = cpEnum->GetCount(&ulTokens);
    }

  // Create enumerator to store new tokens.
  CComPtr<ISpObjectTokenEnumBuilder> cpBuilder;
  if (SUCCEEDED(hr))
    {
      hr = cpBuilder.CoCreateInstance(CLSID_SpObjectTokenEnum);
    }
  if (SUCCEEDED(hr))
    {
      hr = cpBuilder->SetAttribs(NULL, NULL);
    }

  // Now, for each token, check language string to see if it matches.
  for (ULONG ul = 0; SUCCEEDED(hr) && ul < ulTokens; ul++)
    {
      LANGID tokenLangID(0);
      CComPtr<ISpObjectToken> cpToken;
      hr = cpEnum->Item(ul, &cpToken);
      if (SUCCEEDED(hr))
        {
          // Just look at the first language id
          hr = SpGetLanguageFromToken(cpToken, &tokenLangID);
        }

      if (SUCCEEDED(hr) && PRIMARYLANGID(tokenLangID) == PRIMARYLANGID(priLangID))
        {
          // Add to builder
          hr = cpBuilder->AddTokens(1, &(cpToken.p));
        }
    }

  if (SUCCEEDED(hr))
    {
      hr = cpBuilder->Reset();
    }
  if (SUCCEEDED(hr))
    {
      *ppEnum = cpBuilder.Detach();
    }

  return hr;
}


// Get the object token for the default shared recognizer engine.
static HRESULT SpGetDefaultSharedRecognizerToken(ISpObjectToken **ppSharedRecognizerToken, LANGID LangID)
{
  HRESULT hr = S_OK;
  const WCHAR pszWindowsCompatAtt[] = L"windowsV6compatible";

  // Find recognizers that match windowsV6compatible and the primary language id of the UI.
  // Use primary language because there may be several recognizers available for different sub-languages,
  // and there is generally only one UI language per primary language.
  // e.g. US and UK English recognizers on a US English system.
  // If so, to distinguish these the locale is used below.

  CComPtr<IEnumSpObjectTokens> cpEnum;
  hr = SpEnumTokensMatchingPrimaryLangIDLocal(SPCAT_RECOGNIZERS, PRIMARYLANGID(LangID), pszWindowsCompatAtt, &cpEnum);

  ULONG ulRecognizers(0);
  if (SUCCEEDED(hr))
    {
      hr = cpEnum->GetCount(&ulRecognizers);
    }

  if (SUCCEEDED(hr))
    {
      if (ulRecognizers == 0)
        {
          // If zero - error.
          hr = SPERR_RECOGNIZER_NOT_FOUND;
        }
      else if (ulRecognizers == 1)
        {
          // If one - we are done.
          hr = cpEnum->Item(0, ppSharedRecognizerToken);
        }
      else
        {
          // More than one recognizer matches - we must pick the best default:
          BOOL fFoundDefault = FALSE;
          if (!fFoundDefault && SUCCEEDED(hr))
            {

              hr = S_OK;
              WCHAR pszLocaleString[] = L"Language=XXXXXXXX";
              SpHexFromUlong(pszLocaleString + wcslen(pszLocaleString) - 8, LangID);
              for (ULONG ul = 0; SUCCEEDED(hr) && ul < ulRecognizers; ul++)
                {
                  CComPtr<ISpObjectToken> cpToken;
                  hr = cpEnum->Item(ul, &cpToken);
                  if (SUCCEEDED(hr))
                    {
                      hr = cpToken->MatchesAttributes(pszLocaleString, &fFoundDefault);
                    }

                  if (SUCCEEDED(hr) && fFoundDefault)
                    {
                      *ppSharedRecognizerToken = cpToken.Detach();
                      break;
                    }
                }
            }

          // If there's no engine that directly matches the locale see if the backup list of SupportedLocales is matched.
          if (!fFoundDefault && (SUCCEEDED(hr) || hr == SPERR_NOT_FOUND))
            {
              // See if there's an engine which has a supported Locale matches the current locale.
              // For example if the langid is U.S English, but the locale is Australian English
              // there may be no recognizer that directly supports Australian English,
              // but a U.K. recognizer might specify it can be used in the Australian English locale with this attribute.

              hr = S_OK;
              WCHAR pszSupportedLocalesString[] = L"SupportedLocales=XXXXXXXX";
              SpHexFromUlong(pszSupportedLocalesString + wcslen(pszSupportedLocalesString) - 8, LangID);
              for (ULONG ul = 0; SUCCEEDED(hr) && ul < ulRecognizers; ul++)
                {
                  CComPtr<ISpObjectToken> cpToken;
                  hr = cpEnum->Item(ul, &cpToken);
                  if (SUCCEEDED(hr))
                    {
                      hr = cpToken->MatchesAttributes(pszSupportedLocalesString, &fFoundDefault);
                    }

                  if (SUCCEEDED(hr) && fFoundDefault)
                    {
                      *ppSharedRecognizerToken = cpToken.Detach();
                      break;
                    }
                }
            }

          // We still haven't found a match - just pick the first recognizer.
          if (!fFoundDefault && (SUCCEEDED(hr) || hr == SPERR_NOT_FOUND))
            {
              CComPtr<ISpObjectToken> cpToken;
              hr = cpEnum->Item(0, ppSharedRecognizerToken);
            }
        }
    }

  return hr;
}



//------------------------------------------------------------------
// Init			
//------------------------------------------------------------------
void FSAPIPlugin::Init(ELanguage srLanguage)
{
  m_currentActor = 0;
  m_GrammarId = 1;
  m_Language = srLanguage;
  m_GrammarStack.Empty();
  m_EnabledGrammars.Empty();
  // InitiaSlize COM
  if (1)
    {
      CComPtr<ISpObjectToken>      cpObjectToken;
      CComPtr<ISpObjectToken>      cpObjectToken1;
		
      // Create a voice for TTS synthesis
      m_cpVoice.CoCreateInstance(CLSID_SpVoice);
      HRESULT hr = m_cpRecognizer.CoCreateInstance(CLSID_SpInprocRecognizer);
      // Create a recognition context for recognition
      if (SUCCEEDED(hr))
        {
          // Get the default audio input token.
          hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &cpObjectToken);
        }
      // Set up the inproc recognizer audio input with an audio input object.
      if (SUCCEEDED(hr)) {
        // Create the default audio input object.
        hr = SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOIN, &m_cpAudioInput);
      }
		
      if (SUCCEEDED(hr)) {
        hr = SpGetDefaultSharedRecognizerToken(&cpObjectToken1, convertLang(m_Language));
      }
      if (SUCCEEDED(hr)) {
        hr = m_cpRecognizer->SetRecognizer(cpObjectToken1);
      }
      if (SUCCEEDED(hr)) {
        // Set the audio input to our object.
        hr = m_cpRecognizer->SetInput(m_cpAudioInput, FALSE);
      }
      if (SUCCEEDED(hr)) {
        hr = m_cpRecognizer->CreateRecoContext(&m_cpRecoContext);
      }
      if (SUCCEEDED(hr)) {
        // Tell it that we only care about speech recognition events
        //hr = m_cpRecoContext->SetInterest(SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_FALSE_RECOGNITION), SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_FALSE_RECOGNITION));
        hr = m_cpRecoContext->SetInterest(SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_SOUND_START) | SPFEI(SPEI_SOUND_END), SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_SOUND_START) | SPFEI(SPEI_SOUND_END));
      }
      if (SUCCEEDED(hr)) {
        hr = m_cpRecoContext->CreateGrammar(0, &m_cpDictationGrammar);
      }
      if (SUCCEEDED(hr))
        {
          //// Dictation mode
          m_cpDictationGrammar->LoadDictation(NULL, SPLO_STATIC);
          m_cpDictationGrammar->SetDictationState(SPRS_ACTIVE);
        }
      if (!SUCCEEDED(hr)) {
        LogSAPIError("Init", hr);
      }
      SPRECOGNIZERSTATUS Status;
      hr = m_cpRecognizer->GetStatus(&Status);
      if (!SUCCEEDED(hr)) {
        LogSAPIError("GetStatus", hr);
      }
		
    }
}

//------------------------------------------------------------------
// Shutdown									
//------------------------------------------------------------------
void FSAPIPlugin::Shutdown()
{
  if (m_cpRecognizer) {
    m_cpVoice.Release();
    m_cpGrammar.Release();
    m_cpRecoContext.Release();
    m_cpRecognizer.Release();
    m_cpAudioInput.Release();
    m_Grammars.Empty();		
  }
}

//------------------------------------------------------------------
// Update		
//------------------------------------------------------------------
void FSAPIPlugin::Update(ASAPIActor *actor)
{
  if (!m_cpRecoContext.p)
    return;
  m_currentActor = actor;
  SPRECOGNIZERSTATUS Status;
  HRESULT hr = m_cpRecognizer->GetStatus(&Status);
  if (!SUCCEEDED(hr)) {
    LogSAPIError("GetStatus", hr);
  }

  // Get all events - note that GetFrom returns S_FALSE when it
  // has no events to get, which passes the SUCCEEDED macro test.
  // We therefore specifically check for S_OK.
  CSpEvent evt;

  while (S_OK == evt.GetFrom(m_cpRecoContext))
    {
      if ((evt.eEventId == SPEI_SOUND_START)) {
        m_currentActor->OnSpeechInputStarted();
      }
      else if ((evt.eEventId == SPEI_SOUND_END)) {
        m_currentActor->OnSpeechInputStopped();
      }
      else if ((evt.eEventId == SPEI_RECOGNITION || evt.eEventId == SPEI_FALSE_RECOGNITION ) &&
               evt.RecoResult())
        {
          hr = S_OK;

          ISpRecoResult* pResult = evt.RecoResult();
	
          SPPHRASE* pphrase = 0;
          if (SUCCEEDED(hr)) {
            hr = pResult->GetPhrase(&pphrase);
            if (SUCCEEDED(hr)) {
              auto props = pphrase->pProperties;
              CoTaskMemFree(pphrase);
            }
          }
          CComPtr<ISpeechXMLRecoResult> pResult2;
          hr = pResult->QueryInterface(IID_ISpeechXMLRecoResult, (void**)&pResult2);
          if (SUCCEEDED(hr)) {
            BSTR str;
            LONG LineNumber;
            BSTR ScriptLine;
            BSTR Source;
            BSTR Description;
            LONG ResultCode;
            VARIANT_BOOL IsError;
            hr = pResult2->GetXMLErrorInfo(&LineNumber, &ScriptLine, &Source, &Description, &ResultCode, &IsError);
            if (SUCCEEDED(hr)) {
              _bstr_t bs1(ScriptLine);
              _bstr_t bs2(Source);
              _bstr_t bs3(Description);
              if (IsError) {
                m_currentActor->OnSpeechRecognitionError(FString( (LPCWSTR) bs3), FString((LPCWSTR)bs2), LineNumber,
                                                         FString(LPCWSTR(bs1)));
                return;
              }
            }
            hr = pResult2->GetXMLResult(SPXRO_SML, &str);
            if (SUCCEEDED(hr) && str) {
              _bstr_t bs(str);
			  std::regex reg(" confidence=[^>]+>", std::regex_constants::extended);
			  auto replaced = std::regex_replace((const char *)bs, reg, ">");
			  if (replaced.length() > 0)
			  {
				  auto json = xml2json(replaced.c_str());
				  m_currentActor->HandleSpeechRecognized(FString(json.c_str()));
			  }
              continue;
            }		  
          }
          if (0)
            {
              WCHAR*	pwszText = NULL;
              // Get the phrase's entire text string, including replacements.
              hr = pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, 1, &pwszText, NULL);
              if (SUCCEEDED(hr) && pwszText)
                {
                  // Activate callback
                  m_currentActor->HandleSpeechRecognized(FString(pwszText));
                  CoTaskMemFree(pwszText);
                }
            }
        }
    }
}

//------------------------------------------------------------------
// OnRecognized			
//------------------------------------------------------------------
void FSAPIPlugin::OnRecognized(const FString& text)
{
  if (!m_pCallback)
    return;

  // Find the rule ID
  unsigned ruleID = 0;
  // Callback
  m_pCallback(ruleID, text);
}

//------------------------------------------------------------------
// Speak	
//------------------------------------------------------------------

void FSAPIPlugin::Speak(const FString &SpeechMarkup) {
	UE_LOG(LogTemp, Warning, TEXT("Speak to me!"));
  DoSpeak(SpeechMarkup, true, true, true);
}

void FSAPIPlugin::DoSpeak(const FString& text, bool bUseXML, bool bAsync, bool bPurgeQueue)
{
	UE_LOG(LogTemp, Warning, TEXT("Speak to me again: %s"), *text);
  const wchar_t *pText = text.GetCharArray().GetData();
  if (m_cpVoice.p)
    {
      DWORD flags = bUseXML ? SPF_IS_XML : SPF_IS_NOT_XML;
      if (bAsync)
        flags |= SPF_ASYNC;

      if (bPurgeQueue)
        flags |= SPF_PURGEBEFORESPEAK;
	  UE_LOG(LogTemp, Warning, TEXT("Nearly speaking now!"));
      HRESULT hr = m_cpVoice->Speak(pText, flags, NULL);
	  _com_error err(hr);
	  UE_LOG(LogTemp, Warning, TEXT("Finished with result: %s"), err.ErrorMessage());

    }
}

//------------------------------------------------------------------



//------------------------------------------------------------------
// SetVoice
//------------------------------------------------------------------
void FSAPIPlugin::SetVoice(unsigned idx)
{
  if (!m_cpVoice)
    return;

  // Declare local identifiers:
  HRESULT                        hr = S_OK;
  CComPtr<ISpObjectToken>        cpVoiceToken;
  CComPtr<IEnumSpObjectTokens>   cpEnum;
  ULONG                          ulCount = 0;
  unsigned					   i = 0;

  // Enumerate the available voices.
  hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
  if (SUCCEEDED(hr))
    {
      // Get the voice token
      hr = cpEnum->Item(idx, &cpVoiceToken);

      if (SUCCEEDED(hr))
        {
          hr = m_cpVoice->SetVoice(cpVoiceToken);
        }
    }
}

void FSAPIPlugin::AddWordTransition(const FString &RuleUrl, const TArray<FString> &Words, bool Overwrite, const FString &WordSeparator)
{
	int32 Sharp = RuleUrl.Find(TEXT("#"));
	if (Sharp > 0)
	{
		FString Grammar = RuleUrl.Left(Sharp);
		FString Rule = RuleUrl.RightChop(Sharp+1);
		if (m_Grammars.Contains(Grammar))
		{
			SPSTATEHANDLE pState;
			auto Gr = m_Grammars[Grammar];
			HRESULT hr;
			if (SUCCEEDED(hr = Gr->GetRule(Rule.GetCharArray().GetData(), 0, SPRAF_Dynamic, false, &pState)))
			{
				if (Overwrite)
				{
					Gr->ClearRule(pState);
				}
				for (int i = 0; i < Words.Num(); i++)
				{
					const FString &Word = Words[i];
					hr = Gr->AddWordTransition(pState, 0, Word.GetCharArray().GetData(), WordSeparator.GetCharArray().GetData(), SPWT_LEXICAL, 1.0f, 0);
					if (!SUCCEEDED(hr))
					{
						LogSAPIError("AddWordTransition", hr);
					}
				}
				if (SUCCEEDED(hr))
				{
					hr = Gr->Commit(0);
					if (!SUCCEEDED(hr))
					{
						LogSAPIError("Commit", hr);
					}
				}
			}
			else
			{
				LogSAPIError("GetRule", hr);
			}
		} 
		else
		{
			UE_LOG(LogSAPI, Error, TEXT("Grammar Not Found: %s"), *Grammar);
		}
	}
	else
	{
		UE_LOG(LogSAPI, Error, TEXT("Rule Url should be of the form Grammar#Rule instead of %s"), *RuleUrl);
	}
}




#include "HideWindowsPlatformTypes.h"
