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

float UCBalanceComponent::GetExecutionDownRemainingSeconds() const
{
	if (!IsExecutionDownActive()) return 0.f;

	const UWorld* world = GetWorld();
	return IsValid(world) ? FMath::Max(0.f, world->GetTimerManager().GetTimerRemaining(ExecutionDownTimerHandle)) : 0.f;
}

bool UCBalanceComponent::IsCollapseActive() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::CollapseInActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseLoopActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseOutPending
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionPrimaryActive
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionPrimaryCommitted;
}

bool UCBalanceComponent::IsCollapseLoopActive() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::CollapseLoopActive;
}

bool UCBalanceComponent::IsExecutionDownActive() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::ExecutionDownActive;
}

bool UCBalanceComponent::ShouldUseExecutionDownPose() const
{
	return IncapacitatedPresentation == EIncapacitatedPresentation::ExecutionDown;
}

bool UCBalanceComponent::IsExecutionOpportunityAvailable() const
{
	return IsCollapseLoopActive() && !ExecutionOpportunityReservation.IsValidMinimal();
}

bool UCBalanceComponent::IsExecutionOpportunityReservationCurrent(const FExecutionOpportunityReservation& InReservation) const
{
	return (BalanceLifecycleState == EBalanceLifecycleState::CollapseLoopActive
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionPrimaryActive)
		&& ExecutionOpportunityReservation.Matches(InReservation);
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
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseOutActive
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionPrimaryActive
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionPrimaryCommitted
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionDownActive
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionRecoveryPending
		|| BalanceLifecycleState == EBalanceLifecycleState::ExecutionRecoveryActive;
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
		return;
	}

	if (InBalanceLifecyclePacket.ReactionType == EReactionType::ExecutionRecovery
		&& BalanceLifecycleState == EBalanceLifecycleState::ExecutionRecoveryPending)
	{
		HandleExecutionRecoveryFailure(EBalanceAbortReason::ExecutionRecoveryRejected);
	}
}

// Reaction Execution Lifecycle

bool UCBalanceComponent::HandleBalanceLifecycleReactionExecutionStarted(const FReactionExecutionContext& InContext)
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
		SetBalanceLifecycleState(EBalanceLifecycleState::CollapseOutActive);
		return true;
	}

	if (MatchesLifecycleContext(InContext, EReactionType::ExecutionRecovery))
	{
		if (BalanceLifecycleState != EBalanceLifecycleState::ExecutionRecoveryPending) return false;
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionRecoveryStarted"));
		SetBalanceLifecycleState(EBalanceLifecycleState::ExecutionRecoveryActive);
		return true;
	}

	return false;
}

void UCBalanceComponent::HandleBalanceLifecycleReactionExecutionTerminal(const FReactionExecutionLifecycleEvent& InEvent)
{
	const EReactionType reactionType = InEvent.Context.ReactionDataKey.ReactionType;
	if (reactionType != EReactionType::CollapseIn
		&& reactionType != EReactionType::CollapseOut
		&& reactionType != EReactionType::ExecutionRecovery)
	{
		return;
	}

	if (!MatchesLifecycleContext(InEvent.Context, reactionType)) return;

	if (reactionType == EReactionType::CollapseIn)
	{
		if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed && BalanceLifecycleState == EBalanceLifecycleState::CollapseInActive)
		{
			FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseInCompleted"));
			SetBalanceLifecycleState(EBalanceLifecycleState::CollapseLoopActive);
			StartCollapseLoopTimer();
			return;
		}

		if (BalanceLifecycleState != EBalanceLifecycleState::Accumulating)
		{
			AbortBalanceLifecycle(InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored ? EBalanceAbortReason::CollapseInRejected : EBalanceAbortReason::CollapseInInterrupted);
		}

		return;
	}

	if (reactionType == EReactionType::ExecutionRecovery)
	{
		if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed
			&& BalanceLifecycleState == EBalanceLifecycleState::ExecutionRecoveryActive)
		{
			FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionRecoveryCompleted"));
			ResetBalanceRuntime();
			return;
		}

		if (BalanceLifecycleState != EBalanceLifecycleState::Accumulating)
		{
			HandleExecutionRecoveryFailure(InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored
				? EBalanceAbortReason::ExecutionRecoveryRejected
				: EBalanceAbortReason::ExecutionRecoveryInterrupted);
		}

		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed
		&& BalanceLifecycleState == EBalanceLifecycleState::CollapseOutActive)
	{
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseOutCompletedWithoutReset"));
		AbortBalanceLifecycle(EBalanceAbortReason::ResetNotifyMissing);
		return;
	}

	if (BalanceLifecycleState == EBalanceLifecycleState::Accumulating) return;
	AbortBalanceLifecycle(InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored ? EBalanceAbortReason::CollapseOutRejected : EBalanceAbortReason::CollapseOutInterrupted);
}

