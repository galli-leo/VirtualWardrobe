#include "MagicMirror.h"
#include "CategoryPredicting.h"
#include "WardrobeManager.h"
#include "TextureCreator.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FCategoryPredicting* FCategoryPredicting::Runnable = NULL;
//***********************************************************

FCategoryPredicting::FCategoryPredicting(UWardrobeManager* theManager)
{
	//Link to where data should be stored
	this->manager = theManager;

	Thread = FRunnableThread::Create(this, TEXT("FCategoryPredicting"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

FCategoryPredicting::~FCategoryPredicting()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FCategoryPredicting::Init()
{
	//Init the Data 
	this->IsFinished = false;
	return true;
}

//Run
uint32 FCategoryPredicting::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	prediction pred = std::make_tuple(420, 0.0);
	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
#if PLATFORM_WINDOWS
	if (!isPythonBusy && FTextureCreator::Runnable->IsThreadFinished() && manager->mode != EWardrobeMode::MM_ScanningForPrint)
	{
		gstate = PyGILState_Ensure();
		pred = predictCategoryFromCWD();
		PyGILState_Release(gstate);
		//FPlatformProcess::Sleep(2.0);
	}
	
#else
	int id = 0;
#endif
	int id = std::get<0>(pred);
	float proba = std::get<1>(pred);
	manager->categoryPredicted.Broadcast(id, proba);

	this->IsFinished = true;

	return 0;
}

//stop
void FCategoryPredicting::Stop()
{
	StopTaskCounter.Increment();
}


FCategoryPredicting* FCategoryPredicting::JoyInit(UWardrobeManager* theManager)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FCategoryPredicting(theManager);
	}
	return Runnable;
}

void FCategoryPredicting::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FCategoryPredicting::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FCategoryPredicting::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished;
	return true;
}
