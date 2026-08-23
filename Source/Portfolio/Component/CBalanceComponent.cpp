#include "Component/CBalanceComponent.h"

#include "Type/CReactionDataTypes.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "Type/CReactionTypes.h"
#include "Type/CCombatResultTypes.h"

#include "Engine/World.h"
#include "TimerManager.h"

UCBalanceComponent::UCBalanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCBalanceComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
}

FBalanceAdvanceResult UCBalanceComponent::AdvanceFromParry(const FCombatResultPacket& InPacket)
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

	if (!result.bThresholdCrossed) return result;

	++BalanceLifecycleSerial;
	if (BalanceLifecycleSerial == 0)
	{
		++BalanceLifecycleSerial;
	}

	result.BalanceLifecycleSerial = BalanceLifecycleSerial;
	LastAbortReason = EBalanceAbortReason::None;
	SetBalanceLifecycleState(EBalanceLifecycleState::CollapseInPending);
	return result;
}

void UCBalanceComponent::HandleBalanceLifecycleReactionResolved(const FBalanceLifecyclePacket& InBalanceLifecyclePacket, const FReactionRequestResult& InResult)
{
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

bool UCBalanceComponent::HandleCollapseReactionStarted(const FReactionExecutionContext& InContext)
{
	if (MatchesLifecycleContext(InContext, EReactionType::CollapseIn))
	{
		if (BalanceLifecycleState != EBalanceLifecycleState::CollapseInPending) return false;
		SetBalanceLifecycleState(EBalanceLifecycleState::CollapseActive);
		return true;
	}

	if (MatchesLifecycleContext(InContext, EReactionType::CollapseOut))
	{
		if (BalanceLifecycleState != EBalanceLifecycleState::CollapseOutPending) return false;
		SetBalanceLifecycleState(EBalanceLifecycleState::CollapseRecovering);
		return true;
	}

	return false;
}

void UCBalanceComponent::HandleCollapseReactionTerminal(const FReactionExecutionLifecycleEvent& InEvent)
{
	const EReactionType reactionType = InEvent.Context.ReactionDataKey.ReactionType;
	if (reactionType != EReactionType::CollapseIn && reactionType != EReactionType::CollapseOut) return;
	if (!MatchesLifecycleContext(InEvent.Context, reactionType)) return;

	if (reactionType == EReactionType::CollapseIn)
	{
		if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed
			&& BalanceLifecycleState == EBalanceLifecycleState::CollapseActive)
		{
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

	if (BalanceLifecycleState == EBalanceLifecycleState::Accumulating) return;
	AbortBalanceLifecycle(InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored
		? EBalanceAbortReason::CollapseOutRejected
		: EBalanceAbortReason::CollapseOutInterrupted);
}

bool UCBalanceComponent::BeginCollapseOutRequest(const uint32 InBalanceLifecycleSerial)
{
	if (InBalanceLifecycleSerial == 0 || InBalanceLifecycleSerial != BalanceLifecycleSerial) return false;
	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseActive) return false;

	ClearCollapseLoopTimer();
	SetBalanceLifecycleState(EBalanceLifecycleState::CollapseOutPending);
	return true;
}

bool UCBalanceComponent::CommitCollapseReset(const uint32 InBalanceLifecycleSerial)
{
	if (InBalanceLifecycleSerial == 0 || InBalanceLifecycleSerial != BalanceLifecycleSerial) return false;
	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseRecovering) return false;

	ResetBalanceRuntime();
	return true;
}

void UCBalanceComponent::AbortBalanceLifecycle(const EBalanceAbortReason InReason)
{
	if (BalanceLifecycleState == EBalanceLifecycleState::Accumulating) return;

	LastAbortReason = InReason;
	ResetBalanceRuntime();
}

void UCBalanceComponent::ShutdownBalanceRuntime()
{
	ClearCollapseLoopTimer();
	CurrentBalanceCount = 0;
	BalanceLifecycleState = EBalanceLifecycleState::Accumulating;
}

bool UCBalanceComponent::IsCollapseLoopPoseActive() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::CollapseActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseOutPending;
}

bool UCBalanceComponent::IsBalanceLifecycleBlocking() const
{
	return BalanceLifecycleState != EBalanceLifecycleState::Accumulating;
}

bool UCBalanceComponent::ShouldSuppressCombatTargetFacing() const
{
	return BalanceLifecycleState == EBalanceLifecycleState::CollapseActive
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseOutPending
		|| BalanceLifecycleState == EBalanceLifecycleState::CollapseRecovering;
}

void UCBalanceComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	ShutdownBalanceRuntime();
	Super::EndPlay(InEndPlayReason);
}

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
	OnBalanceLifecycleStateChanged.Broadcast(previousState, BalanceLifecycleState);
}

void UCBalanceComponent::StartCollapseLoopTimer()
{
	ClearCollapseLoopTimer();

	UWorld* world = GetWorld();
	if (!IsValid(world))
	{
		AbortBalanceLifecycle(EBalanceAbortReason::CollapseInInterrupted);
		return;
	}

	if (CollapseLoopDuration <= 0.f)
	{
		HandleCollapseLoopExpired();
		return;
	}

	world->GetTimerManager().SetTimer(CollapseLoopTimerHandle, this, &UCBalanceComponent::HandleCollapseLoopExpired, CollapseLoopDuration, false);
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
	if (BalanceLifecycleState != EBalanceLifecycleState::CollapseActive) return;
	OnBalanceCollapseLoopExpired.Broadcast(BalanceLifecycleSerial);
}

void UCBalanceComponent::ResetBalanceRuntime()
{
	ClearCollapseLoopTimer();
	CurrentBalanceCount = 0;
	SetBalanceLifecycleState(EBalanceLifecycleState::Accumulating);
}

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
