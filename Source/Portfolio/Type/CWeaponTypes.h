#pragma once

#include "CoreMinimal.h"
#include "CWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None = 0,	// Invalid, Unset

	Unarmed,
	Sword,

	All,		// Wildcard

	Max,		// Sentinel
};

USTRUCT(BlueprintType)
struct FWeaponContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType = EWeaponType::Max;

public:
	FWeaponContext() = default;
};
