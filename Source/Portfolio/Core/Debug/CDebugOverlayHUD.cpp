#include "Core/Debug/CDebugOverlayHUD.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CActionComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

namespace
{
	static constexpr float DebugOverlayOriginX = 24.f;
	static constexpr float DebugOverlayOriginY = 36.f;
	static constexpr float DebugOverlayLineHeight = 20.f;
	static constexpr float DebugOverlayFontScale = 1.05f;
	static constexpr float DebugOverlayBackgroundPadding = 10.f;
	static constexpr float DebugOverlayBackgroundWidth = 1040.f;
	static constexpr float DebugOverlayEnemyScanCooldownSeconds = 0.5f;
	static const FLinearColor DebugOverlayBackgroundColor(0.f, 0.f, 0.f, 0.72f);
	static const FLinearColor DebugOverlayPlayerHeaderColor(0.02f, 0.20f, 0.78f, 0.68f);
	static const FLinearColor DebugOverlayEnemyHeaderColor(0.78f, 0.06f, 0.04f, 0.68f);

	FString BoolText(bool bInValue)
	{
		return bInValue ? TEXT("true") : TEXT("false");
	}

	FString MissingText()
	{
		return TEXT("N/A");
	}

	template <typename TComponent>
	TComponent* FindComponent(const APawn* InPawn)
	{
		return IsValid(InPawn) ? InPawn->FindComponentByClass<TComponent>() : nullptr;
	}

	FString FormatExecutionState(const APawn* InPawn)
	{
		const UCStateComponent* stateComp = FindComponent<UCStateComponent>(InPawn);
		return IsValid(stateComp) ? UEnum::GetValueAsString(stateComp->GetCurrentExecutionState()) : MissingText();
	}

	FString FormatActiveAction(const APawn* InPawn)
	{
		const UCActionComponent* actionComp = FindComponent<UCActionComponent>(InPawn);
		if (!IsValid(actionComp)) return MissingText();
		if (!actionComp->IsActive()) return TEXT("None");

		return FString::Printf(
			TEXT("%s[%d]"),
			*UEnum::GetValueAsString(actionComp->GetActiveActionType()),
			actionComp->GetActiveActionIndex());
	}

	FString FormatActiveReaction(const APawn* InPawn)
	{
		const UCReactionComponent* reactionComp = FindComponent<UCReactionComponent>(InPawn);
		if (!IsValid(reactionComp)) return MissingText();
		if (!reactionComp->IsActive()) return TEXT("None");

		return UEnum::GetValueAsString(reactionComp->GetActiveReactionType());
	}

	FString FormatGuardOverlay(const APawn* InPawn)
	{
		const UCDefenseComponent* defenseComp = FindComponent<UCDefenseComponent>(InPawn);
		if (!IsValid(defenseComp)) return MissingText();

		return FString::Printf(
			TEXT("Wants=%s Pose=%s CanGuard=%s CanParry=%s CanStart=%s"),
			*BoolText(defenseComp->WantsGuarding()),
			*BoolText(defenseComp->IsGuardingPose()),
			*BoolText(defenseComp->CanGuard()),
			*BoolText(defenseComp->CanParry()),
			*BoolText(defenseComp->CanStartGuard()));
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
			TEXT("Gait=%s Speed=%.1f Dir=%.1f CanMove=%s Falling=%s"),
			*UEnum::GetValueAsString(movementComp->GetCurrentMovementGait()),
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
			TEXT("HP=%.1f/%.1f DeadState=%s"),
			healthComp->GetCurrentHP(),
			healthComp->GetMaxHP(),
			*UEnum::GetValueAsString(healthComp->GetDeadState()));
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

	bool HasFinalTakenDamageEvidence(const FDebugOverlayCombatSummary& InCombatSummary)
	{
		return InCombatSummary.CaptureState == EDebugOverlayCaptureState::Captured
			&& InCombatSummary.Summary.Contains(TEXT("Final="));
	}

	void AddLine(TArray<FString>& InOutLines, const FString& InLine)
	{
		InOutLines.Add(InLine);
	}

	void AddActorStatusLines(TArray<FString>& InOutLines, const APawn* InPawn)
	{
		AddLine(InOutLines, FString::Printf(TEXT("State: %s"), *FormatExecutionState(InPawn)));
		AddLine(InOutLines, FString::Printf(TEXT("Action: %s"), *FormatActiveAction(InPawn)));
		AddLine(InOutLines, FString::Printf(TEXT("Reaction: %s"), *FormatActiveReaction(InPawn)));
		AddLine(InOutLines, FString::Printf(TEXT("Guard: %s"), *FormatGuardOverlay(InPawn)));
		AddLine(InOutLines, FString::Printf(TEXT("Movement: %s"), *FormatActorMovement(InPawn)));
		AddLine(InOutLines, FString::Printf(TEXT("HP: %s"), *FormatActorHealth(InPawn)));
		AddLine(InOutLines, FString::Printf(TEXT("Runtime LOD: %s"), *FormatRuntimeLODTier()));
		AddLine(InOutLines, FString::Printf(TEXT("AI: %s"), *FormatAISummary()));
	}

	void AddActorPanelLines(TArray<FString>& InOutLines, const TCHAR* InPanelName, const APawn* InPawn)
	{
		AddLine(InOutLines, TEXT(""));
		AddLine(InOutLines, InPanelName);
		AddActorStatusLines(InOutLines, InPawn);
	}

