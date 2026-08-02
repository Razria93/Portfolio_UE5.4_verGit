#include "Core/Debug/FDebugOverlayTextFormatter.h"

#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

namespace
{
	void AppendOverlayLine(TArray<FString>& InOutLines, const FString& InLine)
	{
		InOutLines.Add(InLine);
	}

	void AppendActorStatusLines(TArray<FString>& InOutLines, const FDebugOverlayActorStatusViewData& InStatusViewData)
	{
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("State: %s"), *InStatusViewData.StateText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Action: %s"), *InStatusViewData.ActionText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Reaction: %s"), *InStatusViewData.ReactionText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("HP: %s"), *InStatusViewData.HealthText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Stagger: %s"), *InStatusViewData.StaggerText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Guard: %s"), *InStatusViewData.GuardText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Movement: %s"), *InStatusViewData.MovementText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Runtime LOD: %s"), *InStatusViewData.RuntimeLODText));
	}

	void AppendCurrentAIBlock(TArray<FString>& InOutLines, const FDebugOverlayCurrentAIViewData& InCurrentAIViewData)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Current AI]"));

		if (!InCurrentAIViewData.bHasEnemy)
		{
			AppendOverlayLine(InOutLines, TEXT("NoTarget"));
			return;
		}

		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Controller: %s"), *InCurrentAIViewData.ControllerText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Pawn: %s"), *InCurrentAIViewData.PawnText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("Target: %s"), *InCurrentAIViewData.TargetText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("IntentState: %s"), *InCurrentAIViewData.IntentStateText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("ReturnHome: %s"), *InCurrentAIViewData.ReturnHomeText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("UsePatrol: %s"), *InCurrentAIViewData.UsePatrolText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("HasLOS: %s"), *InCurrentAIViewData.HasLOSText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("DistanceToTarget: %s"), *InCurrentAIViewData.DistanceToTargetText));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("IsCombatAction: %s"), *InCurrentAIViewData.IsCombatActionText));
	}

	void AppendRecentAIEventBlock(TArray<FString>& InOutLines, const FDebugOverlayRecentAIEventViewData& InRecentAIEventViewData)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, TEXT("[Recent AI Event]"));

		switch (InRecentAIEventViewData.State)
		{
		case EDebugOverlayRecentAIEventViewState::NoTarget:
			AppendOverlayLine(InOutLines, TEXT("NoTarget"));
			return;
		case EDebugOverlayRecentAIEventViewState::NotCaptured:
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		case EDebugOverlayRecentAIEventViewState::NotMatched:
			AppendOverlayLine(InOutLines, TEXT("NotMatched"));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Selected: %s"), *InRecentAIEventViewData.SelectedPawnName));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("LastPawn: %s"), *InRecentAIEventViewData.LastPawnName));
			return;
		case EDebugOverlayRecentAIEventViewState::Stale:
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Stale Time: %ss"), *InRecentAIEventViewData.StaleAgeText));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Last Pawn: %s"), *InRecentAIEventViewData.LastPawnName));
			AppendOverlayLine(InOutLines, TEXT("Note: Not current AI evidence"));
			return;
		case EDebugOverlayRecentAIEventViewState::Captured:
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Task: %s"), *InRecentAIEventViewData.TaskText));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Result: %s"), *InRecentAIEventViewData.ResultText));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("Age: %s"), *InRecentAIEventViewData.AgeText));
			AppendOverlayLine(InOutLines, FString::Printf(TEXT("RejectReason: %s"), *InRecentAIEventViewData.RejectReasonText));
			return;
		default:
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}
	}

	void AppendActorPanelLines(TArray<FString>& InOutLines, const FDebugOverlayActorPanelViewData& InActorPanelViewData)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, InActorPanelViewData.HeaderText);

		for (const FString& lineBeforeStatus : InActorPanelViewData.LinesBeforeStatus)
		{
			AppendOverlayLine(InOutLines, lineBeforeStatus);
		}

		if (InActorPanelViewData.bAppendBlankBeforeStatus)
		{
			AppendOverlayLine(InOutLines, TEXT(""));
		}

		AppendActorStatusLines(InOutLines, InActorPanelViewData.Status);

		for (const FString& recentExecutionLine : InActorPanelViewData.RecentExecutionLines)
		{
			AppendOverlayLine(InOutLines, recentExecutionLine);
		}

		if (InActorPanelViewData.bIncludeCurrentAI)
		{
			AppendCurrentAIBlock(InOutLines, InActorPanelViewData.CurrentAI);
		}

		if (InActorPanelViewData.bIncludeRecentAIEvent)
		{
			AppendRecentAIEventBlock(InOutLines, InActorPanelViewData.RecentAIEvent);
		}
	}

	TArray<FString> BuildMainPanelLines(const FDebugOverlayViewData& InViewData)
	{
		TArray<FString> lines;
		lines.Reserve(32);
		AppendOverlayLine(lines, InViewData.MainPanelTitle);

		for (const FDebugOverlayActorPanelViewData& actorPanel : InViewData.ActorPanels)
		{
			AppendActorPanelLines(lines, actorPanel);
		}

		for (const FString& legacyLine : InViewData.MainPanelLines)
		{
			AppendOverlayLine(lines, legacyLine);
		}

		return lines;
	}

	EDebugOverlayTextLineRole ResolveTextLineRole(EDebugOverlayTextPanelRole InPanelRole, const FString& InLine)
	{
		if (InLine.StartsWith(TEXT("[Debug Overlay Pannel_")))
		{
			return EDebugOverlayTextLineRole::PanelTitle;
		}

		if (InPanelRole != EDebugOverlayTextPanelRole::EventLog
			&& (InLine == TEXT("[Player]") || InLine == TEXT("[Enemy]") || InLine == TEXT("[Interaction]")))
		{
			return EDebugOverlayTextLineRole::PanelHeader;
		}

		if (InPanelRole == EDebugOverlayTextPanelRole::EventLog && InLine.StartsWith(TEXT("[Event Log:")))
		{
			return EDebugOverlayTextLineRole::EventLogHeader;
		}

		return EDebugOverlayTextLineRole::Normal;
	}

	FDebugOverlayTextPanel MakeTextPanel(EDebugOverlayTextPanelRole InPanelRole, const TArray<FString>& InLines)
	{
		FDebugOverlayTextPanel panel(InPanelRole);
		panel.Lines.Reserve(InLines.Num());
		for (const FString& line : InLines)
		{
			panel.Lines.Add(FDebugOverlayTextLine(line, ResolveTextLineRole(InPanelRole, line)));
		}

		return panel;
	}
}

FDebugOverlayTextPanels FDebugOverlayTextFormatter::Format(const FDebugOverlayViewData& InViewData)
{
	const TArray<FString> mainPanelLines = BuildMainPanelLines(InViewData);

	FDebugOverlayTextPanels panels;
	panels.MainPanel = MakeTextPanel(EDebugOverlayTextPanelRole::Main, mainPanelLines);
	panels.EventLogPanel = MakeTextPanel(EDebugOverlayTextPanelRole::EventLog, InViewData.EventLogPanelLines);
	panels.InteractionPanel = MakeTextPanel(EDebugOverlayTextPanelRole::Interaction, InViewData.InteractionPanelLines);
	return panels;
}
