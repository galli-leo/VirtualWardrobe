//------------------------------------------------------------------------------
// <copyright file="ColorBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include <io.h>
#include "Fcntl.h"
#include "WardrobeManager.h"
#include <chrono>
#include <Magick++.h>
#include "easylogging++.h"
#include <Python.h>
#include "WardrobeUtils.h"
#include <direct.h>

using namespace Magick;

INITIALIZE_EASYLOGGINGPP



/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(    
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
    )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    WardrobeManager application;
    application.Run(hInstance, nShowCmd);
}



/// <summary>
/// Gets the average distance for a defined rect in the color image. Spots without a distance are not integrated into the calculation to minimize measuring errors.
/// </summary>
long GetAverageDistanceForRect(RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight, DepthSpacePoint* pDepthPoints, UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight, int start_x, int start_y, int width, int height)
{
	long startTime = getMilliseconds();
	long sum = 0;
	for (int x = start_x; x < start_x + width; x++)
	{
		for (int y = start_y; y < start_y + height; y++)
		{
			int colorIndex = x + y*nColorWidth;
			DepthSpacePoint p = pDepthPoints[colorIndex];
			if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
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
	long endTime = getMilliseconds();
	if (endTime - startTime > 25)
	{
		LOG(WARNING) << "It took over 25 ms to execute CutRectFromBuffer!: " << endTime-startTime;
	}
	return float(sum) / (width*height);
}


/// <summary>
/// Removes all pixels not within rangeMin and rangeMax or any pixels without depth information.
/// </summary>
void FilterOnlyObjectsInDistance(RGBQUAD* pBuffer, int width, int height, DepthSpacePoint* pDepthPoints, UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight, int rangeMin, int rangeMax)
{
	for (int colorIndex = 0; colorIndex < (width*height); ++colorIndex)
	{
		DepthSpacePoint p = pDepthPoints[colorIndex];
		if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
		{
			int depthX = static_cast<int>(p.X + 0.5f);
			int depthY = static_cast<int>(p.Y + 0.5f);

			if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
			{
				long depthValue = pDepthBuffer[depthX + (depthY * nDepthWidth)];
				if (!(depthValue > rangeMin && depthValue < rangeMax))
				{
					pBuffer[colorIndex].rgbBlue = 0;
					pBuffer[colorIndex].rgbGreen = 0;
					pBuffer[colorIndex].rgbRed = 0;
				}
			}
			else
			{
				pBuffer[colorIndex].rgbBlue = 0;
				pBuffer[colorIndex].rgbGreen = 0;
				pBuffer[colorIndex].rgbRed = 0;
			}
		}
		else
		{
			pBuffer[colorIndex].rgbBlue = 0;
			pBuffer[colorIndex].rgbGreen = 0;
			pBuffer[colorIndex].rgbRed = 0;
		}
	}
}

RGBQUAD* CutRectFromBuffer(RGBQUAD* pBuffer, int colorWidth, int colorHeight, int start_x, int start_y, int width, int height)
{
	long startTime = getMilliseconds();
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
	long endTime = getMilliseconds();
	if (endTime - startTime > 25)
	{
		LOG(WARNING) << "It took over 25 ms to execute CutRectFromBuffer!: " << endTime - startTime;
	}
	return finalBuffer;
}

int convertCoordFromBiggerRect(int biggerSize, int smallerSize, int coord)
{
	return coord - (biggerSize - smallerSize) / 2;
}

Image WardrobeManager::CreateMagickImageFromBuffer(RGBQUAD* pBuffer, int width, int height)
{
	//Blob blob = Blob(pBuffer, 1920*1080/sizeof(RGBQUAD)*2);
	size_t rgbquad_size = sizeof(RGBQUAD);
	size_t total_bytes = width * height * rgbquad_size / 8;
	/*uint8_t * pCopyBuffer = new uint8_t(total_bytes);
	for (size_t cursor = 0; cursor < total_bytes; ++cursor) {
		if (cursor % rgbquad_size < rgbquad_size - 1) {
			pCopyBuffer[cursor] = (uint8_t)(pBuffer[cursor]);
		}
		else {
			pCopyBuffer[cursor] = 0xFF;
		}
	}*/

	for (size_t i = 0; i < width*height; i++)
	{
		RGBQUAD q = pBuffer[i];
		q.rgbReserved = 255;
	}

	Blob blob = Blob(pBuffer, total_bytes);
	Image img;
	img.size("1920x1080");

	img.magick("RGBA");
	img.depth(8);
	img.read(blob);
	return img;
	TIMED_FUNC();
	HRESULT hr = SaveBitmapToFile(reinterpret_cast<BYTE*>(pBuffer), width, height, sizeof(RGBQUAD) * 8, L"tmp.bmp");

	if (SUCCEEDED(hr))
	{
		try {
			img.read("tmp.bmp");
		}
		catch (Exception &error_)
		{
			printf(error_.what());
		}
	}

	return img;
}
/// <summary>
/// Constructor
/// </summary>
WardrobeManager::WardrobeManager() :
    m_hWnd(NULL),
    m_nStartTime(0),
    m_nLastCounter(0),
    m_nFramesSinceUpdate(0),
    m_fFreq(0),
    m_nNextStatusTime(0LL),
    m_bSaveScreenshot(false),
    m_pKinectSensor(NULL),
    m_pColorFrameReader(NULL),
	m_pMultiSourceFrameReader(NULL),
	m_pDepthCoordinates(NULL),
    m_pD2DFactory(NULL),
    m_pDrawColor(NULL),
    m_pColorRGBX(NULL)
{
    LARGE_INTEGER qpf = {0};
    if (QueryPerformanceFrequency(&qpf))
    {
        m_fFreq = double(qpf.QuadPart);
    }

	SetStdOutToNewConsole();

	//HARDCODED!!
	InitializeMagick("C:\Program Files\ImageMagick-7.0.2-Q16");

	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);

	Py_Initialize();

	char * dir = "\\PythonProgram\\";
	char cCurrentPath[FILENAME_MAX + sizeof(dir)];
	_getcwd(cCurrentPath, sizeof(cCurrentPath));
	strcat(cCurrentPath, dir);
	LOG(INFO) << "Python Program path: " << cCurrentPath;

	PyObject *sysPath = PySys_GetObject("path");
	PyObject *path = PyString_FromString(cCurrentPath);
	int result = PyList_Insert(sysPath, 0, path);
	PySys_SetObject("path", sysPath);
	LOG(INFO) << "List insert result: " << result << convertPyListToString(sysPath);

    // create heap storage for color pixel data in RGBX format
    m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	// create heap storage for the coorinate mapping from color to depth
	m_pDepthCoordinates = new DepthSpacePoint[cColorWidth * cColorHeight];
}
  

/// <summary>
/// Destructor
/// </summary>
WardrobeManager::~WardrobeManager()
{
    // clean up Direct2D renderer
    if (m_pDrawColor)
    {
        delete m_pDrawColor;
        m_pDrawColor = NULL;
    }

    if (m_pColorRGBX)
    {
        delete [] m_pColorRGBX;
        m_pColorRGBX = NULL;
    }

	if (m_pDepthCoordinates)
	{
		delete[] m_pDepthCoordinates;
		m_pDepthCoordinates = NULL;
	}

    // clean up Direct2D
    SafeRelease(m_pD2DFactory);

    // done with color frame reader
    SafeRelease(m_pColorFrameReader);
	SafeRelease(m_pMultiSourceFrameReader);

    // close the Kinect Sensor
    if (m_pKinectSensor)
    {
        m_pKinectSensor->Close();
    }

    SafeRelease(m_pKinectSensor);

	Py_Finalize();
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int WardrobeManager::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG       msg = {0};
    WNDCLASS  wc;

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = DefDlgProcW;
    wc.lpszClassName = L"ColorBasicsAppDlgWndClass";

    if (!RegisterClassW(&wc))
    {
        return 0;
    }

	

    // Create main application window
    HWND hWndApp = CreateDialogParamW(
        NULL,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC)WardrobeManager::MessageRouter, 
        reinterpret_cast<LPARAM>(this));

    // Show window
    ShowWindow(hWndApp, nCmdShow);

    // Main message loop
    while (WM_QUIT != msg.message)
    {
        Update();

		if (WM_KEYDOWN == msg.message){
			this->m_bSaveScreenshot = true;
		}

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If a dialog message will be taken care of by the dialog proc
            if (hWndApp && IsDialogMessageW(hWndApp, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

void WardrobeManager::CreateRectangleOnScreen(RGBQUAD* pBuffer, int width, int height, int thickness=5, int size=556, int red=0, int green=255, int blue=0)
{
	int start_x = (width - size) / 2;
	int start_y = (height - size) / 2;
	for (int x = start_x; x < start_x + size; x++)
	{
		for (int real_y = start_y; real_y > (start_y - thickness); real_y--)
		{
			int array_pos = x + width*real_y;
			pBuffer[array_pos].rgbGreen = green;
			pBuffer[array_pos].rgbBlue = blue;
			pBuffer[array_pos].rgbRed = red;
		}

		for (int real_y = start_y + size - 1; real_y < (start_y + size - 1 + thickness); real_y++)
		{
			int array_pos = x + width*real_y;
			pBuffer[array_pos].rgbGreen = green;
			pBuffer[array_pos].rgbBlue = blue;
			pBuffer[array_pos].rgbRed = red;
		}
	}

	for (int y = start_y; y < start_y + size; y++)
	{
		for (int x = start_x; x > start_x - thickness; x--)
		{
			int array_pos = x + width*y;
			pBuffer[array_pos].rgbGreen = green;
			pBuffer[array_pos].rgbBlue = blue;
			pBuffer[array_pos].rgbRed = red;
		}

		for (int x = start_x + size; x < start_x + size - 1 + thickness; x++)
		{
			int array_pos = x + width*y;
			pBuffer[array_pos].rgbGreen = green;
			pBuffer[array_pos].rgbBlue = blue;
			pBuffer[array_pos].rgbRed = red;
		}
	}
}

bool WardrobeManager::HasFlatSurface(Image edgeImage, int start_x, int start_y, int width, int height)
{
	int maxEdgePixels = 2;
	int currentEdgePixels = 0;

	try {
		Quantum* quant = edgeImage.getPixels(start_x, start_y, width, height);
		for (int y = start_y; y < height+start_y; y++)
		{
			for (int x = start_x; x < width+start_x; x++)
			{
				Quantum realQuant = quant[x + y*width];
				if (realQuant >= 65535)
				{
					//currentEdgePixels += 1;
					int count = 0;
					for (int newY = y - 10; newY < y + 10; newY++)
					{
						if (newY > start_y, newY < start_y+height)
						{
							Quantum q = quant[x + newY*width];
							if (q >=65535)
							{
								count += 1;
							}
						}
					}

					for (int newX = x - 10; newX < x + 10; newX++)
					{
						if (newX > start_x, newX < start_x + width)
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

		LOG(INFO) << "Current Edge Pixels: " << currentEdgePixels;

		if (currentEdgePixels <= maxEdgePixels)
		{
			SetStatusMessage(L"Found TShirt!", 1000, true);
			return true;
		}

	}
	catch (Exception &error_)
	{
		LOG(ERROR) << error_.what();
	}

	return false;
}

void WardrobeManager::ScanForTshirt(RGBQUAD* pBuffer, int width, int height, UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight)
{
	SetStatusMessage(L"Scanning for TShirt...", 500, false);
	int rangeMin = 700;
	int rangeMax = 900;

	HRESULT hr = m_pCoordinateMapper->MapColorFrameToDepthSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, width * height, m_pDepthCoordinates);
	if (SUCCEEDED(hr))
	{
		//FilterOnlyObjectsInDistance(pBuffer, width, height, m_pDepthCoordinates, pDepthBuffer, nDepthWidth, nDepthHeight, rangeMin, rangeMax);

		int distanceSize = 556;
		int start_x = (width - distanceSize) / 2;
		int start_y = (height - distanceSize) / 2;
		long averageValue = GetAverageDistanceForRect(pBuffer, width, height, m_pDepthCoordinates, pDepthBuffer, nDepthWidth, nDepthHeight, start_x, start_y, distanceSize, distanceSize);

		if (averageValue > rangeMin && averageValue < rangeMax)
		{
			LOG(INFO) << ("Found good distance in center square!");
		}
		else
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
		}

		int size = 512;
		//Use old start_x and y. start_x and y are relative to the full color image (1920x1080).
		//Therefore they need to be converted.
		RGBQUAD* cutRect = CutRectFromBuffer(pBuffer, width, height, start_x, start_y, distanceSize, distanceSize);
		Image cutRectImage = CreateMagickImageFromBuffer(cutRect, distanceSize, distanceSize);
		Image edgeImage = Image(cutRectImage);
		try{
			edgeImage.edge();
			edgeImage.write("testing.png");

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
		}

		
	}

	
	//printf("Average Distance in center 512x512: %f", averageValue);

}

/// <summary>
/// Main processing function
/// </summary>
void WardrobeManager::Update()
{
    if (!m_pMultiSourceFrameReader)
    {
        return;
    }

	IMultiSourceFrame* pMultiSourceFrame = NULL;
    IColorFrame* pColorFrame = NULL;
	IDepthFrame* pDepthFrame = NULL;
	IBodyIndexFrame* pBodyIndexFrame = NULL;

    HRESULT hr = m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);

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
		UINT16 *pDepthBuffer = NULL;

		IFrameDescription* pColorFrameDescription = NULL;
		int nColorWidth = 0;
		int nColorHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nColorBufferSize = 0;
		RGBQUAD *pColorBuffer = NULL;

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
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
			}
			else if (m_pColorRGBX)
			{
				pColorBuffer = m_pColorRGBX;
				nColorBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
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
			ProcessFrame(nDepthTime, pDepthBuffer, nDepthWidth, nDepthHeight,
				pColorBuffer, nColorWidth, nColorHeight,
				pBodyIndexBuffer, nBodyIndexWidth, nBodyIndexHeight);
        }

		SafeRelease(pDepthFrameDescription);
		SafeRelease(pColorFrameDescription);
		SafeRelease(pBodyIndexFrameDescription);
    }

	SafeRelease(pDepthFrame);
	SafeRelease(pColorFrame);
	SafeRelease(pBodyIndexFrame);
	SafeRelease(pMultiSourceFrame);
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK WardrobeManager::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WardrobeManager* pThis = NULL;
    
    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<WardrobeManager*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<WardrobeManager*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK WardrobeManager::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Bind application window handle
            m_hWnd = hWnd;

            // Init Direct2D
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

            // Create and initialize a new Direct2D image renderer (take a look at ImageRenderer.h)
            // We'll use this to draw the data we receive from the Kinect to the screen
            m_pDrawColor = new ImageRenderer();
            HRESULT hr = m_pDrawColor->Initialize(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), m_pD2DFactory, cColorWidth, cColorHeight, cColorWidth * sizeof(RGBQUAD)); 
            if (FAILED(hr))
            {
                SetStatusMessage(L"Failed to initialize the Direct2D draw device.", 10000, true);
            }

            // Get and initialize the default Kinect sensor
            InitializeDefaultSensor();
        }
        break;

        // If the titlebar X is clicked, destroy app
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            // Quit the main message pump
            PostQuitMessage(0);
            break;

        // Handle button press
        case WM_COMMAND:
            // If it was for the screenshot control and a button clicked event, save a screenshot next frame 
            if (IDC_BUTTON_SCREENSHOT == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
            {
                m_bSaveScreenshot = true;
            }
            break;
    }

    return FALSE;
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT WardrobeManager::InitializeDefaultSensor()
{
    HRESULT hr;

    hr = GetDefaultKinectSensor(&m_pKinectSensor);
    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pKinectSensor)
    {
        // Initialize the Kinect and get the color reader
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

        hr = m_pKinectSensor->Open();

        if (SUCCEEDED(hr))
        {
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_BodyIndex,
				&m_pMultiSourceFrameReader);
        }
    }

    if (!m_pKinectSensor || FAILED(hr))
    {
        SetStatusMessage(L"No ready Kinect found!", 10000, true);
        return E_FAIL;
    }

    return hr;
}

