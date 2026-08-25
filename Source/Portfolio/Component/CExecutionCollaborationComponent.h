#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CActionTypes.h"
#include "Type/CExecutionCollaborationTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CReactionTypes.h"
#include "CExecutionCollaborationComponent.generated.h"

struct FCombatTargetChange;
struct FReactionExecutionLifecycleEvent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCExecutionCollaborationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCExecutionCollaborationComponent();

private:
	// Component References
	UPROPERTY(Transient)
	ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCBalanceComponent* BalanceComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatTargetComponent* CombatTargetComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalTargetComponent* CombatSignalTargetComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionOrchestratorComponent* ActionOrchestratorComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionOrchestratorComponent* ReactionOrchestratorComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Injected = nullptr;

	// Session Runtime
	FExecutionCollaborationContext ActiveContext;
	EExecutionCollaborationState CollaborationState = EExecutionCollaborationState::None;
	bool bIsSourceRole = false;
	bool bSourceActionTerminal = false;
	bool bTargetReactionTerminal = false;
	uint32 NextSessionSerial = 1;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

public:
	// Source Request
	bool RequestExecutionForCurrentTarget(EExecutionOutcomePolicy InOutcomePolicy = EExecutionOutcomePolicy::Standard);

	// Source Commit
	bool HandleSourceExecutionCommit(uint32 InActionRequestSerial);

	// Target Outcome
	bool CommitExecutionOutcome(const FExecutionOutcomePacket& InPacket);

	// Query
	bool HasActiveExecutionSession() const;
	EExecutionCollaborationState GetExecutionCollaborationState() const { return CollaborationState; }

private:
	// Participant Event Observation
	UFUNCTION()
	void HandleActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, uint32 InActionRequestSerial, EActionEventType InActionEventType);

	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);
	void HandleDeadStateChanged(EDeadState InPreviousState, EDeadState InNewState);
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);

	// Partner Coordination
	bool AcceptExecutionReservation(const FExecutionSessionId& InSessionId, AActor* InSourceActor, int32 InSourceTargetRevision, EExecutionOutcomePolicy InOutcomePolicy, FExecutionCollaborationContext& OutContext);
	void ReceivePartnerSourceActionTerminal(const FExecutionSessionId& InSessionId);
	void ReceivePartnerTargetReactionTerminal(const FExecutionSessionId& InSessionId);
	void ReceivePartnerCommit(const FExecutionSessionId& InSessionId);
	void ReceivePartnerCancellation(const FExecutionSessionId& InSessionId, EExecutionCollaborationCancelReason InReason);

	// Session Control
	bool StartTargetExecutionReaction();
	bool StartSourceExecutionAction();
	void CancelActiveExecutionSession(EExecutionCollaborationCancelReason InReason, bool bNotifyPartner);
	void CompleteActiveExecutionSession();
	void TryCompleteActiveExecutionSession();
	void CancelLocalExecutionParticipant(bool bWasSourceRole);

	// Session Validation
	bool IsActiveSession(const FExecutionSessionId& InSessionId) const;
	bool CanStartSourceExecution();
	bool CanStartTargetExecution();
	bool IsSourceTargetSnapshotCurrent() const;
	bool IsTargetExecutionOpportunityCurrent() const;
	class UCExecutionCollaborationComponent* FindPartnerCollaborationComponent() const;
	uint32 AllocateSessionSerial();
	void ResetActiveExecutionSession();
};