bool UCBalanceComponent::TryCommitBalanceLifecycleReset(const FReactionExecutionContext& InContext)
{
	const EReactionType reactionType = InContext.ReactionDataKey.ReactionType;
	if (reactionType != EReactionType::CollapseOut) return false;
	if (!MatchesLifecycleContext(InContext, reactionType)) return false;

	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseOutActive) return false;

	FBalanceDebug::RecordLifecycleEvent(this, TEXT("CollapseResetCommitted"));
	ResetBalanceRuntime();
	return true;
}

// Incapacitated Presentation

bool UCBalanceComponent::TrySetIncapacitatedPresentation(const FReactionExecutionContext& InContext, const EIncapacitatedPresentation InPresentation)
{
	FBalanceDebug::RecordLifecycleEvent(
		this,
		TEXT("IncapacitatedPresentationRequested"),
		FString::Printf(
			TEXT("Target=%s | ContextReaction=%s | ContextLifecycle=%u | Current=%s"),
			*UEnum::GetValueAsString(InPresentation),
			*UEnum::GetValueAsString(InContext.ReactionDataKey.ReactionType),
			InContext.BalanceLifecycleSerial,
			*UEnum::GetValueAsString(IncapacitatedPresentation)));

	if (InPresentation == EIncapacitatedPresentation::None)
	{
		const bool bIsCollapseOut = MatchesLifecycleContext(InContext, EReactionType::CollapseOut)
			&& BalanceLifecycleState == EBalanceLifecycleState::CollapseOutActive;
		const bool bIsExecutionRecovery = MatchesLifecycleContext(InContext, EReactionType::ExecutionRecovery)
			&& BalanceLifecycleState == EBalanceLifecycleState::ExecutionRecoveryActive;
		if (!bIsCollapseOut && !bIsExecutionRecovery)
		{
			FBalanceDebug::RecordLifecycleEvent(this, TEXT("IncapacitatedPresentationRejected"), TEXT("Reason=NoneRequiresActiveCollapseOutOrExecutionRecovery"));
			return false;
		}
	}
	else if (InPresentation == EIncapacitatedPresentation::Collapse)
	{
		if (!MatchesLifecycleContext(InContext, EReactionType::CollapseIn)
			|| BalanceLifecycleState != EBalanceLifecycleState::CollapseInActive)
		{
			FBalanceDebug::RecordLifecycleEvent(this, TEXT("IncapacitatedPresentationRejected"), TEXT("Reason=CollapseRequiresActiveCollapseIn"));
			return false;
		}
	}
	else if (InPresentation == EIncapacitatedPresentation::ExecutionDown)
	{
		if (!MatchesLifecycleContext(InContext, EReactionType::ExecutionStandard)
			|| BalanceLifecycleState != EBalanceLifecycleState::ExecutionPrimaryCommitted)
		{
			FBalanceDebug::RecordLifecycleEvent(this, TEXT("IncapacitatedPresentationRejected"), TEXT("Reason=ExecutionDownRequiresCommittedStandardExecution"));
			return false;
		}
	}
	else
	{
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("IncapacitatedPresentationRejected"), TEXT("Reason=InvalidPresentation"));
		return false;
	}

	SetIncapacitatedPresentation(InPresentation);
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("IncapacitatedPresentationAccepted"));
	return true;
}

