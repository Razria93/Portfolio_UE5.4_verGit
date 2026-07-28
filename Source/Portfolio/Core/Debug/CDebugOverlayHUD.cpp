#include "Core/Debug/CDebugOverlayHUD.h"

#include "Component/CActionComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Engine/Canvas.h"
#include "GameFramework/Pawn.h"

namespace
{
	static constexpr float DebugOverlayOriginX = 24.f;
	static constexpr float DebugOverlayOriginY = 36.f;
	static constexpr float DebugOverlayLineHeight = 20.f;
	static constexpr float DebugOverlayFontScale = 1.05f;
	static constexpr float DebugOverlayBackgroundPadding = 10.f;
	static constexpr float DebugOverlayBackgroundWidth = 920.f;
	static const FLinearColor DebugOverlayBackgroundColor(0.f, 0.f, 0.f, 0.72f);

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
}

void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
	Super::DrawHUD();

	if (!FDebugOverlaySnapshotStore::IsEnabled()) return;

	const APawn* pawn = GetOwningPawn();

	FDebugOverlaySnapshot snapshot;
	const bool bHasSnapshot = FDebugOverlaySnapshotStore::GetSnapshotCopy(GetWorld(), snapshot);

	TArray<FString> lines;
	lines.Reserve(32);

	AddLine(lines, TEXT("[Debug Overlay P0]"));
	AddLine(lines, FString::Printf(TEXT("ExecutionState: %s"), *FormatExecutionState(pawn)));
	AddLine(lines, FString::Printf(TEXT("ActiveAction: %s"), *FormatActiveAction(pawn)));
	AddLine(lines, FString::Printf(TEXT("ActiveReaction: %s"), *FormatActiveReaction(pawn)));
	AddLine(lines, FString::Printf(TEXT("GuardOverlay: %s"), *FormatGuardOverlay(pawn)));
	AddLine(lines, FString::Printf(TEXT("RuntimeLODTier: %s"), *FormatRuntimeLODTier()));

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
		DrawText(line, FLinearColor::White, DebugOverlayOriginX, y, nullptr, DebugOverlayFontScale, false);
		y += DebugOverlayLineHeight;
	}
#endif
}
