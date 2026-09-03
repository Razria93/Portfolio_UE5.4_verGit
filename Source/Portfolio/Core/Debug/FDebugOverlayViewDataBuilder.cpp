#include "Core/Debug/FDebugOverlayViewDataBuilder.h"

#include "AI/Blackboard/CAIKey.h"
#include "Character/CAnimInstance.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CActionComponent.h"
#include "Component/CCharacterFeedbackComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Controller/CAIController.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlayEventCategory.h"
#include "Type/CActionKeyTypes.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

namespace
{
	// ===== Constants =====

	static constexpr float DebugOverlayRecentAIEventStaleSeconds = 5.0f;

	// ===== Common Text Helpers (Formatting) =====

	FString FormatBoolText(bool bInValue)
	{
		return bInValue ? TEXT("true") : TEXT("false");
	}

	FString FormatMissingText()
	{
		return TEXT("N/A");
	}

	FString FormatBuilderAgeSeconds(float InAgeSeconds)
	{
		return FString::Printf(TEXT("%.2f"), FMath::Max(0.f, InAgeSeconds));
	}

	FString FormatGuardActionPhase(EGuardActionPhase InPhase)
	{
		switch (InPhase)
		{
		case EGuardActionPhase::In:
			return TEXT("Guard In");
		case EGuardActionPhase::Out:
			return TEXT("Guard Out");
		default:
			return TEXT("Guard");
		}
	}

	FString FormatCompactEnumText(const FString& InValue)
	{
		int32 separatorIndex = INDEX_NONE;
		return InValue.FindLastChar(TEXT(':'), separatorIndex)
			&& separatorIndex > 0
			&& InValue[separatorIndex - 1] == TEXT(':')
			&& separatorIndex + 1 < InValue.Len()
			? InValue.RightChop(separatorIndex + 1)
			: InValue;
	}

	FString FormatCompactReasonText(const FString& InValue)
	{
		return FormatCompactEnumText(InValue.IsEmpty() ? FString(TEXT("None")) : InValue);
	}

	FString FindEventSummaryValue(const FString& InSummary, const TCHAR* InKey)
	{
		if (InSummary.IsEmpty() || !InKey || !*InKey)
		{
			return FString();
		}

		const FString colonPrefix = FString::Printf(TEXT("%s:"), InKey);
		const FString equalsPrefix = FString::Printf(TEXT("%s="), InKey);
		TArray<FString> fields;
		InSummary.ParseIntoArray(fields, TEXT("|"), true);
		for (FString& field : fields)
		{
			field.TrimStartAndEndInline();
			if (field.StartsWith(colonPrefix, ESearchCase::IgnoreCase))
			{
				return field.RightChop(colonPrefix.Len()).TrimStartAndEnd();
			}

			if (field.StartsWith(equalsPrefix, ESearchCase::IgnoreCase))
			{
				return field.RightChop(equalsPrefix.Len()).TrimStartAndEnd();
			}
		}

		return FString();
	}

	bool IsEventSummaryValueMeaningful(const FString& InValue)
	{
		return !InValue.IsEmpty()
			&& !InValue.Equals(TEXT("None"), ESearchCase::IgnoreCase)
			&& !InValue.Equals(TEXT("N/A"), ESearchCase::IgnoreCase)
			&& !InValue.Equals(TEXT("false"), ESearchCase::IgnoreCase);
	}

	void AddEventSummaryPart(TArray<FString>& InOutParts, const FString& InValue, const TCHAR* InLabel = nullptr)
	{
		if (!IsEventSummaryValueMeaningful(InValue))
		{
			return;
		}

		InOutParts.Add(InLabel && *InLabel
			? FString::Printf(TEXT("%s: %s"), InLabel, *InValue)
			: InValue);
	}

	FString FormatEventActorName(const FString& InActorName, const FString& InFocusedSubjectName)
	{
		if (InActorName.IsEmpty())
		{
			return FString();
		}

		return !InFocusedSubjectName.IsEmpty() && InActorName.Equals(InFocusedSubjectName, ESearchCase::CaseSensitive)
			? TEXT("Self")
			: InActorName;
	}

	FString BuildEventActorRelationship(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		const FString sourceName = FormatEventActorName(
			!InEvent.SourceName.IsEmpty() ? InEvent.SourceName : InEvent.OwnerName,
			InFocusedSubjectName);
		const FString targetName = FormatEventActorName(InEvent.TargetName, InFocusedSubjectName);
		if (!sourceName.IsEmpty() && !targetName.IsEmpty() && !sourceName.Equals(targetName, ESearchCase::CaseSensitive))
		{
			return FString::Printf(TEXT("%s -> %s"), *sourceName, *targetName);
		}

		return !sourceName.IsEmpty() ? sourceName : targetName;
	}

