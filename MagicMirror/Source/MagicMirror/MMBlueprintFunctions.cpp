// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicMirror.h"
#include "MMBlueprintFunctions.h"

#if PLATFORM_WINDOWS
#include "KinectFunctionLibrary.h"
#endif


UTexture2D* UMMBlueprintFunctions::CreateSquare(UTexture2D* texture, int32 thickness = 5, int32 size = 556, FColor color = FColor(0, 0, 255)){

	FTexture2DMipMap* biggestMipMap = &texture->PlatformData->Mips[0];
	int32 width = biggestMipMap->SizeX;
	int32 height = biggestMipMap->SizeY;
	FByteBulkData* rawImageData = &biggestMipMap->BulkData;
	FColor* pBuffer = static_cast<FColor*>(rawImageData->Lock(LOCK_READ_WRITE));

	if (width < size + thickness || height < size + thickness){
		rawImageData->Unlock();
		return texture;
	}

	int32 start_x = (width - size) / 2;
	int32 start_y = (height - size) / 2;
	for (int32 x = start_x; x < start_x + size; x++)
	{
		for (int32 real_y = start_y; real_y > (start_y - thickness); real_y--)
		{
			int32 array_pos = x + width*real_y;
			pBuffer[array_pos] = color;
		}

		for (int32 real_y = start_y + size - 1; real_y < (start_y + size - 1 + thickness); real_y++)
		{
			int32 array_pos = x + width*real_y;
			pBuffer[array_pos] = color;
		}
	}

	for (int32 y = start_y; y < start_y + size; y++)
	{
		for (int32 x = start_x; x > start_x - thickness; x--)
		{
			int32 array_pos = x + width*y;
			pBuffer[array_pos] = color;
		}

		for (int32 x = start_x + size; x < start_x + size - 1 + thickness; x++)
		{
			int32 array_pos = x + width*y;
			pBuffer[array_pos] = color;
		}
	}

	rawImageData->Unlock();
	texture->UpdateResource();
	return texture;
}

void UMMBlueprintFunctions::CropTexture(UTexture2D* texture, FVector2D topLeft, FVector2D size){
	FTexture2DMipMap* biggestMipMap = &texture->PlatformData->Mips[0];
	int32 width = biggestMipMap->SizeX;
	int32 height = biggestMipMap->SizeY;
	FByteBulkData* rawImageData = &biggestMipMap->BulkData;
	FColor* pBuffer = static_cast<FColor*>(rawImageData->Lock(LOCK_READ_WRITE));

	if (topLeft.X > width || topLeft.Y > height || width < topLeft.X + size.X || height < topLeft.Y + size.Y){
		rawImageData->Unlock();
		return;
	}

	for (int32 y = 0; y < height; y++)
	{
		for (int32 x = 0; x < width; x++)
		{
			if (!(x >= topLeft.X && x < topLeft.X + size.X && y >= topLeft.Y && y < topLeft.Y + height))
			{
				pBuffer[y * width + x] = FColor(0, 0, 0, 0);
			}
		}
	}

	rawImageData->Unlock();
	texture->UpdateResource();
	return;
}

UWardrobeManager* UMMBlueprintFunctions::StartWardrobeManager(EWardrobeMode startingMode, FString dbFile)
{
	UWardrobeManager* wardrobeManager = NewObject<UWardrobeManager>();
	wardrobeManager->StartWardrobeManager(startingMode, dbFile);

	return wardrobeManager;
}

USkeletalMesh* UMMBlueprintFunctions::GetMeshForName(FString name)
{
	USkeletalMesh* mesh = LoadObject<USkeletalMesh>(NULL, *FString::Printf(TEXT("/Game/Clothes/%s.%s"), *name, *name), NULL, LOAD_None, NULL);
	return mesh;
}

void UMMBlueprintFunctions::DisplayMessage(FText message, FText title)
{
	FMessageDialog::Open(EAppMsgType::Ok, message, &title);
}

/*
void UMMBlueprintFunctions::TickWardrobeManager(float deltaTime)
{
	this->wardrobeManager->Tick(deltaTime);
}*/

FVector2D UMMBlueprintFunctions::GetFocalDistanceForFOV(FVector2D FOV, APlayerController* controller)
{
	int32 Width, Height;
	controller->GetViewportSize(Width, Height);

	float fx = Width / (2.0 * FMath::Tan(FMath::DegreesToRadians(FOV.X/2.0)));
	float fy = Height / (2.0 * FMath::Tan(FMath::DegreesToRadians(FOV.Y/2.0)));

	return FVector2D(fx, fy);
}


TArray<FVector> UMMBlueprintFunctions::GetWorldPositionFromHeadAndSpine(TArray<FVector2D> ScreenPos, FVector2D FOV, FVector2D WorldDifference, APlayerController* controller)
{
#if PLATFORM_WINDOWS
	if (ScreenPos.Num() == 0)
	{
		return TArray<FVector>();
	}

	int32 Width, Height;
	controller->GetViewportSize(Width, Height);

	FVector2D focal = UMMBlueprintFunctions::GetFocalDistanceForFOV(FOV, controller);
	FVector Head;
	FVector Spine;
	FVector2D HeadScreen = ScreenPos[0];
	FVector2D SpineScreen = ScreenPos[1];
	HeadScreen.Y = HeadScreen.Y - Height/2.0;
	SpineScreen.Y = SpineScreen.Y - Height/2.0;

	Spine.Y = (-SpineScreen.Y * WorldDifference.Y) / (SpineScreen.Y - HeadScreen.Y);
	Spine.X = 0;
	Spine.Z = (focal.Y * Spine.Y) / SpineScreen.Y;
	Spine.Y += 68.0;

	printd("Some information: Screen Ys: %f, %f; World Y: %f", SpineScreen.Y, HeadScreen.Y, Spine.Y);

	Head.Z = Spine.Z;
	Head.Y = (Head.Z * HeadScreen.Y) / focal.Y + 68.0f;
	Head.X = 0;

	TArray<FVector> Coords;
	Coords.Add(Head);
	Coords.Add(Spine);

	return Coords;

#else
	return FVector();
#endif
}