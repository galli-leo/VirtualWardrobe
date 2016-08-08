//------------------------------------------------------------------------------
// 
//     The Kinect for Windows APIs used here are preliminary and subject to change
// 
//------------------------------------------------------------------------------

#include "IKinectV2PluginPCH.h"
#include "ImageUtils.h"
#include "KinectSensor.h"
#include "AllowWindowsPlatformTypes.h"

#define BODY_WAIT_OBJECT WAIT_OBJECT_0
#define COLOR_WAIT_OBJECT WAIT_OBJECT_0 + 1
#define INFRARED_WAIT_OBJECT WAIT_OBJECT_0 + 2
#define DEPTH_WAIT_OBJECT WAIT_OBJECT_0 + 3
#define BODYINDEX_WAIT_OBJECT WAIT_OBJECT_0 + 4
#define POINTER_ENTERED_WAIT_OBJECT WAIT_OBJECT_0 + 5
#define POINTER_EXITED_WAIT_OBJECT WAIT_OBJECT_0 + 6
#define POINTER_MOVED_WAIT_OBJECT WAIT_OBJECT_0 + 7
#define AUDIO_WAIT_OBJECT WAIT_OBJECT_0 + 8

static const DWORD c_FaceFrameFeatures =
FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
| FaceFrameFeatures::FaceFrameFeatures_Happy
| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
| FaceFrameFeatures::FaceFrameFeatures_LookingAway
| FaceFrameFeatures::FaceFrameFeatures_Glasses
| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;

static const uint32 BodyColorLUT[] = {
	0x0000FF00,
	0x00FF0000,
	0xFFFF4000,
	0x40FFFF00,
	0xFF40FF00,
	0xFF808000,
};

/*
FKinectSensor& FKinectSensor::Get(){

static FKinectSensor Kinect;

return Kinect;

}
*/
//#include "AllowWindowsPlatformTypes.h"

static uint32 ThreadNameWorkaround = 0;

