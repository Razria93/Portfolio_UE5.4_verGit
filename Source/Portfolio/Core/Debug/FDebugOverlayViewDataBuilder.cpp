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

	void AppendActorStatusLines(TArray<FString>& InOutLines, const APawn* InPawn)
	{
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("State: %s"), *FormatExecutionState(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Action: %s"), *FormatActiveAction(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Reaction: %s"), *FormatActiveReaction(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("HP: %s"), *FormatActorHealth(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Stagger: %s"), *FormatParryStaggerStack(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Guard: %s"), *FormatGuardOverlay(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Movement: %s"), *FormatActorMovement(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Runtime LOD: %s"), *FormatRuntimeLODTier()));
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

	void AppendEnemyCurrentAIBlock(TArray<FString>& InOutLines, const ACEnemy* InEnemy)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Current AI]"));

		if (!IsValid(InEnemy))
		{
			AppendOverlayLine(InOutLines, TEXT("NoTarget"));
			return;
		}

		const ACAIController* aiController = Cast<ACAIController>(InEnemy->GetController());
		const UBlackboardComponent* blackboardComp = IsValid(aiController) ? aiController->GetBlackboardComponent() : nullptr;
		const bool bHasTarget = HasAITargetActor(blackboardComp);

		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Controller: %s"), *GetNameSafe(aiController)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Pawn: %s"), *GetNameSafe(InEnemy)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Target: %s"), *FormatAITargetActor(blackboardComp)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("IntentState: %s"), *FormatAIIntentState(blackboardComp)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("ReturnHome: %s"), *FormatBlackboardBool(blackboardComp, CAIKey::Navigation::bReturnHome.KeyName)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("UsePatrol: %s"), *FormatBlackboardBool(blackboardComp, CAIKey::Patrol::bUsePatrol.KeyName)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("HasLOS: %s"), *FormatBlackboardBool(blackboardComp, CAIKey::Perception::bHasLOS.KeyName)));
		const FString distanceToTarget = bHasTarget ? FormatBlackboardFloat(blackboardComp, CAIKey::Metric::DistanceToTarget.KeyName) : MissingText();
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("DistanceToTarget: %s"), *distanceToTarget));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("IsCombatAction: %s"), *FormatBlackboardBool(blackboardComp, CAIKey::Engage::bIsCombatAction.KeyName)));
	}

	void AppendEnemyRecentAIEventBlock(TArray<FString>& InOutLines, const ACEnemy* InEnemy, const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot, const UWorld* InWorld)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent AI Event]"));

		if (!IsValid(InEnemy))
		{
			AppendOverlayLine(InOutLines, TEXT("NoTarget"));
			return;
		}

		if (!bInHasSnapshot || InSnapshot.LastAI.CaptureState != EDebugOverlayCaptureState::Captured)
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}

		const FString enemyName = GetNameSafe(InEnemy);
		if (InSnapshot.LastAI.PawnName != enemyName)
		{
			AppendOverlayLine(InOutLines, TEXT("NotMatched"));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Selected: %s"), *enemyName));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("LastPawn: %s"), *InSnapshot.LastAI.PawnName));
			return;
		}

		const float currentTime = IsValid(InWorld) ? InWorld->GetTimeSeconds() : InSnapshot.LastAI.WorldTimeSeconds;
		const float eventAge = currentTime - InSnapshot.LastAI.WorldTimeSeconds;
		const bool bEventStale = eventAge > DebugOverlayRecentAIEventStaleSeconds;

		if (bEventStale)
		{
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Stale Time: %ss"), *FormatAgeSeconds(eventAge)));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Last Pawn: %s"), *InSnapshot.LastAI.PawnName));
			AppendOverlayLine(InOutLines, TEXT("Note: Not current AI evidence"));
			return;
		}

		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Task: %s"), *CompactEnumText(InSnapshot.LastAI.SubState)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Result: %s"), *CompactEnumText(InSnapshot.LastAI.RequestResult)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Age: %s"), *FormatAgeSeconds(eventAge)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("RejectReason: %s"), *CompactReasonText(InSnapshot.LastAI.RejectReason)));
	}

	void AppendActorPanelLines(TArray<FString>& InOutLines, const TCHAR* InPanelName, const APawn* InPawn, const UObject* InWorldContextObject, bool bInHasSnapshot)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, InPanelName);
		AppendActorStatusLines(InOutLines, InPawn);
		AppendActorRecentExecutionBlock(InOutLines, InWorldContextObject, bInHasSnapshot, InPawn);
	}

	void AppendMainActorPanelLines(
		TArray<FString>& InOutLines,
		const APawn* InPlayerPawn,
		const ACEnemy* InEnemy,
		const TArray<FString>& InEnemySourceLines,
		const FDebugOverlaySnapshot& InSnapshot,
		bool bInHasSnapshot,
		const UObject* InWorldContextObject,
		const UWorld* InWorld)
	{
		AppendOverlayLine(InOutLines, TEXT("[Debug Overlay Pannel_01]"));
		AppendActorPanelLines(InOutLines, TEXT("[Player]"), InPlayerPawn, InWorldContextObject, bInHasSnapshot);

		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Enemy]"));
		for (const FString& enemySourceLine : InEnemySourceLines)
		{
			AppendOverlayLine(InOutLines, enemySourceLine);
		}

		AppendOverlayLine(InOutLines, TEXT(""));
		AppendActorStatusLines(InOutLines, InEnemy);
		AppendActorRecentExecutionBlock(InOutLines, InWorldContextObject, bInHasSnapshot, InEnemy);
		AppendEnemyCurrentAIBlock(InOutLines, InEnemy);
		AppendEnemyRecentAIEventBlock(InOutLines, InEnemy, InSnapshot, bInHasSnapshot, InWorld);
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
	viewData.MainPanelLines.Reserve(32);
	viewData.EventLogPanelLines.Reserve(eventLogLimit + 2);
	viewData.InteractionPanelLines.Reserve(16);

	AppendMainActorPanelLines(
		viewData.MainPanelLines,
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
