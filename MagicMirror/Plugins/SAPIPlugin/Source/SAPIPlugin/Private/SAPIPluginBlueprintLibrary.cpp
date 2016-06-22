#include "SAPIPluginPrivatePCH.h"
#include "SAPIPluginBlueprintLibrary.h"

USAPIPluginBlueprintLibrary::USAPIPluginBlueprintLibrary(const FObjectInitializer &Init) {
}

void USAPIPluginBlueprintLibrary::Speak(const FString &SpeechMarkup) {
  ISAPIPlugin::Get().Speak(SpeechMarkup);

}

void USAPIPluginBlueprintLibrary::SetLanguage(ELanguage Language) {
  ISAPIPlugin::Get().SetLanguage(Language);
}

void USAPIPluginBlueprintLibrary::SetGrammar(const FString &GrammarFile, bool Enabled)
{
  ISAPIPlugin::Get().SetGrammar(GrammarFile, Enabled);
}

void USAPIPluginBlueprintLibrary::SetRule(const FString &RuleUrl, bool Enabled)
{
	ISAPIPlugin::Get().SetRule(RuleUrl, Enabled);
}

void USAPIPluginBlueprintLibrary::PushGrammar(const FString &GrammarFile)
{
  ISAPIPlugin::Get().PushGrammar(GrammarFile);
}

void USAPIPluginBlueprintLibrary::PopGrammar()
{
  ISAPIPlugin::Get().PopGrammar();
}

void USAPIPluginBlueprintLibrary::AddWordTransition(const FString &RuleUrl, const TArray<FString> &Words, bool Overwrite, const FString &WordSeparator)
{
	ISAPIPlugin::Get().AddWordTransition(RuleUrl, Words, Overwrite, WordSeparator);
}

void USAPIPluginBlueprintLibrary::PauseSpeechRecognition()
{
  ISAPIPlugin::Get().Pause();
}

void USAPIPluginBlueprintLibrary::ResumeSpeechRecognition()
{
  ISAPIPlugin::Get().Resume();
}

void USAPIPluginBlueprintLibrary::SendAudio(const TArray<uint8> &Samples, int32 Channels, int32 SamplesPerSecond)
{
	ISAPIPlugin::Get().SendAudio(Samples, Channels, SamplesPerSecond);
}

