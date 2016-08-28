// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicMirror.h"
#include "WardrobeManager.h"
#if PLATFORM_WINDOWS
//#include "KinectFunctionLibrary.h"
#endif
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
#include "TextureCreator.h"

#if PLATFORM_WINDOWS
#ifndef WINDOWS_PLATFORM_TYPES_GUARD
#include "AllowWindowsPlatformTypes.h"
#endif
#endif


#if PLATFORM_WINDOWS
using namespace Magick;
#else
FString SFC(const char* arr)
{
    return FString(ANSI_TO_TCHAR(arr));
}
#endif



FString UWardrobeManager::texturePath = FPaths::Combine(*FPaths::GameContentDir(), *FString("PythonProgram"), *FString("textures"));// FString("E:/Unreal Projects/IntelligentMirror/MagicMirror/PythonProgram/textures/");

SQLite::Database* UWardrobeManager::database = NULL;

TArray<FCategory> UWardrobeManager::categories;


FClothingItem::FClothingItem(){
	id = -1;
	category = FCategory();
	texture = UTexture2D::CreateTransient(1024, 1024);
}

FClothingItem::FClothingItem(SQLite::Statement *query){
	id = query->getColumn("id");
	uint32 catID = query->getColumn("category");
	texture = UTexture2D::CreateTransient(1024, 1024);
	for (FCategory cat : UWardrobeManager::categories)
	{
		if (cat.id == catID)
		{
			category = cat;
		}
	}
}


void UWardrobeManager::Tick(float deltaTime)
{
	//Super::Tick(deltaTime)

	this->currentTime += deltaTime;

	

	//bool result = this->UpdateFrames();

	bool result = true;

	if (!result)
	{
		return;
	}

	float secondsDiff = (currentTime - this->lastAction);

	switch (this->mode)
	{
	case EWardrobeMode::MM_Categorizing:
		break;
	case EWardrobeMode::MM_Scanning:
		
#if PLATFORM_WINDOWS
		if (this->lastAction == -1 || secondsDiff > this->scanInterval)
		{
			
			//Did 2 seconds pass since the last scan or do we not have a scan at all?
			if ((this->hasSecondScan && (currentTime - this->timeSinceSecondScan > 4)))
			{
				this->hasFirstScan = false;
				this->hasSecondScan = false;
				this->ScanForTShirt();
			}
			else if (!(this->hasFirstScan && this->hasSecondScan))
			{
				this->ScanForTShirt();
			}

		}
#endif

		break;

	case EWardrobeMode::MM_ScanningForPrint:
#if PLATFORM_WINDOWS
		if (secondsDiff > this->scanInterval)
		{
			this->timeSinceScanningForPrint += deltaTime;
			this->ScanForPrint();
		}
#endif
		break;


	case EWardrobeMode::MM_Outfitting:
		break;
	}

	this->lastAction = currentTime;
}

void UWardrobeManager::StartWardrobeManager(EWardrobeMode mode = EWardrobeMode::MM_Scanning, FString databaseFile = "shirt_db.db")
{
	this->mode = mode;
	this->databaseFile = databaseFile;

	//HARDCODED!!
#if PLATFORM_WINDOWS
	InitializeMagick("C:\\Program Files\\ImageMagick-7.0.2-Q16");
#endif
	//UWardrobeManager::database = SQLite::Database((char*)"E:/Unreal Projects/IntelligentMirror/MagicMirror/PythonProgram/shirt_db.db");

	printd("SQLPath: %s", *FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GameContentDir(), *FString("PythonProgram"), *FString("shirt_db.db"))));
    FString rel_ss = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GameContentDir(), *FString("PythonProgram"), *FString("shirt_db.db")));
    FString ss = FPaths::Combine(*FPaths::GameContentDir(), *FString("PythonProgram"), *FString("shirt_db.db"));

	try
	{
		UWardrobeManager::database = new SQLite::Database(TCHAR_TO_ANSI(*FPaths::Combine(*FPaths::GameContentDir(), *FString("PythonProgram"), *FString("shirt_db.db"))));

		SQLite::Statement query(*database, "SELECT id as id, fullname as fullname, name as name, layer as layer, istrousers as istrousers FROM categories");

		while (query.executeStep())
		{
			FCategory cat(&query);
			categories.Add(cat);
		}

		GetClothesFromDB();
	}
	catch (std::exception& e)
	{
		printe("SQL Error: %s", *SFC(e.what()));
	}
    catch (...)
    {
        printe("Unkown SQL error type!");
    }

	this->scanningCategory = GetCategory(1);

