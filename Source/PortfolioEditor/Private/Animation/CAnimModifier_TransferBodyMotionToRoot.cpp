#include "Animation/CAnimModifier_TransferBodyMotionToRoot.h"

#include "Animation/AnimData/AnimDataModel.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "AnimPose.h"
#include "EngineLogs.h"
#include "ReferenceSkeleton.h"

#define LOCTEXT_NAMESPACE "CAnimModifier_TransferBodyMotionToRoot"

namespace
{
	struct FBoneTrackKeys
	{
		TArray<FVector> Location;
		TArray<FQuat> Rotation;
		TArray<FVector> Scale;

		void Add(const FTransform& InTransform)
		{
			Location.Add(InTransform.GetLocation());
			Rotation.Add(InTransform.GetRotation());
			Scale.Add(InTransform.GetScale3D());
		}
	};

	FVector MakeTransferTranslation(const FVector& InSourceDelta, const bool bInTransferHorizontal, const bool bInTransferVertical)
	{
		return FVector(
			bInTransferHorizontal ? InSourceDelta.X : 0.f,
			bInTransferHorizontal ? InSourceDelta.Y : 0.f,
			bInTransferVertical ? InSourceDelta.Z : 0.f);
	}
}

UCAnimModifier_TransferBodyMotionToRoot::UCAnimModifier_TransferBodyMotionToRoot()
{
	bReapplyPostOwnerChange = false;
}