/// <summary>
/// Handle new frame data
/// <param name="nTime">timestamp of frame</param>
/// <param name="pBuffer">pointer to frame data</param>
/// <param name="nWidth">width (in pixels) of input image data</param>
/// <param name="nHeight">height (in pixels) of input image data</param>
/// </summary>
void WardrobeManager::ProcessFrame(INT64 nTime,
	UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight,
	RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight,
	BYTE* pBodyIndexBuffer, int nBodyIndexWidth, int nBodyIndexHeight)
{
    if (m_hWnd)
    {
        if (!m_nStartTime)
        {
            m_nStartTime = nTime;
        }

        double fps = 0.0;

        LARGE_INTEGER qpcNow = {0};
        if (m_fFreq)
        {
            if (QueryPerformanceCounter(&qpcNow))
            {
                if (m_nLastCounter)
                {
                    m_nFramesSinceUpdate++;
                    fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
                }
            }
        }

        WCHAR szStatusMessage[64];
        StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    Time = %I64d", fps, (nTime - m_nStartTime));

        if (SetStatusMessage(szStatusMessage, 1000, false))
        {
            m_nLastCounter = qpcNow.QuadPart;
            m_nFramesSinceUpdate = 0;
        }
    }

    // Make sure we've received valid data
    if (pColorBuffer && (nColorWidth == cColorWidth) && (nColorHeight == cColorHeight))
	{
		long currentTime = getMilliseconds();
		long secondsDiff = (currentTime - lastScan) / 1000;

		if (this->lastScan == -1 || secondsDiff > this->scanInterval)
		{
			this->lastScan = currentTime;
			//Did 2 seconds pass since the last scan or do we not have a scan at all?
			if ((this->hasSecondScan && (currentTime - this->timeSinceSecondScan > 4000)))
			{
				this->hasFirstScan = false;
				this->hasSecondScan = false;
				this->ScanForTshirt(pColorBuffer, cColorWidth, cColorHeight, pDepthBuffer, nDepthWidth, nDepthHeight);
			}
			else if (!(this->hasFirstScan && this->hasSecondScan))
			{
				this->ScanForTshirt(pColorBuffer, cColorWidth, cColorHeight, pDepthBuffer, nDepthWidth, nDepthHeight);
			}
			
		}
		

		//Create Rectangle where optimal tshirt placement is
		this->CreateRectangleOnScreen(pColorBuffer, cColorWidth, cColorHeight, 5);

		int size = 512;
		int start_x = (cColorWidth - size) / 2;
		int start_y = (cColorHeight - size) / 2;
		RGBQUAD* cutRect = CutRectFromBuffer(pColorBuffer, cColorWidth, cColorHeight, start_x, start_y, size, size);
		// Draw the data with Direct2D
		m_pDrawColor->Draw(reinterpret_cast<BYTE*>(pColorBuffer), cColorWidth * cColorHeight * sizeof(RGBQUAD));
		//m_pDrawColor->Draw(reinterpret_cast<BYTE*>(cutRect), size * size * sizeof(RGBQUAD));

        if (m_bSaveScreenshot)
        {
            WCHAR szScreenshotPath[MAX_PATH];

            // Retrieve the path to My Photos
            GetScreenshotFileName(szScreenshotPath, _countof(szScreenshotPath));

            // Write out the bitmap to disk
            HRESULT hr = SaveBitmapToFile(reinterpret_cast<BYTE*>(pColorBuffer), nColorWidth, nColorHeight, sizeof(RGBQUAD) * 8, szScreenshotPath);

            WCHAR szStatusMessage[64 + MAX_PATH];
            if (SUCCEEDED(hr))
            {
                // Set the status bar to show where the screenshot was saved
                StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L"Screenshot saved to %s", szScreenshotPath);
            }
            else
            {
                StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L"Failed to write screenshot to %s", szScreenshotPath);
            }

            SetStatusMessage(szStatusMessage, 5000, true);

            // toggle off so we don't save a screenshot again next frame
            m_bSaveScreenshot = false;
        }
    }
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
/// <param name="showTimeMsec">time in milliseconds to ignore future status messages</param>
/// <param name="bForce">force status update</param>
bool WardrobeManager::SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce)
{
    INT64 now = GetTickCount64();

    if (m_hWnd && (bForce || (m_nNextStatusTime <= now)))
    {
        SetDlgItemText(m_hWnd, IDC_STATUS, szMessage);
		//LOG(INFO) << "Status update: " << szMessage;
        m_nNextStatusTime = now + nShowTimeMsec;

        return true;
    }

    return false;
}

