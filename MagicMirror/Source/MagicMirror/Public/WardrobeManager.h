// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"

#ifndef WINDOWS_PLATFORM_TYPES_GUARD
#include "AllowWindowsPlatformTypes.h"
#endif
#include "Kinect.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include "HideWindowsPlatformTypes.h"

#include <Magick++.h>
#include "TextureCreator.h"
#include "PythonUtils.h"
#include "Tickable.h"
#include "WardrobeManager.generated.h"



UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EWardrobeMode : uint8
{
	MM_None			UMETA(DisplayName = "None"),
	MM_Outfitting 	UMETA(DisplayName = "Outfitting"),
	MM_Scanning 	UMETA(DisplayName = "Scanning"),
	MM_Categorizing	UMETA(DisplayName = "Categorizing")

};



USTRUCT(BlueprintType)
struct FCategory{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly)
		FString fullname;
	UPROPERTY(BlueprintReadOnly)
		int32 id;
	UPROPERTY(BlueprintReadOnly)
		FString name;

	FCategory(){
		fullname = FString("");
		id = -1;
		name = FString("");
	}

	FCategory(SQLite::Statement *query){
		fullname = FString(query->getColumn("fullname").getText());
		id = query->getColumn("id");
		name = FString(query->getColumn("name").getText());
	}
};


USTRUCT(BlueprintType)
struct FClothingItem{
	GENERATED_BODY()
		UPROPERTY(BlueprintReadOnly)
		int32 id;
	UPROPERTY(BlueprintReadOnly)
		FCategory category;
	UPROPERTY(BlueprintReadOnly)
		UTexture2D* texture;

	FClothingItem();

	FClothingItem(SQLite::Statement *query);
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTshirtScannedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTshirtProcessedDelegate, UTexture2D*, texture);
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

		UPROPERTY(BlueprintReadWrite)
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

		UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
		FTshirtProcessedDelegate TshirtProcessed;

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		TArray<FCategory> GetCategories();

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		FClothingItem NextClothingItem();

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		FClothingItem PreviousClothingItem();

		UPROPERTY(BlueprintReadOnly, Category = "Wardrobe")
		FClothingItem currentClothingItem;

		UPROPERTY(BlueprintReadOnly, Category = "Wardrobe")
			UTexture2D* currentItemTexture;


		static FString texturePath;

		~UWardrobeManager();

		static SQLite::Database database;

		TArray<FClothingItem> items;

		int currentItemPos;

		UTexture2D* LoadTextureForItem(FClothingItem &item);

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		void GetClothesFromDB();

		static TArray<FCategory> categories;

private:
	

	float currentTime = 0;

	RGBQUAD* pBuffer;
	uint16* pDepthBuffer;
	DepthSpacePoint* m_pDepthCoordinates;

	static const uint32 colorWidth = 1920;
	static const uint32 colorHeight = 1080;
	static const uint32 depthWidth = 512;
	static const uint32 depthHeight = 424;



	UTexture2D* LoadImageFromFile(FString file);

	long GetAverageDistanceForRect(RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight, DepthSpacePoint* pDepthPoints, UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight, int start_x, int start_y, int width, int height);
	void ScanForTShirt();
	Magick::Image			CreateMagickImageFromBuffer(RGBQUAD* pBuffer, int width, int height);
	bool					HasFlatSurface(Magick::Image img, int start_x, int start_y, int width, int height);
	bool UpdateFrames();
	RGBQUAD* CutRectFromBuffer(RGBQUAD* pBuffer, int colorWidth, int colorHeight, int start_x, int start_y, int width, int height);
	void InitSensor();

	bool hasFirstScan = false;
	bool hasSecondScan = false;
	float scanInterval = 1 / 3;
	float timeSinceSecondScan = -1;
	float lastAction = -1;

	UTexture2D* colorFrame;

	UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
	FTshirtScannedDelegate TshirtScanned;

	

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



