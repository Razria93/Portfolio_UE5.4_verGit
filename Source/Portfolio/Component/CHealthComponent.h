#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CHealthStructure.h"
#include "CHealthComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCHealthComponent(); // TODO: Extend ResourceComponent

private:
	// === Initialize ===
	UPROPERTY(EditAnywhere, Category = "Initialize", meta = (ClampMin = 0.00))
	float InitMaxHP = 0.f;

	UPROPERTY(EditAnywhere, Category = "Initialize", meta = (ClampMin = 0.00))
	float InitCurrentHP = 0.f;

	UPROPERTY(EditAnywhere, Category = "Initialize")
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
	/* === Cached Objects === */
	class AActor* OwnerActor_Cached;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeHealth(float InInitMaxHP, float InInitCurrentHP, EMaxHPUpdatePolicy InUpdatePolicy);

public:
	/* === Skill API === */
	// (Current not used)
	bool TryKill();
	bool TryRevive(float InReviveHP);
	bool TryCancelRevive();
	bool TryUpdateMaxHP(float InNewMaxHP, EMaxHPUpdatePolicy InUpdatePolicy);

public:
	float TakeDamage(float InTakeDamageAmount);
	float TakeHeal(float InTakeHealAmount);

public:
	bool CanKill() const;
	bool CanRevive() const;

public:
	/* === Getter === */
	float GetMaxHP() const { return MaxHP; }
	float GetCurrentHP() const { return CurrentHP; }
	float GetPreviousHP() const { return PreviousHP; }
	EDeadState GetDeadState() const { return DeadState; }

public:
	void EnterDeadState();
	void EnterAliveState();

private:
	void UpdateDeadState();

private:
	void PrintTakeDamageContextInfo();
	void PrintTakeHealContextInfo();

private:
	void PrintHealthContextInfo(const FString& InLabel = TEXT("")) const;
	void PrintDeadContextInfo(const FString& InLabel = TEXT("")) const;
};
