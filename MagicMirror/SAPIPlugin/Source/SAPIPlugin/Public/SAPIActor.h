// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "LatentActions.h"
#include "ISAPIPlugin.h"
#include "SAPIActor.generated.h"

template <class T> class FContinueAction : public FPendingLatentAction
{
  bool Called;
 public:
  const FName ExecutionFunction;
  const int32 OutputLink;
  const FWeakObjectPtr CallbackTarget;
  T &Result;

  virtual void Call(const T &Value) {
    Result = Value;
    Called = true;
  }

  void operator()(const T &Value) {
	  Call(Value);
  }

 FContinueAction(T& ResultParam, const FLatentActionInfo& LatentInfo) :
  Called(false)
    , Result(ResultParam)
    , ExecutionFunction(LatentInfo.ExecutionFunction)
    , OutputLink(LatentInfo.Linkage)
    , CallbackTarget(LatentInfo.CallbackTarget)
    {
    }
  
  virtual void UpdateOperation(FLatentResponse& Response) override
  {
    Response.FinishAndTriggerIf(Called, ExecutionFunction, OutputLink, CallbackTarget);
  }
};

/** 
 */
UCLASS()
class SAPIPLUGIN_API ASAPIActor : public AActor
{
  GENERATED_UCLASS_BODY()
    public:
  virtual void BeginPlay() override;
  
  virtual void Tick(float DeltaSeconds) override;
  UFUNCTION(BlueprintImplementableEvent)
    void OnSpeechInputStarted();
  UFUNCTION(BlueprintImplementableEvent)
    void OnSpeechInputStopped();
  UFUNCTION(BlueprintImplementableEvent)
    void OnSpeechRecognized(const FString &SpeechText);
  UFUNCTION(BlueprintImplementableEvent)
    void OnSpeechRecognitionError(const FString &ErrorMessage, const FString &Source, int32 LineNumber, const FString &ScriptLine);
  UFUNCTION(BlueprintCallable, Category = "SAPI", meta = (Latent, WorldContext = "WorldContextObject", LatentInfo = "LatentInfo"))
    virtual void GetResponse(FString &ResponseText,
                             UObject* WorldContextObject,
                             struct FLatentActionInfo LatentInfo);
  virtual void HandleSpeechRecognized(const FString &SpeechText);
 private:
  FContinueAction<FString> *Continuation;
  
};
