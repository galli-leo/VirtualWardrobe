#include "MagicMirror.h"
#include "TextureCreator.h"
#include "WardrobeManager.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FTextureCreator* FTextureCreator::Runnable = NULL;
//***********************************************************

FTextureCreator::FTextureCreator(UWardrobeManager* theManager, FCategory theCat)
{
	//Link to where data should be stored
	this->manager = theManager;
	this->category = theCat;

	Thread = FRunnableThread::Create(this, TEXT("FTextureCreator"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

FTextureCreator::~FTextureCreator()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FTextureCreator::Init()
{
	//Init the Data 
	this->IsFinished = false;
	return true;
}

//Run
uint32 FTextureCreator::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
#if PLATFORM_WINDOWS
	gstate = PyGILState_Ensure();
	int id = newItemWithTexturesFromCWD(category.id);
	PyGILState_Release(gstate);
#else
	int id = 0;
#endif
	FString finalTexturePath = FString::Printf(TEXT("%s/tshirt/%04d/final_texture.png"), *UWardrobeManager::texturePath, id);
	printw("Final Texture Path: %s", *finalTexturePath)
	UTexture2D* finalTexture = this->LoadImageFromFile(finalTexturePath);
	manager->TshirtProcessed.Broadcast(finalTexture);

	this->IsFinished = true;

	return 0;
}

//stop
void FTextureCreator::Stop()
{
	StopTaskCounter.Increment();
}

UTexture2D* FTextureCreator::LoadImageFromFile(FString file)
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

FTextureCreator* FTextureCreator::JoyInit(UWardrobeManager* theManager, FCategory theCat)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FTextureCreator(theManager, theCat);
	}
	return Runnable;
}

void FTextureCreator::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FTextureCreator::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FTextureCreator::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished;
	return true;
}
