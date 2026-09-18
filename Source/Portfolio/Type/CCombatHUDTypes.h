#pragma once

#include "CoreMinimal.h"
#include "CCombatHUDTypes.generated.h"

UENUM(BlueprintType)
enum class EHUDResourceAvailability : uint8
{
	Unavailable,
	Available,
	Unimplemented
};

USTRUCT(BlueprintType)
struct FHUDResourceViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EHUDResourceAvailability Availability = EHUDResourceAvailability::Unavailable;
	UPROPERTY(BlueprintReadOnly)
	float Current = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float Maximum = 0.f;

	float GetFraction() const
	{
		return Availability == EHUDResourceAvailability::Available && Maximum > 0.f
			? FMath::Clamp(Current / Maximum, 0.f, 1.f) : 0.f;
	}
};

USTRUCT(BlueprintType)
struct FCombatHUDViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bHasPlayer = false;
	UPROPERTY(BlueprintReadOnly)
	bool bHasTarget = false;
	UPROPERTY(BlueprintReadOnly)
	FText TargetName;
	UPROPERTY(BlueprintReadOnly)
	FHUDResourceViewData PlayerHealth;
	UPROPERTY(BlueprintReadOnly)
	FHUDResourceViewData TargetHealth;
	UPROPERTY(BlueprintReadOnly)
	int32 BalanceRemaining = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 BalanceMaximum = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 TargetRevision = 0;
};
