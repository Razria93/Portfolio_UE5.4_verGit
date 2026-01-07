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
	void PrintDefaultDamageEvent(AActor* InDamagedActor, AController* InDamageInstigator, AActor* InDamageCauser, const FDamageSpecKey& InDamageSpecKey, const FDamageSpec& InDamageSpec, const FDamageResult& InDamageResult, float InTakedDamage, float InFinalDamage);
};
