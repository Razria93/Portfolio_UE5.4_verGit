#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CBalanceTypes.h"
#include "Type/CExecutionCollaborationTypes.h"
#include "Type/CReactionTypes.h"
#include "CBalanceComponent.generated.h"

struct FCharacterComponentReferences;
struct FReactionExecutionLifecycleEvent;
struct FReactionRequestResult;

DECLARE_MULTICAST_DELEGATE(FOnBalanceValuesChanged);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBalanceLifecycleStateChanged, EBalanceLifecycleState, EBalanceLifecycleState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnIncapacitatedPresentationChanged, EIncapacitatedPresentation);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExecutionDownPresentationChanged, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBalanceLifecycleReactionRequested, const FBalanceLifecyclePacket&);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCBalanceComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class FBalanceLifecycleBoundaryTest;

public:
	// Construction
	UCBalanceComponent();

private:
	// Config
	// Balance
	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 1))
	int32 BalanceThreshold = 3;

	// Collapse
	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 0.0))
	float CollapseLoopDuration = 5.f;

	// Execution Recovery
	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 0.0))
	float ExecutionDownDuration = 3.f;

	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 0.0))
	float ExecutionRecoveryRetryDelay = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 0))
	int32 MaxExecutionRecoveryRetryCount = 2;

private:
	// Runtime
	// Balance Value Runtime
	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	int32 CurrentBalanceCount = 0;

	// Lifecycle Identity, State and Diagnostics
	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	uint32 BalanceLifecycleSerial = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	EBalanceLifecycleState BalanceLifecycleState = EBalanceLifecycleState::Accumulating;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	EBalanceAbortReason LastAbortReason = EBalanceAbortReason::None;

	// Incapacitated Presentation Runtime
	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	EIncapacitatedPresentation IncapacitatedPresentation = EIncapacitatedPresentation::None;

	// Timer Runtime
	FTimerHandle CollapseLoopTimerHandle;
	FTimerHandle ExecutionDownTimerHandle;
	FTimerHandle ExecutionRecoveryRetryTimerHandle;

	// Execution Recovery Runtime
	int32 ExecutionRecoveryRetryCount = 0;

	// Execution Opportunity Runtime
	FExecutionOpportunityReservation ExecutionOpportunityReservation;

	// Result Deduplication Runtime
	TMap<TWeakObjectPtr<class AActor>, uint64> LastAcceptedParryResultSerialByTarget;

public:
	// Balance Observation
	FOnBalanceValuesChanged OnBalanceValuesChanged;
	FOnBalanceLifecycleStateChanged OnBalanceLifecycleStateChanged;

	// Presentation Observation
	FOnIncapacitatedPresentationChanged OnIncapacitatedPresentationChanged;
	FOnExecutionDownPresentationChanged OnExecutionDownPresentationChanged;

	// Reaction Request
	FOnBalanceLifecycleReactionRequested OnBalanceLifecycleReactionRequested;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

public:
	// Query: Balance State
	int32 GetCurrentBalanceCount() const { return CurrentBalanceCount; }
	int32 GetBalanceThreshold() const { return BalanceThreshold; }
	uint32 GetBalanceLifecycleSerial() const { return BalanceLifecycleSerial; }
	EBalanceLifecycleState GetBalanceLifecycleState() const { return BalanceLifecycleState; }
	EBalanceAbortReason GetLastAbortReason() const { return LastAbortReason; }

	bool IsCollapseActive() const;
	bool IsCollapseLoopActive() const;
	bool IsExecutionDownActive() const;

	// Query: Timers
	float GetCollapseLoopDuration() const { return CollapseLoopDuration; }
	float GetCollapseLoopRemainingSeconds() const;

	float GetExecutionDownDuration() const { return ExecutionDownDuration; }
	float GetExecutionDownRemainingSeconds() const;

	// Query: Presentation
	EIncapacitatedPresentation GetIncapacitatedPresentation() const { return IncapacitatedPresentation; }
	bool IsCollapsePresentationActive() const { return IncapacitatedPresentation == EIncapacitatedPresentation::Collapse; }
	bool IsExecutionDownPresentationActive() const { return IncapacitatedPresentation == EIncapacitatedPresentation::ExecutionDown; }
	bool ShouldUseExecutionDownPose() const;

	// Query: Execution Opportunity
	bool IsExecutionOpportunityAvailable() const;
	bool IsExecutionOpportunityReservationCurrent(const FExecutionOpportunityReservation& InReservation) const;

	// Query: Combat Policy
	bool IsBalanceLifecycleBlocking() const;
	bool ShouldSuppressCombatTargetFacing() const;

