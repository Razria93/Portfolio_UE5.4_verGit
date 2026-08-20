#include "Core/Debug/FDebugOverlayViewDataBuilder.h"

#include "AI/Blackboard/CAIKey.h"
#include "Character/CAnimInstance.h"
#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "Component/CActionComponent.h"
#include "Component/CCharacterFeedbackComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Controller/CAIController.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
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

	FString FormatParryStaggerStack(const APawn* InPawn)
	{
		if (const ACPlayer* player = Cast<ACPlayer>(InPawn))
		{
			return FString::Printf(TEXT("%d/%d"), player->GetParryResultCount(), player->GetParryStaggerThreshold());
		}

		if (const ACEnemy* enemy = Cast<ACEnemy>(InPawn))
		{
			return FString::Printf(TEXT("%d/%d"), enemy->GetParryResultCount(), enemy->GetParryStaggerThreshold());
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

	FString FormatActorMovement(const APawn* InPawn)
	{
		const UCMovementComponent* movementComp = FindComponent<UCMovementComponent>(InPawn);
		if (!IsValid(movementComp)) return FormatMissingText();

		const ACharacter* character = Cast<ACharacter>(InPawn);
		const USkeletalMeshComponent* meshComp = IsValid(character) ? character->GetMesh() : nullptr;
		const UCAnimInstance* animInstance = IsValid(meshComp) ? Cast<UCAnimInstance>(meshComp->GetAnimInstance()) : nullptr;
		const FString presentationText = IsValid(animInstance)
			? FormatCompactEnumText(UEnum::GetValueAsString(animInstance->GetLocomotionPresentationMode()))
			: FormatMissingText();

		return FString::Printf(
			TEXT("Gait: %s | Rotation: %s | Presentation: %s | CanMove: %s | Falling: %s"),
			*FormatCompactEnumText(UEnum::GetValueAsString(movementComp->GetCurrentMovementGait())),
			*FormatCompactEnumText(UEnum::GetValueAsString(movementComp->GetCurrentMovementRotationMode())),
			*presentationText,
			*FormatBoolText(movementComp->CanMove()),
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
		statusViewData.StaggerText = FormatParryStaggerStack(InPawn);
		statusViewData.GuardText = FormatGuardOverlay(InPawn);
		statusViewData.MovementText = FormatActorMovement(InPawn);
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

	// [Recent Execution]
	// ===== Actor Panel ViewData =====

	FDebugOverlayRecentExecutionViewData BuildActorRecentExecutionViewData(const UWorld* InWorld, bool bInHasSnapshot, const APawn* InPawn)
	{
		FDebugOverlayRecentExecutionViewData recentExecutionViewData;
		recentExecutionViewData.HeaderText = TEXT("[Recent Execution]");
		if (!bInHasSnapshot)
		{
			recentExecutionViewData.State = EDebugOverlayRecentExecutionViewState::NotCaptured;
			return recentExecutionViewData;
		}

		if (!IsValid(InPawn))
		{
			recentExecutionViewData.State = EDebugOverlayRecentExecutionViewState::NoActor;
			return recentExecutionViewData;
		}

		const TArray<FDebugOverlayEventEntry> executionEvents = FDebugOverlaySnapshotStore::GetRecentEventsForSubjectCopy(
			InWorld,
			1,
			TEXT("Execution"),
			GetNameSafe(InPawn));

		if (executionEvents.IsEmpty())
		{
			recentExecutionViewData.State = EDebugOverlayRecentExecutionViewState::NoEvents;
			return recentExecutionViewData;
		}

		recentExecutionViewData.State = EDebugOverlayRecentExecutionViewState::Captured;
		recentExecutionViewData.SummaryText = executionEvents[0].Summary;
		return recentExecutionViewData;
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
			viewData.DeadInText = FormatMissingText();
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
		viewData.DeadInText = IsValid(reactionComp) && reactionComp->IsActiveReactionType(EReactionType::Dead) ? TEXT("Active") : (InEnemy->IsDeathPresentationRequested() ? TEXT("Exited") : TEXT("Inactive"));
		viewData.PresentationText = IsValid(feedbackComp) ? FormatCompactEnumText(UEnum::GetValueAsString(feedbackComp->GetDeathPresentationState())) : FormatMissingText();
		viewData.FallbackTimerText = InEnemy->IsDeathPresentationFallbackPending() ? TEXT("Pending") : TEXT("Inactive");
		viewData.FinalizationText = InEnemy->IsDeathFinalized() ? TEXT("Finalized") : (InEnemy->IsDeathFinalizationRequested() ? TEXT("Requested") : TEXT("Inactive"));
		return viewData;
	}

	// [Panel_01]
	// ===== Main Actor Panel ViewData =====

	FDebugOverlayActorPanelViewData BuildActorPanelViewData(const TCHAR* InPanelName, const APawn* InPawn, const UWorld* InWorld, bool bInHasSnapshot)
	{
		FDebugOverlayActorPanelViewData actorPanelViewData;
		actorPanelViewData.HeaderText = InPanelName;
		actorPanelViewData.Status = BuildActorStatusViewData(InPawn);
		actorPanelViewData.RecentExecution = BuildActorRecentExecutionViewData(InWorld, bInHasSnapshot, InPawn);
		return actorPanelViewData;
	}

	void AppendPlayerPanelViewData(FDebugOverlayViewData& InOutViewData, const APawn* InPlayerPawn, const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting, const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotion, bool bInHasSnapshot, const UWorld* InWorld)
	{
		FDebugOverlayActorPanelViewData playerPanelViewData = BuildActorPanelViewData(TEXT("[Player]"), InPlayerPawn, InWorld, bInHasSnapshot);
		playerPanelViewData.bIncludeTargeting = InPlayerTargeting.Details.bHasSnapshot;
		playerPanelViewData.Targeting = InPlayerTargeting;
		playerPanelViewData.bIncludeLocomotion = InPlayerLocomotion.Details.bHasSnapshot;
		playerPanelViewData.Locomotion = InPlayerLocomotion;
		InOutViewData.ActorPanels.Add(playerPanelViewData);
	}

	void AppendEnemyPanelViewData(FDebugOverlayViewData& InOutViewData, const ACEnemy* InEnemy, const FDebugOverlayFocusViewData& InEnemyFocus, const FDebugOverlayCombatParticipationViewData& InCombatParticipation, bool bInHasSnapshot, const UWorld* InWorld)
	{
		FDebugOverlayActorPanelViewData enemyPanelViewData = BuildActorPanelViewData(TEXT("[Enemy]"), InEnemy, InWorld, bInHasSnapshot);
		enemyPanelViewData.bIncludeFocus = true;
		enemyPanelViewData.Focus = InEnemyFocus;
		enemyPanelViewData.bIncludeCombatParticipation = InCombatParticipation.FocusedEnemyDetails.bHasSnapshot;
		enemyPanelViewData.CombatParticipation = InCombatParticipation;
		enemyPanelViewData.bAppendBlankBeforeStatus = true;
		enemyPanelViewData.bIncludeDeathLifecycle = true;
		enemyPanelViewData.DeathLifecycle = BuildEnemyDeathLifecycleViewData(InEnemy);
		enemyPanelViewData.bIncludeCurrentAI = true;
		enemyPanelViewData.CurrentAI = BuildEnemyCurrentAIViewData(InEnemy);
		enemyPanelViewData.bIncludeRecentAIEvent = true;
		enemyPanelViewData.RecentAIEvent = BuildEnemyRecentAIEventViewData(InEnemy, bInHasSnapshot, InWorld);
		InOutViewData.ActorPanels.Add(enemyPanelViewData);
	}

	void BuildMainActorPanelData(FDebugOverlayViewData& InOutViewData, const APawn* InPlayerPawn, const ACEnemy* InEnemy, const FDebugOverlayFocusViewData& InEnemyFocus, const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting, const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotion, const FDebugOverlayCombatParticipationViewData& InCombatParticipation, bool bInHasSnapshot, const UWorld* InWorld)
	{
		AppendPlayerPanelViewData(InOutViewData, InPlayerPawn, InPlayerTargeting, InPlayerLocomotion, bInHasSnapshot, InWorld);
		AppendEnemyPanelViewData(InOutViewData, InEnemy, InEnemyFocus, InCombatParticipation, bInHasSnapshot, InWorld);
	}

	// [Panel_02]
	// ===== EventLog ViewData =====

	FDebugOverlayEventLogViewData BuildEventLogViewData(bool bInHasSnapshot, const TArray<FDebugOverlayEventEntry>& InEvents, const FString& InEventLogFilter, int32 InEventLogLimit)
	{
		FDebugOverlayEventLogViewData eventLogViewData;
		eventLogViewData.bHasSnapshot = bInHasSnapshot;
		eventLogViewData.DisplayLimit = InEventLogLimit;
		eventLogViewData.FilterText = InEventLogFilter;
		eventLogViewData.Entries.Reserve(InEvents.Num());

		for (const FDebugOverlayEventEntry& eventEntry : InEvents)
		{
			FDebugOverlayEventLogEntryViewData entryViewData;
			entryViewData.CategoryText = eventEntry.Category;
			entryViewData.EventNameText = eventEntry.EventName;
			entryViewData.SummaryText = eventEntry.Summary;
			eventLogViewData.Entries.Add(entryViewData);
		}

		return eventLogViewData;
	}

	// [Panel_03]
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

	FDebugOverlayWorldSummaryViewData BuildWorldSummaryViewData(const FDebugOverlaySnapshot& InSnapshot, const FDebugOverlayCombatParticipationViewData& InCombatParticipation, bool bInHasSnapshot)
	{
		FDebugOverlayWorldSummaryViewData worldSummaryViewData;
		worldSummaryViewData.HeaderText = TEXT("[World Summary]");
		worldSummaryViewData.SummaryBlocks.Reserve(3);
		worldSummaryViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent Execution]"),
			InSnapshot.LastExecution.Summary,
			InSnapshot.LastExecution.CaptureState,
			bInHasSnapshot,
			false));
		worldSummaryViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent Combat]"),
			InSnapshot.LastCombat.Summary,
			InSnapshot.LastCombat.CaptureState,
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

FDebugOverlayViewData FDebugOverlayViewDataBuilder::Build(const UWorld* InWorld, const APawn* InViewerPawn, const ACEnemy* InDisplayEnemy, const FDebugOverlayFocusViewData& InEnemyFocus, const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting, const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotion, const FDebugOverlayCombatParticipationViewData& InCombatParticipation)
{
	FDebugOverlaySnapshot snapshot;
	const bool bHasSnapshot = FDebugOverlaySnapshotStore::TryGetSnapshotCopy(InWorld, snapshot);
	const FString eventLogFilter = FDebugOverlaySnapshotStore::GetEventLogFilter();
	const int32 eventLogLimit = FDebugOverlaySnapshotStore::GetEventLogDisplayLimit();
	const TArray<FDebugOverlayEventEntry> recentEvents = FDebugOverlaySnapshotStore::GetRecentEventsCopy(
		InWorld,
		eventLogLimit,
		eventLogFilter);

	FDebugOverlayViewData viewData;
	viewData.ActorPanels.Reserve(2);

	viewData.MainPanelTitle = TEXT("[Debug Overlay Panel_01]");
	BuildMainActorPanelData(viewData, InViewerPawn, InDisplayEnemy, InEnemyFocus, InPlayerTargeting, InPlayerLocomotion, InCombatParticipation, bHasSnapshot, InWorld);

	viewData.EventLogPanelTitle = TEXT("[Debug Overlay Panel_02]");
	viewData.EventLog = BuildEventLogViewData(bHasSnapshot, recentEvents, eventLogFilter, eventLogLimit);

	viewData.WorldSummaryPanelTitle = TEXT("[Debug Overlay Panel_03]");
	viewData.WorldSummary = BuildWorldSummaryViewData(snapshot, InCombatParticipation, bHasSnapshot);

	return viewData;
}
