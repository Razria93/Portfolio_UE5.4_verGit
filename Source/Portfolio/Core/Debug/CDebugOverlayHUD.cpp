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
	static constexpr float DebugOverlayEnemyScanCooldownSeconds = 0.5f;
	static constexpr float DebugOverlayRecentCombatTargetStaleSeconds = 3.0f;
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

	void AppendEnemyRecentAIBlock(TArray<FString>& InOutLines, const ACEnemy* InEnemy, const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent AI]"));

		if (!IsValid(InEnemy))
		{
			AppendOverlayLine(InOutLines, TEXT("NoTarget"));
			return;
		}

		const ACAIController* aiController = Cast<ACAIController>(InEnemy->GetController());
		const UBlackboardComponent* blackboardComp = IsValid(aiController) ? aiController->GetBlackboardComponent() : nullptr;

		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Controller: %s"), *GetNameSafe(aiController)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Pawn: %s"), *GetNameSafe(InEnemy)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Target: %s"), *FormatAITargetActor(blackboardComp)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("IntentState: %s"), *FormatAIIntentState(blackboardComp)));

		const FString enemyName = GetNameSafe(InEnemy);
		const bool bHasMatchingRecentTask = bInHasSnapshot
			&& InSnapshot.LastAI.CaptureState == EDebugOverlayCaptureState::Captured
			&& InSnapshot.LastAI.PawnName == enemyName;

		if (!bHasMatchingRecentTask)
		{
			AppendOverlayLine(InOutLines, FString::Printf(
				TEXT("RecentTask: %s"),
				bInHasSnapshot && InSnapshot.LastAI.CaptureState == EDebugOverlayCaptureState::Captured ? TEXT("NotMatched") : TEXT("NotCaptured")));
			return;
		}

		AppendOverlayLine(InOutLines, FString::Printf(TEXT("RecentTask: %s"), *CompactEnumText(InSnapshot.LastAI.SubState)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Result: %s"), *CompactEnumText(InSnapshot.LastAI.RequestResult)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("RejectReason: %s"), *CompactReasonText(InSnapshot.LastAI.RejectReason)));
	}

	void AppendActorPanelLines(TArray<FString>& InOutLines, const TCHAR* InPanelName, const APawn* InPawn, const UObject* InWorldContextObject, bool bInHasSnapshot)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, InPanelName);
		AppendActorStatusLines(InOutLines, InPawn);
		AppendActorRecentExecutionBlock(InOutLines, InWorldContextObject, bInHasSnapshot, InPawn);
	}

	void AppendSnapshotLines(TArray<FString>& InOutLines, const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot)
	{
		AppendOverlayLine(InOutLines, TEXT("[Recent Execution]"));
		if (bInHasSnapshot)
		{
			AppendSummaryLines(InOutLines, InSnapshot.LastExecution.Summary, InSnapshot.LastExecution.CaptureState);
		}
		else
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
		}

		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent Combat]"));
		if (bInHasSnapshot)
		{
			AppendSummaryLines(InOutLines, InSnapshot.LastCombat.Summary, InSnapshot.LastCombat.CaptureState);
		}
		else
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
		}

	}

	// Panel Styling
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

	float CalculateOverlayLinesHeight(
		const TArray<FString>& InLines,
		bool bInDrawPanelHeaders,
		bool bInDrawEventLogHeaders)
	{
		float height = 0.f;
		for (const FString& line : InLines)
		{
			height += DebugOverlayLineHeight;
			if (IsOverlayHeaderLine(line, bInDrawPanelHeaders, bInDrawEventLogHeaders))
			{
				height += DebugOverlayHeaderBottomPadding;
			}
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
			const float lineHeight = DebugOverlayLineHeight
				+ (IsOverlayHeaderLine(line, bInDrawPanelHeaders, bInDrawEventLogHeaders) ? DebugOverlayHeaderBottomPadding : 0.f);

			if (usedHeight + lineHeight > InMaxTextHeight)
			{
				return lineIndex;
			}

			usedHeight += lineHeight;
		}

		return InLines.Num();
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

	// Enemy Source Formatting
	FString FormatAgeSeconds(float InAgeSeconds)
	{
		return FString::Printf(TEXT("%.2f"), FMath::Max(0.f, InAgeSeconds));
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

	const APawn* pawn = GetOwningPawn();
	TArray<FString> enemySourceLines;
	const ACEnemy* enemy = ResolveDisplayEnemy(enemySourceLines);

	FDebugOverlaySnapshot snapshot;
	const bool bHasSnapshot = FDebugOverlaySnapshotStore::TryGetSnapshotCopy(GetWorld(), snapshot);
	const FString eventLogFilter = FDebugOverlaySnapshotStore::GetEventLogFilter();
	const int32 eventLogLimit = FDebugOverlaySnapshotStore::GetEventLogDisplayLimit();
	const TArray<FDebugOverlayEventEntry> recentEvents = FDebugOverlaySnapshotStore::GetRecentEventsCopy(
		GetWorld(),
		eventLogLimit,
		eventLogFilter);

	TArray<FString> lines;
	lines.Reserve(32);
	TArray<FString> eventLogLines;
	eventLogLines.Reserve(eventLogLimit + 2);

	AppendOverlayLine(lines, TEXT("[Debug Overlay Pannel_01]"));
	AppendActorPanelLines(lines, TEXT("[Player]"), pawn, GetWorld(), bHasSnapshot);

	AppendOverlayLine(lines, TEXT(""));
	AppendOverlayLine(lines, TEXT("[Enemy]"));
	for (const FString& enemySourceLine : enemySourceLines)
	{
		AppendOverlayLine(lines, enemySourceLine);
	}

	AppendOverlayLine(lines, TEXT(""));
	AppendActorStatusLines(lines, enemy);
	AppendActorRecentExecutionBlock(lines, GetWorld(), bHasSnapshot, enemy);
	AppendEnemyRecentAIBlock(lines, enemy, snapshot, bHasSnapshot);

	AppendOverlayLine(lines, TEXT(""));
	AppendOverlayLine(lines, TEXT("[Interaction]"));
	AppendSnapshotLines(lines, snapshot, bHasSnapshot);

	AppendOverlayLine(eventLogLines, TEXT("[Debug Overlay Pannel_02]"));
	AppendEventLogBlock(eventLogLines, bHasSnapshot, recentEvents, eventLogFilter, eventLogLimit);

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
		const float eventLogBackgroundX = backgroundX + backgroundWidth + DebugOverlayPanelGap;
		const float eventLogBackgroundY = backgroundY;
		const float eventLogAvailableWidth = FMath::Max(0.f, Canvas->SizeX - eventLogBackgroundX - DebugOverlayRightMargin);
		const float eventLogAvailableHeight = FMath::Max(0.f, Canvas->SizeY - eventLogBackgroundY - DebugOverlayBottomMargin);
		const float maxEventLogTextHeight = FMath::Max(0.f, eventLogAvailableHeight - (DebugOverlayBackgroundPadding * 2.f));
		const int32 eventLogLineCount = CalculateVisibleOverlayLineCount(eventLogLines, maxEventLogTextHeight, false, true);
		TArray<FString> visibleEventLogLines;
		visibleEventLogLines.Reserve(eventLogLineCount);
		for (int32 lineIndex = 0; lineIndex < eventLogLineCount; ++lineIndex)
		{
			visibleEventLogLines.Add(eventLogLines[lineIndex]);
		}

		const float eventLogBackgroundHeight = FMath::Min(
			CalculateOverlayLinesHeight(visibleEventLogLines, false, true) + (DebugOverlayBackgroundPadding * 2.f),
			eventLogAvailableHeight);

		if (eventLogAvailableWidth >= DebugOverlayMinEventLogPanelWidth && eventLogBackgroundHeight > 0.f && !visibleEventLogLines.IsEmpty())
		{
			DrawOverlayLines(
				*this,
				visibleEventLogLines,
				eventLogBackgroundX + DebugOverlayBackgroundPadding,
				eventLogBackgroundY + DebugOverlayBackgroundPadding,
				eventLogBackgroundX,
				eventLogBackgroundY,
				eventLogAvailableWidth,
				eventLogBackgroundHeight,
				false,
				true);
		}
	}
#endif
}
