#include "Component/CBalanceComponent.h"

#include "Core/Debug/FBalanceDebug.h"
#include "Type/CReactionDataTypes.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "Type/CReactionTypes.h"
#include "Type/CCombatResultTypes.h"

#include "Engine/World.h"
#include "TimerManager.h"

// Construction

UCBalanceComponent::UCBalanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Component Reference

void UCBalanceComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
}

// Lifecycle

void UCBalanceComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	ShutdownBalanceRuntime();
	Super::EndPlay(InEndPlayReason);
}

// Query: Balance State

float UCBalanceComponent::GetCollapseLoopRemainingSeconds() const
{
	if (!IsCollapseLoopActive()) return 0.f;

	const UWorld* world = GetWorld();
	return IsValid(world) ? FMath::Max(0.f, world->GetTimerManager().GetTimerRemaining(CollapseLoopTimerHandle)) : 0.f;
}

bool UCBalanceComponent::IsCollapsePoseActive() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::CollapseInActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseLoopActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseOutPending;
}

bool UCBalanceComponent::IsCollapseLoopActive() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::CollapseLoopActive;
}

bool UCBalanceComponent::IsExecutionOpportunityAvailable() const
{
	return IsCollapseLoopActive() && !ExecutionOpportunityReservation.IsValidMinimal();
}

bool UCBalanceComponent::IsExecutionOpportunityReservationCurrent(const FExecutionOpportunityReservation& InReservation) const
{
	return IsCollapseLoopActive() && ExecutionOpportunityReservation.Matches(InReservation);
}

bool UCBalanceComponent::IsBalanceLifecycleBlocking() const
{
	return BalanceLifecycleState != EBalanceLifecycleState::Accumulating;
}

bool UCBalanceComponent::ShouldSuppressCombatTargetFacing() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::CollapseInActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseLoopActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseOutPending
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseRecovering;
}

// Balance Result Ingress

FBalanceAdvanceResult UCBalanceComponent::AdvanceBalanceFromParry(const FCombatResultPacket& InPacket)
{
	FBalanceAdvanceResult result;
	result.PreviousCount = CurrentBalanceCount;
	result.CurrentCount = CurrentBalanceCount;
	result.Threshold = BalanceThreshold;
	result.BalanceLifecycleSerial = BalanceLifecycleSerial;

	if (!InPacket.IsParryResult() || InPacket.CombatSignalResultSerial == 0) return result;
	if (BalanceLifecycleState != EBalanceLifecycleState::Accumulating) return result;
	if (IsDuplicateParryPacket(InPacket)) return result;

	RememberAcceptedParryPacket(InPacket);

	const int32 threshold = FMath::Max(1, BalanceThreshold);
	CurrentBalanceCount = FMath::Min(CurrentBalanceCount + 1, threshold);

	result.CurrentCount = CurrentBalanceCount;
	result.Threshold = threshold;
	result.bThresholdCrossed = result.PreviousCount < threshold && CurrentBalanceCount >= threshold;

	if (!result.bThresholdCrossed)
	{
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("ParryAccepted"));
		return result;
	}

	++BalanceLifecycleSerial;
	if (BalanceLifecycleSerial == 0)
	{
		++BalanceLifecycleSerial;
	}

	result.BalanceLifecycleSerial = BalanceLifecycleSerial;
	LastAbortReason = EBalanceAbortReason::None;
	SetBalanceLifecycleState(EBalanceLifecycleState::CollapseInPending);
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ThresholdCrossed"));
	return result;
}

// Reaction Request Resolution

void UCBalanceComponent::HandleBalanceLifecycleReactionRequestResolved(const FBalanceLifecyclePacket& InBalanceLifecyclePacket, const FReactionRequestResult& InResult)
{
	FBalanceDebug::RecordLifecycleEvent(
		this,
		InResult.IsAccepted() ? TEXT("LifecycleRequestAccepted") : TEXT("LifecycleRequestRejected"),
		FString::Printf(TEXT("Reaction=%s"), *UEnum::GetValueAsString(InBalanceLifecyclePacket.ReactionType)));

	if (InResult.IsAccepted()) return;
	if (InBalanceLifecyclePacket.BalanceLifecycleSerial == 0 || InBalanceLifecyclePacket.BalanceLifecycleSerial != BalanceLifecycleSerial) return;

	if (InBalanceLifecyclePacket.ReactionType == EReactionType::CollapseIn
		&& BalanceLifecycleState == EBalanceLifecycleState::CollapseInPending)
	{
		AbortBalanceLifecycle(EBalanceAbortReason::CollapseInRejected);
		return;
	}

	if (InBalanceLifecyclePacket.ReactionType == EReactionType::CollapseOut
		&& BalanceLifecycleState == EBalanceLifecycleState::CollapseOutPending)
	{
		AbortBalanceLifecycle(EBalanceAbortReason::CollapseOutRejected);
	}
}

