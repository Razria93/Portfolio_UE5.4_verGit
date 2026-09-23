#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_BodyGlow.generated.h"

/** PerfectDodge body glow. Playback state belongs to the component, not this shared marker. */
UCLASS(meta = (DisplayName = "Body Glow (Point)"))
class PORTFOLIO_API UCAnimNotify_BodyGlow : public UAnimNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "BodyGlow")
	bool bStart = true;
	UPROPERTY(EditAnywhere, Category = "BodyGlow", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bStart"))
	float PeakWeight = 1.f;
	UPROPERTY(EditAnywhere, Category = "BodyGlow", meta = (ClampMin = "0", Units = "s", EditCondition = "bStart"))
	float FadeIn = .08f;
	// Start: fallback release on interruption/timeout. End: authored release.
	UPROPERTY(EditAnywhere, Category = "BodyGlow", meta = (ClampMin = "0", Units = "s"))
	float FadeOut = .22f;
	UPROPERTY(EditAnywhere, Category = "BodyGlow", meta = (ClampMin = "0.01", Units = "s", EditCondition = "bStart"))
	float SafetyTimeout = 8.f;
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
