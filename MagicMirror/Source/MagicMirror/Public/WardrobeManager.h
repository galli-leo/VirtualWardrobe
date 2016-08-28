// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"

#if PLATFORM_WINDOWS
#ifndef WINDOWS_PLATFORM_TYPES_GUARD
#include "AllowWindowsPlatformTypes.h"
#endif
#include "Kinect.h"

#include "HideWindowsPlatformTypes.h"

#include <Magick++.h>
#endif

#if PLATFORM_WINDOWS
#ifndef WINDOWS_PLATFORM_TYPES_GUARD
#include "AllowWindowsPlatformTypes.h"
#endif
#endif
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#if PLATFORM_WINDOWS
#include "HideWindowsPlatformTypes.h"
#endif

#include "PrintTextureAdder.h"
#if PLATFORM_WINDOWS
#include "KinectFunctionLibrary.h"
#include "PythonUtils.h"
#endif
#include "Tickable.h"

//#include "WardrobeStructs.h"
#include "WardrobeManager.generated.h"



UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EWardrobeMode : uint8
{
	MM_None			UMETA(DisplayName = "None"),
	MM_Outfitting 	UMETA(DisplayName = "Outfitting"),
	MM_Scanning 	UMETA(DisplayName = "Scanning"),
	MM_ScanningForPrint 	UMETA(DisplayName = "Scanning for print"),
	MM_ScanningForPattern 	UMETA(DisplayName = "Scanning for pattern"),
	MM_Categorizing	UMETA(DisplayName = "Categorizing")

};


USTRUCT(BlueprintType)
struct FCategory{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		FString fullname;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		int32 id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		bool isTrousers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Structs")
		int32 layer;

	FCategory(){
		fullname = FString("");
		id = -1;
		name = FString("");
		isTrousers = false;
		layer = 0;
	}

	FCategory(FString fullname, int32 theID, FString name, bool trousers, int32 theLayer){
		fullname = fullname;
		id = theID;
		name = name;
		isTrousers = trousers;
		layer = theLayer;
	}

	FCategory(SQLite::Statement *query){
		fullname = FString(query->getColumn("fullname").getText());
		id = query->getColumn("id");
		name = FString(query->getColumn("name").getText());
		isTrousers = ((int)query->getColumn("istrousers") != 0);
		layer = query->getColumn("layer");
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScanningStatusUpdate, FString, newStatus);
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

		UPROPERTY(BlueprintReadWrite)
		FCategory scanningCategory;

		//UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		void StartWardrobeManager(EWardrobeMode startingMode, FString dbFile);
		//UPROPERTY(BlueprintReadOnly)

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		void Tick(float deltaTime);

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
			void ScanPrint(FClothingItem tshirt);

		UFUNCTION(BlueprintCallable, Category = "Kinect")
			FVector2D MapSkeletonToScreenCoords(const FVector& spacePoint, int32 ScreenSizeX, int32 ScreenSizeY);

#if PLATFORM_WINDOWS
		
#endif
		UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
		FTshirtProcessedDelegate TshirtProcessed;

		UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
			FScanningStatusUpdate ScanningStatusUpdate;

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		TArray<FCategory> GetCategories();

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
			FCategory GetCategory(int32 id);



		static FString texturePath;

		~UWardrobeManager();

		static SQLite::Database* database;

		/************************************************************************/
		/* Outfitting                                                           */
		/************************************************************************/

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
			FClothingItem NextClothingItem();

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
			FClothingItem PreviousClothingItem();

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
			FClothingItem RefreshClothingItems();

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
			FClothingItem FilterClothingItems(FCategory category, FClothingItem recommendedItem);

		UPROPERTY(BlueprintReadOnly, Category = "Wardrobe")
			FClothingItem currentClothingItem;

		UPROPERTY(BlueprintReadOnly, Category = "Wardrobe")
			FCategory currentFilter;

		UPROPERTY(BlueprintReadOnly, Category = "Wardrobe")
			UTexture2D* currentItemTexture;


		TArray<FClothingItem> items;

		int currentItemPos;

		UTexture2D* LoadTextureForItem(FClothingItem &item);

		UFUNCTION(BlueprintCallable, Category = "Wardrobe")
		void GetClothesFromDB();

		static TArray<FCategory> categories;

private:
	

	float currentTime = 0;

#if PLATFORM_WINDOWS
	RGBQUAD* pBuffer;
	uint16* pDepthBuffer;
	DepthSpacePoint* m_pDepthCoordinates;
	ICoordinateMapper* m_pCoordinateMapper;
#endif

	static const uint32 colorWidth = 1920;
	static const uint32 colorHeight = 1080;
	static const uint32 depthWidth = 512;
	static const uint32 depthHeight = 424;



	UTexture2D* LoadImageFromFile(FString file);
#if PLATFORM_WINDOWS
	long GetAverageDistanceForRect(RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight, DepthSpacePoint* pDepthPoints, UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight, int start_x, int start_y, int width, int height);
	void ScanForTShirt();
	void ScanForPrint();
	Magick::Image			CreateMagickImageFromBuffer(RGBQUAD* pBuffer, int width, int height);
	bool					HasFlatSurface(Magick::Image img, int start_x, int start_y, int width, int height);
	bool UpdateFrames();
	RGBQUAD* CutRectFromBuffer(RGBQUAD* pBuffer, int colorWidth, int colorHeight, int start_x, int start_y, int width, int height);
	void InitSensor();
#endif

	FClothingItem currentPrintScan;
	float timeSinceScanningForPrint = -1;
	bool hasFirstScan = false;
	bool hasSecondScan = false;
	float scanInterval = 1 / 3;
	float timeSinceSecondScan = -1;
	float lastAction = -1;
	float timeToScanForPrint = 7.5;

	UTexture2D* colorFrame;

	UPROPERTY(BlueprintAssignable, Category = "Wardrobe")
	FTshirtScannedDelegate TshirtScanned;

	
	UFUNCTION()
	void OnNewRawColorFrameReceived();

	UFUNCTION()
	void OnNewRawDepthFrameReceived();
	
};



