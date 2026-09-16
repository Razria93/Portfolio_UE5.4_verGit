#pragma once

#include "CoreMinimal.h"
#include "CWeaponTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None = 0,

	Unarmed,
	Sword,

	All,

	Max,
};

UENUM(BlueprintType)
enum class EWeaponSocketSlot : uint8
{
	None = 0,

	Hand,
	Holster,

	Max,
};

// Runtime Context

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
