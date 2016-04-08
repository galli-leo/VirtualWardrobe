// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "MMPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MAGICMIRROR_API AMMPlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintPure, Category = "Mirror")
		FVector2D GetObjectScreenSize(AActor* InActor);

};
