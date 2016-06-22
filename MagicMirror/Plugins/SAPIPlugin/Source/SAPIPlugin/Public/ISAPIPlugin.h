// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "Engine.h"
#include "SAPIPluginTypes.h"
SAPIPLUGIN_API DECLARE_LOG_CATEGORY_EXTERN(LogSAPI, Warning, All);
/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class ISAPIPlugin : public IModuleInterface
{

 public:
  /**
   * Singleton-like access to this module's interface.  This is just for convenience!
   * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
   *
   * @return Returns singleton instance, loading the module on demand if needed
   */
  static inline ISAPIPlugin& Get()
  {
    return FModuleManager::LoadModuleChecked< ISAPIPlugin >( "SAPIPlugin" );
  }
  
  /**
   * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
   *
   * @return True if the module is loaded and ready to use
   */
  static inline bool IsAvailable()
  {
    return FModuleManager::Get().IsModuleLoaded( "SAPIPlugin" );
  }
  
  virtual void Update(class ASAPIActor* Actor) = 0;
  virtual void Speak(const FString &SpeechMarkup) = 0;
  virtual void SetLanguage(ELanguage Language) = 0;
  virtual void SetGrammar(const FString &GrammarFile, bool Enabled) = 0; 
  virtual void SetRule(const FString &RuleUrl, bool Enabled) = 0;
  virtual void PushGrammar(const FString &GrammarFile) = 0;
  virtual void PopGrammar() = 0;
  virtual void Pause() = 0;
  virtual void Resume() = 0;
  virtual void AddWordTransition(const FString &RuleUrl, const TArray<FString> &Words, bool Overwrite = false, const FString &WordSeparator = TEXT(";")) = 0;
  virtual void SendAudio(const TArray<uint8> &Samples, int Channels, int SamplesPerSecond) = 0;
};

