#pragma once

#include "Engine/EngineTypes.h"
#include "Animation/AnimInstance.h"
#include "KinectFunctionLibrary.h"
#include "Animation/AnimNodeSpaceConversions.h"
#include "KinectAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FKinectBoneRetargetAdjustment
{

	GENERATED_USTRUCT_BODY()

public:

	FKinectBoneRetargetAdjustment() :BoneDirAxis(EAxis::Z), BoneNormalAxis(EAxis::X), BoneBinormalAxis(EAxis::Y), BoneDirAdjustment(FRotator()), BoneNormalAdjustment(FRotator())
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		FRotator BoneDirAdjustment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		FRotator BoneNormalAdjustment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		bool bInvertDir = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		bool bInvertNormal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		TEnumAsByte<EAxis::Type> BoneDirAxis = EAxis::Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		TEnumAsByte<EAxis::Type> BoneNormalAxis = EAxis::X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		TEnumAsByte<EAxis::Type> BoneBinormalAxis = EAxis::Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Kinect)
		bool bDebugDraw = false;

};



UCLASS(MinimalApi, BlueprintType, Blueprintable)
class UKinectAnimInstance : public UAnimInstance
{

	GENERATED_UCLASS_BODY()

public:

	/**************************************************************************************************
	 * Native evaluate animation.
	 *
	 * @author	Leon Rosengarten
	 *
	 * @param [in,out]	Output	The output.
	 *
	 * @return	true to indicate to Super not to evaluate animation graph again
	 **************************************************************************************************/

	virtual bool NativeEvaluateAnimation(FPoseContext& Output) override;

	//UKinectAnimInstance(const class FObjectInitializer& PCIP);

	virtual void NativeInitializeAnimation() override;

	void ProccessSkeleton();

public:

	//*************************************************************************
	// Functions
	//*************************************************************************

	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**************************************************************************************************
		 * Executes the kinect body event action.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	KinectPlayer	The kinect player.
		 * @param	Skeleton		The skeleton.
		 **************************************************************************************************/

		void OnKinectBodyEvent(EAutoReceiveInput::Type KinectPlayer, const FBody& Skeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation")

		/**************************************************************************************************
		 * Override bone rotation by name.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	BoneName			Name of the bone.
		 * @param	BoneBoneRotation	The bone rotation.
		 **************************************************************************************************/

		void OverrideBoneRotationByName(FName BoneName, FRotator BoneBoneRotation);

	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**************************************************************************************************
		 * Sets override enabled.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	Enable	true to enable, false to disable.
		 **************************************************************************************************/

		void SetOverrideEnabled(bool Enable);

	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**************************************************************************************************
		 * Resets the override.
		 *
		 * @author	Leon Rosengarten
		 **************************************************************************************************/

		void ResetOverride();

	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**************************************************************************************************
		 * Removes the bone override by name described by BoneName.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	BoneName	Name of the bone.
		 **************************************************************************************************/

		void RemoveBoneOverrideByName(FName BoneName);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category = Kinect)
		TArray<FName> BonesToRetarget;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category = Kinect)
		TArray<FRotator> KinectBoneRotators;


	/** New translation of bone to apply. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category = Kinect)
		TArray<FKinectBoneRetargetAdjustment> BoneAdjustments;
	
public:

	//*************************************************************************
	// Properties
	//*************************************************************************

	UPROPERTY(EditAnywhere, Category = "Kinect")
		TEnumAsByte<EAutoReceiveInput::Type> ReceiveInputFromPlayer;	///< The receive input from player

	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "Kinect")
		bool KinectOverrideEnabled;										///< true to enable, false to disable the kinect override

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kinect")
		bool EvaluateAnimationGraph;									///< true to evaluate animation graph



protected:

private:

	TMap<FName, FRotator> OverLayMap;   ///< The over lay map

	TMap<int32, FRotator> AdjasmentMap;

	TMap<int32, FRotator> BindMap;

	FBody CurrBody;

};


