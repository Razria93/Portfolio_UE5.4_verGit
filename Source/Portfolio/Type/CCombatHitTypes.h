#pragma once

#include "CoreMinimal.h"
#include "Type/CWeaponTypes.h"
#include "Type/CActionTypes.h"
#include "CCombatHitTypes.generated.h"

UENUM(BlueprintType)
enum class EDamageImpactInfoSource : uint8
{
	None = 0,

	SweepResult,
	ClosestPoint,

	Max,
};

USTRUCT(BlueprintType)
struct FOverlapContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class AActor* OwnerActor = nullptr;							// AttackActor

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;						// WeaponActor

	UPROPERTY(Transient)
	class UPrimitiveComponent* OverlappedComponent = nullptr;	// Hit Collision of WeaponActor

	UPROPERTY(Transient)
	class UShapeComponent* OverlapShape = nullptr;			// Cast result: UShapeComponent (nullptr if cast fails)

	UPROPERTY(Transient)
	class AActor* OtherActor = nullptr;							// DamagedActor

	UPROPERTY(Transient)
	class UPrimitiveComponent* OtherComponent = nullptr;		// DamagedComponent

	UPROPERTY(Transient)
	int32 OtherBodyIndex = INDEX_NONE;

	UPROPERTY(Transient)
	bool bFromSweep = false;

	UPROPERTY(Transient)
	FHitResult SweepResult;

	UPROPERTY(Transient)
	int32 HitWindowId = INDEX_NONE;

public:
	FOverlapContext() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FDamageImpactInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bHasHitResult = false;

	UPROPERTY(Transient)
	EDamageImpactInfoSource Source = EDamageImpactInfoSource::None;

	UPROPERTY(Transient)
	FHitResult HitResult = FHitResult();

public:
	FDamageImpactInfo() = default;

public:
	bool IsValidMinimal() const
	{
		return bHasHitResult
			&& Source != EDamageImpactInfoSource::None
			&& Source != EDamageImpactInfoSource::Max;
	}
};

USTRUCT(BlueprintType)
struct FHitContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FOverlapContext OverlapContext = FOverlapContext();

	UPROPERTY(Transient)
	FWeaponContext WeaponContext = FWeaponContext();

	UPROPERTY(Transient)
	FActionContext ActionContext = FActionContext();

	UPROPERTY(Transient)
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

public:
	FHitContext() = default;
};

USTRUCT()
struct FCombatSignalHitWindowKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	int32 HitWindowId = INDEX_NONE;

	bool operator==(const FCombatSignalHitWindowKey& InOther) const
	{
		return DamageCauser == InOther.DamageCauser
			&& HitWindowId == InOther.HitWindowId;
	}
};

FORCEINLINE uint32 GetTypeHash(const FCombatSignalHitWindowKey& InOther)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(InOther.DamageCauser));
	H = HashCombine(H, GetTypeHash(InOther.HitWindowId));

	return H;
}