void UCAnimModifier_TransferBodyMotionToRoot::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	if (!IsValid(AnimationSequence))
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot failed: invalid animation sequence."));
		return;
	}

	const USkeleton* skeleton = AnimationSequence->GetSkeleton();
	const IAnimationDataModel* model = AnimationSequence->GetDataModel();
	if (!IsValid(skeleton) || model == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot failed for %s: missing skeleton or animation data model."), *GetNameSafe(AnimationSequence));
		return;
	}

	const FReferenceSkeleton& referenceSkeleton = skeleton->GetReferenceSkeleton();
	const int32 rootBoneIndex = referenceSkeleton.FindBoneIndex(RootBone);
	const int32 sourceBoneIndex = referenceSkeleton.FindBoneIndex(SourceBone);
	if (rootBoneIndex == INDEX_NONE || sourceBoneIndex == INDEX_NONE)
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot failed for %s: RootBone=%s SourceBone=%s must exist on the skeleton."), *GetNameSafe(AnimationSequence), *RootBone.ToString(), *SourceBone.ToString());
		return;
	}

	if (rootBoneIndex == sourceBoneIndex || !IsSourceBelowRoot(referenceSkeleton, sourceBoneIndex, rootBoneIndex))
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot failed for %s: SourceBone=%s must be a descendant of RootBone=%s."), *GetNameSafe(AnimationSequence), *SourceBone.ToString(), *RootBone.ToString());
		return;
	}

	const int32 numKeys = model->GetNumberOfKeys();
	if (numKeys <= 1 || ReferenceFrame < 0 || ReferenceFrame >= numKeys)
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot failed for %s: frame count=%d, ReferenceFrame=%d is invalid."), *GetNameSafe(AnimationSequence), numKeys, ReferenceFrame);
		return;
	}

	TArray<int32> directRootChildIndices;
	referenceSkeleton.GetDirectChildBones(rootBoneIndex, directRootChildIndices);
	if (directRootChildIndices.IsEmpty())
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot failed for %s: RootBone=%s has no direct children to compensate."), *GetNameSafe(AnimationSequence), *RootBone.ToString());
		return;
	}

	TArray<FName> directRootChildren;
	directRootChildren.Reserve(directRootChildIndices.Num());
	for (const int32 childBoneIndex : directRootChildIndices)
	{
		directRootChildren.Add(referenceSkeleton.GetBoneName(childBoneIndex));
	}

	FAnimPose referencePose;
	UAnimPoseExtensions::GetAnimPoseAtFrame(AnimationSequence, ReferenceFrame, FAnimPoseEvaluationOptions(), referencePose);
	// In UE 5.4 FAnimPose exposes this basis as World, documented by the engine as "World (component) space".
	const FTransform referenceSourceWorldTransform = UAnimPoseExtensions::GetBonePose(referencePose, SourceBone, EAnimPoseSpaces::World);
	const FTransform referenceRootLocalTransform = UAnimPoseExtensions::GetBonePose(referencePose, RootBone, EAnimPoseSpaces::Local);

	float maximumExistingRootTranslation = 0.f;
	float maximumExistingRootYaw = 0.f;
	FVector finalSourceDelta = FVector::ZeroVector;

	for (int32 keyIndex = 0; keyIndex < numKeys; ++keyIndex)
	{
		FAnimPose pose;
		UAnimPoseExtensions::GetAnimPoseAtFrame(AnimationSequence, keyIndex, FAnimPoseEvaluationOptions(), pose);

		const FTransform rootLocalTransform = UAnimPoseExtensions::GetBonePose(pose, RootBone, EAnimPoseSpaces::Local);
		maximumExistingRootTranslation = FMath::Max(maximumExistingRootTranslation, (rootLocalTransform.GetLocation() - referenceRootLocalTransform.GetLocation()).Size());
		maximumExistingRootYaw = FMath::Max(maximumExistingRootYaw, FMath::Abs((rootLocalTransform.GetRotation() * referenceRootLocalTransform.GetRotation().Inverse()).Rotator().Yaw));

		if (keyIndex == numKeys - 1)
		{
			const FTransform sourceWorldTransform = UAnimPoseExtensions::GetBonePose(pose, SourceBone, EAnimPoseSpaces::World);
			finalSourceDelta = sourceWorldTransform.GetLocation() - referenceSourceWorldTransform.GetLocation();
		}
	}

	if (maximumExistingRootTranslation > ExistingRootMotionTolerance || maximumExistingRootYaw > ExistingRootMotionTolerance)
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot rejected %s: RootBone=%s already has motion (MaxTranslation=%.3f, MaxYaw=%.3f). Use an in-place source animation."), *GetNameSafe(AnimationSequence), *RootBone.ToString(), maximumExistingRootTranslation, maximumExistingRootYaw);
		return;
	}

	const FVector finalTransferDelta = MakeTransferTranslation(finalSourceDelta, bTransferHorizontalTranslation, bTransferVerticalTranslation);
	if (finalTransferDelta.SizeSquared() <= FMath::Square(ExistingRootMotionTolerance))
	{
		UE_LOG(LogAnimation, Error, TEXT("TransferBodyMotionToRoot rejected %s: SourceBone=%s has no transferable net movement. FinalDelta=%s."), *GetNameSafe(AnimationSequence), *SourceBone.ToString(), *finalTransferDelta.ToCompactString());
		return;
	}

	AppliedTrackBackups.Reset();
	BackupTrack(*model, RootBone);
	for (const FName childBoneName : directRootChildren)
	{
		BackupTrack(*model, childBoneName);
	}

	FBoneTrackKeys rootKeys;
	rootKeys.Location.Reserve(numKeys);
	rootKeys.Rotation.Reserve(numKeys);
	rootKeys.Scale.Reserve(numKeys);

	TMap<FName, FBoneTrackKeys> childKeys;
	for (const FName childBoneName : directRootChildren)
	{
		FBoneTrackKeys& keys = childKeys.Add(childBoneName);
		keys.Location.Reserve(numKeys);
		keys.Rotation.Reserve(numKeys);
		keys.Scale.Reserve(numKeys);
	}

	for (int32 keyIndex = 0; keyIndex < numKeys; ++keyIndex)
	{
		FAnimPose pose;
		UAnimPoseExtensions::GetAnimPoseAtFrame(AnimationSequence, keyIndex, FAnimPoseEvaluationOptions(), pose);

		TMap<FName, FTransform> originalDirectChildWorldTransforms;
		for (const FName childBoneName : directRootChildren)
		{
			originalDirectChildWorldTransforms.Add(childBoneName, UAnimPoseExtensions::GetBonePose(pose, childBoneName, EAnimPoseSpaces::World));
		}

		const FTransform sourceWorldTransform = UAnimPoseExtensions::GetBonePose(pose, SourceBone, EAnimPoseSpaces::World);
		const FVector transferTranslation = MakeTransferTranslation(sourceWorldTransform.GetLocation() - referenceSourceWorldTransform.GetLocation(), bTransferHorizontalTranslation, bTransferVerticalTranslation);

		FTransform rootLocalTransform = UAnimPoseExtensions::GetBonePose(pose, RootBone, EAnimPoseSpaces::Local);
		rootLocalTransform.AddToTranslation(transferTranslation);
		if (bTransferYaw)
		{
			const FQuat sourceYawDelta = FRotator(0.f, (sourceWorldTransform.GetRotation() * referenceSourceWorldTransform.GetRotation().Inverse()).Rotator().Yaw, 0.f).Quaternion();
			rootLocalTransform.SetRotation((sourceYawDelta * rootLocalTransform.GetRotation()).GetNormalized());
		}

		UAnimPoseExtensions::SetBonePose(pose, rootLocalTransform, RootBone, EAnimPoseSpaces::Local);
		for (const FName childBoneName : directRootChildren)
		{
			UAnimPoseExtensions::SetBonePose(pose, originalDirectChildWorldTransforms.FindChecked(childBoneName), childBoneName, EAnimPoseSpaces::World);
		}

		rootKeys.Add(UAnimPoseExtensions::GetBonePose(pose, RootBone, EAnimPoseSpaces::Local));
		for (const FName childBoneName : directRootChildren)
		{
			childKeys.FindChecked(childBoneName).Add(UAnimPoseExtensions::GetBonePose(pose, childBoneName, EAnimPoseSpaces::Local));
		}
	}

	IAnimationDataController& controller = AnimationSequence->GetController();
	const bool bShouldTransact = false;
	controller.OpenBracket(LOCTEXT("TransferBodyMotionToRoot_Bracket", "Transfer Body Motion To Root"), bShouldTransact);

	for (const FBodyMotionToRootTrackBackup& backup : AppliedTrackBackups)
	{
		if (!backup.bTrackExistedBeforeApply)
		{
			controller.AddBoneCurve(backup.BoneName, bShouldTransact);
		}
	}

	controller.SetBoneTrackKeys(RootBone, rootKeys.Location, rootKeys.Rotation, rootKeys.Scale, bShouldTransact);
	for (const FName childBoneName : directRootChildren)
	{
		const FBoneTrackKeys& keys = childKeys.FindChecked(childBoneName);
		controller.SetBoneTrackKeys(childBoneName, keys.Location, keys.Rotation, keys.Scale, bShouldTransact);
	}

	controller.CloseBracket(bShouldTransact);

	UE_LOG(LogAnimation, Display, TEXT("TransferBodyMotionToRoot applied: Animation=%s Source=%s Root=%s Frames=%d NetTransfer=%s RootChildren=%d."), *GetNameSafe(AnimationSequence), *SourceBone.ToString(), *RootBone.ToString(), numKeys, *finalTransferDelta.ToCompactString(), directRootChildren.Num());
}