#if PLATFORM_WINDOWS
	pBuffer = new RGBQUAD[colorWidth * colorHeight];
	pDepthBuffer = new uint16[depthWidth * depthHeight];
	m_pDepthCoordinates = new DepthSpacePoint[colorWidth * colorHeight];
#endif
	colorFrame = UTexture2D::CreateTransient(colorWidth, colorHeight);
	printd("Starting Wardrobe Manager");

	FString GameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());

#if PLATFORM_WINDOWS
	//InitPython(GameDir);

	//UKinectFunctionLibrary::newRawColorFrame.Broadcast();
	UKinectFunctionLibrary::newRawColorFrame.AddUObject(this, &UWardrobeManager::OnNewRawColorFrameReceived);

	UKinectFunctionLibrary::newRawDepthFrame.AddUObject(this, &UWardrobeManager::OnNewRawDepthFrameReceived);

#endif
	//UKinectFunctionLibrary::newRawColorFrame.
	//UKinectFunctionLibrary::newRawColorFrame.BindRaw();
	//InitSensor();
}

void UWardrobeManager::GetClothesFromDB()
{
	if (database == NULL)
	{
		printe("Error database not loaded.");
		return;
	}
	try
	{
		SQLite::Statement cQuery(*database, "SELECT id as id, category as category FROM clothes");

		while (cQuery.executeStep())
		{
			FClothingItem item(&cQuery);
			items.Add(item);
		}

		currentItemPos = 0;

		if (items.Num() > 0)
		{
			currentClothingItem = items[currentItemPos];
			LoadTextureForItem(currentClothingItem);
		}
	}
	catch (std::exception& e)
	{
		printe("SQL Error: %s", *SFC(e.what()));
	}
}

FVector2D UWardrobeManager::MapSkeletonToScreenCoords(const FVector& cameraPoint, int32 ScreenSizeX, int32 ScreenSizeY)
{
	CameraSpacePoint spacePoint = CameraSpacePoint();
	spacePoint.X = cameraPoint.X;
	spacePoint.Y = cameraPoint.Y;
	spacePoint.Z = cameraPoint.Z;

	ColorSpacePoint colorSpace;
	this->m_pCoordinateMapper->MapCameraPointToColorSpace(spacePoint, &colorSpace);
	FVector2D colorPoint;
	colorPoint.X = colorSpace.X / colorWidth * ScreenSizeX;
	colorPoint.Y = colorSpace.Y / colorHeight * ScreenSizeY;

	return colorPoint;
}

TArray<FCategory> UWardrobeManager::GetCategories()
{
	return UWardrobeManager::categories;
}

FCategory UWardrobeManager::GetCategory(int32 id)
{
	for (FCategory cat : UWardrobeManager::categories)
	{
		if (cat.id == id)
		{
			return cat;
		}
	}

	return FCategory();
}

FClothingItem UWardrobeManager::NextClothingItem()
{
	if (items.Num() == 0)
	{
		currentClothingItem = FClothingItem();
		currentItemPos = 0;
		return currentClothingItem;
	}

	if (currentItemPos == items.Num()-1)
	{
		currentItemPos = 0;
	}
	else
	{
		currentItemPos++;
	}

	//currentClothingItem.texture->BeginDestroy();

	currentClothingItem = items[currentItemPos];

	currentItemTexture = LoadTextureForItem(currentClothingItem);


	return currentClothingItem;
}

FClothingItem UWardrobeManager::PreviousClothingItem()
{
	if (items.Num() == 0)
	{
		currentClothingItem = FClothingItem();
		currentItemPos = 0;
		return currentClothingItem;
	}

	if (currentItemPos == 0)
	{
		currentItemPos = items.Num() - 1;
	}
	else
	{
		currentItemPos--;
	}

	//currentClothingItem.texture->BeginDestroy();

	currentClothingItem = items[currentItemPos];

	LoadTextureForItem(currentClothingItem);

	return currentClothingItem;
}


FClothingItem UWardrobeManager::RefreshClothingItems()
{
	int numItems = items.Num();

	FClothingItem item = this->FilterClothingItems(scanningCategory, currentClothingItem);

	currentItemPos = items.Num() - 1;
	item = this->items[currentItemPos];
	LoadTextureForItem(item);
	this->currentClothingItem = item;


	return item;
}

//Category.id = -1 -> All clothes

