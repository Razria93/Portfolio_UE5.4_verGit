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

	FString FormatCompactEnumText(const FString& InQualifiedName)
	{
		int32 separatorIndex = INDEX_NONE;
		if (!InQualifiedName.FindLastChar(TEXT(':'), separatorIndex)) return InQualifiedName;
		return InQualifiedName.Mid(separatorIndex + 1);
	}

	FString FormatBoolText(const bool bInValue)
	{
		return bInValue ? TEXT("true") : TEXT("false");
	}

	FString FormatCountSegments(const FBalanceDebugSnapshot& InSnapshot)
	{
		const int32 threshold = FMath::Max(0, InSnapshot.Threshold);
	FString segments;
	for (int32 index = 0; index < threshold; ++index)
	{
			segments += index < InSnapshot.CurrentCount ? TEXT("■") : TEXT("□");
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
		case EBalanceLifecycleState::CollapseRecovering: return FColor(180, 80, 255);
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

		const FString stateText = FormatCompactEnumText(UEnum::GetValueAsString(InBalanceComp->GetBalanceLifecycleState()));
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
	snapshot.bIsCollapsePoseActive = balanceComp->IsCollapsePoseActive();
	snapshot.bIsCollapseLoopActive = balanceComp->IsCollapseLoopActive();
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
	details.LifecycleText = FormatCompactEnumText(UEnum::GetValueAsString(InSnapshot.LifecycleState));
	details.LifecycleSerialText = FString::FromInt(InSnapshot.LifecycleSerial);
	details.LoopLifetimeText = InSnapshot.bIsCollapseLoopActive
		? FString::Printf(TEXT("%.2f / %.2f s"), InSnapshot.LoopRemainingSeconds, InSnapshot.LoopDurationSeconds)
		: TEXT("--");
	details.CollapsePoseText = FormatBoolText(InSnapshot.bIsCollapsePoseActive);
	details.CollapseLoopText = FormatBoolText(InSnapshot.bIsCollapseLoopActive);
	details.LifecycleBlockingText = FormatBoolText(InSnapshot.bIsLifecycleBlocking);
	details.FacingSuppressedText = FormatBoolText(InSnapshot.bIsFacingSuppressed);
	details.LastAbortText = FormatCompactEnumText(UEnum::GetValueAsString(InSnapshot.LastAbortReason));
	return details;
}

void FBalanceDebug::DrawWorldDebug(UWorld* InWorld, const ACEnemy* InEnemy, const FBalanceDebugSnapshot& InSnapshot)
{
#if !UE_BUILD_SHIPPING
	if (!ShouldDrawWorldText() || !IsValid(InWorld) || !IsValid(InEnemy) || !InSnapshot.bHasSnapshot) return;

	TArray<FString> textLines;
	const FString lifecycleText = FormatCompactEnumText(UEnum::GetValueAsString(InSnapshot.LifecycleState));
	textLines.Add(InSnapshot.bIsCollapseLoopActive
		? FString::Printf(TEXT("[COLLAPSE | %s | %.2f s]"), *lifecycleText, InSnapshot.LoopRemainingSeconds)
		: FString::Printf(TEXT("[BALANCE | %s]"), *lifecycleText));

	const FString countText = ShouldDrawLifecycleBar()
		? FString::Printf(TEXT("[%s] %d / %d | L#%u"), *FormatCountSegments(InSnapshot), InSnapshot.CurrentCount, InSnapshot.Threshold, InSnapshot.LifecycleSerial)
		: FString::Printf(TEXT("Count: %d / %d | L#%u"), InSnapshot.CurrentCount, InSnapshot.Threshold, InSnapshot.LifecycleSerial);
	textLines.Add(countText);
	textLines.Add(FString::Printf(
		TEXT("Pose: %s | Block: %s | Facing: %s"),
		InSnapshot.bIsCollapsePoseActive ? TEXT("On") : TEXT("Off"),
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
			*FormatCompactEnumText(UEnum::GetValueAsString(InPreviousState)),
			*FormatCompactEnumText(UEnum::GetValueAsString(InNewState))));
}
