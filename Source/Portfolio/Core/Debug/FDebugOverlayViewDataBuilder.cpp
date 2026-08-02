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

	FString FormatAgeSeconds(float InAgeSeconds)
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

	FString CaptureStateText(EDebugOverlayCaptureState InState)
	{
		switch (InState)
		{
		case EDebugOverlayCaptureState::Captured:
			return TEXT("Captured");
		case EDebugOverlayCaptureState::Unavailable:
			return TEXT("Unavailable");
		case EDebugOverlayCaptureState::Stale:
			return TEXT("Stale");
		case EDebugOverlayCaptureState::NotCaptured:
		default:
			return TEXT("NotCaptured");
		}
	}

	FString ValueOrNotCaptured(const FString& InValue, EDebugOverlayCaptureState InState)
	{
		return InState == EDebugOverlayCaptureState::Captured && !InValue.IsEmpty()
			? InValue
			: CaptureStateText(InState);
	}

	void AppendOverlayLine(TArray<FString>& InOutLines, const FString& InLine)
	{
		InOutLines.Add(InLine);
	}

	void AppendSummaryLines(TArray<FString>& InOutLines, const FString& InSummary, EDebugOverlayCaptureState InCaptureState)
	{
		const FString summary = ValueOrNotCaptured(InSummary, InCaptureState);
		if (InCaptureState != EDebugOverlayCaptureState::Captured || summary.IsEmpty())
		{
			AppendOverlayLine(InOutLines, summary);
			return;
		}

		TArray<FString> summaryParts;
		summary.ParseIntoArray(summaryParts, TEXT(" | "), true);
		if (summaryParts.IsEmpty())
		{
			AppendOverlayLine(InOutLines, summary);
			return;
		}

		for (const FString& summaryPart : summaryParts)
		{
			AppendOverlayLine(InOutLines, summaryPart);
		}
	}

	void AppendSnapshotSummaryBlock(TArray<FString>& InOutLines, const TCHAR* InBlockName, const FString& InSummary, EDebugOverlayCaptureState InCaptureState, bool bInHasSnapshot, bool bInAppendLeadingBlank)
	{
		if (bInAppendLeadingBlank)
		{
			AppendOverlayLine(InOutLines, TEXT(""));
		}

		AppendOverlayLine(InOutLines, InBlockName);
		if (bInHasSnapshot)
		{
			AppendSummaryLines(InOutLines, InSummary, InCaptureState);
		}
		else
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
		}
	}

	FString FormatEventLogEntryLine(const FDebugOverlayEventEntry& InEntry)
	{
		return FString::Printf(
			TEXT("%s/%s: %s"),
			*InEntry.Category,
			*InEntry.EventName,
			*InEntry.Summary);
	}

	void AppendEventLogBlock(TArray<FString>& InOutLines, bool bInHasSnapshot, const TArray<FDebugOverlayEventEntry>& InEvents, const FString& InEventLogFilter, int32 InEventLogLimit)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("[Event Log: %s]"), *InEventLogFilter));

		if (!bInHasSnapshot)
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}

		if (InEventLogLimit == 0)
		{
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("NoEvents(Filter: %s Limit: 0)"), *InEventLogFilter));
			return;
		}

		if (InEvents.IsEmpty())
		{
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("NoEvents(Filter: %s)"), *InEventLogFilter));
			return;
		}

		for (const FDebugOverlayEventEntry& eventEntry : InEvents)
		{
			AppendOverlayLine(InOutLines, FormatEventLogEntryLine(eventEntry));
		}
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

	void AppendActorRecentExecutionBlock(TArray<FString>& InOutLines, const UObject* InWorldContextObject, bool bInHasSnapshot, const APawn* InPawn)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent Execution]"));

		if (!bInHasSnapshot)
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}

		if (!IsValid(InPawn))
		{
			AppendOverlayLine(InOutLines, TEXT("N/A"));
			return;
		}

		const TArray<FDebugOverlayEventEntry> executionEvents = FDebugOverlaySnapshotStore::GetRecentEventsForSubjectCopy(
			InWorldContextObject,
			1,
			TEXT("Execution"),
			GetNameSafe(InPawn));

		if (executionEvents.IsEmpty())
		{
			AppendOverlayLine(InOutLines, TEXT("NoEvents(Filter: Execution)"));
			return;
		}

		AppendSummaryLines(InOutLines, executionEvents[0].Summary, EDebugOverlayCaptureState::Captured);
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

	FDebugOverlayRecentAIEventViewData BuildEnemyRecentAIEventViewData(const ACEnemy* InEnemy, const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot, const UWorld* InWorld)
	{
		FDebugOverlayRecentAIEventViewData recentAIEventViewData;
		if (!IsValid(InEnemy))
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::NoTarget;
			return recentAIEventViewData;
		}

		if (!bInHasSnapshot || InSnapshot.LastAI.CaptureState != EDebugOverlayCaptureState::Captured)
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::NotCaptured;
			return recentAIEventViewData;
		}

		const FString enemyName = GetNameSafe(InEnemy);
		recentAIEventViewData.SelectedPawnName = enemyName;
		recentAIEventViewData.LastPawnName = InSnapshot.LastAI.PawnName;
		if (InSnapshot.LastAI.PawnName != enemyName)
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::NotMatched;
			return recentAIEventViewData;
		}

		const float currentTime = IsValid(InWorld) ? InWorld->GetTimeSeconds() : InSnapshot.LastAI.WorldTimeSeconds;
		const float eventAge = currentTime - InSnapshot.LastAI.WorldTimeSeconds;
		const bool bEventStale = eventAge > DebugOverlayRecentAIEventStaleSeconds;

		if (bEventStale)
		{
			recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::Stale;
			recentAIEventViewData.StaleAgeText = FormatAgeSeconds(eventAge);
			return recentAIEventViewData;
		}

		recentAIEventViewData.State = EDebugOverlayRecentAIEventViewState::Captured;
		recentAIEventViewData.TaskText = CompactEnumText(InSnapshot.LastAI.SubState);
		recentAIEventViewData.ResultText = CompactEnumText(InSnapshot.LastAI.RequestResult);
		recentAIEventViewData.AgeText = FormatAgeSeconds(eventAge);
		recentAIEventViewData.RejectReasonText = CompactReasonText(InSnapshot.LastAI.RejectReason);
		return recentAIEventViewData;
	}

	FDebugOverlayActorPanelViewData BuildActorPanelViewData(const TCHAR* InPanelName, const APawn* InPawn, const UObject* InWorldContextObject, bool bInHasSnapshot)
	{
		FDebugOverlayActorPanelViewData actorPanelViewData;
		actorPanelViewData.HeaderText = InPanelName;
		actorPanelViewData.Status = BuildActorStatusViewData(InPawn);
		AppendActorRecentExecutionBlock(actorPanelViewData.RecentExecutionLines, InWorldContextObject, bInHasSnapshot, InPawn);
		return actorPanelViewData;
	}

	void BuildMainActorPanelData(
		FDebugOverlayViewData& InOutViewData,
		const APawn* InPlayerPawn,
		const ACEnemy* InEnemy,
		const TArray<FString>& InEnemySourceLines,
		const FDebugOverlaySnapshot& InSnapshot,
		bool bInHasSnapshot,
		const UObject* InWorldContextObject,
		const UWorld* InWorld)
	{
		InOutViewData.MainPanelTitle = TEXT("[Debug Overlay Pannel_01]");
		InOutViewData.ActorPanels.Add(BuildActorPanelViewData(TEXT("[Player]"), InPlayerPawn, InWorldContextObject, bInHasSnapshot));

		FDebugOverlayActorPanelViewData enemyPanelViewData = BuildActorPanelViewData(TEXT("[Enemy]"), InEnemy, InWorldContextObject, bInHasSnapshot);
		enemyPanelViewData.LinesBeforeStatus = InEnemySourceLines;
		enemyPanelViewData.bAppendBlankBeforeStatus = true;
		enemyPanelViewData.bIncludeCurrentAI = true;
		enemyPanelViewData.CurrentAI = BuildEnemyCurrentAIViewData(InEnemy);
		enemyPanelViewData.bIncludeRecentAIEvent = true;
		enemyPanelViewData.RecentAIEvent = BuildEnemyRecentAIEventViewData(InEnemy, InSnapshot, bInHasSnapshot, InWorld);
		InOutViewData.ActorPanels.Add(enemyPanelViewData);
	}

	void AppendInteractionPanelLines(TArray<FString>& InOutLines, const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Interaction]"));

		AppendSnapshotSummaryBlock(InOutLines, TEXT("[Recent Execution]"), InSnapshot.LastExecution.Summary, InSnapshot.LastExecution.CaptureState, bInHasSnapshot, false);
		AppendSnapshotSummaryBlock(InOutLines, TEXT("[Recent Combat]"), InSnapshot.LastCombat.Summary, InSnapshot.LastCombat.CaptureState, bInHasSnapshot, true);
	}
}

