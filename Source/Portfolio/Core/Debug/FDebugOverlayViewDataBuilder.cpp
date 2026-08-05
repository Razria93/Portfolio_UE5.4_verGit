#include "Core/Debug/FDebugOverlayViewDataBuilder.h"

#include "AI/Blackboard/CAIKey.h"
#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "Component/CActionComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Controller/CAIController.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Type/CActionKeyTypes.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

namespace
{
	static constexpr float DebugOverlayRecentAIEventStaleSeconds = 5.0f;

	FString BoolText(bool bInValue)
	{
		return bInValue ? TEXT("true") : TEXT("false");
	}

	FString MissingText()
	{
		return TEXT("N/A");
	}

	FString FormatBuilderAgeSeconds(float InAgeSeconds)
	{
		return FString::Printf(TEXT("%.2f"), FMath::Max(0.f, InAgeSeconds));
	}

	FString CompactEnumText(const FString& InValue)
	{
		int32 separatorIndex = INDEX_NONE;
		return InValue.FindLastChar(TEXT(':'), separatorIndex)
			&& separatorIndex > 0
			&& InValue[separatorIndex - 1] == TEXT(':')
			&& separatorIndex + 1 < InValue.Len()
			? InValue.RightChop(separatorIndex + 1)
			: InValue;
	}

	FString CompactReasonText(const FString& InValue)
	{
		return CompactEnumText(InValue.IsEmpty() ? FString(TEXT("None")) : InValue);
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

	template <typename TComponent>
	TComponent* FindComponent(const APawn* InPawn)
	{
		return IsValid(InPawn) ? InPawn->FindComponentByClass<TComponent>() : nullptr;
	}

	FString FormatExecutionState(const APawn* InPawn)
	{
		const UCStateComponent* stateComp = FindComponent<UCStateComponent>(InPawn);
		return IsValid(stateComp) ? CompactEnumText(UEnum::GetValueAsString(stateComp->GetCurrentExecutionState())) : MissingText();
	}

	FString FormatActiveAction(const APawn* InPawn)
	{
		const UCActionComponent* actionComp = FindComponent<UCActionComponent>(InPawn);
		if (!IsValid(actionComp)) return MissingText();
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
			*CompactEnumText(UEnum::GetValueAsString(actionType)),
			actionIndex);
	}

	FString FormatActiveReaction(const APawn* InPawn)
	{
		const UCReactionComponent* reactionComp = FindComponent<UCReactionComponent>(InPawn);
		if (!IsValid(reactionComp)) return MissingText();
		if (!reactionComp->IsActive()) return TEXT("None");

		return CompactEnumText(UEnum::GetValueAsString(reactionComp->GetActiveReactionType()));
	}

	FString FormatGuardOverlay(const APawn* InPawn)
	{
		const UCDefenseComponent* defenseComp = FindComponent<UCDefenseComponent>(InPawn);
		if (!IsValid(defenseComp)) return MissingText();

		return FString::Printf(
			TEXT("Wants: %s | Pose: %s | CanGuard: %s | CanParry: %s | CanStart: %s"),
			*BoolText(defenseComp->WantsGuarding()),
			*BoolText(defenseComp->IsGuardingPose()),
			*BoolText(defenseComp->CanGuard()),
			*BoolText(defenseComp->CanParry()),
			*BoolText(defenseComp->CanStartGuard()));
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

		return MissingText();
	}

	FString FormatRuntimeLODTier()
	{
		return MissingText();
	}

	FString FormatAIIntentState(const UBlackboardComponent* InBlackboardComp)
	{
		if (!IsValid(InBlackboardComp)) return MissingText();

		const uint8 intentStateValue = InBlackboardComp->GetValueAsEnum(CAIKey::State::AIIntentState.KeyName);
		if (intentStateValue >= static_cast<uint8>(EAIIntentState::Max)) return MissingText();

		return CompactEnumText(UEnum::GetValueAsString(static_cast<EAIIntentState>(intentStateValue)));
	}

