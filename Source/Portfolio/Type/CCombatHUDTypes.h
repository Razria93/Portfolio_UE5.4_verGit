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
		if (Availability != EHUDResourceAvailability::Available
			|| Maximum <= 0.f
			|| !FMath::IsFinite(Current)
			|| !FMath::IsFinite(Maximum))
		{
			return 0.f;
		}

		return FMath::Clamp(Current / Maximum, 0.f, 1.f);
	}

	FText GetValueText() const
	{
		if (Availability != EHUDResourceAvailability::Available || Maximum <= 0.f || !FMath::IsFinite(Current) || !FMath::IsFinite(Maximum))
			return FText::FromString(TEXT("\u2014")); // \u2014 : '-'

		FNumberFormattingOptions options;

		options.SetUseGrouping(true);
		options.SetMaximumFractionalDigits(0);
		options.SetMinimumFractionalDigits(0);

		return FText::Format(NSLOCTEXT(
			"CombatHUD",
			"ResourceValue",
			"{0} / {1}"),
			FText::AsNumber(FMath::CeilToDouble(FMath::Clamp(Current, 0.f, Maximum)), &options), // Current
			FText::AsNumber(FMath::CeilToDouble(Maximum), &options)); // Maximum
	}
};

UENUM(BlueprintType)
enum class EHUDActionState : uint8
{
	Unimplemented, Unavailable, Ready, Active
};

USTRUCT(BlueprintType)
struct FHUDActionViewData
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	EHUDActionState Guard = EHUDActionState::Unavailable;

	UPROPERTY(BlueprintReadOnly)
	EHUDActionState Dodge = EHUDActionState::Unavailable;

	UPROPERTY(BlueprintReadOnly)
	EHUDActionState Execution = EHUDActionState::Unavailable;

	UPROPERTY(BlueprintReadOnly)
	bool bParrySuccess = false;

	bool operator==(const FHUDActionViewData& Other) const
	{
		return Guard == Other.Guard
			&& Dodge == Other.Dodge
			&& Execution == Other.Execution
			&& bParrySuccess == Other.bParrySuccess;
	}
};

USTRUCT(BlueprintType)
struct FCombatHUDViewData
{
	GENERATED_BODY()

	// Player
	UPROPERTY(BlueprintReadOnly)
	bool bHasPlayer = false;

	UPROPERTY(BlueprintReadOnly)
	FHUDActionViewData Actions;

	UPROPERTY(BlueprintReadOnly)
	FHUDResourceViewData PlayerHealth;

	// Target
	UPROPERTY(BlueprintReadOnly)
	bool bHasTarget = false;

	UPROPERTY(BlueprintReadOnly)
	FText TargetName;

	UPROPERTY(BlueprintReadOnly)
	FHUDResourceViewData TargetHealth;

	UPROPERTY(BlueprintReadOnly)
	int32 BalanceRemaining = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 BalanceMaximum = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TargetRevision = 0;
};
