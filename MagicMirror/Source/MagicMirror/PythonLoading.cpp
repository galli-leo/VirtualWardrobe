#include "MagicMirror.h"
#include "PythonLoading.h"
#include "WardrobeManager.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FPythonLoading* FPythonLoading::Runnable = NULL;
//***********************************************************

FPythonLoading::FPythonLoading(UWardrobeManager* theManager, FString theDir)
{

	//Link to where data should be stored
	this->manager = theManager;
	this->dir = theDir;

	Thread = FRunnableThread::Create(this, TEXT("FPythonLoading"), 0, TPri_Normal); //windows default = 8mb for thread, could specify more
}

FPythonLoading::~FPythonLoading()
{
	
	delete Thread;
	Thread = NULL;
}

//Init
bool FPythonLoading::Init()
{
	
	//Init the Data 
	this->IsFinished = false;
	return true;
}

//Run
uint32 FPythonLoading::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);
    

	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
#if PLATFORM_WINDOWS
    	gstate = PyGILState_Ensure();
	printd("Initializing Python with Dir: %s", *this->dir)
	InitPython(this->dir);
    	PyGILState_Release(gstate);
#else
    printd("Not on windows -> No python");
#endif

	this->manager->PythonLoaded.Broadcast();

	this->IsFinished = true;

	return 0;
}

//stop
void FPythonLoading::Stop()
{
	StopTaskCounter.Increment();
}


FPythonLoading* FPythonLoading::JoyInit(UWardrobeManager* theManager, FString theDir)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FPythonLoading(theManager, theDir);
	}
	return Runnable;
}

void FPythonLoading::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FPythonLoading::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FPythonLoading::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished;
	return true;
}