	FString BuildCombatActorRelationship(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		const FString sourceName = FormatEventActorName(
			!InEvent.SourceName.IsEmpty() ? InEvent.SourceName : InEvent.OwnerName,
			InFocusedSubjectName);
		const FString targetName = FormatEventActorName(InEvent.TargetName, InFocusedSubjectName);
		AddEventSummaryPart(parts, sourceName, TEXT("Source"));
		AddEventSummaryPart(parts, targetName, TEXT("Target"));
		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactCombatEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		AddEventSummaryPart(parts, BuildCombatActorRelationship(InEvent, InFocusedSubjectName));
		const FDebugOverlayCombatEventDetails& combatDetails = InEvent.CombatDetails;
		switch (combatDetails.Kind)
		{
		case EDebugOverlayCombatEventKind::CollisionWindow:
			AddEventSummaryPart(parts, combatDetails.CollisionState);
			if (combatDetails.HitWindowId != INDEX_NONE)
			{
				AddEventSummaryPart(parts, FString::FromInt(combatDetails.HitWindowId), TEXT("Window"));
			}
			AddEventSummaryPart(parts, combatDetails.CollisionName, TEXT("Collision"));
			AddEventSummaryPart(parts, combatDetails.Reason, TEXT("Reason"));
			break;

		case EDebugOverlayCombatEventKind::TargetResolution:
			AddEventSummaryPart(parts, FormatCompactEnumText(combatDetails.DefenseOutcome), TEXT("Defense"));
			AddEventSummaryPart(parts, FormatCompactEnumText(combatDetails.ReactionOutcome), TEXT("Reaction"));
			if (combatDetails.bHasDamageBreakdown)
			{
				AddEventSummaryPart(parts, FString::Printf(TEXT("%.3f -> %.3f"), combatDetails.RequestDamage, combatDetails.FinalTakenDamage), TEXT("Damage"));
			}
			if (combatDetails.bHasDamageCommit)
			{
				AddEventSummaryPart(parts, FString::Printf(TEXT("%.3f"), combatDetails.CommittedDamage), TEXT("Commit"));
			}
			break;

		case EDebugOverlayCombatEventKind::ResultDelivery:
			AddEventSummaryPart(parts, FormatCompactEnumText(combatDetails.DefenseOutcome), TEXT("Defense"));
			if (combatDetails.bHasDamageCommit)
			{
				AddEventSummaryPart(parts, FString::Printf(TEXT("%.3f"), combatDetails.CommittedDamage), TEXT("Commit"));
			}
			break;

		case EDebugOverlayCombatEventKind::None:
		default:
			return InEvent.Summary;
		}

		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactAIEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		AddEventSummaryPart(parts, BuildEventActorRelationship(InEvent, InFocusedSubjectName));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("IntentState")));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("SubState")));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Result")));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("RejectReason")), TEXT("Reject"));
		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactFacingEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		FString targetName = FindEventSummaryValue(InEvent.Summary, TEXT("Target"));
		if (!InFocusedSubjectName.IsEmpty() && targetName.StartsWith(InFocusedSubjectName, ESearchCase::CaseSensitive))
		{
			targetName = TEXT("Self") + targetName.RightChop(InFocusedSubjectName.Len());
		}

		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Policy")));
		AddEventSummaryPart(parts, targetName, TEXT("Target"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Focus")), TEXT("Focus"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Rotation")), TEXT("Rotation"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Decision")), TEXT("Decision"));
		if (FindEventSummaryValue(InEvent.Summary, TEXT("Dead")).Equals(TEXT("true"), ESearchCase::IgnoreCase))
		{
			parts.Add(TEXT("Dead"));
		}
		if (FindEventSummaryValue(InEvent.Summary, TEXT("BalanceSuppressed")).Equals(TEXT("true"), ESearchCase::IgnoreCase))
		{
			parts.Add(TEXT("Balance Suppressed"));
		}

		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactBalanceEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		AddEventSummaryPart(parts, BuildEventActorRelationship(InEvent, InFocusedSubjectName));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Count")), TEXT("Count"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("State")), TEXT("State"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Previous")), TEXT("Previous"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("New")), TEXT("New"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Reason")), TEXT("Reason"));
		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactActionReactionEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		AddEventSummaryPart(parts, BuildEventActorRelationship(InEvent, InFocusedSubjectName));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Subject")));
		const FString decision = FindEventSummaryValue(InEvent.Summary, TEXT("Decision"));
		const FString apply = FindEventSummaryValue(InEvent.Summary, TEXT("Apply"));
		if (IsEventSummaryValueMeaningful(decision) || IsEventSummaryValueMeaningful(apply))
		{
			parts.Add(FString::Printf(TEXT("%s / %s"), *FormatCompactReasonText(decision), *FormatCompactReasonText(apply)));
		}
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("RejectReason")), TEXT("Reject"));
		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactExecutionSessionEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		AddEventSummaryPart(parts, BuildEventActorRelationship(InEvent, InFocusedSubjectName));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Role")));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("State")));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Outcome")));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("Session")), TEXT("Session"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("SourceTerminal")), TEXT("Source"));
		AddEventSummaryPart(parts, FindEventSummaryValue(InEvent.Summary, TEXT("TargetTerminal")), TEXT("Target"));
		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactDeathEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		TArray<FString> parts;
		AddEventSummaryPart(parts, BuildEventActorRelationship(InEvent, InFocusedSubjectName));
		TArray<FString> fields;
		InEvent.Summary.ParseIntoArray(fields, TEXT("|"), true);
		for (FString& field : fields)
		{
			field.TrimStartAndEndInline();
			if (!field.StartsWith(TEXT("Owner:"), ESearchCase::IgnoreCase))
			{
				AddEventSummaryPart(parts, field);
			}
		}

		return FString::Join(parts, TEXT(" | "));
	}

	FString BuildCompactEventSummary(const FDebugOverlayEventEntry& InEvent, const FString& InFocusedSubjectName)
	{
		FString compactSummary;
		if (InEvent.Category.Equals(DebugOverlayEventCategory::Combat, ESearchCase::IgnoreCase))
		{
			compactSummary = BuildCompactCombatEventSummary(InEvent, InFocusedSubjectName);
		}
		else if (InEvent.Category.Equals(DebugOverlayEventCategory::AI, ESearchCase::IgnoreCase))
		{
			compactSummary = BuildCompactAIEventSummary(InEvent, InFocusedSubjectName);
		}
		else if (InEvent.Category.Equals(DebugOverlayEventCategory::Facing, ESearchCase::IgnoreCase))
		{
			compactSummary = BuildCompactFacingEventSummary(InEvent, InFocusedSubjectName);
		}
		else if (InEvent.Category.Equals(DebugOverlayEventCategory::Balance, ESearchCase::IgnoreCase))
		{
			compactSummary = BuildCompactBalanceEventSummary(InEvent, InFocusedSubjectName);
		}
		else if (InEvent.Category.Equals(DebugOverlayEventCategory::ActionReaction, ESearchCase::IgnoreCase))
		{
			compactSummary = BuildCompactActionReactionEventSummary(InEvent, InFocusedSubjectName);
		}
		else if (InEvent.Category.Equals(DebugOverlayEventCategory::ExecutionSession, ESearchCase::IgnoreCase))
		{
			compactSummary = BuildCompactExecutionSessionEventSummary(InEvent, InFocusedSubjectName);
		}
		else if (InEvent.Category.Equals(DebugOverlayEventCategory::Death, ESearchCase::IgnoreCase))
		{
			compactSummary = BuildCompactDeathEventSummary(InEvent, InFocusedSubjectName);
		}

		return compactSummary.IsEmpty() ? InEvent.Summary : compactSummary;
	}

	// ===== Component Access Helpers =====

	template <typename TComponent>
	TComponent* FindComponent(const APawn* InPawn)
	{
		return IsValid(InPawn) ? InPawn->FindComponentByClass<TComponent>() : nullptr;
	}

	// [Player / Enemy Status]
	// ===== Actor Status Formatting =====

	FString FormatExecutionState(const APawn* InPawn)
	{
		const UCStateComponent* stateComp = FindComponent<UCStateComponent>(InPawn);
		return IsValid(stateComp) ? FormatCompactEnumText(UEnum::GetValueAsString(stateComp->GetCurrentExecutionState())) : FormatMissingText();
	}

	FString FormatActiveAction(const APawn* InPawn)
	{
		const UCActionComponent* actionComp = FindComponent<UCActionComponent>(InPawn);
		if (!IsValid(actionComp)) return FormatMissingText();
		if (!actionComp->IsActive()) return TEXT("None");

		const EActionType actionType = actionComp->GetActiveActionType();
		const int32 actionIndex = actionComp->GetActiveActionIndex();

		if (actionType == EActionType::Guard)
		{
			FActionDataKey actionDataKey;
			actionDataKey.ActionType = actionType;
			actionDataKey.ActionIndex = actionIndex;

			const EGuardActionPhase guardPhase = ResolveGuardActionPhase(actionDataKey);
			if (guardPhase == EGuardActionPhase::In || guardPhase == EGuardActionPhase::Out)
			{
				return FormatGuardActionPhase(guardPhase);
			}

			return TEXT("Guard");
		}

		return FString::Printf(
			TEXT("%s[%d]"),
			*FormatCompactEnumText(UEnum::GetValueAsString(actionType)),
			actionIndex);
	}

	FString FormatActiveReaction(const APawn* InPawn)
	{
		const UCReactionComponent* reactionComp = FindComponent<UCReactionComponent>(InPawn);
		if (!IsValid(reactionComp)) return FormatMissingText();
		if (!reactionComp->IsActive()) return TEXT("None");

		return FormatCompactEnumText(UEnum::GetValueAsString(reactionComp->GetActiveReactionType()));
	}

	FString FormatActorHealth(const APawn* InPawn)
	{
		const UCHealthComponent* healthComp = FindComponent<UCHealthComponent>(InPawn);
		if (!IsValid(healthComp)) return FormatMissingText();

		return FString::Printf(
			TEXT("%.1f/%.1f (DeadState: %s)"),
			healthComp->GetCurrentHP(),
			healthComp->GetMaxHP(),
			*FormatCompactEnumText(UEnum::GetValueAsString(healthComp->GetDeadState())));
	}

	FString FormatActorBalance(const APawn* InPawn)
	{
		if (const ACEnemy* enemy = Cast<ACEnemy>(InPawn))
		{
			const UCBalanceComponent* balanceComp = enemy->GetBalanceComp();
			return IsValid(balanceComp)
				? FString::Printf(TEXT("%d/%d | %s"), balanceComp->GetCurrentBalanceCount(), balanceComp->GetBalanceThreshold(), *FormatCompactEnumText(UEnum::GetValueAsString(balanceComp->GetBalanceLifecycleState())))
				: FormatMissingText();
		}

		return FormatMissingText();
	}

	FString FormatGuardOverlay(const APawn* InPawn)
	{
		const UCDefenseComponent* defenseComp = FindComponent<UCDefenseComponent>(InPawn);
		if (!IsValid(defenseComp)) return FormatMissingText();

		return FString::Printf(
			TEXT("Wants: %s | Pose: %s | CanGuard: %s | CanParry: %s | CanStart: %s"),
			*FormatBoolText(defenseComp->WantsGuarding()),
			*FormatBoolText(defenseComp->IsGuardingPose()),
			*FormatBoolText(defenseComp->CanGuard()),
			*FormatBoolText(defenseComp->CanParry()),
			*FormatBoolText(defenseComp->CanStartGuard()));
	}

	void BuildActorMovementText(const APawn* InPawn, FString& OutMovementGaitRotationText, FString& OutLocomotionPresentationStateText)
	{
		const UCMovementComponent* movementComp = FindComponent<UCMovementComponent>(InPawn);
		if (!IsValid(movementComp))
		{
			OutMovementGaitRotationText = FormatMissingText();
			OutLocomotionPresentationStateText = FormatMissingText();
			return;
		}

		const ACharacter* character = Cast<ACharacter>(InPawn);
		const USkeletalMeshComponent* meshComp = IsValid(character) ? character->GetMesh() : nullptr;
		const UCAnimInstance* animInstance = IsValid(meshComp) ? Cast<UCAnimInstance>(meshComp->GetAnimInstance()) : nullptr;
		const FString presentationText = IsValid(animInstance)
			? FormatCompactEnumText(UEnum::GetValueAsString(animInstance->GetLocomotionPresentationMode()))
			: FormatMissingText();

		OutMovementGaitRotationText = FString::Printf(
			TEXT("Gait: %s | Rotation: %s"),
			*FormatCompactEnumText(UEnum::GetValueAsString(movementComp->GetCurrentMovementGait())),
			*FormatCompactEnumText(UEnum::GetValueAsString(movementComp->GetCurrentMovementRotationMode())));
		OutLocomotionPresentationStateText = FString::Printf(
			TEXT("Locomotion: Presentation: %s | Enabled: %s | Falling: %s"),
			*presentationText,
			*FormatBoolText(movementComp->IsMovementEnabled()),
			*FormatBoolText(movementComp->IsFalling()));
	}

	FString FormatRuntimeLODTier()
	{
		return FormatMissingText();
	}

	FDebugOverlayActorStatusViewData BuildActorStatusViewData(const APawn* InPawn)
	{
		FDebugOverlayActorStatusViewData statusViewData;
		statusViewData.StateText = FormatExecutionState(InPawn);
		statusViewData.ActionText = FormatActiveAction(InPawn);
		statusViewData.ReactionText = FormatActiveReaction(InPawn);
		statusViewData.HealthText = FormatActorHealth(InPawn);
		statusViewData.BalanceText = FormatActorBalance(InPawn);
		statusViewData.GuardText = FormatGuardOverlay(InPawn);
		BuildActorMovementText(InPawn, statusViewData.MovementGaitRotationText, statusViewData.LocomotionPresentationStateText);
		statusViewData.RuntimeLODText = FormatRuntimeLODTier();
		return statusViewData;
	}

	// ===== AI Text Helpers (Formatting) =====

	FString FormatAIIntentState(const UBlackboardComponent* InBlackboardComp)
	{
		if (!IsValid(InBlackboardComp)) return FormatMissingText();

		const uint8 intentStateValue = InBlackboardComp->GetValueAsEnum(CAIKey::State::AIIntentState.KeyName);
		if (intentStateValue >= static_cast<uint8>(EAIIntentState::Max)) return FormatMissingText();

		return FormatCompactEnumText(UEnum::GetValueAsString(static_cast<EAIIntentState>(intentStateValue)));
	}

	FString FormatAITargetActor(const UBlackboardComponent* InBlackboardComp)
	{
		if (!IsValid(InBlackboardComp)) return FormatMissingText();

		const UObject* targetObject = InBlackboardComp->GetValueAsObject(CAIKey::CombatTarget::Actor.KeyName);
		return IsValid(targetObject) ? GetNameSafe(targetObject) : FormatMissingText();
	}

	FString FormatBlackboardBool(const UBlackboardComponent* InBlackboardComp, FName InKeyName)
	{
		return IsValid(InBlackboardComp) ? FormatBoolText(InBlackboardComp->GetValueAsBool(InKeyName)) : FormatMissingText();
	}

	FString FormatBlackboardFloat(const UBlackboardComponent* InBlackboardComp, FName InKeyName)
	{
		return IsValid(InBlackboardComp) ? FString::Printf(TEXT("%.1f"), InBlackboardComp->GetValueAsFloat(InKeyName)) : FormatMissingText();
	}

	// ===== AI Query Helpers =====

	bool HasAITargetActor(const UBlackboardComponent* InBlackboardComp)
	{
		if (!IsValid(InBlackboardComp)) return false;

		const UObject* targetObject = InBlackboardComp->GetValueAsObject(CAIKey::CombatTarget::Actor.KeyName);
		return IsValid(targetObject);
	}

	bool TryGetCapturedRecentAISummary(const UWorld* InWorld, const ACEnemy* InEnemy, bool bInHasSnapshot, FDebugOverlayAISummary& OutSummary)
	{
		OutSummary = FDebugOverlayAISummary();

		if (!bInHasSnapshot || !IsValid(InEnemy))
		{
			return false;
		}

		const FString enemyName = GetNameSafe(InEnemy);
		if (enemyName.IsEmpty())
		{
			return false;
		}

		if (!FDebugOverlaySnapshotStore::TryGetRecentAIForPawn(InWorld, enemyName, OutSummary))
		{
			return false;
		}

		return OutSummary.CaptureState == EDebugOverlayCaptureState::Captured;
	}

	// [Recent Action / Reaction]
	// ===== Actor Panel ViewData =====

	FDebugOverlayRecentActionReactionViewData BuildActorRecentActionReactionViewData(const UWorld* InWorld, bool bInHasSnapshot, const APawn* InPawn)
	{
		FDebugOverlayRecentActionReactionViewData recentActionReactionViewData;
		recentActionReactionViewData.HeaderText = TEXT("[Recent Action / Reaction]");
		if (!bInHasSnapshot)
		{
			recentActionReactionViewData.State = EDebugOverlayRecentActionReactionViewState::NotCaptured;
			return recentActionReactionViewData;
		}

		if (!IsValid(InPawn))
		{
			recentActionReactionViewData.State = EDebugOverlayRecentActionReactionViewState::NoActor;
			return recentActionReactionViewData;
		}

		const TArray<FDebugOverlayEventEntry> actionReactionEvents = FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(
			InWorld,
			1,
			DebugOverlayEventCategory::ActionReaction,
			InPawn);

		if (actionReactionEvents.IsEmpty())
		{
			recentActionReactionViewData.State = EDebugOverlayRecentActionReactionViewState::NoEvents;
			return recentActionReactionViewData;
		}

		recentActionReactionViewData.State = EDebugOverlayRecentActionReactionViewState::Captured;
		recentActionReactionViewData.SummaryText = actionReactionEvents[0].Summary;
		return recentActionReactionViewData;
	}

	// [Current AI / Recent AI Event]
	// ===== Enemy Panel ViewData =====

	FDebugOverlayCurrentAIViewData BuildEnemyCurrentAIViewData(const ACEnemy* InEnemy)
	{
		FDebugOverlayCurrentAIViewData currentAIViewData;
		currentAIViewData.bHasEnemy = IsValid(InEnemy);
		if (!currentAIViewData.bHasEnemy)
		{
			return currentAIViewData;
		}

		const ACAIController* aiController = Cast<ACAIController>(InEnemy->GetController());
		const UBlackboardComponent* blackboardComp = IsValid(aiController) ? aiController->GetBlackboardComponent() : nullptr;
		const bool bHasTarget = HasAITargetActor(blackboardComp);

		currentAIViewData.ControllerText = GetNameSafe(aiController);
		currentAIViewData.PawnText = GetNameSafe(InEnemy);
		currentAIViewData.TargetText = FormatAITargetActor(blackboardComp);
		currentAIViewData.IntentStateText = FormatAIIntentState(blackboardComp);
		currentAIViewData.ReturnHomeText = FormatBlackboardBool(blackboardComp, CAIKey::Navigation::bReturnHome.KeyName);
		currentAIViewData.UsePatrolText = FormatBlackboardBool(blackboardComp, CAIKey::Patrol::bUsePatrol.KeyName);
		currentAIViewData.HasLOSText = FormatBlackboardBool(blackboardComp, CAIKey::Perception::bHasLOS.KeyName);
		currentAIViewData.DistanceToTargetText = bHasTarget ? FormatBlackboardFloat(blackboardComp, CAIKey::Metric::DistanceToTarget.KeyName) : FormatMissingText();
		currentAIViewData.IsCombatActionText = FormatBlackboardBool(blackboardComp, CAIKey::Engage::bIsCombatAction.KeyName);
		return currentAIViewData;
	}

	FDebugOverlayRecentAIEventViewData BuildEnemyRecentAIEventViewData(const ACEnemy* InEnemy, bool bInHasSnapshot, const UWorld* InWorld)
	{
		FDebugOverlayRecentAIEventViewData recentAIEventViewData;
		if (!IsValid(InEnemy))
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::NoTarget;
			return recentAIEventViewData;
		}

		if (!bInHasSnapshot)
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::NotCaptured;
			return recentAIEventViewData;
		}

		FDebugOverlayAISummary cachedSummary;
		if (!TryGetCapturedRecentAISummary(InWorld, InEnemy, bInHasSnapshot, cachedSummary))
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::NotCaptured;
			return recentAIEventViewData;
		}

		const float currentTime = IsValid(InWorld) ? InWorld->GetTimeSeconds() : cachedSummary.WorldTimeSeconds;
		const float eventAge = currentTime - cachedSummary.WorldTimeSeconds;
		const bool bEventStale = eventAge > DebugOverlayRecentAIEventStaleSeconds;

		if (bEventStale)
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::Stale;
			recentAIEventViewData.StaleAgeText = FormatBuilderAgeSeconds(eventAge);
			recentAIEventViewData.TaskText = FormatCompactEnumText(cachedSummary.SubState);
			recentAIEventViewData.ResultText = FormatCompactEnumText(cachedSummary.RequestResult);
			recentAIEventViewData.RejectReasonText = FormatCompactReasonText(cachedSummary.RejectReason);
			return recentAIEventViewData;
		}

		recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::Captured;
		recentAIEventViewData.TaskText = FormatCompactEnumText(cachedSummary.SubState);
		recentAIEventViewData.ResultText = FormatCompactEnumText(cachedSummary.RequestResult);
		recentAIEventViewData.AgeText = FormatBuilderAgeSeconds(eventAge);
		recentAIEventViewData.RejectReasonText = FormatCompactReasonText(cachedSummary.RejectReason);
		return recentAIEventViewData;
	}

	FDebugOverlayDeathLifecycleViewData BuildEnemyDeathLifecycleViewData(const ACEnemy* InEnemy)
	{
		FDebugOverlayDeathLifecycleViewData viewData;
		if (!IsValid(InEnemy))
		{
			viewData.HealthStateText = FormatMissingText();
			viewData.LifecycleText = FormatMissingText();
			viewData.DeathEntryText = FormatMissingText();
			viewData.PresentationText = FormatMissingText();
			viewData.FallbackTimerText = FormatMissingText();
			viewData.FinalizationText = FormatMissingText();
			return viewData;
		}

		const UCHealthComponent* healthComp = InEnemy->GetHealthComp();
		const UCReactionComponent* reactionComp = InEnemy->GetReactionComp();
		const UCCharacterFeedbackComponent* feedbackComp = InEnemy->GetCharacterFeedbackComp();

		viewData.HealthStateText = IsValid(healthComp) ? FormatCompactEnumText(UEnum::GetValueAsString(healthComp->GetDeadState())) : FormatMissingText();
		viewData.LifecycleText = InEnemy->IsDeathLifecycleActive() ? TEXT("Active") : TEXT("Inactive");
		const EReactionType deathEntryReactionType = InEnemy->GetDeathPresentationMode() == EDeathPresentationMode::ExecutionLethal
			? EReactionType::ExecutionLethal
			: EReactionType::Dead;
		const FString deathEntryTypeText = FormatCompactEnumText(UEnum::GetValueAsString(deathEntryReactionType));
		const FString deathEntryStateText = IsValid(reactionComp) && reactionComp->IsActiveReactionType(deathEntryReactionType)
			? TEXT("Active")
			: (InEnemy->IsDeathPresentationRequested() ? TEXT("Exited") : TEXT("Inactive"));
		const FExecutionSessionId& expectedSessionId = InEnemy->GetExpectedExecutionLethalDeathSessionId();
		viewData.DeathEntryText = expectedSessionId.IsValidMinimal()
			? FString::Printf(TEXT("%s / %s / Session %u"), *deathEntryTypeText, *deathEntryStateText, expectedSessionId.Serial)
			: FString::Printf(TEXT("%s / %s"), *deathEntryTypeText, *deathEntryStateText);
		viewData.PresentationText = IsValid(feedbackComp) ? FormatCompactEnumText(UEnum::GetValueAsString(feedbackComp->GetDeathPresentationState())) : FormatMissingText();
		viewData.FallbackTimerText = InEnemy->IsDeathPresentationFallbackPending() ? TEXT("Pending") : TEXT("Inactive");
		viewData.FinalizationText = InEnemy->IsDeathFinalized() ? TEXT("Finalized") : (InEnemy->IsDeathFinalizationRequested() ? TEXT("Requested") : TEXT("Inactive"));
		return viewData;
	}

	// [Character Details]
	// ===== Main Actor Panel ViewData =====

	FDebugOverlayActorPanelViewData BuildActorPanelViewData(const TCHAR* InPanelName, const APawn* InPawn, const UWorld* InWorld, bool bInHasSnapshot)
	{
		FDebugOverlayActorPanelViewData actorPanelViewData;
		actorPanelViewData.HeaderText = InPanelName;
		actorPanelViewData.Status = BuildActorStatusViewData(InPawn);
		actorPanelViewData.RecentActionReaction = BuildActorRecentActionReactionViewData(InWorld, bInHasSnapshot, InPawn);
		return actorPanelViewData;
	}

	void AppendPlayerPanelViewData(FDebugOverlayViewData& InOutViewData, const APawn* InPlayerPawn, const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting, const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotion, const FDebugOverlayExecutionSessionViewData& InExecutionSession, bool bInHasSnapshot, const UWorld* InWorld, const FDebugOverlayPanelVisibility& InVisibility)
	{
		FDebugOverlayActorPanelViewData playerPanelViewData = BuildActorPanelViewData(TEXT("[Player]"), InPlayerPawn, InWorld, bInHasSnapshot);
		playerPanelViewData.bIncludeStatus = InVisibility.bShowPlayerStatus;
		playerPanelViewData.bIncludeTargeting = InVisibility.bShowPlayerTargeting && InPlayerTargeting.Details.bHasSnapshot;
		playerPanelViewData.Targeting = InPlayerTargeting;
		playerPanelViewData.bIncludeLocomotion = InVisibility.bShowPlayerLocomotion && InPlayerLocomotion.Details.bHasSnapshot;
		playerPanelViewData.Locomotion = InPlayerLocomotion;
		playerPanelViewData.bIncludeExecutionSession = InVisibility.bShowPlayerExecutionSession && InExecutionSession.Details.bHasSnapshot;
		playerPanelViewData.ExecutionSession = InExecutionSession;
		playerPanelViewData.bIncludeRecentActionReaction = InVisibility.bShowPlayerRecentActionReaction;
		if (!playerPanelViewData.bIncludeStatus
			&& !playerPanelViewData.bIncludeTargeting
			&& !playerPanelViewData.bIncludeLocomotion
			&& !playerPanelViewData.bIncludeExecutionSession
			&& !playerPanelViewData.bIncludeRecentActionReaction)
		{
			return;
		}

		InOutViewData.ActorPanels.Add(playerPanelViewData);
	}

	void AppendEnemyPanelViewData(FDebugOverlayViewData& InOutViewData, const ACEnemy* InEnemy, const FDebugOverlayFocusViewData& InEnemyFocus, const FDebugOverlayBalanceCollapseViewData& InBalanceCollapse, const FDebugOverlayCombatTargetFacingViewData& InCombatTargetFacing, const FDebugOverlayExecutionSessionViewData& InExecutionSession, const FDebugOverlayCombatParticipationViewData& InCombatParticipation, bool bInHasSnapshot, const UWorld* InWorld, const FDebugOverlayPanelVisibility& InVisibility)
	{
		FDebugOverlayActorPanelViewData enemyPanelViewData = BuildActorPanelViewData(TEXT("[Enemy]"), InEnemy, InWorld, bInHasSnapshot);
		enemyPanelViewData.bIncludeFocus = InVisibility.bShowEnemyFocus;
		enemyPanelViewData.Focus = InEnemyFocus;
		enemyPanelViewData.bIncludeStatus = InVisibility.bShowEnemyStatus;
		enemyPanelViewData.bIncludeBalanceCollapse = InVisibility.bShowEnemyBalanceCollapse && InBalanceCollapse.Details.bHasSnapshot;
		enemyPanelViewData.BalanceCollapse = InBalanceCollapse;
		enemyPanelViewData.bIncludeCombatTargetFacing = InVisibility.bShowEnemyCombatTargetFacing && InCombatTargetFacing.Details.bHasSnapshot;
		enemyPanelViewData.CombatTargetFacing = InCombatTargetFacing;
		enemyPanelViewData.bIncludeExecutionSession = InVisibility.bShowEnemyExecutionSession && InExecutionSession.Details.bHasSnapshot;
		enemyPanelViewData.ExecutionSession = InExecutionSession;
		enemyPanelViewData.bIncludeCombatParticipation = InVisibility.bShowEnemyCombatParticipation && InCombatParticipation.FocusedEnemyDetails.bHasSnapshot;
		enemyPanelViewData.CombatParticipation = InCombatParticipation;
		enemyPanelViewData.bAppendBlankBeforeStatus = enemyPanelViewData.bIncludeFocus && enemyPanelViewData.bIncludeStatus;
		enemyPanelViewData.bIncludeDeathLifecycle = InVisibility.bShowEnemyDeathLifecycle;
		enemyPanelViewData.DeathLifecycle = BuildEnemyDeathLifecycleViewData(InEnemy);
		enemyPanelViewData.bIncludeRecentActionReaction = InVisibility.bShowEnemyRecentActionReaction;
		enemyPanelViewData.bIncludeCurrentAI = InVisibility.bShowEnemyCurrentAI;
		enemyPanelViewData.CurrentAI = BuildEnemyCurrentAIViewData(InEnemy);
		enemyPanelViewData.bIncludeRecentAIEvent = InVisibility.bShowEnemyRecentAIEvent;
		enemyPanelViewData.RecentAIEvent = BuildEnemyRecentAIEventViewData(InEnemy, bInHasSnapshot, InWorld);
		if (!enemyPanelViewData.bIncludeFocus
			&& !enemyPanelViewData.bIncludeStatus
			&& !enemyPanelViewData.bIncludeBalanceCollapse
			&& !enemyPanelViewData.bIncludeCombatTargetFacing
			&& !enemyPanelViewData.bIncludeExecutionSession
			&& !enemyPanelViewData.bIncludeCombatParticipation
			&& !enemyPanelViewData.bIncludeDeathLifecycle
			&& !enemyPanelViewData.bIncludeRecentActionReaction
			&& !enemyPanelViewData.bIncludeCurrentAI
			&& !enemyPanelViewData.bIncludeRecentAIEvent)
		{
			return;
		}

		InOutViewData.ActorPanels.Add(enemyPanelViewData);
	}

	void BuildMainActorPanelData(FDebugOverlayViewData& InOutViewData, const APawn* InPlayerPawn, const ACEnemy* InEnemy, const FDebugOverlayFocusViewData& InEnemyFocus, const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting, const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotion, const FDebugOverlayExecutionSessionViewData& InPlayerExecutionSession, const FDebugOverlayBalanceCollapseViewData& InBalanceCollapse, const FDebugOverlayCombatTargetFacingViewData& InCombatTargetFacing, const FDebugOverlayExecutionSessionViewData& InEnemyExecutionSession, const FDebugOverlayCombatParticipationViewData& InCombatParticipation, bool bInHasSnapshot, const UWorld* InWorld, const FDebugOverlayPanelVisibility& InVisibility)
	{
		if (InVisibility.bShowPlayer)
		{
			AppendPlayerPanelViewData(InOutViewData, InPlayerPawn, InPlayerTargeting, InPlayerLocomotion, InPlayerExecutionSession, bInHasSnapshot, InWorld, InVisibility);
		}

		if (InVisibility.bShowEnemy)
		{
			AppendEnemyPanelViewData(InOutViewData, InEnemy, InEnemyFocus, InBalanceCollapse, InCombatTargetFacing, InEnemyExecutionSession, InCombatParticipation, bInHasSnapshot, InWorld, InVisibility);
		}
	}

	// [Event Log]
	// ===== EventLog ViewData =====

	FDebugOverlayEventLogViewData BuildEventLogViewData(bool bInHasSnapshot, const TArray<FDebugOverlayEventEntry>& InEvents, const FString& InEventLogFilter, const FString& InEventLogScope, int32 InEventLogLimit, const FString& InSubjectName = FString(), const bool bInFocusedScopeWithoutSubject = false)
	{
		FDebugOverlayEventLogViewData eventLogViewData;
		eventLogViewData.bHasSnapshot = bInHasSnapshot;
		eventLogViewData.DisplayLimit = InEventLogLimit;
		eventLogViewData.FilterText = InEventLogFilter;
		eventLogViewData.ScopeText = InEventLogScope;
		eventLogViewData.SubjectText = InSubjectName;
		eventLogViewData.bFocusedScopeWithoutSubject = bInFocusedScopeWithoutSubject;
		eventLogViewData.Entries.Reserve(InEvents.Num());

		for (const FDebugOverlayEventEntry& eventEntry : InEvents)
		{
			FDebugOverlayEventLogEntryViewData entryViewData;
			entryViewData.CategoryText = eventEntry.Category;
			entryViewData.EventNameText = eventEntry.EventName;
			entryViewData.SummaryText = BuildCompactEventSummary(eventEntry, InSubjectName);
			eventLogViewData.Entries.Add(entryViewData);
		}

		return eventLogViewData;
	}

	// [World Summary]
	// ===== World Summary ViewData =====

	FDebugOverlayRecentSummaryBlockViewData BuildRecentSummaryBlockViewData(const TCHAR* InBlockName, const FString& InSummary, EDebugOverlayCaptureState InCaptureState, bool bInHasSnapshot, bool bInAppendLeadingBlank)
	{
		FDebugOverlayRecentSummaryBlockViewData blockViewData;
		blockViewData.HeaderText = InBlockName;
		blockViewData.SummaryText = InSummary;
		blockViewData.CaptureState = InCaptureState;
		blockViewData.bHasSnapshot = bInHasSnapshot;
		blockViewData.bAppendLeadingBlank = bInAppendLeadingBlank;
		return blockViewData;
	}

	FString BuildRecentCombatResolutionSummary(const FDebugOverlayCombatResolutionSummary& InResolution)
	{
		TArray<FString> parts;
		AddEventSummaryPart(parts, InResolution.SourceName, TEXT("Source"));
		AddEventSummaryPart(parts, InResolution.TargetName, TEXT("Target"));
		AddEventSummaryPart(parts, FormatCompactEnumText(InResolution.DefenseOutcome), TEXT("Defense"));
		AddEventSummaryPart(parts, FormatCompactEnumText(InResolution.ReactionOutcome), TEXT("Reaction"));
		if (InResolution.bHasDamageBreakdown)
		{
			AddEventSummaryPart(parts, FString::Printf(TEXT("%.1f -> %.1f"), InResolution.RequestDamage, InResolution.FinalTakenDamage), TEXT("Damage"));
		}
		if (InResolution.bHasDamageCommit)
		{
			AddEventSummaryPart(parts, FString::Printf(TEXT("%.1f"), InResolution.CommittedDamage), TEXT("Commit"));
		}

		return FString::Join(parts, TEXT(" | "));
	}

	FDebugOverlayWorldSummaryViewData BuildWorldSummaryViewData(const FDebugOverlaySnapshot& InSnapshot, const FDebugOverlayCombatParticipationViewData& InCombatParticipation, bool bInHasSnapshot)
	{
		FDebugOverlayWorldSummaryViewData worldSummaryViewData;
		worldSummaryViewData.HeaderText = TEXT("[World Summary]");
		worldSummaryViewData.SummaryBlocks.Reserve(3);
		worldSummaryViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent Action / Reaction]"),
			InSnapshot.LastActionReaction.Summary,
			InSnapshot.LastActionReaction.CaptureState,
			bInHasSnapshot,
			false));
		worldSummaryViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent Combat]"),
			BuildRecentCombatResolutionSummary(InSnapshot.LastCombatResolution),
			InSnapshot.LastCombatResolution.CaptureState,
			bInHasSnapshot,
			true));
		worldSummaryViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent AI Event]"),
			InSnapshot.LastAI.Summary,
			InSnapshot.LastAI.CaptureState,
			bInHasSnapshot,
			true));
		worldSummaryViewData.bIncludeCombatParticipation = !InCombatParticipation.WorldSummaryLines.IsEmpty();
		worldSummaryViewData.CombatParticipation = InCombatParticipation;
		return worldSummaryViewData;
	}
}

