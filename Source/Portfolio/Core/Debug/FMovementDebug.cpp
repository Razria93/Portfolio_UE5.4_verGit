#include "Core/Debug/FMovementDebug.h"
#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarMovementAudit(
		TEXT("Portfolio.Debug.MovementAudit"),
		0,
		TEXT("Print movement gate, gait, and RuntimeLOD diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatMovementGateState(EExecutionState InExecutionState, bool bInCanMove, bool bInIntentBlocked)
	{
		return FString::Printf(
			TEXT("ExecutionState=%s | CanMove=%s | RuntimeLODIntentBlocked=%s"),
			*UEnum::GetValueAsString(InExecutionState),
			bInCanMove ? TEXT("true") : TEXT("false"),
			bInIntentBlocked ? TEXT("true") : TEXT("false"));
	}
}

// Gate

bool FMovementDebug::ShouldAuditMovement()
{
#if !UE_BUILD_SHIPPING
	return CVarMovementAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Runtime LOD Diagnostic Hook

void FMovementDebug::RecordRuntimeLODMovementModeAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, int32 InPreviousMode, int32 InNewMode, bool bInTickEnabled, bool bInCanMove, bool bInIntentBlocked)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|RuntimeLOD|ModeApplied] Owner=%s | Component=%s | PreviousMode=%d | NewMode=%d | TickEnabled=%s | CanMove=%s | RuntimeLODIntentBlocked=%s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InPreviousMode,
		InNewMode,
		bInTickEnabled ? TEXT("true") : TEXT("false"),
		bInCanMove ? TEXT("true") : TEXT("false"),
		bInIntentBlocked ? TEXT("true") : TEXT("false")));
}

void FMovementDebug::RecordRuntimeLODMovementIntentBlockedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|RuntimeLOD|IntentBlocked] Event=%s | Owner=%s | Component=%s"),
		InEvent ? InEvent : TEXT("Block"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent)));
}

void FMovementDebug::RecordRuntimeLODMovementIntentAllowedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|RuntimeLOD|IntentAllowed] Event=%s | Owner=%s | Component=%s"),
		InEvent ? InEvent : TEXT("Allow"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent)));
}

// Movement Gate Diagnostic Hook

void FMovementDebug::RecordMovementInputAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, EMovementGait InGait)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|InputAccepted] Owner=%s | Component=%s | Axis=(%.3f, %.3f) | Gait=%s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InAxis2D.X,
		InAxis2D.Y,
		*UEnum::GetValueAsString(InGait)));
}

void FMovementDebug::RecordMovementInputRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, const TCHAR* InReason, EExecutionState InExecutionState, bool bInCanMove, bool bInIntentBlocked)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|InputRejected] Reason=%s | Owner=%s | Component=%s | Axis=(%.3f, %.3f) | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InAxis2D.X,
		InAxis2D.Y,
		*FormatMovementGateState(InExecutionState, bInCanMove, bInIntentBlocked)));
}

// Movement Data Diagnostic Hook

void FMovementDebug::RecordMovementGaitAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, float InSpeed)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|GaitApplied] Owner=%s | Component=%s | Gait=%s | Speed=%.3f"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*UEnum::GetValueAsString(InGait),
		InSpeed));
}

void FMovementDebug::RecordMovementGaitRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, const TCHAR* InReason)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|GaitRejected] Reason=%s | Owner=%s | Component=%s | Gait=%s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*UEnum::GetValueAsString(InGait)));
}