uint32 FKinectSensor::Run(){



	while (!bStop) {
		SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_RunTime);

		HRESULT hr;
		HANDLE handles[] = {	//Do not change the order of this array! if you do the switch below will redirect events to the wrong handler!
			reinterpret_cast<HANDLE>(BodyEventHandle),
			reinterpret_cast<HANDLE>(ColorEventHandle),
			reinterpret_cast<HANDLE>(InfraredEventHandle),
			reinterpret_cast<HANDLE>(DepthEventHandle),
			reinterpret_cast<HANDLE>(BodyIndexEventHandle),
			reinterpret_cast<HANDLE>(PointerEnteredEventHandle),
			reinterpret_cast<HANDLE>(PointerExitedEventHandle),
			reinterpret_cast<HANDLE>(PointerMovedEventHandle),
			//reinterpret_cast<HANDLE>(AudioBeamEventHandle)
		};


		switch (WaitForMultipleObjects(!GIsEditor ? _countof(handles) : _countof(handles) - 3, handles, false, 0))
		{
		case BODY_WAIT_OBJECT:
		{
			TComPtr<IBodyFrameArrivedEventArgs> pArgs = nullptr;
			hr = m_pBodyFrameReader->GetFrameArrivedEventData(BodyEventHandle, &pArgs);
			if (SUCCEEDED(hr)){
				ProcessBodyFrame(pArgs);
			}
			pArgs.Reset();
			//SAFE_RELEASE(pArgs);

		}
		break;
		case COLOR_WAIT_OBJECT:
		{
			TComPtr<IColorFrameArrivedEventArgs> pArgs = nullptr;
			hr = m_pColorFrameReader->GetFrameArrivedEventData(ColorEventHandle, &pArgs);
			if (SUCCEEDED(hr)){
				ProcessColorFrame(pArgs);
			}
			pArgs.Reset();
			//SAFE_RELEASE(pArgs);
		}
		break;
		case INFRARED_WAIT_OBJECT:
		{
			TComPtr<IInfraredFrameArrivedEventArgs> pArgs = nullptr;
			hr = m_pInfraredFrameReader->GetFrameArrivedEventData(InfraredEventHandle, &pArgs);
			if (SUCCEEDED(hr)){
				ProcessInfraredFrame(pArgs);
			}
			pArgs.Reset();
			//SAFE_RELEASE(pArgs);
		}
		break;
		case DEPTH_WAIT_OBJECT:
		{
			TComPtr<IDepthFrameArrivedEventArgs> pArgs = nullptr;
			hr = m_pDepthFrameReader->GetFrameArrivedEventData(DepthEventHandle, &pArgs);
			if (SUCCEEDED(hr)){
				ProcessDepthFrame(pArgs);
			}
			pArgs.Reset();
			//SAFE_RELEASE(pArgs);
		}
		break;
		case BODYINDEX_WAIT_OBJECT:
		{

			TComPtr<IBodyIndexFrameArrivedEventArgs> pArgs = nullptr;

			hr = m_pBodyIndexFrameReader->GetFrameArrivedEventData(BodyIndexEventHandle, &pArgs);

			if (SUCCEEDED(hr)){
				ProcessBodyIndex(pArgs);
			}
			pArgs.Reset();
			//SAFE_RELEASE(pArgs);
		}
		break;
		case  POINTER_ENTERED_WAIT_OBJECT:
		{
			TComPtr<IKinectPointerEventArgs> pArgs;
			m_pCoreWindow->GetPointerEnteredEventData(PointerEnteredEventHandle, &pArgs);

			pArgs.Reset();
		}
		break;
		case POINTER_EXITED_WAIT_OBJECT:
		{

		}
		break;
		case POINTER_MOVED_WAIT_OBJECT:
		{
			TComPtr<IKinectPointerEventArgs> pArgs;
			m_pCoreWindow->GetPointerMovedEventData(PointerMovedEventHandle, &pArgs);

			pArgs.Reset();

		}
		break;
		case AUDIO_WAIT_OBJECT:
		{
			TComPtr<IAudioBeamFrameArrivedEventArgs> pArgs = nullptr;

			hr = m_pAudioBeamFrameReader->GetFrameArrivedEventData(AudioBeamEventHandle, &pArgs);

			if (SUCCEEDED(hr)){

			}

			pArgs.Reset();
			//SAFE_RELEASE(pArgs);
		}
		break;
		}
	}

	return 0;
}

#include "HideWindowsPlatformTypes.h"

FKinectSensor::FKinectSensor() : bStop(true),
m_pKinectSensor(nullptr),
m_pBodyFrameReader(nullptr),
m_pColorFrameReader(nullptr),
m_pDepthFrameReader(nullptr),
m_pInfraredFrameReader(nullptr),
m_pColorFrameRGBX(nullptr),
m_pInfraredFrameRGBX(nullptr),
m_pDepthFrameRGBX(nullptr),
m_pBodyIndexFrameRGBX(nullptr),
m_usRawDepthBuffer(nullptr),
pKinectThread(nullptr)

{

	m_pColorFrameRGBX = new RGBQUAD[cColorWidth * cColorHeight];
	m_pDepthFrameRGBX = new RGBQUAD[cInfraredWidth*cInfraredHeight];
	m_pInfraredFrameRGBX = new RGBQUAD[cInfraredWidth*cInfraredHeight];
	m_pBodyIndexFrameRGBX = new RGBQUAD[cInfraredWidth*cInfraredHeight];
	m_usRawDepthBuffer = new uint16[cInfraredWidth*cInfraredHeight];

	DepthSpacePointArray.AddZeroed(cColorWidth*cColorHeight);

}


FKinectSensor:: ~FKinectSensor(){



	//SAFE_RELEASE(m_pKinectSensor);

	DepthSpacePointArray.Empty();

	if (m_pColorFrameRGBX)
	{
		delete[] m_pColorFrameRGBX;
		m_pColorFrameRGBX = nullptr;
	}
	if (m_pInfraredFrameRGBX){
		delete[] m_pInfraredFrameRGBX;
		m_pInfraredFrameRGBX = nullptr;
	}
	if (m_pDepthFrameRGBX){
		delete[] m_pDepthFrameRGBX;
		m_pDepthFrameRGBX = nullptr;
	}

	if (m_pBodyIndexFrameRGBX)
	{
		delete[] m_pBodyIndexFrameRGBX;
		m_pBodyIndexFrameRGBX = nullptr;
	}

	if (m_usRawDepthBuffer)
	{
		delete[] m_usRawDepthBuffer;
		m_usRawDepthBuffer = nullptr;
	}

}