bool UCBalanceComponent::TryEnterExecutionDownPresentation(const FReactionExecutionContext& InContext)
{
	return TrySetIncapacitatedPresentation(InContext, EIncapacitatedPresentation::ExecutionDown);
}

bool UCBalanceComponent::TryExitExecutionDownPresentation(const FReactionExecutionContext& InContext)
{
	return TrySetIncapacitatedPresentation(InContext, EIncapacitatedPresentation::None);
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

bool UCBalanceComponent::ActivateExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation)
{
	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseLoopActive) return false;
	if (!ExecutionOpportunityReservation.Matches(InReservation)) return false;

	SetBalanceLifecycleState(EBalanceLifecycleState::ExecutionPrimaryActive);
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionOpportunityActivated"));
	return true;
}

bool UCBalanceComponent::ReleaseExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation)
{
	if (!ExecutionOpportunityReservation.Matches(InReservation)) return false;

	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseLoopActive
		&& BalanceLifecycleState != EBalanceLifecycleState::ExecutionPrimaryActive)
	{
		return false;
	}

	const float resumeDuration = ExecutionOpportunityReservation.SuspendedLoopRemainingSeconds;

	ClearExecutionOpportunityReservation();
	SetIncapacitatedPresentation(EIncapacitatedPresentation::None);

	if (BalanceLifecycleState == EBalanceLifecycleState::ExecutionPrimaryActive)
	{
		SetBalanceLifecycleState(EBalanceLifecycleState::CollapseLoopActive);
	}

	StartCollapseLoopTimer(resumeDuration);

	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionOpportunityReleased"));
	return true;
}

bool UCBalanceComponent::CommitExecutionOpportunityReservation(const FExecutionOpportunityReservation& InReservation)
{
	if (BalanceLifecycleState != EBalanceLifecycleState::ExecutionPrimaryActive) return false;
	if (!ExecutionOpportunityReservation.Matches(InReservation)) return false;

	ClearCollapseLoopTimer();
	ClearExecutionOpportunityReservation();
	SetBalanceLifecycleState(EBalanceLifecycleState::ExecutionPrimaryCommitted);

	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionOpportunityCommitted"));
	return true;
}

bool UCBalanceComponent::EnterExecutionDownLifecycle(const uint32 InBalanceLifecycleSerial)
{
	if (InBalanceLifecycleSerial == 0 || InBalanceLifecycleSerial != BalanceLifecycleSerial) return false;
	if (BalanceLifecycleState != EBalanceLifecycleState::ExecutionPrimaryCommitted) return false;

	ClearExecutionRecoveryRetryTimer();
	ExecutionRecoveryRetryCount = 0;
	SetBalanceLifecycleState(EBalanceLifecycleState::ExecutionDownActive);
	StartExecutionDownTimer();
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionDownEntered"));
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
	ClearExecutionDownTimer();
	ClearExecutionRecoveryRetryTimer();
	ExecutionRecoveryRetryCount = 0;
	ClearExecutionOpportunityReservation();
	SetIncapacitatedPresentation(EIncapacitatedPresentation::None);
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


void UCBalanceComponent::StartExecutionDownTimer()
{
	ClearExecutionDownTimer();

	UWorld* world = GetWorld();
	if (!IsValid(world))
	{
		AbortBalanceLifecycle(EBalanceAbortReason::ExecutionRecoveryInterrupted);
		return;
	}

	if (ExecutionDownDuration <= 0.f)
	{
		HandleExecutionDownExpired();
		return;
	}

	world->GetTimerManager().SetTimer(ExecutionDownTimerHandle, this, &UCBalanceComponent::HandleExecutionDownExpired, ExecutionDownDuration, false);
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionDownTimerArmed"));
}

void UCBalanceComponent::ClearExecutionDownTimer()
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(ExecutionDownTimerHandle);
	}

	ExecutionDownTimerHandle.Invalidate();
}

void UCBalanceComponent::HandleExecutionDownExpired()
{
	FBalanceDebug::RecordLifecycleEvent(this, TEXT("ExecutionDownTimerExpired"));
	RequestExecutionRecovery();
}