// Reaction Execution Lifecycle

bool UCBalanceComponent::HandleCollapseReactionExecutionStarted(const FReactionExecutionContext& InContext)
{
	if (MatchesLifecycleContext(InContext, EReactionType::CollapseIn))
	{
		if (BalanceLifecycleState != EBalanceLifecycleState::CollapseInPending) return false;
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseInStarted"));
		SetBalanceLifecycleState(EBalanceLifecycleState::CollapseInActive);
		return true;
	}

	if (MatchesLifecycleContext(InContext, EReactionType::CollapseOut))
	{
		if (BalanceLifecycleState != EBalanceLifecycleState::CollapseOutPending) return false;
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseOutStarted"));
		SetBalanceLifecycleState(EBalanceLifecycleState::CollapseRecovering);
		return true;
	}

	return false;
}

void UCBalanceComponent::HandleCollapseReactionExecutionTerminal(const FReactionExecutionLifecycleEvent& InEvent)
{
	const EReactionType reactionType = InEvent.Context.ReactionDataKey.ReactionType;
	if (reactionType != EReactionType::CollapseIn && reactionType != EReactionType::CollapseOut) return;
	if (!MatchesLifecycleContext(InEvent.Context, reactionType)) return;

	if (reactionType == EReactionType::CollapseIn)
	{
		if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed
			&& BalanceLifecycleState == EBalanceLifecycleState::CollapseInActive)
		{
			FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseInCompleted"));
			SetBalanceLifecycleState(EBalanceLifecycleState::CollapseLoopActive);
			StartCollapseLoopTimer();
			return;
		}

		if (BalanceLifecycleState != EBalanceLifecycleState::Accumulating)
		{
			AbortBalanceLifecycle(InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored
				? EBalanceAbortReason::CollapseInRejected
				: EBalanceAbortReason::CollapseInInterrupted);
		}
		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed
		&& BalanceLifecycleState == EBalanceLifecycleState::CollapseRecovering)
	{
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseOutCompletedWithoutReset"));
		AbortBalanceLifecycle(EBalanceAbortReason::ResetNotifyMissing);
		return;
	}

	if (BalanceLifecycleState == EBalanceLifecycleState::Accumulating) return;
	AbortBalanceLifecycle(InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored
		? EBalanceAbortReason::CollapseOutRejected
		: EBalanceAbortReason::CollapseOutInterrupted);
}

bool UCBalanceComponent::TryCommitCollapseReset(const uint32 InBalanceLifecycleSerial)
{
	if (InBalanceLifecycleSerial == 0 || InBalanceLifecycleSerial != BalanceLifecycleSerial) return false;
	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseRecovering) return false;

	FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseResetCommitted"));
	ResetBalanceRuntime();
	return true;
}

// Execution Opportunity Reservation

bool UCBalanceComponent::TryReserveExecutionOpportunity(const FExecutionSessionId& InSessionId, FExecutionOpportunityReservation& OutReservation)
{
	OutReservation = FExecutionOpportunityReservation();
	if (!InSessionId.IsValidMinimal() || !IsExecutionOpportunityAvailable()) return false;

	FExecutionOpportunityReservation reservation;
	reservation.SessionId = InSessionId;
	reservation.BalanceLifecycleSerial = BalanceLifecycleSerial;
	reservation.SuspendedLoopRemainingSeconds = GetCollapseLoopRemainingSeconds();

	ClearCollapseLoopTimer();
	ExecutionOpportunityReservation = reservation;
	OutReservation = reservation;

	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionOpportunityReserved"));
	return true;
}

bool UCBalanceComponent::ReleaseExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation)
{
	if (!ExecutionOpportunityReservation.Matches(InReservation)) return false;

	const float resumeDuration = ExecutionOpportunityReservation.SuspendedLoopRemainingSeconds;
	ClearExecutionOpportunityReservation();

	if (IsCollapseLoopActive())
	{
		StartCollapseLoopTimer(resumeDuration);
	}

	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionOpportunityReleased"));
	return true;
}

bool UCBalanceComponent::ConsumeExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation)
{
	if (!ExecutionOpportunityReservation.Matches(InReservation)) return false;

	ClearCollapseLoopTimer();
	ClearExecutionOpportunityReservation();
	RequestCollapseOutFromExecutionConsume();

	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionOpportunityConsumed"));
	return true;
}

// Lifecycle Release

void UCBalanceComponent::AbortBalanceLifecycle(const EBalanceAbortReason InReason)
{
	if (BalanceLifecycleState == EBalanceLifecycleState::Accumulating) return;

	LastAbortReason = InReason;
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("LifecycleAborted"), FString::Printf(TEXT("Reason=%s"), *UEnum::GetValueAsString(InReason)));
	ResetBalanceRuntime();
}