FDebugOverlayViewData FDebugOverlayViewDataBuilder::Build(const FDebugOverlayViewDataBuildContext& InContext)
{
	FDebugOverlaySnapshot snapshot;
	const bool bHasSnapshot = FDebugOverlaySnapshotStore::TryGetSnapshotCopy(InContext.World, snapshot);
	const FString eventLogFilter = FDebugOverlaySnapshotStore::GetEventLogFilter();
	const int32 eventLogLimit = FDebugOverlaySnapshotStore::GetEventLogDisplayLimit();
	const TArray<FDebugOverlayEventEntry> recentEvents = FDebugOverlaySnapshotStore::GetRecentEventsCopy(
		InContext.World,
		eventLogLimit,
		eventLogFilter);

	FDebugOverlayViewData viewData;
	viewData.ActorPanels.Reserve(2);
	viewData.EventLogPanelLines.Reserve(eventLogLimit + 2);
	viewData.InteractionPanelLines.Reserve(16);

	BuildMainActorPanelData(
		viewData,
		InContext.ViewerPawn,
		InContext.DisplayEnemy,
		InContext.EnemySourceLines,
		snapshot,
		bHasSnapshot,
		InContext.WorldContextObject,
		InContext.World);

	AppendOverlayLine(viewData.EventLogPanelLines, TEXT("[Debug Overlay Pannel_02]"));
	AppendEventLogBlock(viewData.EventLogPanelLines, bHasSnapshot, recentEvents, eventLogFilter, eventLogLimit);

	AppendOverlayLine(viewData.InteractionPanelLines, TEXT("[Debug Overlay Pannel_03]"));
	AppendInteractionPanelLines(viewData.InteractionPanelLines, snapshot, bHasSnapshot);

	return viewData;
}
