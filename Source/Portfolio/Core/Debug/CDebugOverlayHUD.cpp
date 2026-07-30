#include "Core/Debug/CDebugOverlayHUD.h"

#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "Component/CActionComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Core/Debug/CDebugOverlayTargetComponent.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Type/CActionKeyTypes.h"

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
	static constexpr float DebugOverlayBackgroundWidth = 1040.f;
	static constexpr float DebugOverlayEnemyScanCooldownSeconds = 0.5f;
	static constexpr float DebugOverlayRecentCombatTargetStaleSeconds = 3.0f;
	static const FLinearColor DebugOverlayBackgroundColor(0.f, 0.f, 0.f, 0.72f);
	static const FLinearColor DebugOverlayPlayerHeaderColor(0.02f, 0.20f, 0.78f, 0.68f);
	static const FLinearColor DebugOverlayEnemyHeaderColor(0.78f, 0.06f, 0.04f, 0.68f);

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
			TEXT("Wants=%s | Pose=%s | CanGuard=%s | CanParry=%s | CanStart=%s"),
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

	FString FormatAISummary()
	{
		return TEXT("NotCaptured");
	}

	FString FormatActorMovement(const APawn* InPawn)
	{
		const UCMovementComponent* movementComp = FindComponent<UCMovementComponent>(InPawn);
		if (!IsValid(movementComp)) return MissingText();

		return FString::Printf(
			TEXT("Gait=%s | Speed=%.1f | Dir=%.1f | CanMove=%s | Falling=%s"),
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
			TEXT("HP=%.1f/%.1f | DeadState=%s"),
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

	bool HasFinalTakenDamageEvidence(const FDebugOverlayCombatSummary& InCombatSummary)
	{
		return InCombatSummary.CaptureState == EDebugOverlayCaptureState::Captured
			&& InCombatSummary.Summary.Contains(TEXT("Final="));
	}

	void AppendOverlayLine(TArray<FString>& InOutLines, const FString& InLine)
	{
		InOutLines.Add(InLine);
	}

	void AppendActorStatusLines(TArray<FString>& InOutLines, const APawn* InPawn)
	{
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("State: %s"), *FormatExecutionState(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Action: %s"), *FormatActiveAction(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Reaction: %s"), *FormatActiveReaction(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Stagger: %s"), *FormatParryStaggerStack(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Guard: %s"), *FormatGuardOverlay(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Movement: %s"), *FormatActorMovement(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("HP: %s"), *FormatActorHealth(InPawn)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Runtime LOD: %s"), *FormatRuntimeLODTier()));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("AI: %s"), *FormatAISummary()));
	}

	void AppendActorPanelLines(TArray<FString>& InOutLines, const TCHAR* InPanelName, const APawn* InPawn)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, InPanelName);
		AppendActorStatusLines(InOutLines, InPawn);
	}

	void AppendSnapshotLines(TArray<FString>& InOutLines, const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot, const TArray<FDebugOverlayEventEntry>& InFilteredEvents)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent Execution]"));
		AppendOverlayLine(InOutLines, FString::Printf(
			TEXT("Decision: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastExecution.Summary, InSnapshot.LastExecution.CaptureState) : TEXT("NotCaptured")));

		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent Combat]"));

		AppendOverlayLine(InOutLines, FString::Printf(
			TEXT("HitWindow: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastCombat.HitWindowState, InSnapshot.LastCombat.CaptureState) : TEXT("NotCaptured")));

		AppendOverlayLine(InOutLines, FString::Printf(
			TEXT("DefenseOutcome: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastCombat.DefenseOutcome, InSnapshot.LastCombat.CaptureState) : TEXT("NotCaptured")));

		AppendOverlayLine(InOutLines, FString::Printf(
			TEXT("FinalTakenDamage: %s"),
			bInHasSnapshot && HasFinalTakenDamageEvidence(InSnapshot.LastCombat) ? *FString::Printf(TEXT("%.3f"), InSnapshot.LastCombat.FinalTakenDamage) : TEXT("NotCaptured")));

		AppendOverlayLine(InOutLines, FString::Printf(
			TEXT("DamageCommit: %s %.3f"),
			bInHasSnapshot && InSnapshot.LastCombat.bHasDamageCommit ? (InSnapshot.LastCombat.bDamageCommitted ? TEXT("true") : TEXT("false")) : TEXT("NotCaptured"),
			bInHasSnapshot && InSnapshot.LastCombat.bHasDamageCommit ? InSnapshot.LastCombat.CommittedDamage : 0.f));

		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent AI]"));

		AppendOverlayLine(InOutLines, FString::Printf(
			TEXT("CombatTask: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastAI.Summary, InSnapshot.LastAI.CaptureState) : TEXT("NotCaptured")));

		const FString eventLogFilter = FDebugOverlaySnapshotStore::GetEventLogFilter();
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("[Event Log: %s]"), *eventLogFilter));

		const int32 eventLogLimit = FDebugOverlaySnapshotStore::GetEventLogDisplayLimit();
		if (!bInHasSnapshot)
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}

		if (eventLogLimit == 0)
		{
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("NoEvents(Filter=%s Limit=0)"), *eventLogFilter));
			return;
		}

		if (InFilteredEvents.IsEmpty())
		{
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("NoEvents(Filter=%s)"), *eventLogFilter));
			return;
		}

		for (const FDebugOverlayEventEntry& eventEntry : InFilteredEvents)
		{
			AppendOverlayLine(InOutLines, FString::Printf(
				TEXT("%s/%s: %s"),
				*eventEntry.Category,
				*eventEntry.EventName,
				*eventEntry.Summary));
		}
	}

	// Panel Styling
	bool IsPanelHeaderLine(const FString& InLine)
	{
		return InLine == TEXT("[Player]") || InLine == TEXT("[Enemy]");
	}

	FLinearColor GetPanelHeaderColor(const FString& InLine)
	{
		return InLine == TEXT("[Player]") ? DebugOverlayPlayerHeaderColor : DebugOverlayEnemyHeaderColor;
	}

	// Enemy Source Formatting
	FString FormatAgeSeconds(float InAgeSeconds)
	{
		return FString::Printf(TEXT("%.2f"), FMath::Max(0.f, InAgeSeconds));
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
				TEXT("EnemyRecentCombat: Source=%s Target=%s Age=%s"),
				*recentCombatPair.SourceName,
				*recentCombatPair.TargetName,
				*FormatAgeSeconds(pairAge)));
			return recentEnemy;
		}
	}

	if (bPairStale || bSourceInvalid || bTargetInvalid)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(
			TEXT("EnemyRecentCombat: Stale Source=%s Target=%s Age=%s"),
			*recentCombatPair.SourceName,
			*recentCombatPair.TargetName,
			*FormatAgeSeconds(pairAge)));
	}
	else if (!bRecentCombatPairMatched)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(
			TEXT("EnemyRecentCombat: NotMatched Source=%s Target=%s Age=%s"),
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
	AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemyFallback: Selected=%s Policy=FirstValid Count=1"), *GetNameSafe(fallbackEnemy)));
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
	const TArray<FDebugOverlayEventEntry> filteredEvents = FDebugOverlaySnapshotStore::GetRecentEventsCopy(
		GetWorld(),
		FDebugOverlaySnapshotStore::GetEventLogDisplayLimit(),
		FDebugOverlaySnapshotStore::GetEventLogFilter());

	TArray<FString> lines;
	lines.Reserve(32);

	AppendOverlayLine(lines, TEXT("[Debug Overlay P0.5]"));
	AppendActorPanelLines(lines, TEXT("[Player]"), pawn);

	AppendOverlayLine(lines, TEXT(""));
	AppendOverlayLine(lines, TEXT("[Enemy]"));
	for (const FString& enemySourceLine : enemySourceLines)
	{
		AppendOverlayLine(lines, enemySourceLine);
	}

	AppendOverlayLine(lines, TEXT(""));
	AppendActorStatusLines(lines, enemy);

	AppendSnapshotLines(lines, snapshot, bHasSnapshot, filteredEvents);

	const float backgroundX = FMath::Max(0.f, DebugOverlayOriginX - DebugOverlayBackgroundPadding);
	const float backgroundY = FMath::Max(0.f, DebugOverlayOriginY - DebugOverlayBackgroundPadding);
	const float availableWidth = Canvas
		? FMath::Max(0.f, Canvas->SizeX - backgroundX - DebugOverlayBackgroundPadding)
		: DebugOverlayBackgroundWidth;
	const float backgroundWidth = FMath::Min(DebugOverlayBackgroundWidth, availableWidth);
	const float backgroundHeight = (lines.Num() * DebugOverlayLineHeight) + (DebugOverlayBackgroundPadding * 2.f);

	if (backgroundWidth > 0.f && backgroundHeight > 0.f)
	{
		DrawRect(DebugOverlayBackgroundColor, backgroundX, backgroundY, backgroundWidth, backgroundHeight);
	}

	float y = DebugOverlayOriginY;
	for (const FString& line : lines)
	{
		if (IsPanelHeaderLine(line) && backgroundWidth > 0.f)
		{
			DrawRect(GetPanelHeaderColor(line), backgroundX, y - 2.f, backgroundWidth, DebugOverlayLineHeight + 4.f);
		}

		DrawText(line, FLinearColor::White, DebugOverlayOriginX, y, nullptr, DebugOverlayFontScale, false);
		y += DebugOverlayLineHeight;
	}
#endif
}