FClothingItem UWardrobeManager::FilterClothingItems(FCategory category, FClothingItem recommendedItem)
{
	try
	{

		FClothingItem first = FClothingItem();
		first.id = -2;
		first.category = category;

		this->items.Empty();

		currentFilter = category;
		
		if (category.id == -1)
		{
			int id = currentClothingItem.id;
			this->GetClothesFromDB();
			items.Insert(first, 0);
			int index = 0;

			for (FClothingItem item : items)
			{
				if (item.id == id)
				{
					break;
				}
				index++;
			}
			if (index >= items.Num())
			{
				index = items.Num() - 1;
			}

			currentItemPos = index;
			currentClothingItem = this->items[currentItemPos];
			LoadTextureForItem(currentClothingItem);
			return currentClothingItem;
		}

		if (database == NULL)
		{
			printe("Error database file is not open");
			return FClothingItem();
		}

		SQLite::Statement cQuery(*database, "SELECT id as id, category as category FROM clothes WHERE category = ?");

		cQuery.bind(1, category.id);
		items.Insert(first, 0);
		while (cQuery.executeStep())
		{
			FClothingItem item(&cQuery);
			items.Add(item);
		}

		int index = 0;

		for (FClothingItem item : items)
		{
			if (item.id >= recommendedItem.id)
			{
				break;
			}
			index++;
		}

		if (items.Num() > 0)
		{
			if (index >= items.Num())
			{
				index = items.Num() - 1;
			}

			currentItemPos = index;
			currentClothingItem = items[currentItemPos];
			LoadTextureForItem(currentClothingItem);
		}
		else
		{
			currentClothingItem = FClothingItem();
			currentClothingItem.id = -1;
		}

		return currentClothingItem;
	}
	catch (std::exception& e)
	{
		printe("SQL Error: %s", *SFC(e.what()));
	}

	currentClothingItem = FClothingItem();
	currentClothingItem.id = -1;

	return currentClothingItem;
}

UTexture2D* UWardrobeManager::LoadTextureForItem(FClothingItem &currentClothingItem)
{
	FString path = FString::Printf(TEXT("%s/%s/%04d/final_texture.png"), *texturePath, *currentClothingItem.category.name, currentClothingItem.id);
	UTexture2D *fText = LoadImageFromFile(path);
	currentClothingItem.texture = fText;

	return fText;
}

#if PLATFORM_WINDOWS
void UWardrobeManager::InitSensor()
{
	
}
#endif


void UWardrobeManager::OnNewRawColorFrameReceived()
{
#if PLATFORM_WINDOWS
	this->pBuffer = UKinectFunctionLibrary::pBuffer;
	this->m_pCoordinateMapper = UKinectFunctionLibrary::coordinateMapper;
#endif
}

void UWardrobeManager::OnNewRawDepthFrameReceived()
{
#if PLATFORM_WINDOWS
	this->pDepthBuffer = UKinectFunctionLibrary::pDepthBuffer;
#endif
}

#if PLATFORM_WINDOWS
long UWardrobeManager::GetAverageDistanceForRect(RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight, DepthSpacePoint* pDepthPoints, UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight, int start_x, int start_y, int width, int height)
{
	long sum = 0;

	//printd("Args: %i, %i, %i, %i", start_x, start_y, width, height);

	for (int x = start_x; x < start_x + width; x++)
	{
		for (int y = start_y; y < start_y + height; y++)
		{
			int colorIndex = x + y*nColorWidth;

			if (pDepthPoints == NULL)
			{
				return 0;
			}

			//printd("Color Index: X:%i, Y:%i", x,y);

			DepthSpacePoint p = pDepthPoints[colorIndex];
			if (FMath::IsFinite(p.X) && FMath::IsFinite(p.Y))
			{
				int depthX = static_cast<int>(p.X + 0.5f);
				int depthY = static_cast<int>(p.Y + 0.5f);

				if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
				{
					long depthValue = pDepthBuffer[depthX + (depthY * nDepthWidth)];
					sum += depthValue;
				}
			}
		}
	}
	return float(sum) / (width*height);
}
#endif

UWardrobeManager::~UWardrobeManager()
{
	/*if (pBuffer != NULL)
	{
		delete[] pBuffer;
	}

	if (pDepthBuffer != NULL)
	{
		//delete[] pDepthBuffer;
	}*/
#if PLATFORM_WINDOWS
	if (m_pDepthCoordinates != NULL)
	{
		delete[] m_pDepthCoordinates;
	}

	/*if (m_pCoordinateMapper != NULL)
	{
		m_pCoordinateMapper->Release();
	}
	
	if (m_pMultiSourceFrameReader != NULL)
	{
		m_pMultiSourceFrameReader->Release();
	}*/
	
//	Py_DECREF(path);

	Py_Finalize();
#endif
}

