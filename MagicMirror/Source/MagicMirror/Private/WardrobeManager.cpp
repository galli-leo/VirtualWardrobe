// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicMirror.h"
#include "WardrobeManager.h"


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

	bool result = this->UpdateFrames();

	if (!result)
	{
		return;
	}

	switch (this->mode)
	{
	case EWardrobeMode::MM_Categorizing:
		break;
	case EWardrobeMode::MM_Scanning:
		if (this->currentTime - this->lastAction > this->scanInterval)
		{
			this->lastAction = this->currentTime;
			this->ScanForTShirt();
		}
		break;
	case EWardrobeMode::MM_Outfitting:
		break;
	}
}

void UWardrobeManager::StartWardrobeManager(EWardrobeMode mode = EWardrobeMode::MM_Scanning, FString databaseFile = "Fuck all")
{
	this->mode = mode;
	this->databaseFile = databaseFile;
	HRESULT hr;

	pBuffer = new RGBQUAD[colorWidth * colorHeight];
	pDepthBuffer = new uint16[depthWidth * depthHeight];
	colorFrame = UTexture2D::CreateTransient(colorWidth, colorHeight);
	m_pDepthCoordinates = new DepthSpacePoint[colorWidth * colorHeight];

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

/*
void UWardrobeManager::OnNewRawColorFrameReceived(RGBQUAD* pBuffer)
{
	this->pBuffer = pBuffer;
}

void UWardrobeManager::OnNewRawDepthFrameReceived(uint16* pDepthBuffer)
{
	this->pDepthBuffer = pDepthBuffer;
}*/

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
	if (pBuffer != nullptr)
	{
		delete[] pBuffer;
	}

	if (pDepthBuffer != NULL)
	{
		//delete[] pDepthBuffer;
	}

	if (m_pDepthCoordinates != NULL)
	{
		delete[] m_pDepthCoordinates;
	}

	if (m_pCoordinateMapper != NULL)
	{
		m_pCoordinateMapper->Release();
	}
	
	if (m_pMultiSourceFrameReader != NULL)
	{
		m_pMultiSourceFrameReader->Release();
	}
	
}

void UWardrobeManager::ScanForTShirt()
{
	int rangeMin = 750;
	int rangeMax = 900;

	if (m_pKinectSensor == NULL || m_pCoordinateMapper == NULL)
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
		int start_y = (colorWidth - distanceSize) / 2;
		/*long averageValue = GetAverageDistanceForRect(pBuffer, colorWidth, colorHeight, m_pDepthCoordinates, pDepthBuffer, depthWidth, depthHeight, start_x, start_y, distanceSize, distanceSize);

		if (averageValue > rangeMin && averageValue < rangeMax)
		{
			printd("Found good distance in center square!");
		}
		/*else
		{
		if (averageValue < rangeMin)
		{
		SetStatusMessage(L"Please move the TShirt back a bit.", 500, true);
		}

		if (averageValue > rangeMax)
		{
		SetStatusMessage(L"Please move the TShirt forth a bit.", 500, true);
		}

		return;
		}*/

		/*int size = 512;
		//Use old start_x and y. start_x and y are relative to the full color image (1920x1080).
		//Therefore they need to be converted.
		RGBQUAD* cutRect = CutRectFromBuffer(pBuffer, width, height, start_x, start_y, distanceSize, distanceSize);
		Image cutRectImage = CreateMagickImageFromBuffer(cutRect, distanceSize, distanceSize);
		Image edgeImage = Image(cutRectImage);
		try{
		edgeImage.edge();

		if (!this->hasFirstScan)
		{
		start_y = (height - size) / 2;
		int realStart_y = convertCoordFromBiggerRect(height, distanceSize, start_y);
		int realStart_x = convertCoordFromBiggerRect(width, distanceSize, start_x);
		if (HasFlatSurface(edgeImage, realStart_x, realStart_y, size, size))
		{
		edgeImage.write("edges1.png");
		Image finalImg = Image(cutRectImage);
		LOG(INFO) << realStart_x << " " << realStart_y << " " << size;
		finalImg.crop(Geometry(size, size, realStart_x, realStart_y));
		finalImg.write("back1.png");
		this->hasFirstScan = true;
		}
		else
		{
		SetStatusMessage(L"Please move the TShirt to the left a bit.", 1000, true);
		}

		}

		if (!this->hasSecondScan && this->hasFirstScan)
		{
		start_y = (height - size) / 2;
		start_x = start_x + distanceSize - size;
		int realStart_y = convertCoordFromBiggerRect(height, distanceSize, start_y);
		int realStart_x = convertCoordFromBiggerRect(width, distanceSize, start_x);
		if (HasFlatSurface(edgeImage, realStart_x, realStart_y, size, size))
		{
		edgeImage.write("edges2.png");
		LOG(INFO) << realStart_x << " " << realStart_y << " " << size;
		Image finalImg = Image(cutRectImage);
		finalImg.crop(Geometry(size, size, realStart_x, realStart_y));
		finalImg.write("back2.png");
		this->hasSecondScan = true;
		this->timeSinceSecondScan = getMilliseconds();
		}
		else
		{
		SetStatusMessage(L"Please move the TShirt to the right a bit.", 1000, true);
		}
		}

		if (this->hasFirstScan && this->hasSecondScan)
		{
		//TODO: Communicate with Python CInterface!
		SetStatusMessage(L"Scanned TShirt! Prepare the next one!", 2000, true);
		int newItemID = createNewItemWithTextures("back1.png", "back2.png");
		LOG(INFO) << "Created new Clothing Item with ID: " << newItemID;
		}
		delete[]cutRect;
		}
		catch (Exception &error_)
		{
		LOG(ERROR) << error_.what();
		delete[]cutRect;
		}*/
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

				newColorFrame.Broadcast(colorFrame);
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




