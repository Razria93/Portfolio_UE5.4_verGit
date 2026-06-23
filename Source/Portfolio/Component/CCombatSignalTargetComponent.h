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
	FCombatSignalTargetPayload BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const;
	FCombatSignalTargetContext BuildContext(const FCombatSignalTargetPayload& InTakeDamagePayload) const;

private:
	// Evaluate
	bool ValidateContext(FCombatSignalTargetContext& InOutTakeDamageContext);
	bool CanReceiveCombatSignal(FCombatSignalTargetContext& InOutTakeDamageContext);
	void ComputeTargetDamage(FCombatSignalTargetContext& InOutTakeDamageContext) const;
	float ComputeMitigatedDamage(FCombatSignalTargetContext& InOutTakeDamageContext) const;
	float ComputeFinalTakenDamage(FCombatSignalTargetContext& InOutTakeDamageContext) const;
	FCombatSignalTargetResult BuildResult(const FCombatSignalTargetContext& InTakeDamageContext) const;

private:
	// Apply
	void CommitCombatSignalTarget(FCombatSignalTargetContext& InOutTakeDamageContext);

private:
	// Packet
	FCombatSignalTargetPacket BuildPacket(const FCombatSignalTargetPayload& InTakeDamagePayload, const FCombatSignalTargetContext& InTakeDamageContext, const FCombatSignalTargetResult& InTakeDamageResult) const;

private:
	// Notify
	void DispatchAcceptedCombatResult(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	void DispatchRejectedCombatResult(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	void DispatchCombatResultToReceiver(const FCombatSignalTargetPacket& InTakeDamagePacket) const;

private:
	// Helper
	AController* ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const;
	float CommitDamageToHealth(const FCombatSignalTargetContext& InOutTakeDamageContext) const;
	AActor* ResolveCombatResultReceiverActor(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	FCombatResultPacket BuildCombatResultPacket(const FCombatSignalTargetPacket& InTakeDamagePacket) const;

private:
	// Debug
	void PrintCombatSignalTargetSummaryInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	void PrintCombatSignalTargetContextInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	void PrintCombatSignalTargetOutcomeInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	void PrintObjectInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	void PrintSpecKeyInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	void PrintDamageAmountInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
};
