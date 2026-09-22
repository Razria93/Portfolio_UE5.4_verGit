#pragma once

#include "CoreMinimal.h"
#include "CDefenseTuningTypes.generated.h"

// Data / Config

USTRUCT(BlueprintType)
struct FDefenseGuardTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0.0"))
	float GuardDamageTakenMultiplier = 0.5f;
};

USTRUCT(BlueprintType)
struct FDefenseDodgeTuning
{
	GENERATED_BODY()
	// Explicit migration: old assets keep their reviewed numeric intervals until enabled.
	UPROPERTY(EditAnywhere, Category = "Dodge")
	bool bUseMontageWindows = false;

	// Provisional values in montage position seconds, unaffected by playback speed.
	UPROPERTY(EditAnywhere, Category = "Dodge", meta = (ClampMin = "0.0", Units = "s"))
	float InvulnerabilityStart = 0.f;

	UPROPERTY(EditAnywhere, Category = "Dodge", meta = (ClampMin = "0.0", Units = "s"))
	float InvulnerabilityEnd = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Dodge", meta = (ClampMin = "0.0", Units = "s"))
	float PerfectStart = 0.f;

	UPROPERTY(EditAnywhere, Category = "Dodge", meta = (ClampMin = "0.0", Units = "s"))
	float PerfectEnd = 0.18f;

	UPROPERTY(EditAnywhere, Category = "CounterRush", meta = (ClampMin = "0.0", Units = "s"))
	float CounterConnectStart = 0.45f;

	// Real seconds after successful reaction completion (not montage seconds).
	UPROPERTY(EditAnywhere, Category = "CounterRush", meta = (ClampMin = "0.0", Units = "s"))
	float CounterGraceSeconds = 0.4f;

	UPROPERTY(EditAnywhere, Category = "CounterRush", meta = (ClampMin = "0.0", Units = "cm"))
	float CounterMaxDistance = 600.f;

	bool ContainsPerfectPosition(float InPosition) const
	{
		return ContainsInvulnerabilityPosition(InPosition) && FMath::IsFinite(PerfectStart)
			&& FMath::IsFinite(PerfectEnd) && InPosition >= PerfectStart && InPosition < PerfectEnd;
	}

	bool ContainsInvulnerabilityPosition(float InPosition) const
	{
		return FMath::IsFinite(InPosition) && FMath::IsFinite(InvulnerabilityStart)
			&& FMath::IsFinite(InvulnerabilityEnd) && InvulnerabilityStart >= 0.f
			&& InPosition >= InvulnerabilityStart && InPosition < InvulnerabilityEnd;
	}
};
