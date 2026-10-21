#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ReactionBase.h"
#include "Type/CCombatPresentationEnvelope.h"
#include "CAnimNotify_PerfectDodgePresentation.generated.h"

UENUM()
enum class EPerfectDodgePresentationCue : uint8
{
	Slow,
	Vignette
};

/** Point marker selects start; the owner's real-time envelope owns duration, not the montage. */
UCLASS(meta = (DisplayName = "Perfect Dodge Presentation (Point)"))
class PORTFOLIO_API UCAnimNotify_PerfectDodgePresentation : public UCAnimNotify_ReactionBase
{
	GENERATED_BODY()
public:
	UCAnimNotify_PerfectDodgePresentation();
	UPROPERTY(EditAnywhere, Category = "Presentation")
	EPerfectDodgePresentationCue Cue = EPerfectDodgePresentationCue::Slow;
	UPROPERTY(EditAnywhere, Category = "Presentation")
	FCombatPresentationEnvelope Envelope;
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "Cue == EPerfectDodgePresentationCue::Slow"))
	float SlowDilation = .15f;
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (EditCondition = "Cue == EPerfectDodgePresentationCue::Slow"))
	bool bAffectPlayer = true;
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (EditCondition = "Cue == EPerfectDodgePresentationCue::Slow"))
	bool bAffectAttacker = true;
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "Cue == EPerfectDodgePresentationCue::Vignette"))
	float VignetteIntensity = .85f;
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
