#include "Core/Debug/CDebugOverlayHUD.h"

#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "AI/Blackboard/CAIKey.h"
#include "Component/CActionComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Controller/CAIController.h"
#include "Core/Debug/CDebugOverlayTargetComponent.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Type/CActionKeyTypes.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#if !UE_BUILD_SHIPPING
namespace
{
	// Display Constants
	static constexpr float DebugOverlayOriginX = 24.f;
	static constexpr float DebugOverlayOriginY = 36.f;
	static constexpr float DebugOverlayLineHeight = 20.f;
	static constexpr float DebugOverlayFontScale = 1.05f;
	static constexpr float DebugOverlayBackgroundPadding = 10.f;
	static constexpr float DebugOverlayBackgroundWidth = 700.f;
	static constexpr float DebugOverlayHeaderBottomPadding = 5.f;
	static constexpr float DebugOverlayPanelGap = 24.f;
	static constexpr float DebugOverlayRightMargin = 24.f;
	static constexpr float DebugOverlayBottomMargin = 24.f;
	static constexpr float DebugOverlayMinEventLogPanelWidth = 420.f;
	static constexpr float DebugOverlayInteractionPanelWidth = 520.f;
	static constexpr float DebugOverlayEnemyScanCooldownSeconds = 0.5f;
	static constexpr float DebugOverlayRecentCombatTargetStaleSeconds = 3.0f;
	static constexpr float DebugOverlayRecentAIEventStaleSeconds = 5.0f;
	static const FLinearColor DebugOverlayBackgroundColor(0.f, 0.f, 0.f, 0.72f);
	static const FLinearColor DebugOverlayPlayerHeaderColor(0.02f, 0.20f, 0.78f, 0.68f);
	static const FLinearColor DebugOverlayEnemyHeaderColor(0.78f, 0.06f, 0.04f, 0.68f);
	static const FLinearColor DebugOverlayInteractionHeaderColor(0.2f, 0.08f, 0.35f, 0.68f);
	static const FLinearColor DebugOverlayDefaultHeaderColor(0.24f, 0.24f, 0.24f, 0.72f);

	// Text Formatting
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

	// Actor Status Formatting
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

	// Snapshot Lines
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

	// Panel Styling
	struct FDebugOverlayRightPanelGeometry
	{
		float EventLogBackgroundX = 0.f;
		float EventLogBackgroundY = 0.f;
		float EventLogAvailableWidth = 0.f;
		float EventLogAvailableHeight = 0.f;
		float InteractionBackgroundX = 0.f;
		float InteractionBackgroundWidth = 0.f;
		bool bCanDrawInteractionPanel = false;
	};

	bool IsPanelHeaderLine(const FString& InLine)
	{
		return InLine == TEXT("[Player]") || InLine == TEXT("[Enemy]") || InLine == TEXT("[Interaction]");
	}

	bool IsEventLogHeaderLine(const FString& InLine)
	{
		return InLine.StartsWith(TEXT("[Event Log:"));
	}

	FLinearColor GetPanelHeaderColor(const FString& InLine)
	{
		if (InLine == TEXT("[Player]")) return DebugOverlayPlayerHeaderColor;
		if (InLine == TEXT("[Enemy]")) return DebugOverlayEnemyHeaderColor;
		if (InLine == TEXT("[Interaction]")) return DebugOverlayInteractionHeaderColor;
		return DebugOverlayDefaultHeaderColor;
	}

	bool IsOverlayHeaderLine(const FString& InLine, bool bInDrawPanelHeaders, bool bInDrawEventLogHeaders)
	{
		return (bInDrawPanelHeaders && IsPanelHeaderLine(InLine))
			|| (bInDrawEventLogHeaders && IsEventLogHeaderLine(InLine));
	}

	float GetOverlayLineHeight(const FString& InLine, bool bInDrawPanelHeaders, bool bInDrawEventLogHeaders)
	{
		return DebugOverlayLineHeight
			+ (IsOverlayHeaderLine(InLine, bInDrawPanelHeaders, bInDrawEventLogHeaders) ? DebugOverlayHeaderBottomPadding : 0.f);
	}

	float CalculateOverlayLinesHeight(
		const TArray<FString>& InLines,
		bool bInDrawPanelHeaders,
		bool bInDrawEventLogHeaders)
	{
		float height = 0.f;
		for (const FString& line : InLines)
		{
			height += GetOverlayLineHeight(line, bInDrawPanelHeaders, bInDrawEventLogHeaders);
		}

		return height;
	}

