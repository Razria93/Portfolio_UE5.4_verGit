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
	class UCReactionComponent* ReactionComp_Cached;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Entry API
	float RequestTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

private:
	// Pipeline
	float ProcessTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

private:
	float HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, class AController* InDamageInstigator, class AActor* InDamageCauser);

private:
	bool ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser);
	bool ValidateContext(FTakeDamageContext& InOutTakeDamageContext);
	bool CanTakeDamage(FTakeDamageContext& InOutTakeDamageContext);
	void ComputeTakeDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	void CommitTakeDamage(FTakeDamageContext& InOutTakeDamageContext);

private:
	AController* ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const;
	float ComputeMitigatedDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	float ComputeFinalTakenDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	float CommitDamageToHealth(const FTakeDamageContext& InOutTakeDamageContext) const;

private:
	FTakeDamagePayload BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const;
	FTakeDamageContext BuildContext(const FTakeDamagePayload& InTakeDamagePayload) const;
	FTakeDamageResult BuildResult(const FTakeDamageContext& InTakeDamageContext) const;

private:
	void DispatchTakeDamageCommitted(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;
	void DispatchTakeDamageRejected(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;

private:
	void PrintTakeDamageSummaryInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;
	void PrintTakeDamageContextInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;

private:
	void PrintObjectInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;
	void PrintSpecKeyInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;
	void PrintDamageAmountInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;
};
