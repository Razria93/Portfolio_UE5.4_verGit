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

	FText GetValueText() const
	{
		if (Availability != EHUDResourceAvailability::Available || Maximum <= 0.f
			|| !FMath::IsFinite(Current) || !FMath::IsFinite(Maximum))
			return FText::FromString(TEXT("\u2014"));
		// Keep positive fractional health from displaying as zero; gameplay values stay untouched.
		FNumberFormattingOptions options;
		options.SetUseGrouping(true).SetMaximumFractionalDigits(0).SetMinimumFractionalDigits(0);
		return FText::Format(NSLOCTEXT("CombatHUD", "ResourceValue", "{0} / {1}"),
			FText::AsNumber(FMath::CeilToDouble(FMath::Clamp(Current, 0.f, Maximum)), &options),
			FText::AsNumber(FMath::CeilToDouble(Maximum), &options));
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
