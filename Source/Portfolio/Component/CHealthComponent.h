#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CHealthComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCHealthComponent(); // TODO: Extend ResourceComponent

public:
	// === Initialize ===
	UPROPERTY(EditAnywhere, Category = "Initialize", meta = (ClampMin = 0.00))
	float InitMaxHP = 0.f;

	UPROPERTY(EditAnywhere, Category = "Initialize", meta = (ClampMin = 0.00))
	float InitCurrentHP = 0.f;

	UPROPERTY(EditAnywhere, Category = "Initialize")
	bool bFillToInitMaxHP = false;

private:
	UPROPERTY(VisibleAnywhere, meta = (ClampMin = "0.0"))
	float MaxHP = 0.f;

	UPROPERTY(VisibleAnywhere, meta = (ClampMin = "0.0"))
	float PreviousHP = 0.f;

	UPROPERTY(VisibleAnywhere, meta = (ClampMin = "0.0"))
	float CurrentHP = 0.f;

	UPROPERTY(VisibleAnywhere)
	bool bIsDead = false;

private:
	/* === Cached Objects === */
	class AActor* OwnerActor_Cached;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeHealth(float InInitMaxHP, float InInitCurrentHP, bool bFillToMaxHP);

public:
	/* === Getter === */
	float GetMaxHP() const { return MaxHP; }
	float GetCurrentHP() const { return CurrentHP; }

public:
	/* === Setter === */
	void SetMaxHP(float InNewMaxHP, bool bFillToMaxHP);
	void SetCurrentHP(float InNewCurrentHP);
	void SetKill();

public:
	/* === Check / Query === */
	bool IsDead() const { return bIsDead; }

public:
	float TakeDamage(float InTakeDamageAmount);
	float TakeHeal(float InTakeHealAmount);

private:
	void UpdateDeadState();

private:
	void PrintTakeDamageContextInfo();
	void PrintTakeHealContextInfo();

private:
	void PrintHealthContextInfo(const FString& InLabel = TEXT("")) const;
	void PrintDeadContextInfo(const FString& InLabel = TEXT("")) const;
};