#if PLATFORM_WINDOWS
Image UWardrobeManager::CreateMagickImageFromBuffer(RGBQUAD* pBuffer, int width, int height)
{
	double startTime = FPlatformTime::Seconds();
/*	size_t rgbquad_size = sizeof(RGBQUAD);
	size_t total_bytes = width * height * rgbquad_size;

	for (size_t i = 0; i < width*height; i++)
	{
		RGBQUAD q = pBuffer[i];
		q.rgbReserved = 255;
	}

	Blob blob = Blob(pBuffer, total_bytes);
	Image img;
	img.size(Geometry(width, height));

	img.magick("RGBA");
	img.depth(8);
	img.read(blob);
	double endTime = FPlatformTime::Seconds();
	printd("Time taken: %f", (endTime - startTime))
	return img;*/

	
	Image img;
	HRESULT hr = SaveBitmapToFile(reinterpret_cast<BYTE*>(pBuffer), width, height, sizeof(RGBQUAD) * 8, L"tmp.bmp");

	if (SUCCEEDED(hr))
	{
		try {
			img.read("tmp.bmp");
		}
		catch (Exception &error_)
		{
			printe("Error: %s", *SFC(error_.what()));
		}
	}

	double endTime = FPlatformTime::Seconds();
	printd("Time taken: %f", (endTime - startTime))

	return img;
}

RGBQUAD* UWardrobeManager::CutRectFromBuffer(RGBQUAD* pBuffer, int colorWidth, int colorHeight, int start_x, int start_y, int width, int height)
{
	double startTime = FPlatformTime::Seconds();

	RGBQUAD* finalBuffer = new RGBQUAD[width*height];
	int index = 0;
	for (int y = start_y; y < start_y + height; y++)
	{
		for (int x = start_x; x < start_x + width; x++)
		{
			finalBuffer[index].rgbRed = pBuffer[x + y*colorWidth].rgbRed;
			finalBuffer[index].rgbGreen = pBuffer[x + y*colorWidth].rgbGreen;
			finalBuffer[index].rgbBlue = pBuffer[x + y*colorWidth].rgbBlue;
			index++;
		}
	}

	double endTime = FPlatformTime::Seconds();
	printd("Time Taken for this: %f", (endTime - startTime));

	return finalBuffer;
}

bool UWardrobeManager::HasFlatSurface(Image edgeImage, int start_x, int start_y, int width, int height)
{
	int maxEdgePixels = 2;
	int currentEdgePixels = 0;
	int edgePixels = 0;

	double startTime = FPlatformTime::Seconds();

	try {
		Quantum* quant = edgeImage.getPixels(start_x, start_y, width, height);
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				Quantum realQuant = quant[x + y*width];
				if (realQuant >= 65535)
				{
					edgePixels += 1;
					int count = 0;
					for (int newY = y - 10; newY < y + 10; newY++)
					{
						if (newY > 0 && newY < height)
						{
							Quantum q = quant[x + newY*width];
							if (q >= 65535)
							{
								count += 1;
							}
						}
					}

					for (int newX = x - 10; newX < x + 10; newX++)
					{
						if (newX > 0 && newX < width)
						{
							Quantum q = quant[newX + y*width];
							if (q >= 65535)
							{
								count += 1;
							}
						}
					}

					if (count > 3)
					{
						currentEdgePixels += 1;
					}
				}

				if (x >= 555)
				{
					//LOG(INFO) << "X Is at the max: " << x;
				}

			}
		}

		//printd << "Current Edge Pixels: " << currentEdgePixels;

		if (currentEdgePixels <= maxEdgePixels)
		{
			printw("Found TShirt!: %i", edgePixels);
			double endTime = FPlatformTime::Seconds();
			printd("Time Taken for this: %f", (endTime - startTime));

			return true;
		}
		else
		{
			printd("To many edge pixels: %i, %i", currentEdgePixels, edgePixels);
		}

	}
	catch (Exception &error_)
	{
		printe("Error %s",  *SFC(error_.what()));
	}

	double endTime = FPlatformTime::Seconds();
	printd("Time Taken for this: %f", (endTime - startTime));


	return false;
}
#endif

