#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatHitTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "CCombatSignalSourceTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatSignalSourceRejectReason : uint8
{
	None = 0,

	InvalidRequest,

	InvalidAttacker,
	InvalidDamageCauser,
	InvalidTarget,
	InvalidInstigator,

	SpecNotFound,
	ComputeFailed,
	CommitFailed,

	// Reject Reason of 'CanSendCombatSignal'
	InvalidOwner,
	SelfTarget,
	DuplicateHitInWindow,
	FriendlyTarget,
};

USTRUCT(BlueprintType)
struct FCombatSignalSourcePayload
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FHitContext HitContext = FHitContext();

	UPROPERTY(Transient)
	class AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FCombatSignalHitWindowKey HitWindowKey = FCombatSignalHitWindowKey();

	UPROPERTY(Transient)
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

	UPROPERTY(Transient)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

public:
	FCombatSignalSourcePayload() = default;
};

USTRUCT(BlueprintType)
struct FCombatSignalSourceContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	ECombatSignalSourceRejectReason RejectReason = ECombatSignalSourceRejectReason::None;

	UPROPERTY(Transient)
	FHitContext HitContext = FHitContext();

	UPROPERTY(Transient)
	class AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	class AController* Instigator = nullptr;

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FCombatSignalHitWindowKey HitWindowKey = FCombatSignalHitWindowKey();

	UPROPERTY(Transient)
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

	UPROPERTY(Transient)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(Transient)
	FDamageSpec DamageSpec = FDamageSpec();

	UPROPERTY(Transient)
	FDamageAmount DamageAmount = FDamageAmount();

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;

public:
	FCombatSignalSourceContext() = default;
};

USTRUCT(BlueprintType)
struct FCombatSignalSourceResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	ECombatSignalSourceRejectReason RejectReason = ECombatSignalSourceRejectReason::None;

	UPROPERTY(Transient)
	FCombatSignalHitWindowKey HitWindowKey = FCombatSignalHitWindowKey();

	UPROPERTY(Transient)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(Transient)
	float BaseDamage = 0.f;

	UPROPERTY(Transient)
	float RequestDamage = 0.f;

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;

public:
	FCombatSignalSourceResult() = default;
};