bool FKinectSensor::Init(){


	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return false;
	}


	if (!m_pKinectSensor || FAILED(hr))
	{
		return false;
	}


	hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);

	if (FAILED(hr))
	{
		return false;
	}

	TComPtr<IBodyFrameSource> pBodyFrameSource = nullptr;

	hr = m_pKinectSensor->Open();

	TComPtr<IBodyIndexFrameSource> pBodyIndexFrameSource = nullptr;

	if (SUCCEEDED(hr))
	{

		hr = m_pKinectSensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
	
	}
	if (SUCCEEDED(hr)){

		hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
	}


	pBodyIndexFrameSource.Reset();
	if (SUCCEEDED(hr))
	{
		hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
	}

	if (SUCCEEDED(hr))
	{
		hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
	}


	pBodyFrameSource.Reset();

	TComPtr<IColorFrameSource> pColorFrameSource = nullptr;

	hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);

	if (SUCCEEDED(hr)){
		hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
	}

	pColorFrameSource.Reset();
	//SAFE_RELEASE(pColorFrameSource);

	TComPtr<IInfraredFrameSource> pInfraredInfraredSource = nullptr;

	hr = m_pKinectSensor->get_InfraredFrameSource(&pInfraredInfraredSource);

	if (SUCCEEDED(hr)){

		hr = pInfraredInfraredSource->OpenReader(&m_pInfraredFrameReader);

	}
	pInfraredInfraredSource.Reset();
	//SAFE_RELEASE(pInfraredInfraredSource);

	TComPtr<IDepthFrameSource> pDepthFrameSource = nullptr;

	hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);

	if (SUCCEEDED(hr)){
		hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
	}

	pDepthFrameSource.Reset();
	//SAFE_RELEASE(pDepthFrameSource);


	//SAFE_RELEASE(pBodyIndexFrameSource);

	if (!GIsEditor){

		hr = GetKinectCoreWindowForCurrentThread(&m_pCoreWindow);

		if (SUCCEEDED(hr)){
			m_pCoreWindow->SubscribePointerEntered(&PointerEnteredEventHandle);
			m_pCoreWindow->SubscribePointerExited(&PointerExitedEventHandle);
			m_pCoreWindow->SubscribePointerEntered(&PointerMovedEventHandle);
		}

	}


	if (SUCCEEDED(hr))
	{


		m_pColorFrameReader->SubscribeFrameArrived(&ColorEventHandle);

		m_pBodyFrameReader->SubscribeFrameArrived(&BodyEventHandle);

		m_pInfraredFrameReader->SubscribeFrameArrived(&InfraredEventHandle);

		m_pDepthFrameReader->SubscribeFrameArrived(&DepthEventHandle);

		m_pBodyIndexFrameReader->SubscribeFrameArrived(&BodyIndexEventHandle);

		//m_pAudioBeamFrameReader->SubscribeFrameArrived(&AudioBeamEventHandle);

		bStop = false;

		return true;
	}

	return false;
}

void FKinectSensor::Stop(){

	bStop = true;

}


void FKinectSensor::StartSensor()
{
	if (!pKinectThread)
	{

		pKinectThread = FRunnableThread::Create(this, *(FString::Printf(TEXT("FKinectThread%ul"), ThreadNameWorkaround++)), 0, EThreadPriority::TPri_AboveNormal);

	}
}


void FKinectSensor::ShutDownSensor()
{

	if (pKinectThread) {

		pKinectThread->Kill(true);
		
		delete pKinectThread;

		pKinectThread = nullptr;
	}

}

