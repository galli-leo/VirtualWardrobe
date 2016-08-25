
#include "IKinectV2PluginPCH.h"

#include "KinectAnimInstance.h"
#include "AnimationRuntime.h"
#include "AnimationUtils.h"


UKinectAnimInstance::UKinectAnimInstance(const class FObjectInitializer& PCIP) : Super(PCIP),
KinectOverrideEnabled(false),
EvaluateAnimationGraph(true)
{


	if (CurrentSkeleton)
	{
#if WITH_EDITORONLY_DATA
		auto num = CurrentSkeleton->PreviewAttachedAssetContainer.Num();
#endif
	}

	if (BoneAdjustments.Num() < 25)
	{
		BoneAdjustments.Empty();
		BoneAdjustments.AddDefaulted(25);
	}

	if (BonesToRetarget.Num() < 25)
	{
		BonesToRetarget.Empty();
		BonesToRetarget.AddZeroed(25);
	}

	if (KinectBoneRotators.Num() < 25)
	{
		KinectBoneRotators.Empty();
		KinectBoneRotators.AddZeroed(25);
	}
}


bool UKinectAnimInstance::NativeEvaluateAnimation(FPoseContext& Output)
{

	if (RootNode != nullptr && EvaluateAnimationGraph)
	{
		//SCOPE_CYCLE_COUNTER(STAT_AnimGraphEvaluate);

		RootNode->Evaluate(Output);
	}
	else
	{
		Output.ResetToRefPose();
	}


	if (!KinectOverrideEnabled)
		return true;

	USkeletalMeshComponent* OwningComponent = GetOwningComponent();

	//Proof of concept
	if (OwningComponent)
	{

		ProccessSkeleton();


		/*
		uint32 i = 0;
		for (auto BoneName : BonesToRetarget)
		{

			if (BoneName != NAME_None)
			{

				int32 BoneIndex = OwningComponent->GetBoneIndex(BoneName);

				if (BoneIndex >= 0)
				{

					FCSPose<FCompactPose> CSPose;

					CSPose.InitPose(Output.Pose);


					auto BoneTransform = CSPose.GetComponentSpaceTransform(FCompactPoseBoneIndex(BoneIndex));

					//BoneTransform.SetToRelativeTransform(GetOwningComponent()->GetComponentToWorld());



					BoneTransform.SetRotation(KinectBoneRotators[i].Quaternion());


					TArray<FBoneTransform> BoneTransforms;

					BoneTransforms.Add(FBoneTransform(FCompactPoseBoneIndex(BoneIndex), BoneTransform));

					CSPose.SafeSetCSBoneTransforms(BoneTransforms);

//					CSPose.SetComponentSpaceTransform(FCompactPoseBoneIndex(BoneIndex), BoneTransform);




					int32 ParentIndex = OwningComponent->GetBoneIndex(OwningComponent->GetParentBone(BoneName));


					if (ParentIndex >= 0)
					{
						Output.Pose[FCompactPoseBoneIndex(BoneIndex)].SetFromMatrix(BoneTransform.ToMatrixWithScale());

						//Output.Pose[FCompactPoseBoneIndex(BoneIndex)].SetToRelativeTransform(CSPose.GetComponentSpaceTransform(FCompactPoseBoneIndex(BoneIndex)));
					}


					Output.Pose[FCompactPoseBoneIndex(BoneIndex)] = CSPose.GetComponentSpaceTransform(FCompactPoseBoneIndex(BoneIndex));
				}
			}

			++i;

		}
		*/
	}



	return true;
}

#pragma  optimize ("",off)
void UKinectAnimInstance::NativeInitializeAnimation()
{

	AdjasmentMap.Empty();
	BindMap.Empty();

	USkeletalMeshComponent* OwningComponent = GetOwningComponent();

	if (OwningComponent)
	{

		if (OwningComponent->SkeletalMesh)
		{

			int32 BoneNum = OwningComponent->SkeletalMesh->RefSkeleton.GetNum();


			for (int32 i = 0; i < BoneNum; ++i)
			{
				auto BoneWorldTransform = OwningComponent->GetBoneTransform(i, OwningComponent->GetComponentToWorld());

				BindMap.Add(i, BoneWorldTransform.Rotator());

				AdjasmentMap.Add(i, (FRotationMatrix::MakeFromX(FVector(0.f, 0.f, 1.f)).Rotator() - BoneWorldTransform.GetRotation().Rotator().GetNormalized()));


			}


			for (auto BoneMapPair : OverLayMap){


				auto BoneName = BoneMapPair.Key;
				if (BoneName != NAME_None)
				{

				}
			}
		}
	}
}
#pragma  optimize ("",on)


