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

	friend class FBalanceLifecycleBoundaryTest;

public:
	// Construction
	UCExecutionCollaborationComponent();

private:
	// Source Configuration
	UPROPERTY(EditAnywhere, Category = "Execution|Start Geometry")
	FExecutionStartGeometrySettings StartGeometrySettings;

	// Target Configuration
	UPROPERTY(EditAnywhere, Category = "Execution|Outcome")
	EExecutionLethalCondition LethalCondition = EExecutionLethalCondition::Disabled;

	UPROPERTY(EditAnywhere, Category = "Execution|Outcome", meta = (ClampMin = 0.0, ClampMax = 1.0, EditCondition = "LethalCondition == EExecutionLethalCondition::HealthRatio"))
	float LethalHealthRatio = 0.25f;

	// Active Session Runtime
	FExecutionCollaborationContext ActiveContext;
	EExecutionCollaborationState CollaborationState = EExecutionCollaborationState::None;
	bool bIsSourceRole = false;
	bool bSourceActionTerminal = false;
	bool bTargetReactionTerminal = false;

	// Session Identity Runtime
	uint32 NextSessionSerial = 1;

	// Participant Collision Runtime
	TWeakObjectPtr<ACharacter> MovementIgnoredExecutionPartner;

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

public:
	// Event
	FOnExecutionLethalDeathEntryExpected OnExecutionLethalDeathEntryExpected;

	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

public:
	// Query
	bool HasActiveExecutionSession() const;

	EExecutionCollaborationState GetExecutionCollaborationState() const { return CollaborationState; }
	FExecutionCollaborationRuntimeSnapshot GetExecutionCollaborationRuntimeSnapshot() const;
	EExternalCombatInputPolicy GetExternalCombatInputPolicy() const;

	bool QueryCombatExecutionAvailability(EExecutionAvailabilityBlock& OutBlock) const;
	bool BuildSourceExecutionStartGeometrySnapshot(FExecutionStartGeometrySnapshot& OutSnapshot) const;

	// Source Request
	bool RequestCombatExecution();

	// Source Commit
	bool HandleSourceExecutionCommit(uint32 InActionRequestSerial, float InStandardExecutionDamage);

	// Target Outcome
	bool CommitExecutionOutcome(const FExecutionOutcomePacket& InPacket);

private:
	// Start Evaluation
	struct FExecutionStartEvaluation
	{
		EExecutionAvailabilityBlock Block = EExecutionAvailabilityBlock::None;
		FCombatTargetSnapshot TargetSnapshot;
		UCExecutionCollaborationComponent* Target = nullptr;
		float StandardExecutionDamage = 0.f;
		FString FailureDetail;
	};

	bool EvaluateExecutionStart(FExecutionStartEvaluation& OutEvaluation) const;

	// Participant Event Binding
	void BindParticipantEvents();
	void UnbindParticipantEvents();

	// Participant Event Observation
	UFUNCTION()
	void HandleActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, uint32 InActionRequestSerial, EActionEventType InActionEventType);

	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);
	void HandleDeadStateChanged(EDeadState InPreviousState, EDeadState InNewState);
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);

	// Partner Coordination
	bool AcceptExecutionReservation(const FExecutionSessionId& InSessionId, const FCombatTargetSnapshot& InTargetSnapshot, float InStandardExecutionDamage, FExecutionCollaborationContext& OutContext);
	void ReceivePartnerSourceActionTerminal(const FExecutionSessionId& InSessionId);
	void ReceivePartnerTargetReactionTerminal(const FExecutionSessionId& InSessionId);
	void ReceivePartnerCommit(const FExecutionSessionId& InSessionId);
	void ReceivePartnerCancellation(const FExecutionSessionId& InSessionId, EExecutionCollaborationCancelReason InReason);

	// Session Startup
	bool AlignTargetExecutionFacing(const FCombatTargetSnapshot& InTargetSnapshot) const;
	bool StartTargetExecutionReaction();
	bool StartSourceExecutionAction();
	bool ActivateExecutionPair();

	// Session Termination
	void TryCompleteActiveExecutionSession();
	void CompleteActiveExecutionSession();
	void CancelActiveExecutionSession(EExecutionCollaborationCancelReason InReason, bool bNotifyPartner);
	void CancelLocalExecutionParticipant(bool bWasSourceRole, EReactionType InPrimaryReactionType);

	// Participant Movement Collision
	bool ApplyExecutionParticipantMovementIgnore(UCExecutionCollaborationComponent* InPartnerComponent);
	void RestoreExecutionParticipantMovementIgnore();

	// Session Validation
	bool IsActiveSession(const FExecutionSessionId& InSessionId) const;
	bool IsTargetSnapshotCurrent() const;
	bool IsTargetExecutionOpportunityCurrent() const;

	// Participant Validation
	bool CanStartSourceExecution(bool bLogFailure = true, FString* OutFailureDetail = nullptr) const;
	bool CanStartTargetExecution(bool bLogFailure = true, FString* OutFailureDetail = nullptr) const;

	// Execution Data
	bool CanResolveSourceExecutionAction(EExecutionOutcomePolicy InOutcomePolicy, bool bLogFailure = true, FString* OutFailureDetail = nullptr) const;
	bool CanResolveTargetExecutionReaction(EExecutionOutcomePolicy InOutcomePolicy, bool bLogFailure = true, FString* OutFailureDetail = nullptr) const;
	float ResolveStandardExecutionDamageForReservation() const;

	// Start Geometry
	bool IsSourceExecutionStartGeometryValid(const FCombatTargetSnapshot& InTargetSnapshot, bool bLogFailure = true, FString* OutFailureDetail = nullptr) const;

	// Outcome Policy
	EExecutionOutcomePolicy ResolveTargetExecutionOutcomePolicy() const;
	bool CanResolveLethalExecutionOutcome() const;

	// Execution Mapping
	EReactionType GetPrimaryReactionType() const;
	int32 GetExecutionActionIndex(EExecutionOutcomePolicy InOutcomePolicy) const;

	// Partner Lookup
	class UCExecutionCollaborationComponent* FindPartnerCollaborationComponent() const;

	// Session Runtime
	uint32 AllocateSessionSerial();
	void ResetActiveExecutionSession();
};