UTexture2D* UWardrobeManager::LoadImageFromFile(FString file)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	// Note: PNG format.  Other formats are supported
	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	TArray<uint8> RawFileData;

	if (FFileHelper::LoadFileToArray(RawFileData, *file))
	{
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			const TArray<uint8>* UncompressedBGRA = NULL;
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
			{
				UTexture2D* mytex = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

				void* TextureData = mytex->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, UncompressedBGRA->GetTypedData(), UncompressedBGRA->Num());
				mytex->PlatformData->Mips[0].BulkData.Unlock();

				// Update the rendering resource from data.
				mytex->UpdateResource();

				return mytex;
			}
		}
	}

	return UTexture2D::CreateTransient(512, 512);
}

void UWardrobeManager::ScanPrint(FClothingItem tshirt)
{
#if PLATFORM_WINDOWS
	this->mode = EWardrobeMode::MM_ScanningForPrint;
	this->currentPrintScan = tshirt;
	this->timeSinceScanningForPrint = 0;
#endif
}

#if PLATFORM_WINDOWS
void UWardrobeManager::ScanForPrint()
{
	if (timeToScanForPrint - timeSinceScanningForPrint >= 0)
	{
		ScanningStatusUpdate.Broadcast(FString::Printf(TEXT("Get TShirt ready! Taking picture in: %f"), timeToScanForPrint - timeSinceScanningForPrint));
	}

	if (timeToScanForPrint - timeSinceScanningForPrint < 0)
	{
		ScanningStatusUpdate.Broadcast(FString("Print Scanned! Processing."));
		int distanceSize = 556;
		int start_x = (colorWidth - distanceSize) / 2;
		int start_y = (colorHeight - distanceSize) / 2;
		RGBQUAD* cutRect = CutRectFromBuffer(pBuffer, colorWidth, colorHeight, start_x, start_y, distanceSize, distanceSize);
		Image cutRectImage = CreateMagickImageFromBuffer(cutRect, distanceSize, distanceSize);
		cutRectImage.write("print_testing.png");
		/*if (FPrintTextureCreator::PrintCreator->IsThreadFinished() == true)
		{
			FTextureCreator::Shutdown();
			FPrintTextureCreator::Shutdown();
			FPrintTextureCreator::JoyInit(this, this->currentClothingItem.id);
		}*/

		addPrintToItemFromCWD(this->currentClothingItem.id);
		FString finalTexturePath = FString::Printf(TEXT("%s/tshirt/%04d/final_texture.png"), *UWardrobeManager::texturePath, this->currentClothingItem.id);
		UTexture2D* finalTexture = this->LoadImageFromFile(finalTexturePath);
		this->TshirtProcessed.Broadcast(finalTexture);

		this->mode = EWardrobeMode::MM_None;
	}
}