/// <summary>
/// Get the name of the file where screenshot will be stored.
/// </summary>
/// <param name="lpszFilePath">string buffer that will receive screenshot file name.</param>
/// <param name="nFilePathSize">number of characters in lpszFilePath string buffer.</param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT WardrobeManager::GetScreenshotFileName(_Out_writes_z_(nFilePathSize) LPWSTR lpszFilePath, UINT nFilePathSize)
{
    WCHAR* pszKnownPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &pszKnownPath);

    if (SUCCEEDED(hr))
    {
        // Get the time
        WCHAR szTimeString[MAX_PATH];
        GetTimeFormatEx(NULL, 0, NULL, L"hh'-'mm'-'ss", szTimeString, _countof(szTimeString));

        // File name will be KinectScreenshotColor-HH-MM-SS.bmp
        StringCchPrintfW(lpszFilePath, nFilePathSize, L"%s\\KinectScreenshot-Color-%s.bmp", pszKnownPath, szTimeString);
    }

    if (pszKnownPath)
    {
        CoTaskMemFree(pszKnownPath);
    }

    return hr;
}

/// <summary>
/// Save passed in image data to disk as a bitmap
/// </summary>
/// <param name="pBitmapBits">image data to save</param>
/// <param name="lWidth">width (in pixels) of input image data</param>
/// <param name="lHeight">height (in pixels) of input image data</param>
/// <param name="wBitsPerPixel">bits per pixel of image data</param>
/// <param name="lpszFilePath">full file path to output bitmap to</param>
/// <returns>indicates success or failure</returns>
HRESULT WardrobeManager::SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath)
{
    DWORD dwByteCount = lWidth * lHeight * (wBitsPerPixel / 8);

    BITMAPINFOHEADER bmpInfoHeader = {0};

    bmpInfoHeader.biSize        = sizeof(BITMAPINFOHEADER);  // Size of the header
    bmpInfoHeader.biBitCount    = wBitsPerPixel;             // Bit count
    bmpInfoHeader.biCompression = BI_RGB;                    // Standard RGB, no compression
    bmpInfoHeader.biWidth       = lWidth;                    // Width in pixels
    bmpInfoHeader.biHeight      = -lHeight;                  // Height in pixels, negative indicates it's stored right-side-up
    bmpInfoHeader.biPlanes      = 1;                         // Default
    bmpInfoHeader.biSizeImage   = dwByteCount;               // Image size in bytes

    BITMAPFILEHEADER bfh = {0};

    bfh.bfType    = 0x4D42;                                           // 'M''B', indicates bitmap
    bfh.bfOffBits = bmpInfoHeader.biSize + sizeof(BITMAPFILEHEADER);  // Offset to the start of pixel data
    bfh.bfSize    = bfh.bfOffBits + bmpInfoHeader.biSizeImage;        // Size of image + headers

    // Create the file on disk to write to
    HANDLE hFile = CreateFileW(lpszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Return if error opening file
    if (NULL == hFile) 
    {
        return E_ACCESSDENIED;
    }

    DWORD dwBytesWritten = 0;
    
    // Write the bitmap file header
    if (!WriteFile(hFile, &bfh, sizeof(bfh), &dwBytesWritten, NULL))
    {
        CloseHandle(hFile);
        return E_FAIL;
    }
    
    // Write the bitmap info header
    if (!WriteFile(hFile, &bmpInfoHeader, sizeof(bmpInfoHeader), &dwBytesWritten, NULL))
    {
        CloseHandle(hFile);
        return E_FAIL;
    }
    
    // Write the RGB Data
    if (!WriteFile(hFile, pBitmapBits, bmpInfoHeader.biSizeImage, &dwBytesWritten, NULL))
    {
        CloseHandle(hFile);
        return E_FAIL;
    }    

    // Close the file
    CloseHandle(hFile);
    return S_OK;
}