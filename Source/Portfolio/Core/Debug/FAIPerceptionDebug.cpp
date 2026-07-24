#include "Core/Debug/FAIPerceptionDebug.h"

#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarPerceptionCandidateAudit(
		TEXT("Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit"),
		0,
		TEXT("Print ACEnemy perception candidate audit summary for runtime LOD measurement. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarBlackboardEngageLatencyAudit(
		TEXT("Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit"),
		0,
		TEXT("Print ACEnemy Blackboard / Engage latency audit summary for runtime LOD measurement. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif
}

// Gate

bool FAIPerceptionDebug::ShouldAuditPerceptionCandidates()
{
#if !UE_BUILD_SHIPPING
	return CVarPerceptionCandidateAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FAIPerceptionDebug::ShouldAuditBlackboardEngageLatency()
{
#if !UE_BUILD_SHIPPING
	return CVarBlackboardEngageLatencyAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Profiling Audit Summary

void FAIPerceptionDebug::PrintPerceptionCandidateAuditSummary(const AActor* InOwnerActor, const FPerceptionCandidateAuditState& InState)
{
	if (!InState.bEnabled) return;

	const bool bHasRawPerception = InState.FirstRawPerceptionTime >= 0.f;
	const bool bHasValidTarget = InState.FirstValidTargetTime >= 0.f;

	const float firstRawLatency = bHasRawPerception
		? InState.FirstRawPerceptionTime - InState.RuntimeStartTime
		: -1.f;

	const float firstValidLatency = bHasValidTarget
		? InState.FirstValidTargetTime - InState.RuntimeStartTime
		: -1.f;

	FLog::Log(FString::Printf(
		TEXT("[AI|Perception|CandidateAuditSummary] Owner=%s | RawEvents=%d | RawActors=%d | ValidProviders=%d | InvalidProviders=%d | MaxTargetPerceptionStateMap=%d | FirstRawLatency=%.3f | FirstValidLatency=%.3f | StartFrame=%llu | FirstRawFrame=%llu | FirstValidFrame=%llu"),
		*GetNameSafe(InOwnerActor),
		InState.RawPerceptionEventCount,
		InState.RawPerceptionActors.Num(),
		InState.ValidTargetProviderActors.Num(),
		InState.InvalidTargetProviderActors.Num(),
		InState.MaxTargetPerceptionStateMapSize,
		firstRawLatency,
		firstValidLatency,
		InState.RuntimeStartFrame,
		InState.FirstRawPerceptionFrame,
		InState.FirstValidTargetFrame));
}

void FAIPerceptionDebug::PrintBlackboardEngageLatencyAuditSummary(const AActor* InOwnerActor, const FBlackboardEngageLatencyAuditState& InState)
{
	if (!InState.bEnabled) return;

	const bool bHasPerceptionContext = InState.FirstPerceptionContextTime >= 0.f;
	const bool bHasBlackboardTarget = InState.FirstBlackboardTargetTime >= 0.f;
	const bool bHasEngageRequest = InState.FirstEngageRequestTime >= 0.f;
	const bool bHasEngageAssignment = InState.FirstEngageAssignmentTime >= 0.f;

	const float perceptionContextLatency = bHasPerceptionContext
		? InState.FirstPerceptionContextTime - InState.RuntimeStartTime
		: -1.f;

	const float blackboardTargetLatency = bHasBlackboardTarget
		? InState.FirstBlackboardTargetTime - InState.RuntimeStartTime
		: -1.f;

	const float engageRequestLatency = bHasEngageRequest
		? InState.FirstEngageRequestTime - InState.RuntimeStartTime
		: -1.f;

	const float engageAssignmentLatency = bHasEngageAssignment
		? InState.FirstEngageAssignmentTime - InState.RuntimeStartTime
		: -1.f;

	FLog::Log(FString::Printf(
		TEXT("[AI|Perception|BlackboardEngageLatencyAuditSummary] Owner=%s | PerceptionContextLatency=%.3f | BlackboardTargetLatency=%.3f | EngageRequestLatency=%.3f | EngageAssignmentLatency=%.3f | StartFrame=%llu | PerceptionContextFrame=%llu | BlackboardTargetFrame=%llu | EngageRequestFrame=%llu | EngageAssignmentFrame=%llu | PerceptionTarget=%s | BlackboardTarget=%s | EngageRequestTarget=%s | EngageAssignmentTarget=%s"),
		*GetNameSafe(InOwnerActor),
		perceptionContextLatency,
		blackboardTargetLatency,
		engageRequestLatency,
		engageAssignmentLatency,
		InState.RuntimeStartFrame,
		InState.FirstPerceptionContextFrame,
		InState.FirstBlackboardTargetFrame,
		InState.FirstEngageRequestFrame,
		InState.FirstEngageAssignmentFrame,
		*GetNameSafe(InState.FirstPerceptionTargetActor.Get()),
		*GetNameSafe(InState.FirstBlackboardTargetActor.Get()),
		*GetNameSafe(InState.FirstEngageRequestTargetActor.Get()),
		*GetNameSafe(InState.FirstEngageAssignmentTargetActor.Get())));
}
