// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicMirror.h"
#include "WardrobeManager.h"

#include "KinectFunctionLibrary.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
#ifndef WINDOWS_PLATFORM_TYPES_GUARD
#include "AllowWindowsPlatformTypes.h"
#endif



using namespace Magick;

FString UWardrobeManager::texturePath = FString("E:/Unreal Projects/IntelligentMirror/MagicMirror/PythonProgram/textures/");



// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
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

		break;
	case EWardrobeMode::MM_Outfitting:
		break;
	}

	this->lastAction = currentTime;
}

void UWardrobeManager::StartWardrobeManager(EWardrobeMode mode = EWardrobeMode::MM_Scanning, FString databaseFile = "Fuck all")
{
	this->mode = mode;
	this->databaseFile = databaseFile;

	//HARDCODED!!
	InitializeMagick("C:\\Program Files\\ImageMagick-7.0.2-Q16");

	pBuffer = new RGBQUAD[colorWidth * colorHeight];
	pDepthBuffer = new uint16[depthWidth * depthHeight];
	colorFrame = UTexture2D::CreateTransient(colorWidth, colorHeight);
	m_pDepthCoordinates = new DepthSpacePoint[colorWidth * colorHeight];

	printd("Starting Wardrobe Manager");

	FString GameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());

	InitPython(GameDir);

	//UKinectFunctionLibrary::newRawColorFrame.Broadcast();
	UKinectFunctionLibrary::newRawColorFrame.AddUObject(this, &UWardrobeManager::OnNewRawColorFrameReceived);

	UKinectFunctionLibrary::newRawDepthFrame.AddUObject(this, &UWardrobeManager::OnNewRawDepthFrameReceived);
	//UKinectFunctionLibrary::newRawColorFrame.
	//UKinectFunctionLibrary::newRawColorFrame.BindRaw();
	//InitSensor();
}

uint8 UWardrobeManager::TestPython()
{
	//uint8 result = createNewItemWithTextures((const char*)"E:/Unreal Projects/IntelligentMirror/IMBasics/back1.png", (const char*)"E:/Unreal Projects/IntelligentMirror/IMBasics/back2.png");
	//return result;
	return -1;
}

FString UWardrobeManager::GetCurrentPathAsSeenByPython()
{
	//return getCurrentPath();
	return FString("I dont even know.");
}

void UWardrobeManager::InitSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get the color reader
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->Open();
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_BodyIndex,
				&m_pMultiSourceFrameReader);
		}
	}
}


void UWardrobeManager::OnNewRawColorFrameReceived()
{
	this->pBuffer = UKinectFunctionLibrary::pBuffer;
	this->m_pCoordinateMapper = UKinectFunctionLibrary::coordinateMapper;
}

void UWardrobeManager::OnNewRawDepthFrameReceived()
{
	this->pDepthBuffer = UKinectFunctionLibrary::pDepthBuffer;
}

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
//	Py_DECREF(sysPath);

	Py_Finalize();
}

Image UWardrobeManager::CreateMagickImageFromBuffer(RGBQUAD* pBuffer, int width, int height)
{
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

	return img;
}

RGBQUAD* UWardrobeManager::CutRectFromBuffer(RGBQUAD* pBuffer, int colorWidth, int colorHeight, int start_x, int start_y, int width, int height)
{
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

	return finalBuffer;
}

bool UWardrobeManager::HasFlatSurface(Image edgeImage, int start_x, int start_y, int width, int height)
{
	int maxEdgePixels = 2;
	int currentEdgePixels = 0;
	int edgePixels = 0;

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

	return false;
}

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