	int32 CalculateVisibleOverlayLineCount(
		const TArray<FString>& InLines,
		float InMaxTextHeight,
		bool bInDrawPanelHeaders,
		bool bInDrawEventLogHeaders)
	{
		float usedHeight = 0.f;
		for (int32 lineIndex = 0; lineIndex < InLines.Num(); ++lineIndex)
		{
			const FString& line = InLines[lineIndex];
			const float lineHeight = GetOverlayLineHeight(line, bInDrawPanelHeaders, bInDrawEventLogHeaders);

			if (usedHeight + lineHeight > InMaxTextHeight)
			{
				return lineIndex;
			}

			usedHeight += lineHeight;
		}

		return InLines.Num();
	}

	TArray<FString> MakeVisibleOverlayLines(
		const TArray<FString>& InLines,
		float InMaxTextHeight,
		bool bInDrawPanelHeaders,
		bool bInDrawEventLogHeaders)
	{
		const int32 visibleLineCount = CalculateVisibleOverlayLineCount(
			InLines,
			InMaxTextHeight,
			bInDrawPanelHeaders,
			bInDrawEventLogHeaders);

		TArray<FString> visibleLines;
		visibleLines.Reserve(visibleLineCount);
		for (int32 lineIndex = 0; lineIndex < visibleLineCount; ++lineIndex)
		{
			visibleLines.Add(InLines[lineIndex]);
		}

		return visibleLines;
	}

	FDebugOverlayRightPanelGeometry CalculateRightPanelGeometry(
		const UCanvas* InCanvas,
		float InLeftPanelBackgroundX,
		float InLeftPanelBackgroundWidth,
		float InTopBackgroundY,
		bool bInHasInteractionLines)
	{
		FDebugOverlayRightPanelGeometry geometry;
		if (!InCanvas) return geometry;

		geometry.EventLogBackgroundX = InLeftPanelBackgroundX + InLeftPanelBackgroundWidth + DebugOverlayPanelGap;
		geometry.EventLogBackgroundY = InTopBackgroundY;

		const float rightPanelAvailableWidth = FMath::Max(0.f, InCanvas->SizeX - geometry.EventLogBackgroundX - DebugOverlayRightMargin);
		geometry.bCanDrawInteractionPanel = bInHasInteractionLines
			&& rightPanelAvailableWidth >= DebugOverlayMinEventLogPanelWidth + DebugOverlayPanelGap + DebugOverlayInteractionPanelWidth;
		geometry.InteractionBackgroundWidth = geometry.bCanDrawInteractionPanel ? DebugOverlayInteractionPanelWidth : 0.f;
		geometry.InteractionBackgroundX = geometry.bCanDrawInteractionPanel
			? InCanvas->SizeX - DebugOverlayRightMargin - geometry.InteractionBackgroundWidth
			: 0.f;
		geometry.EventLogAvailableWidth = geometry.bCanDrawInteractionPanel
			? FMath::Max(0.f, geometry.InteractionBackgroundX - DebugOverlayPanelGap - geometry.EventLogBackgroundX)
			: rightPanelAvailableWidth;
		geometry.EventLogAvailableHeight = FMath::Max(0.f, InCanvas->SizeY - geometry.EventLogBackgroundY - DebugOverlayBottomMargin);

		return geometry;
	}

	void DrawOverlayLines(
		ACDebugOverlayHUD& InHud,
		const TArray<FString>& InLines,
		float InTextX,
		float InTextY,
		float InBackgroundX,
		float InBackgroundY,
		float InBackgroundWidth,
		float InBackgroundHeight,
		bool bInDrawPanelHeaders,
		bool bInDrawEventLogHeaders)
	{
		if (InBackgroundWidth > 0.f && InBackgroundHeight > 0.f)
		{
			InHud.DrawRect(DebugOverlayBackgroundColor, InBackgroundX, InBackgroundY, InBackgroundWidth, InBackgroundHeight);
		}

		float y = InTextY;
		for (const FString& line : InLines)
		{
			const bool bDrawPanelHeader = bInDrawPanelHeaders && IsPanelHeaderLine(line);
			const bool bDrawEventLogHeader = bInDrawEventLogHeaders && IsEventLogHeaderLine(line);

			if ((bDrawPanelHeader || bDrawEventLogHeader) && InBackgroundWidth > 0.f)
			{
				InHud.DrawRect(GetPanelHeaderColor(line), InBackgroundX, y - 2.f, InBackgroundWidth, DebugOverlayLineHeight + 4.f);
			}

			InHud.DrawText(line, FLinearColor::White, InTextX, y, nullptr, DebugOverlayFontScale, false);
			y += DebugOverlayLineHeight;
			if (bDrawPanelHeader || bDrawEventLogHeader)
			{
				y += DebugOverlayHeaderBottomPadding;
			}
		}
	}