void FKinectSensor::Exit()
{

	if (m_pBodyFrameReader)
		m_pBodyFrameReader->UnsubscribeFrameArrived(BodyEventHandle);

	m_pBodyFrameReader.Reset();
	//SAFE_RELEASE(m_pBodyFrameReader);

	if (m_pColorFrameReader)
		m_pColorFrameReader->UnsubscribeFrameArrived(ColorEventHandle);

	m_pColorFrameReader.Reset();
	//SAFE_RELEASE(m_pColorFrameReader);

	if (m_pInfraredFrameReader)
		m_pInfraredFrameReader->UnsubscribeFrameArrived(InfraredEventHandle);

	m_pInfraredFrameReader.Reset();
	//SAFE_RELEASE(m_pInfraredFrameReader);

	if (m_pDepthFrameReader)
		m_pDepthFrameReader->UnsubscribeFrameArrived(DepthEventHandle);

	m_pDepthFrameReader.Reset();
	//SAFE_RELEASE(m_pDepthFrameReader);

	if (m_pBodyIndexFrameReader)
		m_pBodyIndexFrameReader->UnsubscribeFrameArrived(BodyIndexEventHandle);

	m_pBodyIndexFrameReader.Reset();
	//SAFE_RELEASE(m_pBodyIndexFrameReader);

	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	m_pKinectSensor.Reset();

}

bool FKinectSensor::GetBodyFrame(FBodyFrame& OutBodyFrame){

	FScopeLock lock(&mBodyCriticalSection);
	if (m_bNewBodyFrame)
	{
		OutBodyFrame = BodyFrame;
		m_bNewBodyFrame = false;
		return true;
	}

	return false;

}




/**********************************************************************************************//**
 * Process the body frame described by pArgs.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pArgs	If non-null, the arguments.
 **************************************************************************************************/
#ifdef DEBUG
#pragma optimize("",off)
#endif // DEBUG

void FKinectSensor::ProcessBodyFrame(IBodyFrameArrivedEventArgs*pArgs)
{




	// data fetching loop
	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_BodyProcessTime);

	IBodyFrameReference* pBodyFrameReferance = nullptr;

	HRESULT hr = pArgs->get_FrameReference(&pBodyFrameReferance);

	if (SUCCEEDED(hr)){
		bool processFrame = false;
		IBodyFrame* pBodyFrame = nullptr;
		if (SUCCEEDED(pBodyFrameReferance->AcquireFrame(&pBodyFrame))){
			INT64 nTime = 0;

			hr = pBodyFrame->get_RelativeTime(&nTime);

			IBody* ppBodies[BODY_COUNT] = { 0 };

			if (SUCCEEDED(hr))
			{

				hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
			}

			if (SUCCEEDED(hr))
			{

				Vector4 floorPlane;
				pBodyFrame->get_FloorClipPlane(&floorPlane);
				{
					FScopeLock lock(&mBodyCriticalSection);

					BodyFrame = FBodyFrame(ppBodies, floorPlane);
					m_bNewBodyFrame = true;

				}

				for (int i = 0; i < _countof(ppBodies); ++i)
				{

					SAFE_RELEASE(ppBodies[i]);
				}
			}
			SAFE_RELEASE(pBodyFrame);

		}
		SAFE_RELEASE(pBodyFrameReferance);
	}
}
#ifdef DEBUG
#pragma optimize("",on)
#endif // DEBUG

/**********************************************************************************************//**
 * Process the color frame described by pArgs.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pArgs	If non-null, the arguments.
 **************************************************************************************************/

void FKinectSensor::ProcessColorFrame(IColorFrameArrivedEventArgs*pArgs)
{
	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_ColorProcessTime);

	TComPtr<IColorFrameReference> pColorFrameReferance = nullptr;

	HRESULT hr = pArgs->get_FrameReference(&pColorFrameReferance);

	if (SUCCEEDED(hr)){
		TComPtr<IColorFrame> pColorFrame = nullptr;
		if (SUCCEEDED(pColorFrameReferance->AcquireFrame(&pColorFrame))){
			RGBQUAD *pColorBuffer = NULL;
			pColorBuffer = m_pColorFrameRGBX;
			uint32 nColorBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
			{
				FScopeLock lock(&mColorCriticalSection);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
				m_bNewColorFrame = true;
			}
		}
		pColorFrame.Reset();

	}
	pColorFrameReferance.Reset();

}

