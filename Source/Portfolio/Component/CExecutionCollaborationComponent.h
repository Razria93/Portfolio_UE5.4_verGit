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
class ACharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnExecutionLethalDeathEntryExpected, const FExecutionSessionId&);

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

	// Source Start Geometry Config
	UPROPERTY(EditAnywhere, Category = "Execution|Start Geometry")
	FExecutionStartGeometrySettings StartGeometrySettings;

	// Target Outcome Config
	UPROPERTY(EditAnywhere, Category = "Execution|Outcome")
	EExecutionLethalCondition LethalCondition = EExecutionLethalCondition::Disabled;

	UPROPERTY(EditAnywhere, Category = "Execution|Outcome", meta = (ClampMin = 0.0, ClampMax = 1.0, EditCondition = "LethalCondition == EExecutionLethalCondition::HealthRatio"))
	float LethalHealthRatio = 0.25f;

	// Session Runtime
	FExecutionCollaborationContext ActiveContext;
	EExecutionCollaborationState CollaborationState = EExecutionCollaborationState::None;
	bool bIsSourceRole = false;
	bool bSourceActionTerminal = false;
	bool bTargetReactionTerminal = false;
	uint32 NextSessionSerial = 1;

	// Participant Movement Collision Runtime
	TWeakObjectPtr<ACharacter> MovementIgnoredExecutionPartner;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

public:
	// Lethal Death Entry Bridge
	FOnExecutionLethalDeathEntryExpected OnExecutionLethalDeathEntryExpected;

	// Source Request
	bool RequestCombatExecution();

	// Source Commit
	bool HandleSourceExecutionCommit(uint32 InActionRequestSerial, float InStandardExecutionDamage);

	// Target Outcome
	bool CommitExecutionOutcome(const FExecutionOutcomePacket& InPacket);

	// Query
	bool HasActiveExecutionSession() const;
	EExternalCombatInputPolicy GetExternalCombatInputPolicy() const;
	EExecutionCollaborationState GetExecutionCollaborationState() const { return CollaborationState; }
	FExecutionCollaborationRuntimeSnapshot GetExecutionCollaborationRuntimeSnapshot() const;
	bool BuildSourceExecutionStartGeometrySnapshot(FExecutionStartGeometrySnapshot& OutSnapshot) const;

private:
	// Participant Event Observation
	UFUNCTION()
	void HandleActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, uint32 InActionRequestSerial, EActionEventType InActionEventType);

	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);
	void HandleDeadStateChanged(EDeadState InPreviousState, EDeadState InNewState);
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);

	// Partner Coordination
	bool AcceptExecutionReservation(const FExecutionSessionId& InSessionId, const FCombatTargetSnapshot& InTargetSnapshot, FExecutionCollaborationContext& OutContext);
	void ReceivePartnerSourceActionTerminal(const FExecutionSessionId& InSessionId);
	void ReceivePartnerTargetReactionTerminal(const FExecutionSessionId& InSessionId);
	void ReceivePartnerCommit(const FExecutionSessionId& InSessionId);
	void ReceivePartnerCancellation(const FExecutionSessionId& InSessionId, EExecutionCollaborationCancelReason InReason);

	// Participant Movement Collision Policy
	bool ApplyExecutionParticipantMovementIgnore(UCExecutionCollaborationComponent* InPartnerComponent);
	void RestoreExecutionParticipantMovementIgnore();

	// Session Control
	bool StartTargetExecutionReaction();
	bool StartSourceExecutionAction();
	bool ActivateExecutionPair();
	void CancelActiveExecutionSession(EExecutionCollaborationCancelReason InReason, bool bNotifyPartner);
	void CompleteActiveExecutionSession();
	void TryCompleteActiveExecutionSession();
	void CancelLocalExecutionParticipant(bool bWasSourceRole, EReactionType InPrimaryReactionType);

	// Session Validation
	bool IsActiveSession(const FExecutionSessionId& InSessionId) const;
	bool IsTargetSnapshotCurrent() const;
	bool IsTargetExecutionOpportunityCurrent() const;

	// Execution Startup Validation
	bool CanStartSourceExecution() const;
	bool CanResolveSourceExecutionAction(EExecutionOutcomePolicy InOutcomePolicy) const;
	bool CanStartTargetExecution() const;
	bool CanResolveTargetExecutionReaction(EExecutionOutcomePolicy InOutcomePolicy) const;
	bool IsSourceExecutionStartGeometryValid(const FCombatTargetSnapshot& InTargetSnapshot) const;
	bool AlignTargetExecutionFacing(const FCombatTargetSnapshot& InTargetSnapshot) const;

	// Target Outcome Resolution
	EExecutionOutcomePolicy ResolveTargetExecutionOutcomePolicy() const;
	bool CanResolveLethalExecutionOutcome() const;

	// Execution Policy Resolution
	EReactionType GetPrimaryReactionType() const;
	int32 GetExecutionActionIndex(EExecutionOutcomePolicy InOutcomePolicy) const;

	// Partner Lookup
	class UCExecutionCollaborationComponent* FindPartnerCollaborationComponent() const;

	// Session Runtime
	uint32 AllocateSessionSerial();
	void ResetActiveExecutionSession();
};
