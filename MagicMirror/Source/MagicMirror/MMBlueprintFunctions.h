// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "WardrobeManager.h"
#include "MMBlueprintFunctions.generated.h"

/**
 * 
 */
UCLASS()
class MAGICMIRROR_API UMMBlueprintFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
		UFUNCTION(BlueprintCallable, Category = "Kinect|Textures")
		static UTexture2D* CreateSquare(UTexture2D* texture, int32 thickness, int32 size, FColor color);
		UFUNCTION(BlueprintCallable, Category = "Kinect|Textures")
		static void CropTexture(UTexture2D* texture, FVector2D topLeft, FVector2D size);

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		static UWardrobeManager* StartWardrobeManager(EWardrobeMode startingMode, FString dbFile);

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
			static USkeletalMesh* GetMeshForName(FString name);

		/*UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		static void TickWardrobeManager(float deltaTime);*/

		UPROPERTY(BlueprintReadOnly, Category = "Wardrobe")
		UWardrobeManager* wardrobeManager;
	
};
