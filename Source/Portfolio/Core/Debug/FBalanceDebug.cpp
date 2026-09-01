#include "Core/Debug/FBalanceDebug.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CBalanceComponent.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FLog.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	// ===== Display CVars =====

	TAutoConsoleVariable<int32> CVarBalanceDebugEnabled(
		TEXT("Portfolio.DebugOverlay.Balance.Enabled"),
		0,
		TEXT("Enable Balance and Collapse debug data and world visualization. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarBalanceDrawWorldText(
		TEXT("Portfolio.DebugOverlay.Balance.DrawWorldText"),
		1,
		TEXT("Draw focused Enemy Balance and Collapse state text in the world. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarBalanceDrawLifecycleBar(
		TEXT("Portfolio.DebugOverlay.Balance.DrawLifecycleBar"),
		1,
		TEXT("Include Balance Count segments in focused Enemy world text. 0: disabled, 1: enabled."),
		ECVF_Default);

	// ===== Audit CVar =====

	TAutoConsoleVariable<int32> CVarBalanceAudit(
		TEXT("Portfolio.Debug.BalanceAudit"),
		0,
		TEXT("Also write Balance and Collapse lifecycle diagnostic events to the Output Log. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatBalanceCompactEnumText(const FString& InQualifiedName)
	{
		int32 separatorIndex = INDEX_NONE;
		if (!InQualifiedName.FindLastChar(TEXT(':'), separatorIndex)) return InQualifiedName;
		return InQualifiedName.Mid(separatorIndex + 1);
	}

	FString FormatBalanceBoolText(const bool bInValue)
	{
		return bInValue ? TEXT("true") : TEXT("false");
	}

	FString FormatCountSegments(const FBalanceDebugSnapshot& InSnapshot)
	{
		const int32 threshold = FMath::Max(0, InSnapshot.Threshold);
	FString segments;
	for (int32 index = 0; index < threshold; ++index)
	{
			segments += index < InSnapshot.CurrentCount ? TEXT("\u25A0") : TEXT("\u25A1");
	}
		return segments;
	}

	FColor ResolveLifecycleColor(const EBalanceLifecycleState InLifecycleState)
	{
		switch (InLifecycleState)
		{
		case EBalanceLifecycleState::CollapseInPending: return FColor::Yellow;
		case EBalanceLifecycleState::CollapseInActive: return FColor(255, 140, 0);
		case EBalanceLifecycleState::CollapseLoopActive: return FColor::Red;
		case EBalanceLifecycleState::CollapseOutPending: return FColor::Cyan;
		case EBalanceLifecycleState::CollapseOutActive: return FColor(100, 170, 255);
		case EBalanceLifecycleState::ExecutionPrimaryActive: return FColor(255, 80, 255);
		case EBalanceLifecycleState::ExecutionPrimaryCommitted: return FColor(210, 70, 255);
		case EBalanceLifecycleState::ExecutionDownActive: return FColor(160, 80, 255);
		case EBalanceLifecycleState::ExecutionRecoveryPending: return FColor(150, 90, 255);
		case EBalanceLifecycleState::ExecutionRecoveryActive: return FColor(180, 80, 255);
		case EBalanceLifecycleState::Accumulating:
		default: return FColor::White;
		}
	}

	FVector ResolveWorldTextLocation(const ACEnemy* InEnemy)
	{
		if (!IsValid(InEnemy)) return FVector::ZeroVector;

		// Combat Participation occupies the high actor label space. Keep Balance below it so the
		// Canvas panels do not mask this focused-actor diagnostic block.
		return InEnemy->GetActorLocation() + FVector(0.f, 0.f, 40.f);
	}

	FString BuildAuditSummary(const UCBalanceComponent* InBalanceComp, const FString& InDetail)
	{
		if (!IsValid(InBalanceComp)) return InDetail;

		const FString stateText = FormatBalanceCompactEnumText(UEnum::GetValueAsString(InBalanceComp->GetBalanceLifecycleState()));
		const FString baseSummary = FString::Printf(
			TEXT("Count=%d/%d | State=%s | Lifecycle=%u"),
			InBalanceComp->GetCurrentBalanceCount(),
			InBalanceComp->GetBalanceThreshold(),
			*stateText,
			InBalanceComp->GetBalanceLifecycleSerial());
		return InDetail.IsEmpty() ? baseSummary : FString::Printf(TEXT("%s | %s"), *baseSummary, *InDetail);
	}
}

// ===== Display Gates =====

bool FBalanceDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarBalanceDebugEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FBalanceDebug::ShouldDrawWorldText()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarBalanceDrawWorldText.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FBalanceDebug::ShouldDrawLifecycleBar()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarBalanceDrawLifecycleBar.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// ===== Audit Gate =====

bool FBalanceDebug::ShouldAuditBalance()
{
#if !UE_BUILD_SHIPPING
	return CVarBalanceAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// ===== Display Snapshot / Presentation =====

FBalanceDebugSnapshot FBalanceDebug::BuildSnapshot(const ACEnemy* InEnemy)
{
	FBalanceDebugSnapshot snapshot;
	if (!IsEnabled() || !IsValid(InEnemy)) return snapshot;

	const UCBalanceComponent* balanceComp = InEnemy->GetBalanceComp();
	if (!IsValid(balanceComp)) return snapshot;

	snapshot.bHasSnapshot = true;
	snapshot.CurrentCount = balanceComp->GetCurrentBalanceCount();
	snapshot.Threshold = balanceComp->GetBalanceThreshold();
	snapshot.LifecycleSerial = balanceComp->GetBalanceLifecycleSerial();
	snapshot.LifecycleState = balanceComp->GetBalanceLifecycleState();
	snapshot.LastAbortReason = balanceComp->GetLastAbortReason();
	snapshot.LoopRemainingSeconds = balanceComp->GetCollapseLoopRemainingSeconds();
	snapshot.LoopDurationSeconds = balanceComp->GetCollapseLoopDuration();
	snapshot.ExecutionDownRemainingSeconds = balanceComp->GetExecutionDownRemainingSeconds();
	snapshot.ExecutionDownDurationSeconds = balanceComp->GetExecutionDownDuration();
	snapshot.IncapacitatedPresentation = balanceComp->GetIncapacitatedPresentation();
	snapshot.bIsCollapsePoseActive = balanceComp->IsCollapseActive();
	snapshot.bIsCollapseLoopActive = balanceComp->IsCollapseLoopActive();
	snapshot.bIsExecutionDownPresentationActive = balanceComp->IsExecutionDownPresentationActive();
	snapshot.bIsExecutionDownPoseActive = balanceComp->IsExecutionDownActive();
	snapshot.bIsLifecycleBlocking = balanceComp->IsBalanceLifecycleBlocking();
	snapshot.bIsFacingSuppressed = balanceComp->ShouldSuppressCombatTargetFacing();
	return snapshot;
}

FBalanceDebugOverlayDetails FBalanceDebug::BuildOverlayDetails(const FBalanceDebugSnapshot& InSnapshot)
{
	FBalanceDebugOverlayDetails details;
	if (!IsEnabled() || !InSnapshot.bHasSnapshot) return details;

	details.bHasSnapshot = true;
	details.CountText = FString::Printf(TEXT("%d / %d"), InSnapshot.CurrentCount, InSnapshot.Threshold);
	details.LifecycleText = FormatBalanceCompactEnumText(UEnum::GetValueAsString(InSnapshot.LifecycleState));
	details.LifecycleSerialText = FString::FromInt(InSnapshot.LifecycleSerial);
	details.LoopLifetimeText = InSnapshot.bIsCollapseLoopActive
		? FString::Printf(TEXT("%.2f / %.2f s"), InSnapshot.LoopRemainingSeconds, InSnapshot.LoopDurationSeconds)
		: TEXT("--");
	details.ExecutionDownLifetimeText = InSnapshot.bIsExecutionDownPoseActive
		? FString::Printf(TEXT("%.2f / %.2f s"), InSnapshot.ExecutionDownRemainingSeconds, InSnapshot.ExecutionDownDurationSeconds)
		: TEXT("--");
	details.IncapacitatedPresentationText = FormatBalanceCompactEnumText(UEnum::GetValueAsString(InSnapshot.IncapacitatedPresentation));
	details.CollapsePoseText = FormatBalanceBoolText(InSnapshot.bIsCollapsePoseActive);
	details.CollapseLoopText = FormatBalanceBoolText(InSnapshot.bIsCollapseLoopActive);
	details.ExecutionDownPresentationText = FormatBalanceBoolText(InSnapshot.bIsExecutionDownPresentationActive);
	details.ExecutionDownPoseText = InSnapshot.bIsExecutionDownPoseActive
		? TEXT("Active")
		: (InSnapshot.bIsExecutionDownPresentationActive ? TEXT("Presentation") : TEXT("Off"));
	details.LifecycleBlockingText = FormatBalanceBoolText(InSnapshot.bIsLifecycleBlocking);
	details.FacingSuppressedText = FormatBalanceBoolText(InSnapshot.bIsFacingSuppressed);
	details.LastAbortText = FormatBalanceCompactEnumText(UEnum::GetValueAsString(InSnapshot.LastAbortReason));
	return details;
}

void FBalanceDebug::DrawWorldDebug(UWorld* InWorld, const ACEnemy* InEnemy, const FBalanceDebugSnapshot& InSnapshot)
{
#if !UE_BUILD_SHIPPING
	if (!ShouldDrawWorldText() || !IsValid(InWorld) || !IsValid(InEnemy) || !InSnapshot.bHasSnapshot) return;

	TArray<FString> textLines;
	const FString lifecycleText = FormatBalanceCompactEnumText(UEnum::GetValueAsString(InSnapshot.LifecycleState));
	if (InSnapshot.bIsCollapseLoopActive)
	{
		textLines.Add(FString::Printf(TEXT("[COLLAPSE | %s | %.2f s]"), *lifecycleText, InSnapshot.LoopRemainingSeconds));
	}
	else if (InSnapshot.bIsExecutionDownPoseActive)
	{
		textLines.Add(FString::Printf(TEXT("[EXECUTION DOWN | %s | %.2f s]"), *lifecycleText, InSnapshot.ExecutionDownRemainingSeconds));
	}
	else if (InSnapshot.bIsExecutionDownPresentationActive)
	{
		textLines.Add(FString::Printf(TEXT("[EXECUTION DOWN PRESENTATION | %s]"), *lifecycleText));
	}
	else
	{
		textLines.Add(FString::Printf(TEXT("[BALANCE | %s]"), *lifecycleText));
	}

	const FString countText = ShouldDrawLifecycleBar()
		? FString::Printf(TEXT("[%s] %d / %d | L#%u"), *FormatCountSegments(InSnapshot), InSnapshot.CurrentCount, InSnapshot.Threshold, InSnapshot.LifecycleSerial)
		: FString::Printf(TEXT("Count: %d / %d | L#%u"), InSnapshot.CurrentCount, InSnapshot.Threshold, InSnapshot.LifecycleSerial);
	textLines.Add(countText);
	textLines.Add(FString::Printf(
		TEXT("Incapacitated: %s | Collapse Lifecycle: %s | Execution Down: %s | Block: %s | Facing: %s"),
		*FormatBalanceCompactEnumText(UEnum::GetValueAsString(InSnapshot.IncapacitatedPresentation)),
		InSnapshot.bIsCollapsePoseActive ? TEXT("On") : TEXT("Off"),
		InSnapshot.bIsExecutionDownPoseActive ? TEXT("On") : TEXT("Off"),
		InSnapshot.bIsLifecycleBlocking ? TEXT("On") : TEXT("Off"),
		InSnapshot.bIsFacingSuppressed ? TEXT("Suppressed") : TEXT("Active")));

	DrawDebugString(InWorld, ResolveWorldTextLocation(InEnemy), FString::Join(textLines, TEXT("\n")), nullptr, ResolveLifecycleColor(InSnapshot.LifecycleState), 0.f, false, 1.f);
#endif
}

// ===== Lifecycle Audit Hooks =====

void FBalanceDebug::RecordLifecycleEvent(const UCBalanceComponent* InBalanceComp, const TCHAR* InEvent, const FString& InDetail)
{
	if (!IsValid(InBalanceComp)) return;

	const AActor* ownerActor = InBalanceComp->GetOwner();
	if (!IsValid(ownerActor)) return;

	const FString summary = BuildAuditSummary(InBalanceComp, InDetail);
	FDebugOverlaySnapshotStore::AddEvent(
		ownerActor,
		TEXT("Balance"),
		InEvent ? InEvent : TEXT("Unknown"),
		GetNameSafe(ownerActor),
		GetNameSafe(ownerActor),
		FString(),
		summary);

	if (!ShouldAuditBalance()) return;

	FLog::Log(FString::Printf(
		TEXT("[Balance|%s] Owner=%s | %s"),
		InEvent ? InEvent : TEXT("Unknown"),
		*GetNameSafe(ownerActor),
		*summary));
}

void FBalanceDebug::RecordLifecycleStateChanged(const UCBalanceComponent* InBalanceComp, const EBalanceLifecycleState InPreviousState, const EBalanceLifecycleState InNewState)
{
	RecordLifecycleEvent(
		InBalanceComp,
		TEXT("LifecycleStateChanged"),
		FString::Printf(
			TEXT("Previous=%s | New=%s"),
			*FormatBalanceCompactEnumText(UEnum::GetValueAsString(InPreviousState)),
			*FormatBalanceCompactEnumText(UEnum::GetValueAsString(InNewState))));
}
