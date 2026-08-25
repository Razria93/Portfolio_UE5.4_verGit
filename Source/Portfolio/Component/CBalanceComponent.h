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

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBalanceLifecycleStateChanged, EBalanceLifecycleState, EBalanceLifecycleState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBalanceLifecycleReactionRequested, const FBalanceLifecyclePacket&);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCBalanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Construction
	UCBalanceComponent();

private:
	// Balance Config
	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 1))
	int32 BalanceThreshold = 3;

	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 0.0))
	float CollapseLoopDuration = 5.f;

	// Runtime State
	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	int32 CurrentBalanceCount = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	uint32 BalanceLifecycleSerial = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	EBalanceLifecycleState BalanceLifecycleState = EBalanceLifecycleState::Accumulating;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	EBalanceAbortReason LastAbortReason = EBalanceAbortReason::None;

	// Timer Runtime
	FTimerHandle CollapseLoopTimerHandle;

	// Execution Opportunity Runtime
	FExecutionOpportunityReservation ExecutionOpportunityReservation;

	// Result Deduplication Runtime
	TMap<TWeakObjectPtr<class AActor>, uint64> LastAcceptedParryResultSerialByTarget;

public:
	// Component Reference
	// Intentionally empty: participates in the shared component reference-injection contract.
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

public:
	// Events
	FOnBalanceLifecycleStateChanged OnBalanceLifecycleStateChanged;
	FOnBalanceLifecycleReactionRequested OnBalanceLifecycleReactionRequested;

public:
	// Query: Balance State
	int32 GetCurrentBalanceCount() const { return CurrentBalanceCount; }
	int32 GetBalanceThreshold() const { return BalanceThreshold; }
	uint32 GetBalanceLifecycleSerial() const { return BalanceLifecycleSerial; }
	EBalanceLifecycleState GetBalanceLifecycleState() const { return BalanceLifecycleState; }
	EBalanceAbortReason GetLastAbortReason() const { return LastAbortReason; }
	float GetCollapseLoopDuration() const { return CollapseLoopDuration; }
	float GetCollapseLoopRemainingSeconds() const;

	bool IsCollapsePoseActive() const;
	bool IsCollapseLoopActive() const;
	bool IsExecutionOpportunityAvailable() const;
	bool IsExecutionOpportunityReservationCurrent(const FExecutionOpportunityReservation& InReservation) const;
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
	bool HandleCollapseReactionExecutionStarted(const struct FReactionExecutionContext& InContext);
	void HandleCollapseReactionExecutionTerminal(const FReactionExecutionLifecycleEvent& InEvent);
	bool TryCommitCollapseReset(uint32 InBalanceLifecycleSerial);

public:
	// Execution Opportunity Reservation
	bool TryReserveExecutionOpportunity(const FExecutionSessionId& InSessionId, FExecutionOpportunityReservation& OutReservation);
	bool ReleaseExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation);
	bool ConsumeExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation);

public:
	// Lifecycle Release
	void AbortBalanceLifecycle(EBalanceAbortReason InReason);
	void ShutdownBalanceRuntime();

private:
	// Lifecycle State Transition
	bool MatchesLifecycleContext(const struct FReactionExecutionContext& InContext, EReactionType InReactionType) const;
	void SetBalanceLifecycleState(EBalanceLifecycleState InState);
	void ResetBalanceRuntime();

private:
	// Collapse Loop Timer
	void StartCollapseLoopTimer(float InDurationSeconds = -1.f);
	void ClearCollapseLoopTimer();
	void HandleCollapseLoopExpired();
	void RequestCollapseOutFromLoopExpiry();
	void RequestCollapseOutFromExecutionConsume();

	// Execution Opportunity Runtime
	void ClearExecutionOpportunityReservation();

private:
	// Packet Deduplication
	bool IsDuplicateParryPacket(const struct FCombatResultPacket& InPacket) const;
	void RememberAcceptedParryPacket(const struct FCombatResultPacket& InPacket);
};