	void AppendTargetSelectionSummary(TArray<FString>& InOutLines, const UCDebugOverlayTargetComponent* InTargetComp)
	{
		if (!IsValid(InTargetComp)) return;
		if (!InTargetComp->HasDebugOverlaySelectionSummary()) return;

		AppendOverlayLine(InOutLines, FString::Printf(TEXT("EnemySelect: %s"), *InTargetComp->GetDebugOverlaySelectionSummary()));
	}
}
#endif

#if !UE_BUILD_SHIPPING
// Enemy Resolution
ACEnemy* ACDebugOverlayHUD::ResolveDisplayEnemy(TArray<FString>& OutSourceLines)
{
	if (ACEnemy* targetComponentEnemy = ResolveTargetComponentEnemy(OutSourceLines))
	{
		return targetComponentEnemy;
	}

	AppendOverlayLine(OutSourceLines, TEXT("EnemySource: None"));
	if (const APlayerController* owningPlayerController = GetOwningPlayerController())
	{
		const UCDebugOverlayTargetComponent* targetComp = owningPlayerController->FindComponentByClass<UCDebugOverlayTargetComponent>();
		AppendTargetSelectionSummary(OutSourceLines, targetComp);
	}

	return nullptr;
}

ACEnemy* ACDebugOverlayHUD::ResolveTargetComponentEnemy(TArray<FString>& OutSourceLines) const
{
	const APlayerController* owningPlayerController = GetOwningPlayerController();
	if (!IsValid(owningPlayerController)) return nullptr;

	const UCDebugOverlayTargetComponent* targetComp = owningPlayerController->FindComponentByClass<UCDebugOverlayTargetComponent>();
	if (!IsValid(targetComp)) return nullptr;

	ACEnemy* targetEnemy = Cast<ACEnemy>(targetComp->GetDebugOverlayTargetActor());
	if (!IsValid(targetEnemy)) return nullptr;

	AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemySource: %s"), *targetComp->GetDebugOverlayTargetSource()));
	AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemyTarget: %s"), *targetComp->GetDebugOverlayTargetSummary()));
	AppendTargetSelectionSummary(OutSourceLines, targetComp);
	return targetEnemy;
}

ACEnemy* ACDebugOverlayHUD::ResolveRecentCombatEnemy(const APawn* InViewerPawn, TArray<FString>& OutSourceLines) const
{
	FDebugOverlayRecentCombatPair recentCombatPair;
	if (!FDebugOverlaySnapshotStore::TryGetRecentCombatPair(GetWorld(), recentCombatPair))
	{
		return nullptr;
	}

	const UWorld* world = GetWorld();
	const float currentTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	const float pairAge = currentTime - recentCombatPair.WorldTimeSeconds;
	const bool bPairStale = pairAge > DebugOverlayRecentCombatTargetStaleSeconds;

	AActor* sourceActor = recentCombatPair.SourceActor.Get();
	AActor* targetActor = recentCombatPair.TargetActor.Get();
	const bool bSourceInvalid = !IsValid(sourceActor);
	const bool bTargetInvalid = !IsValid(targetActor);
	bool bRecentCombatPairMatched = false;

	if (!bPairStale && !bSourceInvalid && !bTargetInvalid && IsValid(InViewerPawn))
	{
		ACEnemy* recentEnemy = nullptr;
		if (sourceActor == InViewerPawn)
		{
			recentEnemy = Cast<ACEnemy>(targetActor);
			bRecentCombatPairMatched = true;
		}
		else if (targetActor == InViewerPawn)
		{
			recentEnemy = Cast<ACEnemy>(sourceActor);
			bRecentCombatPairMatched = true;
		}

		if (IsValid(recentEnemy))
		{
			AppendOverlayLine(OutSourceLines, TEXT("EnemySource: RecentCombatTarget"));
			AppendOverlayLine(OutSourceLines, FString::Printf(
				TEXT("EnemyRecentCombat: Source: %s | Target: %s | Age: %s"),
				*recentCombatPair.SourceName,
				*recentCombatPair.TargetName,
				*FormatAgeSeconds(pairAge)));
			return recentEnemy;
		}
	}

	if (bPairStale || bSourceInvalid || bTargetInvalid)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(
			TEXT("EnemyRecentCombat: Stale | Source: %s | Target: %s | Age: %s"),
			*recentCombatPair.SourceName,
			*recentCombatPair.TargetName,
			*FormatAgeSeconds(pairAge)));
	}
	else if (!bRecentCombatPairMatched)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(
			TEXT("EnemyRecentCombat: NotMatched | Source: %s | Target: %s | Age: %s"),
			*recentCombatPair.SourceName,
			*recentCombatPair.TargetName,
			*FormatAgeSeconds(pairAge)));
	}

	return nullptr;
}

