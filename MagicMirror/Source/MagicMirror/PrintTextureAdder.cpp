#include "MagicMirror.h"
#include "PrintTextureAdder.h"
#include "WardrobeManager.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Developer/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FPrintTextureCreator* FPrintTextureCreator::PrintCreator = NULL;
//***********************************************************

FPrintTextureCreator::FPrintTextureCreator(UWardrobeManager* theManager, uint32 id)
{
	//Link to where data should be stored
	this->manager = theManager;
	this->id = id;

	Thread = FRunnableThread::Create(this, TEXT("FPrintTextureCreator"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

FPrintTextureCreator::~FPrintTextureCreator()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FPrintTextureCreator::Init()
{
	//Init the Data 
	this->IsFinished = false;
	return true;
}

//Run
uint32 FPrintTextureCreator::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.1);

	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
	addPrintToItemFromCWD(this->id);
	//FString finalTexturePath = FString::Printf(TEXT("%s/tshirt/%04d/final_texture.png"), *UWardrobeManager::texturePath, this->id);
	//UTexture2D* finalTexture = this->LoadImageFromFile(finalTexturePath);
	//manager->TshirtProcessed.Broadcast(finalTexture);

	this->IsFinished = true;

	return 0;
}

//stop
void FPrintTextureCreator::Stop()
{
	StopTaskCounter.Increment();
}

UTexture2D* FPrintTextureCreator::LoadImageFromFile(FString file)
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

FPrintTextureCreator* FPrintTextureCreator::JoyInit(UWardrobeManager* theManager, uint32 id)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!PrintCreator && FPlatformProcess::SupportsMultithreading())
	{
		PrintCreator = new FPrintTextureCreator(theManager, id);
	}
	return PrintCreator;
}

void FPrintTextureCreator::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FPrintTextureCreator::Shutdown()
{
	if (PrintCreator)
	{
		PrintCreator->EnsureCompletion();
		delete PrintCreator;
		PrintCreator = NULL;
	}
}

bool FPrintTextureCreator::IsThreadFinished()
{
	if (PrintCreator) return PrintCreator->IsFinished;
	return true;
}
