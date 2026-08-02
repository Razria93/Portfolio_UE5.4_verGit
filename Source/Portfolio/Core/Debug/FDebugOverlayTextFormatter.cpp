#include "Core/Debug/FDebugOverlayTextFormatter.h"

#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

namespace
{
	void AppendOverlayLine(TArray<FString>& InOutLines, const FString& InLine)
	{
		InOutLines.Add(InLine);
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

	FString ValueOrNone(const FString& InValue)
	{
		return InValue.IsEmpty() ? FString(TEXT("None")) : InValue;
	}

	FString FormatFocusActorText(const FString& InCurrentActorText)
	{
		constexpr TCHAR SelectedPrefix[] = TEXT("Selected: ");
		const FString actorText = InCurrentActorText.StartsWith(SelectedPrefix)
			? InCurrentActorText.RightChop(FCString::Strlen(SelectedPrefix))
			: InCurrentActorText;
		return ValueOrNone(actorText);
	}

	void AppendFocusLines(TArray<FString>& InOutLines, const FDebugOverlayFocusViewData& InFocusViewData)
	{
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("EnemyFocusMode: %s"), *ValueOrNone(InFocusViewData.CurrentSourceText)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("EnemyFocusActor: %s"), *FormatFocusActorText(InFocusViewData.CurrentActorText)));
		AppendOverlayLine(InOutLines, FString::Printf(TEXT("EnemyFocusCommand: %s"), *ValueOrNone(InFocusViewData.LastCommandText)));
	}

	void AppendRecentExecutionBlockLines(TArray<FString>& InOutLines, const FDebugOverlayRecentExecutionViewData& InRecentExecutionViewData)
	{
		AppendOverlayLine(InOutLines, TEXT(""));
		AppendOverlayLine(InOutLines, InRecentExecutionViewData.HeaderText);

		switch (InRecentExecutionViewData.State)
		{
		case EDebugOverlayRecentExecutionViewState::NotCaptured:
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		case EDebugOverlayRecentExecutionViewState::NoActor:
			AppendOverlayLine(InOutLines, TEXT("N/A"));
			return;
		case EDebugOverlayRecentExecutionViewState::NoEvents:
			AppendOverlayLine(InOutLines, TEXT("NoEvents(Filter: Execution)"));
			return;
		case EDebugOverlayRecentExecutionViewState::Captured:
			AppendSummaryLines(InOutLines, InRecentExecutionViewData.SummaryText, EDebugOverlayCaptureState::Captured);
			return;
		default:
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}
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

		if (InActorPanelViewData.bIncludeFocus)
		{
			AppendFocusLines(InOutLines, InActorPanelViewData.Focus);
		}

		if (InActorPanelViewData.bAppendBlankBeforeStatus)
		{
			AppendOverlayLine(InOutLines, TEXT(""));
		}

		AppendActorStatusLines(InOutLines, InActorPanelViewData.Status);
		AppendRecentExecutionBlockLines(InOutLines, InActorPanelViewData.RecentExecution);

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

	FString FormatEventLogEntryLine(const FDebugOverlayEventLogEntryViewData& InEntry)
	{
		return FString::Printf(
			TEXT("%s/%s: %s"),
			*InEntry.CategoryText,
			*InEntry.EventNameText,
			*InEntry.SummaryText);
	}

	TArray<FString> BuildEventLogPanelLines(const FDebugOverlayViewData& InViewData)
	{
		TArray<FString> lines;
		lines.Reserve(InViewData.EventLog.Entries.Num() + 2);
		AppendOverlayLine(lines, InViewData.EventLogPanelTitle);
		AppendOverlayLine(lines, TEXT(""));
		AppendOverlayLine(lines, FString::Printf(TEXT("[Event Log: %s]"), *InViewData.EventLog.FilterText));

		if (!InViewData.EventLog.bHasSnapshot)
		{
			AppendOverlayLine(lines, TEXT("NotCaptured"));
			return lines;
		}

		if (InViewData.EventLog.DisplayLimit == 0)
		{
			AppendOverlayLine(lines, FString::Printf(TEXT("NoEvents(Filter: %s Limit: 0)"), *InViewData.EventLog.FilterText));
			return lines;
		}

		if (InViewData.EventLog.Entries.IsEmpty())
		{
			AppendOverlayLine(lines, FString::Printf(TEXT("NoEvents(Filter: %s)"), *InViewData.EventLog.FilterText));
			return lines;
		}

		for (const FDebugOverlayEventLogEntryViewData& eventEntry : InViewData.EventLog.Entries)
		{
			AppendOverlayLine(lines, FormatEventLogEntryLine(eventEntry));
		}

		return lines;
	}

	void AppendRecentSummaryBlockLines(TArray<FString>& InOutLines, const FDebugOverlayRecentSummaryBlockViewData& InBlockViewData)
	{
		if (InBlockViewData.bAppendLeadingBlank)
		{
			AppendOverlayLine(InOutLines, TEXT(""));
		}

		AppendOverlayLine(InOutLines, InBlockViewData.HeaderText);
		if (InBlockViewData.bHasSnapshot)
		{
			AppendSummaryLines(InOutLines, InBlockViewData.SummaryText, InBlockViewData.CaptureState);
		}
		else
		{
			AppendOverlayLine(InOutLines, TEXT("NotCaptured"));
		}
	}

	TArray<FString> BuildInteractionPanelLines(const FDebugOverlayViewData& InViewData)
	{
		TArray<FString> lines;
		lines.Reserve(16);
		AppendOverlayLine(lines, InViewData.InteractionPanelTitle);
		AppendOverlayLine(lines, TEXT(""));
		AppendOverlayLine(lines, InViewData.Interaction.HeaderText);

		for (const FDebugOverlayRecentSummaryBlockViewData& summaryBlock : InViewData.Interaction.SummaryBlocks)
		{
			AppendRecentSummaryBlockLines(lines, summaryBlock);
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
	const TArray<FString> eventLogPanelLines = BuildEventLogPanelLines(InViewData);
	const TArray<FString> interactionPanelLines = BuildInteractionPanelLines(InViewData);

	FDebugOverlayTextPanels panels;
	panels.MainPanel = MakeTextPanel(EDebugOverlayTextPanelRole::Main, mainPanelLines);
	panels.EventLogPanel = MakeTextPanel(EDebugOverlayTextPanelRole::EventLog, eventLogPanelLines);
	panels.InteractionPanel = MakeTextPanel(EDebugOverlayTextPanelRole::Interaction, interactionPanelLines);
	return panels;
}