ACEnemy* ACDebugOverlayHUD::ResolveWorldScanFallbackEnemy(TArray<FString>& OutSourceLines)
{
	RefreshCachedEnemyIfNeeded();
	if (LastEnemyScanCount == 0)
	{
		AppendOverlayLine(OutSourceLines, TEXT("EnemySource: None"));
		return nullptr;
	}

	if (LastEnemyScanCount > 1)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemySource: Ambiguous(Count=%d)"), LastEnemyScanCount));
		return nullptr;
	}

	ACEnemy* fallbackEnemy = CachedEnemy.Get();
	if (!IsValid(fallbackEnemy))
	{
		AppendOverlayLine(OutSourceLines, TEXT("EnemySource: Stale"));
		return nullptr;
	}

	AppendOverlayLine(OutSourceLines, TEXT("EnemySource: WorldScanFallback"));
	AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemyFallback: Selected: %s | Policy: FirstValid | Count: 1"), *GetNameSafe(fallbackEnemy)));
	return fallbackEnemy;
}

// Enemy Cache
void ACDebugOverlayHUD::RefreshCachedEnemyIfNeeded()
{
	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	const float currentTime = world->GetTimeSeconds();
	if (CachedEnemy.IsValid() && currentTime - LastEnemyScanTimeSeconds < DebugOverlayEnemyScanCooldownSeconds) return;
	if (!CachedEnemy.IsValid() && LastEnemyScanTimeSeconds >= 0.f && currentTime - LastEnemyScanTimeSeconds < DebugOverlayEnemyScanCooldownSeconds) return;

	LastEnemyScanTimeSeconds = currentTime;
	LastEnemyScanCount = 0;
	CachedEnemy.Reset();

	for (TActorIterator<ACEnemy> actorIt(world); actorIt; ++actorIt)
	{
		ACEnemy* enemy = *actorIt;
		if (!IsValid(enemy)) continue;

		++LastEnemyScanCount;
		if (!CachedEnemy.IsValid())
		{
			CachedEnemy = enemy;
		}
	}
}
#endif