void UKinectAnimInstance::ProccessSkeleton()
{
	uint32 i = 0;

	for (auto Bone : CurrBody.KinectBones)
	{



		if (BonesToRetarget[i] != NAME_None)
		{

			auto DeltaTranform = Bone.MirroredJointTransform.GetRelativeTransform(GetOwningComponent()->GetBoneTransform(0));


			//AxisMeshes[Bone.JointTypeEnd]->SetRelativeLocation(PosableMesh->GetBoneLocationByName(RetargetBoneNames[Bone.JointTypeEnd], EBoneSpaces::ComponentSpace));

			auto BoneBaseTransform = DeltaTranform*GetOwningComponent()->GetBoneTransform(0);



			FRotator PreAdjusmentRotator = BoneBaseTransform.Rotator();

			FRotator PostBoneDirAdjustmentRotator = (BoneAdjustments[Bone.JointTypeEnd].BoneDirAdjustment.Quaternion()*PreAdjusmentRotator.Quaternion()).Rotator();

			FRotator CompSpaceRotator = (PostBoneDirAdjustmentRotator.Quaternion()*BoneAdjustments[Bone.JointTypeEnd].BoneNormalAdjustment.Quaternion()).Rotator();

			FVector Normal, Binormal, Dir;

			UKismetMathLibrary::BreakRotIntoAxes(CompSpaceRotator, Normal, Binormal, Dir);

			Dir *= BoneAdjustments[Bone.JointTypeEnd].bInvertDir ? -1 : 1;

			Normal *= BoneAdjustments[Bone.JointTypeEnd].bInvertNormal ? -1 : 1;


			FVector X, Y, Z;

			switch (BoneAdjustments[Bone.JointTypeEnd].BoneDirAxis)
			{
			case EAxis::X:
				X = Dir;
				break;
			case EAxis::Y:
				Y = Dir;
				break;
			case EAxis::Z:
				Z = Dir;
				break;
			default:
				;
			}

			switch (BoneAdjustments[Bone.JointTypeEnd].BoneBinormalAxis)
			{
			case EAxis::X:
				X = Binormal;
				break;
			case EAxis::Y:
				Y = Binormal;
				break;
			case EAxis::Z:
				Z = Binormal;
				break;
			default:
				;
			}

			switch (BoneAdjustments[Bone.JointTypeEnd].BoneNormalAxis)
			{
			case EAxis::X:
				X = Normal;
				break;
			case EAxis::Y:
				Y = Normal;
				break;
			case EAxis::Z:
				Z = Normal;
				break;
			default:
				;
			}

			FRotator SwiveledRot = UKismetMathLibrary::MakeRotationFromAxes(X, Y, Z);

			SwiveledRot = (GetOwningComponent()->GetBoneTransform(0).Rotator().Quaternion()*SwiveledRot.Quaternion()).Rotator();

			KinectBoneRotators[i] = SwiveledRot;
		}

		++i;
	}
}



void UKinectAnimInstance::OnKinectBodyEvent(EAutoReceiveInput::Type KinectPlayer, const FBody& Skeleton)
{

	CurrBody = Skeleton;

	if (!CurrentSkeleton)
		return;

}

void UKinectAnimInstance::OverrideBoneRotationByName(FName BoneName, FRotator BoneRotation)
{

	OverLayMap.Add(BoneName, BoneRotation);

}

void UKinectAnimInstance::SetOverrideEnabled(bool Enable)
{
	KinectOverrideEnabled = Enable;
}

void UKinectAnimInstance::ResetOverride()
{
	OverLayMap.Empty();
}

void UKinectAnimInstance::RemoveBoneOverrideByName(FName BoneName)
{
	OverLayMap.Remove(BoneName);
}
