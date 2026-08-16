#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatSignalTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "CCombatSignalTargetComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatSignalTargetAccepted, const FCombatSignalTargetPacket&);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatSignalTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCombatSignalTargetComponent();

private:
	// Component Reference
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

	// Runtime State
	uint64 NextAcceptedResultSerial = 1;

public:
	// Event
	FOnCombatSignalTargetAccepted OnCombatSignalTargetAccepted;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	// Validation
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
	bool CanReceiveCombatSignal(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	void ComputeTargetDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	float ComputeMitigatedDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	float ComputeFinalTakenDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	FCombatSignalTargetResult BuildResult(const FCombatSignalTargetContext& InCombatSignalTargetContext) const;

private:
	// Apply
	void CommitCombatSignalTarget(FCombatSignalTargetContext& InOutCombatSignalTargetContext);

private:
	// Packet
	FCombatSignalTargetPacket BuildPacket(const FCombatSignalTargetPayload& InCombatSignalTargetPayload, const FCombatSignalTargetContext& InCombatSignalTargetContext, const FCombatSignalTargetResult& InCombatSignalTargetResult);

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
