#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CCombatSignalTargetComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatSignalTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCombatSignalTargetComponent();

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class AActor* OwnerActor_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCReactionOrchestratorComponent* ReactionOrchestratorComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHitFeedbackComponent* HitFeedbackComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCDefenseComponent* DefenseComp_Cached = nullptr;

protected:
	void BeginPlay() override;

public:
	// Entry
	float RequestCombatSignalTarget(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

private:
	float ProcessCombatSignalTarget(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);
	float HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, class AController* InDamageInstigator, class AActor* InDamageCauser);

private:
	// Receive
	bool ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser);
	FTakeDamagePayload BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const;
	FTakeDamageContext BuildContext(const FTakeDamagePayload& InTakeDamagePayload) const;

private:
	// Evaluate
	bool ValidateContext(FTakeDamageContext& InOutTakeDamageContext);
	bool CanReceiveCombatSignal(FTakeDamageContext& InOutTakeDamageContext);
	void ComputeTargetDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	float ComputeMitigatedDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	float ComputeFinalTakenDamage(FTakeDamageContext& InOutTakeDamageContext) const;
	FTakeDamageResult BuildResult(const FTakeDamageContext& InTakeDamageContext) const;

private:
	// Apply
	void CommitCombatSignalTarget(FTakeDamageContext& InOutTakeDamageContext);

private:
	// Packet
	FTakeDamagePacket BuildPacket(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const;

private:
	// Notify
	void DispatchAcceptedCombatResult(const FTakeDamagePacket& InTakeDamagePacket) const;
	void DispatchRejectedCombatResult(const FTakeDamagePacket& InTakeDamagePacket) const;
	void DispatchCombatResultToReceiver(const FTakeDamagePacket& InTakeDamagePacket) const;

private:
	// Helper
	AController* ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const;
	float CommitDamageToHealth(const FTakeDamageContext& InOutTakeDamageContext) const;
	AActor* ResolveCombatResultReceiverActor(const FTakeDamagePacket& InTakeDamagePacket) const;
	FCombatResultPacket BuildCombatResultPacket(const FTakeDamagePacket& InTakeDamagePacket) const;

private:
	// Debug
	void PrintCombatSignalTargetSummaryInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintCombatSignalTargetContextInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintCombatSignalTargetOutcomeInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintObjectInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintSpecKeyInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PrintDamageAmountInfo(const FTakeDamagePacket& InTakeDamagePacket) const;
};
