#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CHealthTypes.h"
#include "CCombatSignalTargetTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class ECombatSignalTargetRejectReason : uint8
{
	None = 0,

	InvalidTarget,
	InvalidCauser,
	InvalidInstigator,

	AlreadyDead,
	ZeroDamage,

	UnknownCueTag,
};

// Payload

USTRUCT(BlueprintType)
struct FCombatSignalTargetPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	class AController* EventInstigator = nullptr;

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	FHitImpactContext HitImpactContext = FHitImpactContext();

	UPROPERTY(Transient)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(Transient)
	FDamageSpec DamageSpec = FDamageSpec();

	UPROPERTY(Transient)
	FDamageRequestAmount DamageRequestAmount = FDamageRequestAmount();

	UPROPERTY(Transient)
	float RequestedDamage = 0.f;

public:
	FCombatSignalTargetPayload() = default;
};

// Runtime Context

USTRUCT(BlueprintType)
struct FCombatSignalTargetContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	class AController* Instigator = nullptr;

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	FHitImpactContext HitImpactContext = FHitImpactContext();

	UPROPERTY(Transient)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	ECombatSignalTargetRejectReason RejectReason = ECombatSignalTargetRejectReason::None;

	UPROPERTY(Transient)
	EDamageDefenseOutcome DefenseOutcome = EDamageDefenseOutcome::None;

	UPROPERTY(Transient)
	bool bShouldCommitDamage = true;

	UPROPERTY(Transient)
	float HealthPointBefore = 0.f;

	UPROPERTY(Transient)
	EDeadState DeadState_Before = EDeadState::Alive;

	UPROPERTY(Transient)
	float RequestedDamage = 0.f;		// Raw incoming damage requested by Apply pipeline. (ex. [skill] 100)

	UPROPERTY(Transient)
	float MitigatedDamage = 0.f;		// Post-mitigation damage after target defenses. (ex. [guard/resistance] 100 -> 70)

	UPROPERTY(Transient)
	float FinalTakenDamage = 0.f;		// Final damage decided by Take evaluation rules. (ex. [clamp max/min damage-limit] 70 -> 60)

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;		// Actual HP loss committed to Health. (ex. [shield absorbs] 60 -> HP: -30 / SP: -30)

	UPROPERTY(Transient)
	float HealthPointAfter = 0.f;

	UPROPERTY(Transient)
	EDeadState DeadState_After = EDeadState::Alive;

public:
	FCombatSignalTargetContext() = default;
};

// Result

USTRUCT(BlueprintType)
struct FCombatSignalTargetResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	ECombatSignalTargetRejectReason RejectReason = ECombatSignalTargetRejectReason::None;

	UPROPERTY(Transient)
	EDamageDefenseOutcome DefenseOutcome = EDamageDefenseOutcome::None;

	UPROPERTY(Transient)
	bool bShouldCommitDamage = true;

	UPROPERTY(Transient)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(Transient)
	float RequestDamage = 0.f;

	UPROPERTY(Transient)
	float MitigatedDamage = 0.f;

	UPROPERTY(Transient)
	float FinalTakenDamage = 0.f;

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;

	UPROPERTY(Transient)
	EDeadState DeadState_Before = EDeadState::Alive;

	UPROPERTY(Transient)
	EDeadState DeadState_After = EDeadState::Alive;

public:
	FCombatSignalTargetResult() = default;
};

// Packet

USTRUCT(BlueprintType)
struct FCombatSignalTargetPacket
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FCombatSignalTargetPayload Payload;

	UPROPERTY(Transient)
	FCombatSignalTargetContext Context;

	UPROPERTY(Transient)
	FCombatSignalTargetResult Result;
};
