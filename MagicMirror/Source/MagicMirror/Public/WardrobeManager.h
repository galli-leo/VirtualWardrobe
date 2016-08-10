// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "AllowWindowsPlatformTypes.h"

#include "Kinect.h"
#include "HideWindowsPlatformTypes.h"

#include "Tickable.h"
#include "WardrobeManager.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EWardrobeMode : uint8
{
	MM_Outfitting 	UMETA(DisplayName = "Outfitting"),
	MM_Scanning 	UMETA(DisplayName = "Scanning"),
	MM_Categorizing	UMETA(DisplayName = "Categorizing")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTshirtScannedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTshirtCategorizedDelegate);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewKinectColorFrameEvent, const class UTexture2D*, ColorFrameTexture);

/**
 * 
 */
UCLASS(Blueprintable)
class MAGICMIRROR_API UWardrobeManager : public UObject
{
	GENERATED_BODY()

public:

		UPROPERTY(BlueprintReadOnly)
		EWardrobeMode mode;

		UPROPERTY(BlueprintReadWrite)
		FString databaseFile;

		//UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		void StartWardrobeManager(EWardrobeMode startingMode, FString dbFile);
		//UPROPERTY(BlueprintReadOnly)

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		void Tick(float deltaTime);

		UFUNCTION(BlueprintCallable, Category = "Test")
		uint8 TestPython();

		UFUNCTION(BlueprintCallable, Category = "Python")
			FString GetCurrentPathAsSeenByPython();

		~UWardrobeManager();

private:
	

	float currentTime = 0;

	RGBQUAD* pBuffer;
	uint16* pDepthBuffer;
	DepthSpacePoint* m_pDepthCoordinates;

	static const uint32 colorWidth = 1920;
	static const uint32 colorHeight = 1080;
	static const uint32 depthWidth = 512;
	static const uint32 depthHeight = 424;

	long GetAverageDistanceForRect(RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight, DepthSpacePoint* pDepthPoints, UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight, int start_x, int start_y, int width, int height);
	void ScanForTShirt();
	bool UpdateFrames();
	void InitSensor();

	long scanInterval = 5;
	long lastAction = -1;

	UTexture2D* colorFrame;

	UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
	FTshirtScannedDelegate tshirtScanned;

	/*UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
	FNewKinectColorFrameEvent newColorFrame;*/

	IKinectSensor* m_pKinectSensor;
	ICoordinateMapper* m_pCoordinateMapper;
	// Frame reader
	IMultiSourceFrameReader*m_pMultiSourceFrameReader;
	
	UFUNCTION()
	void OnNewRawColorFrameReceived();

	UFUNCTION()
	void OnNewRawDepthFrameReceived();
	
};
