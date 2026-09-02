#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"

#include "CAnimModifier_TransferBodyMotionToRoot.generated.h"

class IAnimationDataModel;
struct FReferenceSkeleton;

USTRUCT()
struct PORTFOLIOEDITOR_API FBodyMotionToRootTrackBackup
{
	GENERATED_BODY()

	UPROPERTY()
	FName BoneName = NAME_None;

	UPROPERTY()
	bool bTrackExistedBeforeApply = false;

	UPROPERTY()
	TArray<FVector> LocationKeys;

	UPROPERTY()
	TArray<FQuat> RotationKeys;

	UPROPERTY()
	TArray<FVector> ScaleKeys;
};

/**
 * Editor-only bake utility. It transfers body motion sampled in FAnimPose's World (component) space into the root bone while
 * counter-transforming every direct root child so the visual animation remains unchanged.
 * Apply it only to a duplicated animation asset; the modifier edits raw bone transform tracks.
 */
UCLASS()
class PORTFOLIOEDITOR_API UCAnimModifier_TransferBodyMotionToRoot : public UAnimationModifier
{
	GENERATED_BODY()

public:
	UCAnimModifier_TransferBodyMotionToRoot();

	/** Bone whose World (component) space movement will become root motion. */
	UPROPERTY(EditAnywhere, Category = "Transfer")
	FName SourceBone = TEXT("pelvis");

	/** Root bone that receives the extracted movement. */
	UPROPERTY(EditAnywhere, Category = "Transfer")
	FName RootBone = TEXT("root");

	/** Frame used as the zero point for transferred movement. */
	UPROPERTY(EditAnywhere, Category = "Transfer", meta = (ClampMin = 0))
	int32 ReferenceFrame = 0;

	/** Transfer horizontal World (component) space translation. */
	UPROPERTY(EditAnywhere, Category = "Transfer")
	bool bTransferHorizontalTranslation = true;

	/** Transfer vertical World (component) space translation. Disabled by default to keep walking capsules grounded. */
	UPROPERTY(EditAnywhere, Category = "Transfer")
	bool bTransferVerticalTranslation = false;

	/** Transfer the source bone's yaw delta to the root. */
	UPROPERTY(EditAnywhere, Category = "Transfer")
	bool bTransferYaw = false;

	/** Existing root movement above this threshold rejects the bake to prevent compounding root motion. */
	UPROPERTY(EditAnywhere, Category = "Safety", meta = (ClampMin = 0.0))
	float ExistingRootMotionTolerance = 0.1f;

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

protected:
	virtual int32 GetNativeClassRevision() const override { return 1; }

private:
	UPROPERTY()
	TArray<FBodyMotionToRootTrackBackup> AppliedTrackBackups;

	bool IsSourceBelowRoot(const FReferenceSkeleton& InReferenceSkeleton, int32 InSourceBoneIndex, int32 InRootBoneIndex) const;
	void BackupTrack(const IAnimationDataModel& InModel, FName InBoneName);
	void RestoreBackedUpTracks(UAnimSequence& InAnimationSequence);
};