void UCBalanceComponent::RequestExecutionRecovery()
{
	if (!IsExecutionDownActive()) return;

	ClearExecutionDownTimer();
	SetBalanceLifecycleState(EBalanceLifecycleState::ExecutionRecoveryPending);

	FBalanceLifecyclePacket balanceLifecyclePacket;
	balanceLifecyclePacket.ReactionType = EReactionType::ExecutionRecovery;
	balanceLifecyclePacket.BalanceLifecycleSerial = BalanceLifecycleSerial;
	OnBalanceLifecycleReactionRequested.Broadcast(balanceLifecyclePacket);
}

void UCBalanceComponent::ClearExecutionOpportunityReservation()
{
	ExecutionOpportunityReservation = FExecutionOpportunityReservation();
}

void UCBalanceComponent::HandleExecutionRecoveryFailure(const EBalanceAbortReason InReason)
{
	if (BalanceLifecycleState != EBalanceLifecycleState::ExecutionRecoveryPending
		&& BalanceLifecycleState != EBalanceLifecycleState::ExecutionRecoveryActive)
	{
		return;
	}

	const int32 maxRetryCount = FMath::Max(0, MaxExecutionRecoveryRetryCount);
	UWorld* world = GetWorld();
	if (ExecutionRecoveryRetryCount >= maxRetryCount || !IsValid(world))
	{
		AbortBalanceLifecycle(InReason);
		return;
	}

	++ExecutionRecoveryRetryCount;
	SetBalanceLifecycleState(EBalanceLifecycleState::ExecutionDownActive);
	SetIncapacitatedPresentation(EIncapacitatedPresentation::ExecutionDown);
	ClearExecutionRecoveryRetryTimer();

	const float retryDelay = FMath::Max(0.f, ExecutionRecoveryRetryDelay);
	world->GetTimerManager().SetTimer(
		ExecutionRecoveryRetryTimerHandle,
		this,
		&UCBalanceComponent::RequestExecutionRecovery,
		retryDelay,
		false);

	FBalanceDebug::RecordLifecycleEvent(
		this,
		TEXT("ExecutionRecoveryRetryScheduled"),
		FString::Printf(TEXT("Reason=%s | Attempt=%d/%d | Delay=%.2f"), *UEnum::GetValueAsString(InReason), ExecutionRecoveryRetryCount, maxRetryCount, retryDelay));
}

void UCBalanceComponent::ClearExecutionRecoveryRetryTimer()
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(ExecutionRecoveryRetryTimerHandle);
	}

	ExecutionRecoveryRetryTimerHandle.Invalidate();
}

void UCBalanceComponent::SetIncapacitatedPresentation(const EIncapacitatedPresentation InPresentation)
{
	if (IncapacitatedPresentation == InPresentation)
	{
		FBalanceDebug::RecordLifecycleEvent(this, TEXT("IncapacitatedPresentationUnchanged"), FString::Printf(TEXT("Presentation=%s"), *UEnum::GetValueAsString(InPresentation)));
		return;
	}

	const EIncapacitatedPresentation previousPresentation = IncapacitatedPresentation;
	IncapacitatedPresentation = InPresentation;
	FBalanceDebug::RecordLifecycleEvent(
		this,
		TEXT("IncapacitatedPresentationChanged"),
		FString::Printf(
			TEXT("Previous=%s | New=%s"),
			*UEnum::GetValueAsString(previousPresentation),
			*UEnum::GetValueAsString(IncapacitatedPresentation)));
	OnIncapacitatedPresentationChanged.Broadcast(IncapacitatedPresentation);
	OnExecutionDownPresentationChanged.Broadcast(IsExecutionDownPresentationActive());
}

void UCBalanceComponent::ResetBalanceRuntime()
{
	ClearCollapseLoopTimer();
	ClearExecutionDownTimer();
	ClearExecutionRecoveryRetryTimer();
	ExecutionRecoveryRetryCount = 0;
	ClearExecutionOpportunityReservation();
	SetIncapacitatedPresentation(EIncapacitatedPresentation::None);
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