/**********************************************************************************************//**
 * Process the infrared frame described by pArgs.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pArgs	If non-null, the arguments.
 **************************************************************************************************/

void FKinectSensor::ProcessInfraredFrame(IInfraredFrameArrivedEventArgs*pArgs)
{

	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_InfraredProcessTime);

	TComPtr<IInfraredFrameReference> pInfraredFrameReferance = nullptr;

	HRESULT hr = pArgs->get_FrameReference(&pInfraredFrameReferance);

	if (SUCCEEDED(hr)){
		
		TComPtr<IInfraredFrame> pInfraredFrame = nullptr;

		if (SUCCEEDED(pInfraredFrameReferance->AcquireFrame(&pInfraredFrame))){
			const uint32 nInfraredBufferSize = cInfraredWidth* cInfraredHeight;
			uint16 pInfraredBuffer[nInfraredBufferSize];
			//pInfraredBuffer = m_pColorFrameRGBX;
			{
				if (SUCCEEDED(pInfraredFrame->CopyFrameDataToArray(nInfraredBufferSize, pInfraredBuffer))){
					FScopeLock lock(&mInfraredCriticalSection);

					ConvertInfraredData(pInfraredBuffer, m_pInfraredFrameRGBX);

					m_bNewInfraredFrame = true;
				}

				//hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
			}
		}
		pInfraredFrame.Reset();

	}
	pInfraredFrameReferance.Reset();
}

/**********************************************************************************************//**
 * Process the depth frame described by pArgs.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pArgs	If non-null, the arguments.
 **************************************************************************************************/

void FKinectSensor::ProcessDepthFrame(IDepthFrameArrivedEventArgs*pArgs)
{

	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_DepthProcessTime);

	TComPtr<IDepthFrameReference> pDepthFrameReferance = nullptr;

	HRESULT hr = pArgs->get_FrameReference(&pDepthFrameReferance);

	if (SUCCEEDED(hr)){

		TComPtr<IDepthFrame> pDepthFrame = nullptr;

		if (SUCCEEDED(pDepthFrameReferance->AcquireFrame(&pDepthFrame))){

			USHORT nDepthMinReliableDistance = 0;
			USHORT nDepthMaxReliableDistance = 0;
			const uint32 nBufferSize = cInfraredWidth*cInfraredHeight;

			if (SUCCEEDED(pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance)) &&
				SUCCEEDED(pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance))
				){

				FScopeLock lock(&mDepthCriticalSection);
				if (SUCCEEDED(pDepthFrame->CopyFrameDataToArray(nBufferSize, m_usRawDepthBuffer))){
					ConvertDepthData(m_usRawDepthBuffer, m_pDepthFrameRGBX, nDepthMinReliableDistance, nDepthMaxReliableDistance);

					m_bNewDepthFrame = true;
				}
			}

		}


		pDepthFrame.Reset();
	}

	pDepthFrameReferance.Reset();
}

/**********************************************************************************************//**
* Updates the color texture described by pTexture.
*
* @author	Leon Rosengarten
* @param [in,out]	pTexture	If non-null, the texture.
**************************************************************************************************/

void FKinectSensor::UpdateColorTexture(UTexture2D*pTexture)
{
	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_ColorUpdateTime);
	
	if (m_bNewColorFrame)
	{
		FScopeLock lock(&mColorCriticalSection);
		UpdateTexture(pTexture, m_pColorFrameRGBX, cColorWidth, cColorHeight);
		m_bNewColorFrame = false;
	}

	return;
}

/**********************************************************************************************//**
 * Updates the infrared texture described by pTexture.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pTexture	If non-null, the texture.
 **************************************************************************************************/

void FKinectSensor::UpdateInfraredTexture(UTexture2D*pTexture)
{
	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_InfraredUpdateTime);
	
	if (m_bNewInfraredFrame)
	{
		FScopeLock lock(&mInfraredCriticalSection);
		UpdateTexture(pTexture, m_pInfraredFrameRGBX, cInfraredWidth, cInfraredHeight);
		m_bNewInfraredFrame = false;
	}

	return;
}
/**********************************************************************************************//**
 * Updates the depth frame texture described by pTexture.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pTexture If non-null, the texture.
 *
 **************************************************************************************************/