public:
	// Balance Result Ingress
	FBalanceAdvanceResult AdvanceBalanceFromParry(const struct FCombatResultPacket& InPacket);

public:
	// Reaction Request Resolution
	void HandleBalanceLifecycleReactionRequestResolved(const FBalanceLifecyclePacket& InBalanceLifecyclePacket, const FReactionRequestResult& InResult);

public:
	// Reaction Execution Lifecycle
	bool HandleBalanceLifecycleReactionExecutionStarted(const struct FReactionExecutionContext& InContext);
	void HandleBalanceLifecycleReactionExecutionTerminal(const FReactionExecutionLifecycleEvent& InEvent);
	bool TryCommitBalanceLifecycleReset(const struct FReactionExecutionContext& InContext);

public:
	// Incapacitated Presentation
	bool TrySetIncapacitatedPresentation(const struct FReactionExecutionContext& InContext, EIncapacitatedPresentation InPresentation);

public:
	// Execution Opportunity Reservation
	bool TryReserveExecutionOpportunity(const FExecutionSessionId& InSessionId, FExecutionOpportunityReservation& OutReservation);
	bool ActivateExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation);
	bool ReleaseExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation);
	bool CommitExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation);

public:
	// Execution Down Lifecycle
	bool EnterExecutionDownLifecycle(uint32 InBalanceLifecycleSerial);

public:
	// Lifecycle Release
	void AbortBalanceLifecycle(EBalanceAbortReason InReason);
	void ShutdownBalanceRuntime();

private:
	// Lifecycle Context Validation
	bool MatchesLifecycleContext(const struct FReactionExecutionContext& InContext, EReactionType InReactionType) const;

	// Lifecycle State Transition
	void SetBalanceLifecycleState(EBalanceLifecycleState InState);
	void NotifyBalanceLifecycleStateChanged(EBalanceLifecycleState InPreviousState);

	// Balance Value Notification
	void NotifyBalanceValuesChanged(int32 InPreviousCount);

	// Runtime Reset
	void ClearBalanceTransientResources();
	void ResetBalanceRuntime();

private:
	// Collapse Loop and Exit
	void StartCollapseLoopTimer(float InDurationSeconds = -1.f);
	void ClearCollapseLoopTimer();
	void HandleCollapseLoopExpired();
	void RequestCollapseOutFromLoopExpiry();

	// Execution Recovery
	void StartExecutionDownTimer();
	void ClearExecutionDownTimer();
	void HandleExecutionDownExpired();
	void RequestExecutionRecovery();
	void HandleExecutionRecoveryFailure(EBalanceAbortReason InReason);
	void ClearExecutionRecoveryRetryTimer();

	// Execution Opportunity Runtime
	void ClearExecutionOpportunityReservation();

	// Incapacitated Presentation Runtime
	void SetIncapacitatedPresentation(EIncapacitatedPresentation InPresentation);
	void NotifyIncapacitatedPresentationChanged(EIncapacitatedPresentation InPreviousPresentation);

private:
	// Packet Deduplication
	bool IsDuplicateParryPacket(const struct FCombatResultPacket& InPacket) const;
	void RememberAcceptedParryPacket(const struct FCombatResultPacket& InPacket);
};
