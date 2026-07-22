#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CHealthStructure.h"
#include "CHealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDeadStateChanged, EDeadState, EDeadState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCHealthComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Health|HP", meta = (ClampMin = 0.00))
	float InitMaxHP = 0.f;

	UPROPERTY(EditAnywhere, Category = "Health|HP", meta = (ClampMin = 0.00))
	float InitCurrentHP = 0.f;

	UPROPERTY(EditAnywhere, Category = "Health|HP")
	EMaxHPUpdatePolicy MaxHPUpdatePolicy = EMaxHPUpdatePolicy::ClampCurrent;

private:
	UPROPERTY(Transient)
	float MaxHP = 0.f;

	UPROPERTY(Transient)
	float PreviousHP = 0.f;

	UPROPERTY(Transient)
	float CurrentHP = 0.f;

private:
	UPROPERTY(Transient)
	EDeadState DeadState = EDeadState::Alive;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

public:
	FOnDeadStateChanged OnDeadStateChanged;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	void InitializeHealth(float InInitMaxHP, float InInitCurrentHP, EMaxHPUpdatePolicy InUpdatePolicy);

public:
	// Skill API
	// (Current not used)
	bool TryKill();
	bool TryRevive(float InReviveHP);
	bool TryCancelRevive();
	bool TryUpdateMaxHP(float InNewMaxHP, EMaxHPUpdatePolicy InUpdatePolicy);

public:
	float TakeDamage(float InTakeDamageAmount);
	float TakeHeal(float InTakeHealAmount);

public:
	bool IsAlive() const;
	bool IsDead() const;
	bool CanKill() const;
	bool CanRevive() const;

public:
	// Query
	float GetMaxHP() const { return MaxHP; }
	float GetCurrentHP() const { return CurrentHP; }
	float GetPreviousHP() const { return PreviousHP; }
	EDeadState GetDeadState() const { return DeadState; }

public:
	void EnterDeadState();
	void EnterAliveState();

public:
	// Notify Routing
	void HandleDeadStateNotify(EDeadState InDeadState);

private:
	void UpdateDeadState();
	void ChangeDeadState(EDeadState InNewDeadState);
};
