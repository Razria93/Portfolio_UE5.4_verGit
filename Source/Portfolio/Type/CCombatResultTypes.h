#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatDamageTypes.h"
#include "CCombatResultTypes.generated.h"

USTRUCT(BlueprintType)
struct FCombatResultPacket
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
	EDamageDefenseOutcome DefenseOutcome = EDamageDefenseOutcome::None;

	UPROPERTY(Transient)
	bool bDamageCommitted = false;

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;

public:
	FCombatResultPacket() = default;

public:
	bool IsValidMinimal() const
	{
		return IsValid(SourceActor) && IsValid(TargetActor) && DefenseOutcome != EDamageDefenseOutcome::None;
	}

	bool IsParryResult() const
	{
		return DefenseOutcome == EDamageDefenseOutcome::Parry;
	}
};
