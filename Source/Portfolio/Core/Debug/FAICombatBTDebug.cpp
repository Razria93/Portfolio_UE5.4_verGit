#include "Core/Debug/FAICombatBTDebug.h"

#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FLog.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarAICombatBTAudit(
		TEXT("Portfolio.Debug.AICombatBTAudit"),
		0,
		TEXT("Print AI combat behavior tree boundary diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarCanMoveDecoratorAudit(
		TEXT("Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit"),
		0,
		TEXT("Print CBTDecorator_CanMove result for runtime LOD debugging. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatAICombatBTController(const AAIController* InAIController)
	{
		return FString::Printf(
			TEXT("Controller=%s | Pawn=%s"),
			*GetNameSafe(InAIController),
			*GetNameSafe(IsValid(InAIController) ? InAIController->GetPawn() : nullptr));
	}

	FString FormatAICombatBTEngageContext(const FEngageContext& InContext)
	{
		return FString::Printf(
			TEXT("Target=%s | Distance=%.3f | InRange=%s | CanCombat=%s | Inner=%.3f | Outer=%.3f | NextActionTime=%.3f"),
			*GetNameSafe(InContext.TargetActor),
			InContext.DistanceToTarget,
			InContext.bInEngageRange ? TEXT("true") : TEXT("false"),
			InContext.bCanCombatAction ? TEXT("true") : TEXT("false"),
			InContext.EngageInnerRange,
			InContext.EngageOuterRange,
			InContext.NextCombatActionTime);
	}

	FString FormatAICombatBTActionResult(const FActionRequestResult& InResult)
	{
		return FString::Printf(
			TEXT("Result=%s | RejectReason=%s"),
			*UEnum::GetValueAsString(InResult.ResultType),
			*UEnum::GetValueAsString(InResult.RejectReason));
	}

	const UObject* ResolveAICombatBTWorldContext(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor)
	{
		if (IsValid(InOwnerPawn)) return InOwnerPawn;
		if (IsValid(InAIController)) return InAIController;
		if (IsValid(InTargetActor)) return InTargetActor;

		return nullptr;
	}
}

// Gate

bool FAICombatBTDebug::ShouldAuditAICombatBT()
{
#if !UE_BUILD_SHIPPING
	return CVarAICombatBTAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FAICombatBTDebug::ShouldAuditCanMoveDecorator()
{
#if !UE_BUILD_SHIPPING
	return CVarCanMoveDecoratorAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Can Move Decorator Diagnostic Hook

void FAICombatBTDebug::RecordCanMoveDecoratorResultForAudit(const APawn* InOwnerPawn, bool bInCanMove)
{
	if (!ShouldAuditCanMoveDecorator()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatBT|CanMoveDecoratorResult] Owner=%s | CanMove=%s"),
		*GetNameSafe(InOwnerPawn),
		bInCanMove ? TEXT("true") : TEXT("false")));
}

// AI Context / Engage Assignment Diagnostic Hook

void FAICombatBTDebug::RecordAIContextClearedForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditAICombatBT()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatBT|%sCleared] Reason=%s | Controller=%s | Pawn=%s | Target=%s"),
		InEvent ? InEvent : TEXT("Context"),
		InReason ? InReason : TEXT("Unknown"),
		*GetNameSafe(InAIController),
		*GetNameSafe(InOwnerPawn),
		*GetNameSafe(InTargetActor)));
}

void FAICombatBTDebug::RecordAIContextEngageAssignmentForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, ECombatRole InCombatRole, bool bInShouldEngage, const TCHAR* InEvent)
{
	if (!ShouldAuditAICombatBT()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatBT|%s] Controller=%s | Pawn=%s | Target=%s | CombatRole=%s | ShouldEngage=%s"),
		InEvent ? InEvent : TEXT("EngageAssignment"),
		*GetNameSafe(InAIController),
		*GetNameSafe(InOwnerPawn),
		*GetNameSafe(InTargetActor),
		*UEnum::GetValueAsString(InCombatRole),
		bInShouldEngage ? TEXT("true") : TEXT("false")));
}

// Engage Context Gate Diagnostic Hook

void FAICombatBTDebug::RecordEngageContextComputedForAudit(const APawn* InOwnerPawn, const FEngageContext& InContext, bool bInCooldownElapsed, bool bInCombatAction, bool bInActiveReaction)
{
	if (!ShouldAuditAICombatBT()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatBT|EngageContextComputed] Pawn=%s | %s | CooldownElapsed=%s | IsCombatAction=%s | IsActiveReaction=%s"),
		*GetNameSafe(InOwnerPawn),
		*FormatAICombatBTEngageContext(InContext),
		bInCooldownElapsed ? TEXT("true") : TEXT("false"),
		bInCombatAction ? TEXT("true") : TEXT("false"),
		bInActiveReaction ? TEXT("true") : TEXT("false")));
}

void FAICombatBTDebug::RecordEngageContextRejectedForAudit(const APawn* InOwnerPawn, const FEngageContext& InContext, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditAICombatBT()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatBT|%sRejected] Reason=%s | Pawn=%s | %s"),
		InEvent ? InEvent : TEXT("EngageContext"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerPawn),
		*FormatAICombatBTEngageContext(InContext)));
}

// Combat Action Task Diagnostic Hook

void FAICombatBTDebug::RecordCombatActionTaskSucceededForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, ECombatActionIntent InIntent, const FActionRequestResult& InResult, float InCooldown, float InNextActionTime)
{
	if (FDebugOverlaySnapshotStore::IsCollecting())
	{
		FDebugOverlaySnapshotStore::RecordAICombatTask(
			ResolveAICombatBTWorldContext(InAIController, InOwnerPawn, InTargetActor),
			InAIController,
			InOwnerPawn,
			InTargetActor,
			UEnum::GetValueAsString(InIntent),
			UEnum::GetValueAsString(InResult.ResultType),
			UEnum::GetValueAsString(InResult.RejectReason),
			TEXT("CombatActionTaskSucceeded"));
	}

	if (!ShouldAuditAICombatBT()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatBT|CombatActionTaskSucceeded] %s | Owner=%s | Target=%s | Intent=%s | Cooldown=%.3f | NextActionTime=%.3f | %s"),
		*FormatAICombatBTController(InAIController),
		*GetNameSafe(InOwnerPawn),
		*GetNameSafe(InTargetActor),
		*UEnum::GetValueAsString(InIntent),
		InCooldown,
		InNextActionTime,
		*FormatAICombatBTActionResult(InResult)));
}

void FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, ECombatActionIntent InIntent, const FActionRequestResult& InResult, const TCHAR* InReason)
{
	if (FDebugOverlaySnapshotStore::IsCollecting())
	{
		FDebugOverlaySnapshotStore::RecordAICombatTask(
			ResolveAICombatBTWorldContext(InAIController, InOwnerPawn, InTargetActor),
			InAIController,
			InOwnerPawn,
			InTargetActor,
			UEnum::GetValueAsString(InIntent),
			UEnum::GetValueAsString(InResult.ResultType),
			InReason ? FString(InReason) : UEnum::GetValueAsString(InResult.RejectReason),
			TEXT("CombatActionTaskRejected"));
	}

	if (!ShouldAuditAICombatBT()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatBT|CombatActionTaskRejected] Reason=%s | %s | Owner=%s | Target=%s | Intent=%s | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*FormatAICombatBTController(InAIController),
		*GetNameSafe(InOwnerPawn),
		*GetNameSafe(InTargetActor),
		*UEnum::GetValueAsString(InIntent),
		*FormatAICombatBTActionResult(InResult)));
}