	FString FormatAITargetActor(const UBlackboardComponent* InBlackboardComp)
	{
		if (!IsValid(InBlackboardComp)) return MissingText();

		const UObject* targetObject = InBlackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName);
		return IsValid(targetObject) ? GetNameSafe(targetObject) : MissingText();
	}

	bool HasAITargetActor(const UBlackboardComponent* InBlackboardComp)
	{
		if (!IsValid(InBlackboardComp)) return false;

		const UObject* targetObject = InBlackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName);
		return IsValid(targetObject);
	}

	FString FormatBlackboardBool(const UBlackboardComponent* InBlackboardComp, FName InKeyName)
	{
		return IsValid(InBlackboardComp) ? BoolText(InBlackboardComp->GetValueAsBool(InKeyName)) : MissingText();
	}

	FString FormatBlackboardFloat(const UBlackboardComponent* InBlackboardComp, FName InKeyName)
	{
		return IsValid(InBlackboardComp) ? FString::Printf(TEXT("%.1f"), InBlackboardComp->GetValueAsFloat(InKeyName)) : MissingText();
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

	FString FormatActorMovement(const APawn* InPawn)
	{
		const UCMovementComponent* movementComp = FindComponent<UCMovementComponent>(InPawn);
		if (!IsValid(movementComp)) return MissingText();

		return FString::Printf(
			TEXT("Gait: %s | Speed: %.1f | Dir: %.1f | CanMove: %s | Falling: %s"),
			*CompactEnumText(UEnum::GetValueAsString(movementComp->GetCurrentMovementGait())),
			movementComp->GetCurrentSpeed(),
			movementComp->GetCurrentDirection(),
			*BoolText(movementComp->CanMove()),
			*BoolText(movementComp->IsFalling()));
	}

	FString FormatActorHealth(const APawn* InPawn)
	{
		const UCHealthComponent* healthComp = FindComponent<UCHealthComponent>(InPawn);
		if (!IsValid(healthComp)) return MissingText();

		return FString::Printf(
			TEXT("%.1f/%.1f (DeadState: %s)"),
			healthComp->GetCurrentHP(),
			healthComp->GetMaxHP(),
			*CompactEnumText(UEnum::GetValueAsString(healthComp->GetDeadState())));
	}

	FDebugOverlayEventLogViewData BuildEventLogViewData(
		bool bInHasSnapshot,
		const TArray<FDebugOverlayEventEntry>& InEvents,
		const FString& InEventLogFilter,
		int32 InEventLogLimit)
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
		currentAIViewData.DistanceToTargetText = bHasTarget ? FormatBlackboardFloat(blackboardComp, CAIKey::Metric::DistanceToTarget.KeyName) : MissingText();
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
			recentAIEventViewData.TaskText = CompactEnumText(cachedSummary.SubState);
			recentAIEventViewData.ResultText = CompactEnumText(cachedSummary.RequestResult);
			recentAIEventViewData.RejectReasonText = CompactReasonText(cachedSummary.RejectReason);
			return recentAIEventViewData;
		}

		recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::Captured;
		recentAIEventViewData.TaskText = CompactEnumText(cachedSummary.SubState);
		recentAIEventViewData.ResultText = CompactEnumText(cachedSummary.RequestResult);
		recentAIEventViewData.AgeText = FormatBuilderAgeSeconds(eventAge);
		recentAIEventViewData.RejectReasonText = CompactReasonText(cachedSummary.RejectReason);
		return recentAIEventViewData;
	}

	FDebugOverlayActorPanelViewData BuildActorPanelViewData(const TCHAR* InPanelName, const APawn* InPawn, const UWorld* InWorld, bool bInHasSnapshot)
	{
		FDebugOverlayActorPanelViewData actorPanelViewData;
		actorPanelViewData.HeaderText = InPanelName;
		actorPanelViewData.Status = BuildActorStatusViewData(InPawn);
		actorPanelViewData.RecentExecution = BuildActorRecentExecutionViewData(InWorld, bInHasSnapshot, InPawn);
		return actorPanelViewData;
	}

	void BuildMainActorPanelData(
		FDebugOverlayViewData& InOutViewData,
		const APawn* InPlayerPawn,
		const ACEnemy* InEnemy,
		const FDebugOverlayFocusViewData& InEnemyFocus,
		bool bInHasSnapshot,
		const UWorld* InWorld)
	{
		InOutViewData.MainPanelTitle = TEXT("[Debug Overlay Panel_01]");
		InOutViewData.ActorPanels.Add(BuildActorPanelViewData(TEXT("[Player]"), InPlayerPawn, InWorld, bInHasSnapshot));

		FDebugOverlayActorPanelViewData enemyPanelViewData = BuildActorPanelViewData(TEXT("[Enemy]"), InEnemy, InWorld, bInHasSnapshot);
		enemyPanelViewData.bIncludeFocus = true;
		enemyPanelViewData.Focus = InEnemyFocus;
		enemyPanelViewData.bAppendBlankBeforeStatus = true;
		enemyPanelViewData.bIncludeCurrentAI = true;
		enemyPanelViewData.CurrentAI = BuildEnemyCurrentAIViewData(InEnemy);
		enemyPanelViewData.bIncludeRecentAIEvent = true;
		enemyPanelViewData.RecentAIEvent = BuildEnemyRecentAIEventViewData(InEnemy, bInHasSnapshot, InWorld);
		InOutViewData.ActorPanels.Add(enemyPanelViewData);
	}

	FDebugOverlayRecentSummaryBlockViewData BuildRecentSummaryBlockViewData(
		const TCHAR* InBlockName,
		const FString& InSummary,
		EDebugOverlayCaptureState InCaptureState,
		bool bInHasSnapshot,
		bool bInAppendLeadingBlank)
	{
		FDebugOverlayRecentSummaryBlockViewData blockViewData;
		blockViewData.HeaderText = InBlockName;
		blockViewData.SummaryText = InSummary;
		blockViewData.CaptureState = InCaptureState;
		blockViewData.bHasSnapshot = bInHasSnapshot;
		blockViewData.bAppendLeadingBlank = bInAppendLeadingBlank;
		return blockViewData;
	}

	FDebugOverlayInteractionViewData BuildInteractionViewData(const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot)
	{
		FDebugOverlayInteractionViewData interactionViewData;
		interactionViewData.HeaderText = TEXT("[World]");
		interactionViewData.SummaryBlocks.Reserve(3);
		interactionViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent Execution]"),
			InSnapshot.LastExecution.Summary,
			InSnapshot.LastExecution.CaptureState,
			bInHasSnapshot,
			false));
		interactionViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent Combat]"),
			InSnapshot.LastCombat.Summary,
			InSnapshot.LastCombat.CaptureState,
			bInHasSnapshot,
			true));
		interactionViewData.SummaryBlocks.Add(BuildRecentSummaryBlockViewData(
			TEXT("[Recent AI Event]"),
			InSnapshot.LastAI.Summary,
			InSnapshot.LastAI.CaptureState,
			bInHasSnapshot,
			true));
		return interactionViewData;
	}
}

FDebugOverlayViewData FDebugOverlayViewDataBuilder::Build(const UWorld* InWorld, const APawn* InViewerPawn, const ACEnemy* InDisplayEnemy, const FDebugOverlayFocusViewData& InEnemyFocus)
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

	BuildMainActorPanelData(
		viewData,
		InViewerPawn,
		InDisplayEnemy,
		InEnemyFocus,
		bHasSnapshot,
		InWorld);

	viewData.EventLogPanelTitle = TEXT("[Debug Overlay Pannel_02]");
	viewData.EventLog = BuildEventLogViewData(bHasSnapshot, recentEvents, eventLogFilter, eventLogLimit);

	viewData.InteractionPanelTitle = TEXT("[Debug Overlay Pannel_03]");
	viewData.Interaction = BuildInteractionViewData(snapshot, bHasSnapshot);

	return viewData;
}
