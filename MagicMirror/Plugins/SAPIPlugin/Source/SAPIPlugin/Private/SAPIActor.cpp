// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "SAPIPluginPrivatePCH.h"
#include "SAPIActor.h"
#include "ISAPIPlugin.h"


ASAPIActor::ASAPIActor(const class FObjectInitializer& PCIP)
: Super(PCIP)
, Continuation(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASAPIActor::BeginPlay()
{
  Super::BeginPlay();
}

void ASAPIActor::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
  ISAPIPlugin::Get().Update(this);
}

void ASAPIActor::GetResponse(FString &ResponseText,
                             UObject* WorldContextObject,
                             struct FLatentActionInfo LatentInfo)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FContinueAction<FString> *Kont = LatentActionManager.FindExistingAction<FContinueAction<FString>>(LatentInfo.CallbackTarget, LatentInfo.UUID);
		if (Kont != NULL)
		{
			LatentActionManager.RemoveActionsForObject(LatentInfo.CallbackTarget);
		}
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Continuation = new FContinueAction<FString>(ResponseText, LatentInfo));
	}
}

void ASAPIActor::HandleSpeechRecognized(const FString &ResponseText)
{
  if (Continuation != 0) {
    FContinueAction<FString> *K = Continuation;
    Continuation = 0;
    K->Call(ResponseText);
  } else {
    OnSpeechRecognized(ResponseText);
  }
}



