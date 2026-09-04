#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "DamageEventId.h"
#include "Type/CWeaponTypes.h"
#include "Type/CActionTypes.h"
#include "Type/CCombatHitTypes.h"
#include "CCombatDamageTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EDamageDefenseOutcome : uint8
{
	None = 0,

	Guard,
	Parry,

	Max,
};

UENUM(BlueprintType)
enum class EDamageReactionOutcome : uint8
{
	None = 0,

	Hit,
	BlockHit,
	Parry,
	CollapseHit,
	Dead,

	Max,
};

// Key / Identifier

USTRUCT(BlueprintType)
struct FDamageSpecKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType = EWeaponType::Max;

	UPROPERTY(EditAnywhere)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FDamageSpecKey() = default;

public:
	bool IsValidMinimal() const
	{
		return WeaponType != EWeaponType::None
			&& WeaponType != EWeaponType::Max
			&& ActionType != EActionType::None
			&& ActionType != EActionType::Max;
	}

public:
	bool operator==(const FDamageSpecKey& InOther) const
	{
		return WeaponType == InOther.WeaponType
			&& ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

FORCEINLINE uint32 GetTypeHash(const FDamageSpecKey& InOther)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InOther.WeaponType)));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InOther.ActionType)));
	H = HashCombine(H, GetTypeHash(InOther.ActionIndex));

	return H;
}

// Global GetTypeHash keeps ADL available for this map key.

// Data / Config

USTRUCT(BlueprintType)
struct FDamageSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float BaseDamage = 0.f;

public:
	FDamageSpec() = default;
};

// Request

USTRUCT(BlueprintType)
struct FDamageRequestAmount
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	float RequestDamage = 0.f;

public:
	FDamageRequestAmount() = default;
};

// Packet

USTRUCT(BlueprintType)
struct FDefaultDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FHitImpactContext HitImpactContext = FHitImpactContext();

	UPROPERTY(Transient)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(Transient)
	FDamageSpec DamageSpec = FDamageSpec();

	UPROPERTY(Transient)
	FDamageRequestAmount DamageRequestAmount = FDamageRequestAmount();

public:
	static const int32 ClassID = (int32)EDamageEventTypeId::DefaultDamage;

public:
	FDefaultDamageEvent() = default;

public:
	virtual int32 GetTypeID() const override { return ClassID; }
	virtual bool IsOfType(int32 InID) const override { return InID == ClassID || FDamageEvent::IsOfType(InID); }
};