void UCBalanceComponent::ShutdownBalanceRuntime()
{
	ClearCollapseLoopTimer();
	ClearExecutionOpportunityReservation();
	CurrentBalanceCount = 0;
	BalanceLifecycleState = EBalanceLifecycleState::Accumulating;
	LastAcceptedParryResultSerialByTarget.Reset();
}

// Lifecycle State Transition

bool UCBalanceComponent::MatchesLifecycleContext(const FReactionExecutionContext& InContext, const EReactionType InReactionType) const
{
	return InContext.BalanceLifecycleSerial != 0
		&& InContext.BalanceLifecycleSerial == BalanceLifecycleSerial
		&& InContext.ReactionDataKey.ReactionType == InReactionType;
}

void UCBalanceComponent::SetBalanceLifecycleState(const EBalanceLifecycleState InState)
{
	if (BalanceLifecycleState == InState) return;

	const EBalanceLifecycleState previousState = BalanceLifecycleState;
	BalanceLifecycleState = InState;
	FBalanceDebug::RecordLifecycleStateChanged(this, previousState, BalanceLifecycleState);
	OnBalanceLifecycleStateChanged.Broadcast(previousState, BalanceLifecycleState);
}

// Collapse Loop Timer

void UCBalanceComponent::StartCollapseLoopTimer(const float InDurationSeconds)
{
	ClearCollapseLoopTimer();

	UWorld* world = GetWorld();
	if (!IsValid(world))
	{
		AbortBalanceLifecycle(EBalanceAbortReason::CollapseInInterrupted);
		return;
	}

	const float durationSeconds = InDurationSeconds >= 0.f ? InDurationSeconds : CollapseLoopDuration;
	if (durationSeconds <= 0.f)
	{
		HandleCollapseLoopExpired();
		return;
	}

	world->GetTimerManager().SetTimer(CollapseLoopTimerHandle, this, &UCBalanceComponent::HandleCollapseLoopExpired, durationSeconds, false);
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("LoopTimerArmed"));
}

void UCBalanceComponent::ClearCollapseLoopTimer()
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(CollapseLoopTimerHandle);
	}

	CollapseLoopTimerHandle.Invalidate();
}

void UCBalanceComponent::HandleCollapseLoopExpired()
{
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("LoopTimerExpired"));
	RequestCollapseOutFromLoopExpiry();
}

void UCBalanceComponent::RequestCollapseOutFromLoopExpiry()
{
	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseLoopActive) return;

	ClearCollapseLoopTimer();
	SetBalanceLifecycleState(EBalanceLifecycleState::CollapseOutPending);

	FBalanceLifecyclePacket balanceLifecyclePacket;
	balanceLifecyclePacket.ReactionType = EReactionType::CollapseOut;
	balanceLifecyclePacket.BalanceLifecycleSerial = BalanceLifecycleSerial;
	OnBalanceLifecycleReactionRequested.Broadcast(balanceLifecyclePacket);
}

void UCBalanceComponent::RequestCollapseOutFromExecutionConsume()
{
	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseLoopActive) return;

	SetBalanceLifecycleState(EBalanceLifecycleState::CollapseOutPending);

	FBalanceLifecyclePacket balanceLifecyclePacket;
	balanceLifecyclePacket.ReactionType = EReactionType::CollapseOut;
	balanceLifecyclePacket.BalanceLifecycleSerial = BalanceLifecycleSerial;
	OnBalanceLifecycleReactionRequested.Broadcast(balanceLifecyclePacket);
}

void UCBalanceComponent::ClearExecutionOpportunityReservation()
{
	ExecutionOpportunityReservation = FExecutionOpportunityReservation();
}

void UCBalanceComponent::ResetBalanceRuntime()
{
	ClearCollapseLoopTimer();
	ClearExecutionOpportunityReservation();
	CurrentBalanceCount = 0;
	SetBalanceLifecycleState(EBalanceLifecycleState::Accumulating);
}

// Packet Deduplication

bool UCBalanceComponent::IsDuplicateParryPacket(const FCombatResultPacket& InPacket) const
{
	if (!IsValid(InPacket.TargetActor)) return true;

	const uint64* lastSerial = LastAcceptedParryResultSerialByTarget.Find(InPacket.TargetActor);
	return lastSerial && InPacket.CombatSignalResultSerial <= *lastSerial;
}

void UCBalanceComponent::RememberAcceptedParryPacket(const FCombatResultPacket& InPacket)
{
	if (!IsValid(InPacket.TargetActor)) return;
	LastAcceptedParryResultSerialByTarget.Add(InPacket.TargetActor, InPacket.CombatSignalResultSerial);
}
