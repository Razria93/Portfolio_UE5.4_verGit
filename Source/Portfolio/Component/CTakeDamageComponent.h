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
	UPROPERTY(Transient)
	class AActor* OwnerActor_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCReactionOrchestratorComponent* ReactionOrchestratorComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCDamageFeedbackComponent* DamageFeedbackComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCDefenseComponent* DefenseComp_Cached = nullptr;

protected:
	void BeginPlay() override;

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
	void DispatchAcceptedCombatResult(const FTakeDamagePacket& InTakeDamagePacket) const;
	void DispatchRejectedCombatResult(const FTakeDamagePacket& InTakeDamagePacket) const;
	void DispatchCombatResultToReceiver(const FTakeDamagePacket& InTakeDamagePacket) const;

private:
	AController* ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const;
	AActor* ResolveCombatResultReceiverActor(const FTakeDamagePacket& InTakeDamagePacket) const;

	float ComputeMitigatedDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	float ComputeFinalTakenDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	float CommitDamageToHealth(const FTakeDamageContext& InOutTakeDamageContext) const;

private:
	FTakeDamagePayload BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const;
	FTakeDamageContext BuildContext(const FTakeDamagePayload& InTakeDamagePayload) const;
	FTakeDamageResult BuildResult(const FTakeDamageContext& InTakeDamageContext) const;
	FTakeDamagePacket BuildPacket(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;
	FCombatResultPacket BuildCombatResultPacket(const FTakeDamagePacket& InTakeDamagePacket) const;

private:
	void PrintTakeDamageSummaryInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintTakeDamageContextInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintTakeDamageOutcomeInfo(const FTakeDamagePacket& InTakeDamagePacket) const;

private:
	void PrintObjectInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintSpecKeyInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintDamageAmountInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
};
