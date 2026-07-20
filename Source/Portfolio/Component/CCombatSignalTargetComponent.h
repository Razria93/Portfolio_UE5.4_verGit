#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CCombatSignalStructure.h"
#include "Type/CWeaponStructure.h"
#include "CCombatSignalTargetComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatSignalTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCombatSignalTargetComponent();

private:
	/* === Injected Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCDefenseComponent* DefenseComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionOrchestratorComponent* ReactionOrchestratorComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHitFeedbackComponent* HitFeedbackComp_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Entry
	float RequestCombatSignalTarget(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);
	bool RequestCombatSignalTarget(const FCombatSignal& InCombatSignal);

private:
	float ProcessCombatSignalTarget(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);
	bool ProcessCombatSignalTarget(const FCombatSignal& InCombatSignal);
	float HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, class AController* InDamageInstigator, class AActor* InDamageCauser);
	bool HandleTimingCueSignal(const FCombatSignal& InCombatSignal);

private:
	// Receive
	bool ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser);
	bool ValidateSignalRequest(const FCombatSignal& InCombatSignal) const;
	FCombatSignalTargetPayload BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const;
	FCombatSignalTargetContext BuildContext(const FCombatSignalTargetPayload& InCombatSignalTargetPayload) const;

private:
	// Evaluate
	bool ValidateContext(FCombatSignalTargetContext& InOutCombatSignalTargetContext);
	bool CanReceiveCombatSignal(FCombatSignalTargetContext& InOutCombatSignalTargetContext);
	void ComputeTargetDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	float ComputeMitigatedDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	float ComputeFinalTakenDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	FCombatSignalTargetResult BuildResult(const FCombatSignalTargetContext& InCombatSignalTargetContext) const;

private:
	// Apply
	void CommitCombatSignalTarget(FCombatSignalTargetContext& InOutCombatSignalTargetContext);

private:
	// Packet
	FCombatSignalTargetPacket BuildPacket(const FCombatSignalTargetPayload& InCombatSignalTargetPayload, const FCombatSignalTargetContext& InCombatSignalTargetContext, const FCombatSignalTargetResult& InCombatSignalTargetResult) const;

private:
	// Notify
	void DispatchAcceptedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	void DispatchRejectedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	void DispatchCombatResultToReceiver(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

private:
	// Helper
	AController* ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const;
	float CommitDamageToHealth(const FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	AActor* ResolveCombatResultReceiverActor(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	FCombatResultPacket BuildCombatResultPacket(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

};