void UCAnimModifier_TransferBodyMotionToRoot::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	if (!IsValid(AnimationSequence) || AppliedTrackBackups.IsEmpty()) return;

	RestoreBackedUpTracks(*AnimationSequence);
	AppliedTrackBackups.Reset();
}

bool UCAnimModifier_TransferBodyMotionToRoot::IsSourceBelowRoot(const FReferenceSkeleton& InReferenceSkeleton, int32 InSourceBoneIndex, int32 InRootBoneIndex) const
{
	for (int32 boneIndex = InSourceBoneIndex; boneIndex != INDEX_NONE; boneIndex = InReferenceSkeleton.GetParentIndex(boneIndex))
	{
		if (boneIndex == InRootBoneIndex) return true;
	}

	return false;
}

void UCAnimModifier_TransferBodyMotionToRoot::BackupTrack(const IAnimationDataModel& InModel, FName InBoneName)
{
	FBodyMotionToRootTrackBackup& backup = AppliedTrackBackups.AddDefaulted_GetRef();
	backup.BoneName = InBoneName;
	backup.bTrackExistedBeforeApply = InModel.IsValidBoneTrackName(InBoneName);
	if (!backup.bTrackExistedBeforeApply)
	{
		return;
	}

	// UE 5.4 does not provide a non-deprecated raw-key accessor.  Read the raw track so Revert
	// can restore its exact sparse key layout rather than resampling it to every frame.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const FBoneAnimationTrack* track = InModel.FindBoneTrackByName(InBoneName);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	if (track == nullptr)
	{
		backup.bTrackExistedBeforeApply = false;
		return;
	}

	backup.LocationKeys.Reserve(track->InternalTrackData.PosKeys.Num());
	for (const FVector3f& locationKey : track->InternalTrackData.PosKeys)
	{
		backup.LocationKeys.Add(FVector(locationKey));
	}

	backup.RotationKeys.Reserve(track->InternalTrackData.RotKeys.Num());
	for (const FQuat4f& rotationKey : track->InternalTrackData.RotKeys)
	{
		backup.RotationKeys.Add(FQuat(rotationKey));
	}

	backup.ScaleKeys.Reserve(track->InternalTrackData.ScaleKeys.Num());
	for (const FVector3f& scaleKey : track->InternalTrackData.ScaleKeys)
	{
		backup.ScaleKeys.Add(FVector(scaleKey));
	}
}

void UCAnimModifier_TransferBodyMotionToRoot::RestoreBackedUpTracks(UAnimSequence& InAnimationSequence)
{
	IAnimationDataController& controller = InAnimationSequence.GetController();
	const IAnimationDataModel* model = InAnimationSequence.GetDataModel();
	if (model == nullptr) return;

	const bool bShouldTransact = false;
	controller.OpenBracket(LOCTEXT("RestoreBodyMotionToRoot_Bracket", "Restore Body Motion To Root"), bShouldTransact);

	for (const FBodyMotionToRootTrackBackup& backup : AppliedTrackBackups)
	{
		const bool bTrackExistsNow = model->IsValidBoneTrackName(backup.BoneName);
		if (!backup.bTrackExistedBeforeApply)
		{
			if (bTrackExistsNow)
			{
				controller.RemoveBoneTrack(backup.BoneName, bShouldTransact);
			}
			continue;
		}

		if (!bTrackExistsNow)
		{
			controller.AddBoneCurve(backup.BoneName, bShouldTransact);
		}

		controller.SetBoneTrackKeys(backup.BoneName, backup.LocationKeys, backup.RotationKeys, backup.ScaleKeys, bShouldTransact);
	}

	controller.CloseBracket(bShouldTransact);
}

#undef LOCTEXT_NAMESPACE