void FKinectSensor::UpdateDepthFrameTexture(UTexture2D*pTexture)
{
	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_DepthUpdateTime);
	
	if (m_bNewDepthFrame)
	{
		FScopeLock lock(&mDepthCriticalSection);
		UpdateTexture(pTexture, m_pDepthFrameRGBX, cInfraredWidth, cInfraredHeight);
		m_bNewDepthFrame = false;
	}
}




/**********************************************************************************************//**
 * Convert infrared data.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pInfraredBuffer	If non-null, buffer for infrared data.
 * @param [in,out]	pInfraredRGBX  	If non-null, the infrared rgbx.
 **************************************************************************************************/

void FKinectSensor::ConvertInfraredData(uint16*pInfraredBuffer, RGBQUAD*pInfraredRGBX)
{
	if (pInfraredRGBX && pInfraredBuffer)
	{
		RGBQUAD* pRGBX = pInfraredRGBX;

		// end pixel is start + width*height - 1
		const uint16* pBufferEnd = pInfraredBuffer + (cInfraredWidth * cInfraredHeight);

		while (pInfraredBuffer < pBufferEnd)
		{
			USHORT ir = *pInfraredBuffer;

			// To convert to a byte, we're discarding the least-significant bits.
			BYTE intensity = static_cast<BYTE>(ir >> 8);

			pRGBX->rgbRed = intensity;
			pRGBX->rgbGreen = intensity;
			pRGBX->rgbBlue = intensity;

			++pRGBX;
			++pInfraredBuffer;
		}
	}
}


/**********************************************************************************************//**
 * Updates the texture.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pTexture	If non-null, the texture.
 * @param	pData				The data.
 * @param	SizeX				The size x coordinate.
 * @param	SizeY				The size y coordinate.
 **************************************************************************************************/

void FKinectSensor::UpdateTexture(UTexture2D*pTexture, const RGBQUAD*pData, uint32 SizeX, uint32 SizeY)
{

	if (pTexture && pData){

		UTexture2D* Texture = pTexture;

		const size_t Size = SizeX * SizeY* sizeof(RGBQUAD);

		uint8* Src = (uint8*)pData;

		uint8* Dest = (uint8*)Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

		FMemory::Memcpy(Dest, Src, Size);

		Texture->PlatformData->Mips[0].BulkData.Unlock();

		Texture->UpdateResource();

	}

}


void FKinectSensor::MapColorFrameToDepthSpace(TArray<FVector2D> &DepthSpacePoints)
{
	if (DepthSpacePoints.Num() == cColorWidth*cColorHeight)
	{
		FScopeLock Lock(&mDepthCriticalSection);

		DepthSpacePoints = DepthSpacePointArray;

	}
}

/**********************************************************************************************//**
 * Convert depth data.
 *
 * @author	Leon Rosengarten
 * @param [in,out]	pDepthBuffer	If non-null, buffer for depth data.
 * @param [in,out]	pDepthRGBX  	If non-null, the depth rgbx.
 * @param	minDepth				The minimum depth.
 * @param	maxDepth				The maximum depth.
 **************************************************************************************************/

void FKinectSensor::ConvertDepthData(uint16*pDepthBuffer, RGBQUAD*pDepthRGBX, USHORT minDepth, USHORT maxDepth)
{
	if (pDepthRGBX && pDepthBuffer)
	{


		m_pCoordinateMapper->MapColorFrameToDepthSpace(cInfraredWidth*cInfraredHeight, m_usRawDepthBuffer,
													   cColorWidth*cColorHeight,
													   (DepthSpacePoint*)&DepthSpacePointArray[0]);

		RGBQUAD* pRGBX = pDepthRGBX;

		// end pixel is start + width*height - 1
		const UINT16* pBufferEnd = pDepthBuffer + (cInfraredWidth * cInfraredHeight);

		while (pDepthBuffer < pBufferEnd)
		{
			USHORT depth = *pDepthBuffer;

			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
			BYTE intensity = static_cast<BYTE>((depth >= minDepth) && (depth <= maxDepth) ? (depth % 256) : 0);

			pRGBX->rgbRed = intensity;
			pRGBX->rgbGreen = intensity;
			pRGBX->rgbBlue = intensity;

			++pRGBX;
			++pDepthBuffer;
		}
	}
}