void UWardrobeManager::ScanForTShirt()
{
	int rangeMin = 700;
	int rangeMax = 900;

	if (m_pCoordinateMapper == NULL || pBuffer == NULL)
	{
		return;
	}

	FCategory filter = scanningCategory;
	if (filter.id < 1)
	{
		filter = this->GetCategory(1);
	}

	//ScanningStatusUpdate.Broadcast(FString::Printf(TEXT("Scanning for %s"), *currentFilter.fullname));

	
	HRESULT hr = m_pCoordinateMapper->MapColorFrameToDepthSpace(depthWidth * depthHeight, (UINT16*)pDepthBuffer, colorWidth * colorHeight, m_pDepthCoordinates);

	if (m_pDepthCoordinates == NULL)
	{
		return;
	}

	if (SUCCEEDED(hr))
	{
		int distanceSize = 556;
		if (scanningCategory.isTrousers)
		{
			distanceSize = 278;
		}
		int start_x = (colorWidth - distanceSize) / 2;
		int start_y = (colorHeight - distanceSize) / 2;
		long averageValue = GetAverageDistanceForRect(pBuffer, colorWidth, colorHeight, m_pDepthCoordinates, pDepthBuffer, depthWidth, depthHeight, start_x, start_y, distanceSize, distanceSize);

		if (averageValue > rangeMin && averageValue < rangeMax)
		{
			printd("Found good distance in center square!: %fcm", (averageValue/10.0));
		}
		else
		{
			if (averageValue < rangeMin)
			{
				//printw("Please move the TShirt back a bit.");
				ScanningStatusUpdate.Broadcast(FString::Printf(TEXT("Scanning for %s. Please move back a bit"), *filter.fullname));
			}

			if (averageValue > rangeMax)
			{
				//printw("Please move the TShirt forth a bit.");
				ScanningStatusUpdate.Broadcast(FString::Printf(TEXT("Scanning for %s. Please move forward a bit"), *filter.fullname));
			}

			return;
		}

		int size = 512;
		if (scanningCategory.isTrousers)
		{
			size = 256;
		}
		//Use old start_x and y. start_x and y are relative to the full color image (1920x1080).
		//Therefore they need to be converted.
		RGBQUAD* cutRect = CutRectFromBuffer(pBuffer, colorWidth, colorHeight, start_x, start_y, distanceSize, distanceSize);
		Image cutRectImage = CreateMagickImageFromBuffer(cutRect, distanceSize, distanceSize);
		Image edgeImage = Image(cutRectImage);
		try{
			double startTime = FPlatformTime::Seconds();
			edgeImage.cannyEdge();
			double endTime = FPlatformTime::Seconds();
			printd("Time Taken for this: %f", (endTime - startTime));
			//cutRectImage.write("testing2.png");
			//edgeImage.write("testing.png");

			if (!this->hasFirstScan)
			{
				start_y = (colorHeight - size) / 2;
				int realStart_y = ConvertCoordFromBiggerRect(colorHeight, distanceSize, start_y);
				int realStart_x = ConvertCoordFromBiggerRect(colorWidth, distanceSize, start_x);
				if (HasFlatSurface(edgeImage, realStart_x, realStart_y, size, size))
				{
					edgeImage.write("edges1.png");
					Image finalImg = Image(cutRectImage);
					//LOG(INFO) << realStart_x << " " << realStart_y << " " << size;
					printw("First Scan succeeded!");
					finalImg.crop(Geometry(size, size, realStart_x, realStart_y));
					finalImg.write("back1.png");
					this->hasFirstScan = true;
				}
				else
				{
					//SetStatusMessage(L"Please move the TShirt to the left a bit.", 1000, true);
					ScanningStatusUpdate.Broadcast(FString::Printf(TEXT("Scanning for %s on the left side."), *filter.fullname));
				}

			}

			if (!this->hasSecondScan && this->hasFirstScan)
			{
				start_y = (colorHeight - size) / 2;
				start_x = start_x + distanceSize - size;
				int realStart_y = ConvertCoordFromBiggerRect(colorHeight, distanceSize, start_y);
				int realStart_x = ConvertCoordFromBiggerRect(colorWidth, distanceSize, start_x);
				if (HasFlatSurface(edgeImage, realStart_x, realStart_y, size, size))
				{
					edgeImage.write("edges2.png");
					//LOG(INFO) << realStart_x << " " << realStart_y << " " << size;
					Image finalImg = Image(cutRectImage);
					finalImg.crop(Geometry(size, size, realStart_x, realStart_y));
					finalImg.write("back2.png");
					this->hasSecondScan = true;
					this->timeSinceSecondScan = this->currentTime;
				}
				else
				{
					//SetStatusMessage(L"Please move the TShirt to the right a bit.", 1000, true);
					ScanningStatusUpdate.Broadcast(FString::Printf(TEXT("Scanning for %s on the right side."), *filter.fullname));
				}
			}

			if (this->hasFirstScan && this->hasSecondScan)
			{
				//TODO: Communicate with Python CInterface!
				//SetStatusMessage(L"Scanned TShirt! Prepare the next one!", 2000, true);
				TshirtScanned.Broadcast();
				ScanningStatusUpdate.Broadcast(FString::Printf(TEXT("%s scanned! Processing."), *filter.fullname));
				/*int newItemID = createNewItemWithTextures("back1.png", "back2.png");
				//LOG(INFO) << "Created new Clothing Item with ID: " << newItemID;
				FString finalTexturePath = FString::Printf(TEXT("%stshirt/%04d/final_texture.png"), *texturePath, newItemID);
				UTexture2D* finalTexture = this->LoadImageFromFile(finalTexturePath);
				this->TshirtProcessed.Broadcast(finalTexture);*/
				if (FTextureCreator::Runnable->IsThreadFinished())
				{
					FTextureCreator::Shutdown();
					FTextureCreator::JoyInit(this, scanningCategory);
				}
			}
			delete[]cutRect;
		}
		catch (Exception &error_)
		{
			printe("Error: %s", *SFC(error_.what()));
			delete[]cutRect;
		}
	}
}


bool UWardrobeManager::UpdateFrames(){

	return false;
}
#endif