// ===== Public API =====

FDebugOverlayViewData FDebugOverlayViewDataBuilder::Build(
	const UWorld* InWorld,
	const APawn* InViewerPawn,
	const ACEnemy* InDisplayEnemy,
	const FDebugOverlayFocusViewData& InEnemyFocus,
	const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting,
	const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotion,
	const FDebugOverlayExecutionSessionViewData& InPlayerExecutionSession,
	const FDebugOverlayBalanceCollapseViewData& InBalanceCollapse,
	const FDebugOverlayCombatTargetFacingViewData& InCombatTargetFacing,
	const FDebugOverlayExecutionSessionViewData& InEnemyExecutionSession,
	const FDebugOverlayCombatParticipationViewData& InCombatParticipation,
	const FDebugOverlayPanelVisibility& InPanelVisibility)
{
	FDebugOverlaySnapshot snapshot;
	const bool bHasSnapshot = FDebugOverlaySnapshotStore::TryGetSnapshotCopy(InWorld, snapshot);
	const FString eventLogFilter = FDebugOverlaySnapshotStore::GetEventLogFilter();
	const FString eventLogScope = FDebugOverlaySnapshotStore::GetEventLogScope();
	const int32 eventLogLimit = FDebugOverlaySnapshotStore::GetEventLogDisplayLimit();
	const bool bFocusedEnemyScope = eventLogScope.Equals(TEXT("FocusedEnemy"), ESearchCase::IgnoreCase);
	const bool bHasFocusedEnemy = IsValid(InDisplayEnemy);
	const FString eventLogSubjectName = bFocusedEnemyScope && bHasFocusedEnemy ? GetNameSafe(InDisplayEnemy) : FString();
	const TArray<FDebugOverlayEventEntry> recentEvents = bFocusedEnemyScope
		? (bHasFocusedEnemy
			? FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(InWorld, eventLogLimit, eventLogFilter, InDisplayEnemy)
			: TArray<FDebugOverlayEventEntry>())
		: FDebugOverlaySnapshotStore::GetRecentEventsCopy(InWorld, eventLogLimit, eventLogFilter);

	FDebugOverlayViewData viewData;
	viewData.ActorPanels.Reserve(2);

	BuildMainActorPanelData(viewData, InViewerPawn, InDisplayEnemy, InEnemyFocus, InPlayerTargeting, InPlayerLocomotion, InPlayerExecutionSession, InBalanceCollapse, InCombatTargetFacing, InEnemyExecutionSession, InCombatParticipation, bHasSnapshot, InWorld, InPanelVisibility);

	viewData.EventLog = BuildEventLogViewData(bHasSnapshot, recentEvents, eventLogFilter, bFocusedEnemyScope ? TEXT("Focused Enemy") : TEXT("World"), eventLogLimit, eventLogSubjectName, bFocusedEnemyScope && !bHasFocusedEnemy);

	viewData.WorldSummary = BuildWorldSummaryViewData(snapshot, InCombatParticipation, bHasSnapshot);

	return viewData;
}