FVector2D FKinectSensor::BodyToScreen(const FVector& bodyPoint, int32 width, int32 height)
{
	// Calculate the body's position on the screen
	DepthSpacePoint depthPoint = { 0 };

	CameraSpacePoint tempBodyPoint;

	tempBodyPoint.X = bodyPoint.X;
	tempBodyPoint.Y = bodyPoint.Y;
	tempBodyPoint.Z = bodyPoint.Z;

	m_pCoordinateMapper->MapCameraPointToDepthSpace(tempBodyPoint, &depthPoint);

	float screenPointX = static_cast<float>(depthPoint.X * width) / cInfraredWidth;
	float screenPointY = static_cast<float>(depthPoint.Y * height) / cInfraredHeight;

	return FVector2D(screenPointX, screenPointY);
}


bool FKinectSensor::IsRunning()
{
	return !bStop;
}

void FKinectSensor::ProcessBodyIndex(IBodyIndexFrameArrivedEventArgs* pArgs)
{
	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_BodyIndexProcessTime);
	const uint32 nBufferSize = cInfraredWidth*cInfraredHeight;
	TComPtr<IBodyIndexFrameReference> pBodyIndexFrameReference = nullptr;

	HRESULT hr = pArgs->get_FrameReference(&pBodyIndexFrameReference);

	if (SUCCEEDED(hr))
	{
		TComPtr<IBodyIndexFrame> pBodyIndexFrame = nullptr;
		hr = pBodyIndexFrameReference->AcquireFrame(&pBodyIndexFrame);
		if (SUCCEEDED(hr))
		{
			FScopeLock scopeLock(&mBodyIndexCriticalSection);
			TArray<uint8> BodyIndexBuffer;
			BodyIndexBuffer.AddZeroed(nBufferSize);

			pBodyIndexFrame->CopyFrameDataToArray(nBufferSize, &BodyIndexBuffer[0]);
			ConvertBodyIndexData(BodyIndexBuffer, m_pBodyIndexFrameRGBX);

			m_bNewBodyIndexFrame = true;

		}
		pBodyIndexFrame.Reset();

	}

	pBodyIndexFrameReference.Reset();

}

void FKinectSensor::ConvertBodyIndexData(const TArray<uint8> BodyIndexBufferData, RGBQUAD* pBodyIndexRGBX)
{
	if (BodyIndexBufferData.Num() > 0 && pBodyIndexRGBX)
	{

		RGBQUAD* pRGBX = pBodyIndexRGBX;

		for (size_t i = 0; i < BodyIndexBufferData.Num(); i++)
		{

			if (BodyIndexBufferData[i] < 6)
			{

				uint32 LUTColor = BodyColorLUT[BodyIndexBufferData[i]];
				pRGBX->rgbBlue = (LUTColor & 0x0000ff00) >> 8;
				pRGBX->rgbGreen = (LUTColor & 0x00ff0000) >> 16;
				pRGBX->rgbRed = (LUTColor & 0xff000000) >> 24;
			
			}
			else
			{
				pRGBX->rgbBlue = pRGBX->rgbGreen = pRGBX->rgbRed = 0x00;
			}

			++pRGBX;
		}
	}
}


void FKinectSensor::UpdateBodyIndexTexture(UTexture2D* pTexture)
{
	SCOPE_CYCLE_COUNTER(STAT_KINECT_SENSOR_BodyIndexUpdateTime);
	
	if (m_bNewBodyIndexFrame)
	{
		FScopeLock lock(&mDepthCriticalSection);
		UpdateTexture(pTexture, m_pBodyIndexFrameRGBX, 512, 424);
		m_bNewBodyIndexFrame = false; 
	}
}
