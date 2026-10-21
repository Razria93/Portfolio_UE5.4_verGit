#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_WeaponGlow.generated.h"

UENUM()
enum class EWeaponGlowCue : uint8 { Start, End };

/** Point markers only. Playback/transfer state lives on the character, never on this shared asset. */
UCLASS(meta = (DisplayName = "Weapon Glow (Point)"))
class PORTFOLIO_API UCAnimNotify_WeaponGlow : public UAnimNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "WeaponGlow")
	EWeaponGlowCue Cue = EWeaponGlowCue::Start;
	UPROPERTY(EditAnywhere, Category = "WeaponGlow")
	bool bCounterRush = false;
	UPROPERTY(EditAnywhere, Category = "WeaponGlow", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "Cue == EWeaponGlowCue::Start"))
	float PeakWeight = 1.f;
	UPROPERTY(EditAnywhere, Category = "WeaponGlow", meta = (ClampMin = "0", Units = "s", EditCondition = "Cue == EWeaponGlowCue::Start"))
	float FadeIn = .08f;
	// Start: fallback release duration. End: explicitly authored release duration.
	UPROPERTY(EditAnywhere, Category = "WeaponGlow", meta = (ClampMin = "0", Units = "s"))
	float FadeOut = .18f;
	UPROPERTY(EditAnywhere, Category = "WeaponGlow", meta = (ClampMin = "0.01", Units = "s", EditCondition = "Cue == EWeaponGlowCue::Start"))
	float SafetyTimeout = 8.f;
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
