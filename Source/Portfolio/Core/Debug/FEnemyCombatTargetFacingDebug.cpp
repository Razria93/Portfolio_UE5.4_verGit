#include "Core/Debug/FEnemyCombatTargetFacingDebug.h"

#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FLog.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CEnemyCombatTargetFacingComponent.h"

#include "AIController.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCombatTargetFacingDebugEnabled(
		TEXT("Portfolio.DebugOverlay.CombatTargetFacing.Enabled"),
		1,
		TEXT("Enable Combat Target Facing debug snapshot data. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarCombatTargetFacingAudit(
		TEXT("Portfolio.Debug.CombatTargetFacingAudit"),
		0,
		TEXT("Print enemy combat-target facing decisions. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatPolicyState(const EEnemyCombatTargetFacingPolicyState InPolicyState)
	{
		switch (InPolicyState)
		{
		case EEnemyCombatTargetFacingPolicyState::Tracking: return TEXT("Tracking");
		case EEnemyCombatTargetFacingPolicyState::HoldForReaction: return TEXT("HoldForReaction");
		case EEnemyCombatTargetFacingPolicyState::SuppressedDead: return TEXT("Suppressed(Dead)");
		case EEnemyCombatTargetFacingPolicyState::SuppressedBalance: return TEXT("Suppressed(Balance)");
		case EEnemyCombatTargetFacingPolicyState::NoCombatTarget: return TEXT("NoCombatTarget");
		case EEnemyCombatTargetFacingPolicyState::NoAIController: return TEXT("NoAIController");
		case EEnemyCombatTargetFacingPolicyState::Uninitialized:
		default: return TEXT("Uninitialized");
		}
	}

	FString FormatCompactEnum(const UEnum* InEnum, const int64 InValue)
	{
		if (!IsValid(InEnum)) return TEXT("None");

		FString valueText = InEnum->GetNameStringByValue(InValue);
		const int32 scopeIndex = valueText.Find(TEXT("::"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		return scopeIndex == INDEX_NONE ? valueText : valueText.Mid(scopeIndex + 2);
	}

	FString FormatTargetChangeReason(const ECombatTargetChangeReason InReason)
	{
		return FormatCompactEnum(StaticEnum<ECombatTargetChangeReason>(), static_cast<int64>(InReason));
	}

	FString FormatRotationMode(const EMovementRotationMode InRotationMode)
	{
		return FormatCompactEnum(StaticEnum<EMovementRotationMode>(), static_cast<int64>(InRotationMode));
	}

	FString FormatFocusDirective(const EEnemyCombatTargetFacingFocusDirective InDirective)
	{
		switch (InDirective)
		{
		case EEnemyCombatTargetFacingFocusDirective::SetCombatTarget: return TEXT("SetCombatTarget");
		case EEnemyCombatTargetFacingFocusDirective::ClearGameplayFocus: return TEXT("ClearGameplayFocus");
		case EEnemyCombatTargetFacingFocusDirective::HoldCurrentFocus: return TEXT("HoldCurrentFocus");
		case EEnemyCombatTargetFacingFocusDirective::None:
		default: return TEXT("None");
		}
	}

	FString FormatRotationDirective(const EEnemyCombatTargetFacingRotationDirective InDirective)
	{
		switch (InDirective)
		{
		case EEnemyCombatTargetFacingRotationDirective::ControllerDesired: return TEXT("ControllerDesired");
		case EEnemyCombatTargetFacingRotationDirective::OrientToMovement: return TEXT("OrientToMovement");
		case EEnemyCombatTargetFacingRotationDirective::HoldCurrentRotation: return TEXT("HoldCurrentRotation");
		case EEnemyCombatTargetFacingRotationDirective::None:
		default: return TEXT("None");
		}
	}

	FString FormatReaction(const FEnemyCombatTargetFacingRuntimeSnapshot& InRuntime)
	{
		if (!InRuntime.bIsReactionActive) return TEXT("None / Inactive");
		return FString::Printf(
			TEXT("%s / Active"),
			*FormatCompactEnum(StaticEnum<EReactionType>(), static_cast<int64>(InRuntime.ActiveReactionType)));
	}

	FString BuildConsistencyText(const FEnemyCombatTargetFacingRuntimeSnapshot& InRuntime)
	{
		if (InRuntime.PolicyState == EEnemyCombatTargetFacingPolicyState::HoldForReaction)
		{
			return TEXT("Deferred (not asserted)");
		}
		if (InRuntime.PolicyState == EEnemyCombatTargetFacingPolicyState::NoAIController
			|| InRuntime.PolicyState == EEnemyCombatTargetFacingPolicyState::Uninitialized)
		{
			return TEXT("Unavailable");
		}

		const bool bShouldTrack = InRuntime.PolicyState == EEnemyCombatTargetFacingPolicyState::Tracking;
		const bool bFocusMatches = bShouldTrack
			? InRuntime.GameplayFocusActor == InRuntime.CombatTargetActor
			: !IsValid(InRuntime.GameplayFocusActor);
		const bool bRotationMatches = bShouldTrack
			? InRuntime.RotationMode == EMovementRotationMode::ControllerDesired
			: InRuntime.RotationMode == EMovementRotationMode::OrientToMovement;

		if (bFocusMatches && bRotationMatches) return TEXT("OK");
		if (!bFocusMatches && !bRotationMatches) return TEXT("Mismatch: Focus + Rotation");
		return !bFocusMatches ? TEXT("Mismatch: Focus") : TEXT("Mismatch: Rotation");
	}

	FString BuildControllerBindingText(const FEnemyCombatTargetFacingRuntimeSnapshot& InRuntime)
	{
		if (!IsValid(InRuntime.OwnerAIController))
		{
			return FString::Printf(TEXT("Owner: None | Bound: %s | Missing Owner Controller"), *GetNameSafe(InRuntime.BoundAIController));
		}

		return FString::Printf(
			TEXT("Owner: %s | Bound: %s | %s"),
			*GetNameSafe(InRuntime.OwnerAIController),
			*GetNameSafe(InRuntime.BoundAIController),
			InRuntime.bControllerBindingMatchesOwner ? TEXT("Match") : TEXT("Mismatch"));
	}

	FString BuildTransitionSummary(const FEnemyCombatTargetFacingRuntimeSnapshot& InRuntime, const EMovementRotationMode InPreviousRotationMode, const AActor* InPreviousGameplayFocusActor)
	{
		return FString::Printf(
			TEXT("#%u | Decision=%s | Policy=%s | Target=%s@%d | Binding=Owner:%s Bound:%s Match:%s | Focus=%s -> %s | Rotation=%s -> %s | ExpectedFocus=%s | ExpectedRotation=%s | Dead=%s | BalanceSuppressed=%s | Reaction=%s | Deferred=%s"),
			InRuntime.LastTransitionSequence,
			*InRuntime.LastDecision,
			*FormatPolicyState(InRuntime.PolicyState),
			*GetNameSafe(InRuntime.CombatTargetActor),
			InRuntime.CombatTargetRevision,
			*GetNameSafe(InRuntime.OwnerAIController),
			*GetNameSafe(InRuntime.BoundAIController),
			InRuntime.bControllerBindingMatchesOwner ? TEXT("true") : TEXT("false"),
			*GetNameSafe(InPreviousGameplayFocusActor),
			*GetNameSafe(InRuntime.GameplayFocusActor),
			*FormatRotationMode(InPreviousRotationMode),
			*FormatRotationMode(InRuntime.RotationMode),
			*FormatFocusDirective(InRuntime.ExpectedFocusDirective),
			*FormatRotationDirective(InRuntime.ExpectedRotationDirective),
			InRuntime.bIsDead ? TEXT("true") : TEXT("false"),
			InRuntime.bIsBalanceSuppressed ? TEXT("true") : TEXT("false"),
			*FormatReaction(InRuntime),
			InRuntime.bDeferredSyncPending ? TEXT("true") : TEXT("false"));
	}
}

// Display Snapshot / Presentation

bool FEnemyCombatTargetFacingDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarCombatTargetFacingDebugEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

FEnemyCombatTargetFacingDebugSnapshot FEnemyCombatTargetFacingDebug::BuildSnapshot(const ACEnemy* InEnemy)
{
	FEnemyCombatTargetFacingDebugSnapshot snapshot;
	if (!IsEnabled() || !IsValid(InEnemy)) return snapshot;

	const UCEnemyCombatTargetFacingComponent* facingComp = InEnemy->GetEnemyCombatTargetFacingComp();
	if (!IsValid(facingComp)) return snapshot;

	snapshot.bHasSnapshot = true;
	snapshot.Runtime = facingComp->GetRuntimeSnapshot();
	return snapshot;
}

FEnemyCombatTargetFacingDebugOverlayDetails FEnemyCombatTargetFacingDebug::BuildOverlayDetails(const FEnemyCombatTargetFacingDebugSnapshot& InSnapshot)
{
	FEnemyCombatTargetFacingDebugOverlayDetails details;
	if (!IsEnabled() || !InSnapshot.bHasSnapshot || !InSnapshot.Runtime.bHasComponent) return details;

	const FEnemyCombatTargetFacingRuntimeSnapshot& runtime = InSnapshot.Runtime;
	details.bHasSnapshot = true;
	details.PolicyText = FormatPolicyState(runtime.PolicyState);
	details.ConsistencyText = BuildConsistencyText(runtime);
	details.ControllerBindingText = BuildControllerBindingText(runtime);
	details.CombatTargetText = FString::Printf(
		TEXT("%s | Rev: %d | Reason: %s"),
		*GetNameSafe(runtime.CombatTargetActor),
		runtime.CombatTargetRevision,
		*FormatTargetChangeReason(runtime.CombatTargetChangeReason));
	details.GameplayFocusText = GetNameSafe(runtime.GameplayFocusActor);
	details.RotationText = FormatRotationMode(runtime.RotationMode);
	details.ReactionText = FormatReaction(runtime);
	details.DeferredText = runtime.bDeferredSyncPending
		? (runtime.bDeferredSyncQueued ? TEXT("Pending / Queued") : TEXT("Pending"))
		: TEXT("None");
	details.ExpectedText = FString::Printf(
		TEXT("Focus: %s | Rotation: %s"),
		*FormatFocusDirective(runtime.ExpectedFocusDirective),
		*FormatRotationDirective(runtime.ExpectedRotationDirective));
	details.LastDecisionSequenceText = FString::Printf(TEXT("#%u"), runtime.LastTransitionSequence);
	details.LastDecisionTimeText = FString::Printf(TEXT("%.3fs"), runtime.LastTransitionWorldTimeSeconds);
	details.LastDecisionTriggerText = runtime.LastEventName.IsEmpty() ? TEXT("None") : runtime.LastEventName;
	details.LastDecisionResultText = runtime.LastDecision.IsEmpty() ? TEXT("None") : runtime.LastDecision;
	return details;
}

// Gate

bool FEnemyCombatTargetFacingDebug::ShouldAuditCombatTargetFacing()
{
#if !UE_BUILD_SHIPPING
	return CVarCombatTargetFacingAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Facing Decision Diagnostic Hook

void FEnemyCombatTargetFacingDebug::RecordFacingDecision(
	const UCEnemyCombatTargetFacingComponent* InFacingComponent,
	const EMovementRotationMode InPreviousRotationMode,
	const AActor* InPreviousGameplayFocusActor,
	const TCHAR* InEvent,
	const TCHAR* InDecision)
{
	if (!IsValid(InFacingComponent)) return;

	const AActor* ownerActor = InFacingComponent->GetOwner();
	if (!IsValid(ownerActor)) return;

	const FEnemyCombatTargetFacingRuntimeSnapshot runtime = InFacingComponent->GetRuntimeSnapshot();
	const FString eventName = InEvent ? InEvent : TEXT("Unknown");
	const FString decision = InDecision ? InDecision : TEXT("Unknown");
	const FString summary = BuildTransitionSummary(runtime, InPreviousRotationMode, InPreviousGameplayFocusActor);

	FDebugOverlayFacingTransition transition;
	transition.Current.TransitionSequence = runtime.LastTransitionSequence;
	transition.Current.OwnerName = GetNameSafe(ownerActor);
	transition.Current.OwnerControllerName = GetNameSafe(runtime.OwnerAIController);
	transition.Current.BoundControllerName = GetNameSafe(runtime.BoundAIController);
	transition.Current.CombatTargetName = GetNameSafe(runtime.CombatTargetActor);
	transition.Current.GameplayFocusName = GetNameSafe(runtime.GameplayFocusActor);
	transition.Current.RotationMode = FormatRotationMode(runtime.RotationMode);
	transition.Current.PolicyState = FormatPolicyState(runtime.PolicyState);
	transition.Current.ExpectedFocusDirective = FormatFocusDirective(runtime.ExpectedFocusDirective);
	transition.Current.ExpectedRotationDirective = FormatRotationDirective(runtime.ExpectedRotationDirective);
	transition.Current.EventName = eventName;
	transition.Current.Decision = decision;
	transition.Current.bControllerBindingMatchesOwner = runtime.bControllerBindingMatchesOwner;
	transition.Current.Summary = summary;
	transition.OwnerActor = const_cast<AActor*>(ownerActor);
	transition.CombatTargetActor = runtime.CombatTargetActor;
	transition.PreviousGameplayFocusName = GetNameSafe(InPreviousGameplayFocusActor);
	transition.PreviousRotationMode = FormatRotationMode(InPreviousRotationMode);
	FDebugOverlaySnapshotStore::RecordFacingTransition(ownerActor, transition);

	if (!ShouldAuditCombatTargetFacing()) return;

	FLog::Log(FString::Printf(
		TEXT("[Enemy|CombatTargetFacing|%s] Owner=%s | %s"),
		*eventName,
		*GetNameSafe(ownerActor),
		*summary));
}
