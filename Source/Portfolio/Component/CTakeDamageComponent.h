#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CTakeDamageComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCTakeDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCTakeDamageComponent();

private:
	/* === Cached Objects === */
	class AActor* OwnerActor_Cached;
	class UCHealthComponent* HealthComp_Cached;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	float RequestTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

private:
	float ProcessTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

private:
	float HandleDefaultDamage(const FDefaultDamageEvent& InDefaultDamageEvent, class AController* InDamageInstigator, class AActor* InDamageCauser);

private:
	AController* ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser);

private:
	void PrintTakeDamageSummaryInfo(const FTakeDamageContext& InTakeDamageContext) const;
	void PrintTakeDamageContextInfo(const FTakeDamageContext& InTakeDamageContext) const;

private:
	void PrintTakeDamageObjectInfo(const FTakeDamageContext& InTakeDamageContext) const;
	void PrintTakeDamageSpecKeyInfo(const FTakeDamageContext& InTakeDamageContext) const;
	void PrintTakeDamageSpecInfo(const FTakeDamageContext& InTakeDamageContext) const;
	void PrintTakeDamageResultInfo(const FTakeDamageContext& InTakeDamageContext) const;
	void PrintTakeDamageAmountInfo(const FTakeDamageContext& InTakeDamageContext) const;
};
