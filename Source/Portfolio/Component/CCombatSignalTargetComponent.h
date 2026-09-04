#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatSignalTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "Type/CExecutionCollaborationTypes.h"
#include "CCombatSignalTargetComponent.generated.h"

enum class EReactionNotifyCommand : uint8;
enum class EReactionType : uint8;
enum class EIncapacitatedPresentation : uint8;
struct FBalanceLifecyclePacket;
struct FReactionExecutionContext;
struct FReactionExecutionLifecycleEvent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatSignalTargetAccepted, const FCombatSignalTargetPacket&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCombatSignalTargetReactionResolved, const FCombatSignalTargetPacket&, const struct FReactionRequestResult&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBalanceLifecycleReactionRequestResolved, const FBalanceLifecyclePacket&, const struct FReactionRequestResult&);

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

	UPROPERTY(Transient)
	class UCBalanceComponent* BalanceComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCExecutionCollaborationComponent* ExecutionCollaborationComp_Injected = nullptr;

	// Runtime State
	uint64 NextAcceptedResultSerial = 1;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Component Lifecycle
	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

private:
	// Component Reference Validation
	bool ValidateRequiredComponentReferences() const;

public:
	// Event
	FOnCombatSignalTargetAccepted OnCombatSignalTargetAccepted;
	FOnCombatSignalTargetReactionResolved OnCombatSignalTargetReactionResolved;
	FOnBalanceLifecycleReactionRequestResolved OnBalanceLifecycleReactionRequestResolved;

public:
	// Combat Damage Pipeline - Entry
	float RequestCombatDamageTarget(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

	// Combat Signal Pipeline - Entry
	bool RequestCombatSignalTarget(const FCombatSignal& InCombatSignal);

	// Combat Result Pipeline - Entry
	void RequestCombatResultTarget(const FCombatResultPacket& InCombatResultPacket);

	// Execution Outcome - Entry
	bool RequestExecutionOutcomeTarget(const FExecutionOutcomePacket& InExecutionOutcomePacket);
	bool TryResolveExecutionAppliedDamage(EExecutionOutcomePolicy InOutcomePolicy, float InStandardExecutionDamage, float& OutAppliedDamage) const;

private:
	// Combat Result Pipeline - Validation
	bool ValidateCombatResultTargetRequest(const FCombatResultPacket& InCombatResultPacket) const;

	// Combat Damage Pipeline - Process / Handler
	float ProcessCombatDamageTarget(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);
	float HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, class AController* InDamageInstigator, class AActor* InDamageCauser);

	// Combat Signal Pipeline - Process / Handler
	bool ProcessCombatSignalTarget(const FCombatSignal& InCombatSignal);
	bool HandleTimingCueSignal(const FCombatSignal& InCombatSignal);

	// Combat Result Pipeline - Process / Handler
	void ProcessCombatResultTarget(const FCombatResultPacket& InCombatResultPacket);
	void HandleParryCombatResult(const FCombatResultPacket& InCombatResultPacket);

	// Execution Outcome - Process / Handler
	bool ProcessExecutionOutcomeTarget(const FExecutionOutcomePacket& InExecutionOutcomePacket);

	// Balance / Collapse Lifecycle Event Handlers
	void HandleBalanceLifecycleReactionRequested(const FBalanceLifecyclePacket& InBalanceLifecyclePacket);
	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);
	void HandleReactionExecutionNotifyCommand(const FReactionExecutionContext& InContext, EReactionNotifyCommand InCommand);
	void HandleReactionIncapacitatedPresentationRequested(const FReactionExecutionContext& InContext, EIncapacitatedPresentation InPresentation);

private:
	// Combat Damage Pipeline - Validation
	bool ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser);
	bool ValidateSignalRequest(const FCombatSignal& InCombatSignal) const;
	FCombatSignalTargetPayload BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const;
	FCombatSignalTargetContext BuildContext(const FCombatSignalTargetPayload& InCombatSignalTargetPayload) const;

private:
	// Combat Damage Pipeline - Evaluate
	bool ValidateContext(FCombatSignalTargetContext& InOutCombatSignalTargetContext);
	bool CanReceiveCombatSignal(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	void ComputeTargetDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	float ComputeMitigatedDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	float ComputeFinalTakenDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	void ResolveDamageReactionOutcome(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	FCombatSignalTargetResult BuildResult(const FCombatSignalTargetContext& InCombatSignalTargetContext) const;

private:
	// Combat Damage Pipeline - Apply
	void CommitCombatSignalTarget(FCombatSignalTargetContext& InOutCombatSignalTargetContext);

private:
	// Combat Damage Pipeline - Packet
	FCombatSignalTargetPacket BuildPacket(const FCombatSignalTargetPayload& InCombatSignalTargetPayload, const FCombatSignalTargetContext& InCombatSignalTargetContext, const FCombatSignalTargetResult& InCombatSignalTargetResult);

private:
	// Combat Damage Pipeline - Dispatch
	void DispatchAcceptedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	void DispatchRejectedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	void DispatchCombatResultToReceiver(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

	// Combat Result Pipeline - Dispatch
	void DispatchBalanceLifecycleReaction(const FBalanceLifecyclePacket& InBalanceLifecyclePacket) const;

private:
	// Combat Damage Pipeline - Helper
	AController* ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const;
	float CommitDamageToHealth(const FCombatSignalTargetContext& InOutCombatSignalTargetContext) const;
	AActor* ResolveCombatResultReceiverActor(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	FCombatResultPacket BuildCombatResultPacket(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

};
