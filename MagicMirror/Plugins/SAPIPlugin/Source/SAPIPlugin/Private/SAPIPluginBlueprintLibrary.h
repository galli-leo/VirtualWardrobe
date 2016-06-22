

#pragma once
#include "ISAPIPlugin.h"
#include "SAPIPluginTypes.h"
#include "SAPIPluginBlueprintLibrary.generated.h"



/**
 * A blueprint library class to expose the functionality of the SAPIPlugin plugin to blueprints
 * in all blueprint contexts.
 */
UCLASS()
class USAPIPluginBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void Speak(const FString &SpeechMarkup);
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void SetLanguage(ELanguage Language);
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void PauseSpeechRecognition();
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void ResumeSpeechRecognition();
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void SetGrammar(const FString &GrammarFile, bool Enabled);
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void SetRule(const FString &RuleUrl, bool Enabled);
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void PushGrammar(const FString &GrammarFile);
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void PopGrammar();
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void AddWordTransition(const FString &RuleUrl, const TArray<FString> &Words, bool Overwrite = false, const FString &WordSeparator = TEXT(";"));
	UFUNCTION(BlueprintCallable, Category = "SAPI")
		static void SendAudio(const TArray<uint8> &Samples, int32 Channels, int32 SamplesPerSecond);
};