	void AddSnapshotLines(TArray<FString>& InOutLines, const FDebugOverlaySnapshot& InSnapshot, bool bInHasSnapshot)
	{
		AddLine(InOutLines, TEXT(""));
		AddLine(InOutLines, TEXT("[Recent Execution]"));
		AddLine(InOutLines, FString::Printf(
			TEXT("Decision: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastExecution.Summary, InSnapshot.LastExecution.CaptureState) : TEXT("NotCaptured")));

		AddLine(InOutLines, TEXT(""));
		AddLine(InOutLines, TEXT("[Recent Combat]"));
		AddLine(InOutLines, FString::Printf(
			TEXT("HitWindow: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastCombat.HitWindowState, InSnapshot.LastCombat.CaptureState) : TEXT("NotCaptured")));
		AddLine(InOutLines, FString::Printf(
			TEXT("DefenseOutcome: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastCombat.DefenseOutcome, InSnapshot.LastCombat.CaptureState) : TEXT("NotCaptured")));
		AddLine(InOutLines, FString::Printf(
			TEXT("FinalTakenDamage: %s"),
			bInHasSnapshot && HasFinalTakenDamageEvidence(InSnapshot.LastCombat)
				? *FString::Printf(TEXT("%.3f"), InSnapshot.LastCombat.FinalTakenDamage)
				: TEXT("NotCaptured")));
		AddLine(InOutLines, FString::Printf(
			TEXT("DamageCommit: %s %.3f"),
			bInHasSnapshot && InSnapshot.LastCombat.bHasDamageCommit ? (InSnapshot.LastCombat.bDamageCommitted ? TEXT("true") : TEXT("false")) : TEXT("NotCaptured"),
			bInHasSnapshot && InSnapshot.LastCombat.bHasDamageCommit ? InSnapshot.LastCombat.CommittedDamage : 0.f));

		AddLine(InOutLines, TEXT(""));
		AddLine(InOutLines, TEXT("[Recent AI]"));
		AddLine(InOutLines, FString::Printf(
			TEXT("CombatTask: %s"),
			bInHasSnapshot ? *ValueOrNotCaptured(InSnapshot.LastAI.Summary, InSnapshot.LastAI.CaptureState) : TEXT("NotCaptured")));

		AddLine(InOutLines, TEXT(""));
		AddLine(InOutLines, TEXT("[Event Log]"));

		if (!bInHasSnapshot || InSnapshot.RecentEvents.IsEmpty())
		{
			AddLine(InOutLines, TEXT("NotCaptured"));
			return;
		}

		for (const FDebugOverlayEventEntry& eventEntry : InSnapshot.RecentEvents)
		{
			AddLine(InOutLines, FString::Printf(
				TEXT("%s/%s: %s"),
				*eventEntry.Category,
				*eventEntry.EventName,
				*eventEntry.Summary));
		}
	}

	bool IsPanelHeaderLine(const FString& InLine)
	{
		return InLine == TEXT("[Player]") || InLine == TEXT("[Enemy]");
	}

	FLinearColor GetPanelHeaderColor(const FString& InLine)
	{
		return InLine == TEXT("[Player]") ? DebugOverlayPlayerHeaderColor : DebugOverlayEnemyHeaderColor;
	}
}

#if !UE_BUILD_SHIPPING
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

ACEnemy* ACDebugOverlayHUD::ResolveDisplayEnemy()
{
	RefreshCachedEnemyIfNeeded();
	return LastEnemyScanCount == 1 ? CachedEnemy.Get() : nullptr;
}
#endif

void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
	Super::DrawHUD();

	if (!FDebugOverlaySnapshotStore::IsEnabled()) return;

	const APawn* pawn = GetOwningPawn();
	const ACEnemy* enemy = ResolveDisplayEnemy();

	FDebugOverlaySnapshot snapshot;
	const bool bHasSnapshot = FDebugOverlaySnapshotStore::GetSnapshotCopy(GetWorld(), snapshot);

	TArray<FString> lines;
	lines.Reserve(32);

	AddLine(lines, TEXT("[Debug Overlay P0.5]"));
	AddActorPanelLines(lines, TEXT("[Player]"), pawn);

	AddLine(lines, TEXT(""));
	AddLine(lines, TEXT("[Enemy]"));
	if (LastEnemyScanCount == 0)
	{
		AddLine(lines, TEXT("EnemyFallback: NotCaptured(NoEnemy)"));
	}
	else if (LastEnemyScanCount > 1)
	{
		AddLine(lines, FString::Printf(TEXT("EnemyFallback: Ambiguous(Count=%d)"), LastEnemyScanCount));
	}
	else if (!IsValid(enemy))
	{
		AddLine(lines, TEXT("EnemyFallback: NotCaptured(StaleEnemy)"));
	}
	else
	{
		AddLine(lines, FString::Printf(TEXT("EnemySource: WorldScanFallback")));
		AddLine(lines, FString::Printf(TEXT("EnemyFallback: Selected=%s Policy=FirstValid Count=1"), *GetNameSafe(enemy)));
	}

	AddLine(lines, TEXT(""));
	AddActorStatusLines(lines, enemy);

	AddSnapshotLines(lines, snapshot, bHasSnapshot);

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