void UWardrobeManager::ScanForTShirt()
{
	int rangeMin = 700;
	int rangeMax = 900;

	if (m_pCoordinateMapper == NULL || pBuffer == NULL)
	{
		return;
	}

	
	HRESULT hr = m_pCoordinateMapper->MapColorFrameToDepthSpace(depthWidth * depthHeight, (UINT16*)pDepthBuffer, colorWidth * colorHeight, m_pDepthCoordinates);

	if (m_pDepthCoordinates == NULL)
	{
		return;
	}

	if (SUCCEEDED(hr))
	{
		int distanceSize = 556;
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
			}

			if (averageValue > rangeMax)
			{
				//printw("Please move the TShirt forth a bit.");
			}

			return;
		}

		int size = 512;
		//Use old start_x and y. start_x and y are relative to the full color image (1920x1080).
		//Therefore they need to be converted.
		RGBQUAD* cutRect = CutRectFromBuffer(pBuffer, colorWidth, colorHeight, start_x, start_y, distanceSize, distanceSize);
		Image cutRectImage = CreateMagickImageFromBuffer(cutRect, distanceSize, distanceSize);
		Image edgeImage = Image(cutRectImage);
		try{
			edgeImage.cannyEdge();
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
				}
			}

			if (this->hasFirstScan && this->hasSecondScan)
			{
				//TODO: Communicate with Python CInterface!
				//SetStatusMessage(L"Scanned TShirt! Prepare the next one!", 2000, true);
				TshirtScanned.Broadcast();
				/*int newItemID = createNewItemWithTextures("back1.png", "back2.png");
				//LOG(INFO) << "Created new Clothing Item with ID: " << newItemID;
				FString finalTexturePath = FString::Printf(TEXT("%stshirt/%04d/final_texture.png"), *texturePath, newItemID);
				UTexture2D* finalTexture = this->LoadImageFromFile(finalTexturePath);
				this->TshirtProcessed.Broadcast(finalTexture);*/
				if (FTextureCreator::Runnable->IsThreadFinished())
				{
					FTextureCreator::Shutdown();
					FTextureCreator::JoyInit(this);
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
	if (!m_pMultiSourceFrameReader)
	{
		return false;
	}

	IMultiSourceFrame* pMultiSourceFrame = NULL;
	IColorFrame* pColorFrame = NULL;
	IDepthFrame* pDepthFrame = NULL;
	IBodyIndexFrame* pBodyIndexFrame = NULL;

	HRESULT hr = m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);

	while (hr == E_PENDING)
	{
		hr = m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);
	}

	if (SUCCEEDED(hr))
	{
		IDepthFrameReference* pDepthFrameReference = NULL;

		hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
		}

		SafeRelease(pDepthFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		IColorFrameReference* pColorFrameReference = NULL;

		hr = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pColorFrameReference->AcquireFrame(&pColorFrame);
		}

		SafeRelease(pColorFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		IBodyIndexFrameReference* pBodyIndexFrameReference = NULL;

		hr = pMultiSourceFrame->get_BodyIndexFrameReference(&pBodyIndexFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameReference->AcquireFrame(&pBodyIndexFrame);
		}

		SafeRelease(pBodyIndexFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		INT64 nDepthTime = 0;
		IFrameDescription* pDepthFrameDescription = NULL;
		int nDepthWidth = 0;
		int nDepthHeight = 0;
		UINT nDepthBufferSize = 0;

		IFrameDescription* pColorFrameDescription = NULL;
		int nColorWidth = 0;
		int nColorHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nColorBufferSize = 0;

		IFrameDescription* pBodyIndexFrameDescription = NULL;
		int nBodyIndexWidth = 0;
		int nBodyIndexHeight = 0;
		UINT nBodyIndexBufferSize = 0;
		BYTE *pBodyIndexBuffer = NULL;

		// get depth frame data

		hr = pDepthFrame->get_RelativeTime(&nDepthTime);

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pDepthFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameDescription->get_Width(&nDepthWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameDescription->get_Height(&nDepthHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
		}

		//get color frame data

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameDescription->get_Width(&nColorWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameDescription->get_Height(&nColorHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			/*if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
				
			}
			else*/ if (pBuffer)
			{
				RGBQUAD* pColorBuffer = pBuffer;
				nColorBufferSize = colorWidth * colorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
				UTexture2D* Texture = colorFrame;

				const size_t Size = colorWidth * colorHeight* sizeof(RGBQUAD);

				uint8* Src = (uint8*)pBuffer;

				uint8* Dest = (uint8*)Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

				FMemory::Memcpy(Dest, Src, Size);

				Texture->PlatformData->Mips[0].BulkData.Unlock();

				Texture->UpdateResource();

				//newColorFrame.Broadcast(colorFrame);
			}
			else
			{
				hr = E_FAIL;
			}
		}

		// get body index frame data

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrame->get_FrameDescription(&pBodyIndexFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameDescription->get_Width(&nBodyIndexWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameDescription->get_Height(&nBodyIndexHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrame->AccessUnderlyingBuffer(&nBodyIndexBufferSize, &pBodyIndexBuffer);
		}

		if (SUCCEEDED(hr))
		{
			return true;/*ProcessFrame(nDepthTime, pDepthBuffer, nDepthWidth, nDepthHeight,
				pColorBuffer, nColorWidth, nColorHeight,
				pBodyIndexBuffer, nBodyIndexWidth, nBodyIndexHeight);*/
		}

		SafeRelease(pDepthFrameDescription);
		SafeRelease(pColorFrameDescription);
		SafeRelease(pBodyIndexFrameDescription);
	}

	SafeRelease(pDepthFrame);
	SafeRelease(pColorFrame);
	SafeRelease(pBodyIndexFrame);
	SafeRelease(pMultiSourceFrame);

	return false;
}