// Rendering
void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
	Super::DrawHUD();

	if (!FDebugOverlaySnapshotStore::IsEnabled()) return;

	UWorld* world = GetWorld();
	const APawn* pawn = GetOwningPawn();
	TArray<FString> enemySourceLines;
	const ACEnemy* enemy = ResolveDisplayEnemy(enemySourceLines);

	FDebugOverlaySnapshot snapshot;
	const bool bHasSnapshot = FDebugOverlaySnapshotStore::TryGetSnapshotCopy(world, snapshot);
	const FString eventLogFilter = FDebugOverlaySnapshotStore::GetEventLogFilter();
	const int32 eventLogLimit = FDebugOverlaySnapshotStore::GetEventLogDisplayLimit();
	const TArray<FDebugOverlayEventEntry> recentEvents = FDebugOverlaySnapshotStore::GetRecentEventsCopy(
		world,
		eventLogLimit,
		eventLogFilter);

	TArray<FString> lines;
	lines.Reserve(32);
	TArray<FString> eventLogLines;
	eventLogLines.Reserve(eventLogLimit + 2);
	TArray<FString> interactionLines;
	interactionLines.Reserve(16);

	AppendMainActorPanelLines(lines, pawn, enemy, enemySourceLines, snapshot, bHasSnapshot, world, world);

	AppendOverlayLine(eventLogLines, TEXT("[Debug Overlay Pannel_02]"));
	AppendEventLogBlock(eventLogLines, bHasSnapshot, recentEvents, eventLogFilter, eventLogLimit);

	AppendOverlayLine(interactionLines, TEXT("[Debug Overlay Pannel_03]"));
	AppendInteractionPanelLines(interactionLines, snapshot, bHasSnapshot);

	const float backgroundX = FMath::Max(0.f, DebugOverlayOriginX - DebugOverlayBackgroundPadding);
	const float backgroundY = FMath::Max(0.f, DebugOverlayOriginY - DebugOverlayBackgroundPadding);
	const float availableWidth = Canvas
		? FMath::Max(0.f, Canvas->SizeX - backgroundX - DebugOverlayBackgroundPadding)
		: DebugOverlayBackgroundWidth;
	const float backgroundWidth = FMath::Min(DebugOverlayBackgroundWidth, availableWidth);
	const float backgroundHeight = CalculateOverlayLinesHeight(lines, true, false) + (DebugOverlayBackgroundPadding * 2.f);

	DrawOverlayLines(
		*this,
		lines,
		DebugOverlayOriginX,
		DebugOverlayOriginY,
		backgroundX,
		backgroundY,
		backgroundWidth,
		backgroundHeight,
		true,
		false);

	if (Canvas && !eventLogLines.IsEmpty())
	{
		const FDebugOverlayRightPanelGeometry rightPanelGeometry = CalculateRightPanelGeometry(
			Canvas,
			backgroundX,
			backgroundWidth,
			backgroundY,
			!interactionLines.IsEmpty());
		const float maxEventLogTextHeight = FMath::Max(0.f, rightPanelGeometry.EventLogAvailableHeight - (DebugOverlayBackgroundPadding * 2.f));
		const TArray<FString> visibleEventLogLines = MakeVisibleOverlayLines(eventLogLines, maxEventLogTextHeight, false, true);

		const float eventLogBackgroundHeight = FMath::Min(
			CalculateOverlayLinesHeight(visibleEventLogLines, false, true) + (DebugOverlayBackgroundPadding * 2.f),
			rightPanelGeometry.EventLogAvailableHeight);

		if (rightPanelGeometry.EventLogAvailableWidth >= DebugOverlayMinEventLogPanelWidth && eventLogBackgroundHeight > 0.f && !visibleEventLogLines.IsEmpty())
		{
			DrawOverlayLines(
				*this,
				visibleEventLogLines,
				rightPanelGeometry.EventLogBackgroundX + DebugOverlayBackgroundPadding,
				rightPanelGeometry.EventLogBackgroundY + DebugOverlayBackgroundPadding,
				rightPanelGeometry.EventLogBackgroundX,
				rightPanelGeometry.EventLogBackgroundY,
				rightPanelGeometry.EventLogAvailableWidth,
				eventLogBackgroundHeight,
				false,
				true);
		}

		if (rightPanelGeometry.bCanDrawInteractionPanel)
		{
			const float interactionAvailableHeight = FMath::Max(0.f, Canvas->SizeY - rightPanelGeometry.EventLogBackgroundY - DebugOverlayBottomMargin);
			const float maxInteractionTextHeight = FMath::Max(0.f, interactionAvailableHeight - (DebugOverlayBackgroundPadding * 2.f));
			const TArray<FString> visibleInteractionLines = MakeVisibleOverlayLines(interactionLines, maxInteractionTextHeight, true, false);

			const float interactionBackgroundHeight = FMath::Min(
				CalculateOverlayLinesHeight(visibleInteractionLines, true, false) + (DebugOverlayBackgroundPadding * 2.f),
				interactionAvailableHeight);

			if (interactionBackgroundHeight > 0.f && !visibleInteractionLines.IsEmpty())
			{
				DrawOverlayLines(
					*this,
					visibleInteractionLines,
					rightPanelGeometry.InteractionBackgroundX + DebugOverlayBackgroundPadding,
					rightPanelGeometry.EventLogBackgroundY + DebugOverlayBackgroundPadding,
					rightPanelGeometry.InteractionBackgroundX,
					rightPanelGeometry.EventLogBackgroundY,
					rightPanelGeometry.InteractionBackgroundWidth,
					interactionBackgroundHeight,
					true,
					false);
			}
		}
	}
#endif
}
