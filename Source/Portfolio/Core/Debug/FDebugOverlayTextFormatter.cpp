#include "Core/Debug/FDebugOverlayTextFormatter.h"

#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

namespace
{
	// ===== Format Text Helpers =====

	FString FormatCaptureStateText(EDebugOverlayCaptureState InState)
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

	FString FormatValueOrCaptureStateText(const FString& InValue, EDebugOverlayCaptureState InState)
	{
		return InState == EDebugOverlayCaptureState::Captured && !InValue.IsEmpty()
			? InValue
			: FormatCaptureStateText(InState);
	}

	FString FormatValueOrNoneText(const FString& InValue)
	{
		return InValue.IsEmpty() ? FString(TEXT("None")) : InValue;
	}

	// ===== Append Line Helpers =====

	void AppendFormattedOverlayLine(TArray<FString>& InOutLines, const FString& InLine)
	{
		InOutLines.Add(InLine);
	}

	void AppendSummaryLines(TArray<FString>& InOutLines, const FString& InSummary, EDebugOverlayCaptureState InCaptureState)
	{
		const FString summary = FormatValueOrCaptureStateText(InSummary, InCaptureState);
		if (InCaptureState != EDebugOverlayCaptureState::Captured || summary.IsEmpty())
		{
			AppendFormattedOverlayLine(InOutLines, summary);
			return;
		}

		TArray<FString> summaryParts;
		summary.ParseIntoArray(summaryParts, TEXT(" | "), true);
		if (summaryParts.IsEmpty())
		{
			AppendFormattedOverlayLine(InOutLines, summary);
			return;
		}

		for (const FString& summaryPart : summaryParts)
		{
			AppendFormattedOverlayLine(InOutLines, summaryPart);
		}
	}

	// [Player / Enemy Status]
	// ===== Actor Status Lines =====

	void AppendActorStatusLines(TArray<FString>& InOutLines, const FDebugOverlayActorStatusViewData& InStatusViewData)
	{
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("State: %s"), *InStatusViewData.StateText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Action: %s"), *InStatusViewData.ActionText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Reaction: %s"), *InStatusViewData.ReactionText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("HP: %s"), *InStatusViewData.HealthText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Balance: %s"), *InStatusViewData.BalanceText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Guard: %s"), *InStatusViewData.GuardText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Movement: %s"), *InStatusViewData.MovementText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Runtime LOD: %s"), *InStatusViewData.RuntimeLODText));
	}

	void AppendPlayerLocomotionLines(TArray<FString>& InOutLines, const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotionViewData)
	{
		const FMovementDebugOverlayDetails& locomotion = InPlayerLocomotionViewData.Details;
		if (!locomotion.bHasSnapshot) return;

		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Locomotion Inputs]"));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Speed: %s | Direction: %s"), *locomotion.SpeedText, *locomotion.DirectionText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Velocity World XY: %s"), *locomotion.VelocityWorldText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Velocity Local: %s"), *locomotion.VelocityLocalText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Last Input World XY: %s"), *locomotion.LastInputWorldText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Last Input Local: %s"), *locomotion.LastInputLocalText));
	}

	// [Focus]
	// ===== Focus Lines =====

	void AppendFocusLines(TArray<FString>& InOutLines, const FDebugOverlayFocusViewData& InFocusViewData)
	{
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("FocusDriver: %s"), *FormatValueOrNoneText(InFocusViewData.FocusDriverText)));
		if (!InFocusViewData.RecentFocusStateText.IsEmpty())
		{
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("RecentFocusState: %s"), *InFocusViewData.RecentFocusStateText));
		}
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("RuntimeFocusSource: %s"), *FormatValueOrNoneText(InFocusViewData.CurrentSourceText)));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("FocusActor: %s"), *FormatValueOrNoneText(InFocusViewData.CurrentActorNameText)));

	}

	void AppendPlayerTargetingLines(TArray<FString>& InOutLines, const FDebugOverlayPlayerTargetingViewData& InPlayerTargetingViewData)
	{
		const FTargetingDebugOverlayDetails& targeting = InPlayerTargetingViewData.Details;
		if (!targeting.bHasSnapshot) return;

		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Targeting]"));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Runtime Target: %s"), *targeting.RuntimeTargetText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Distance: %s"), *targeting.DistanceText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Dot: %s"), *targeting.DotText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Angle Score: %s"), *targeting.AngleScoreText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Distance Score: %s"), *targeting.DistanceScoreText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Final Score: %s"), *targeting.FinalScoreText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("In Range: %s | In View Cone: %s"), *targeting.RangeText, *targeting.ViewConeText));
	}

	// [Balance / Collapse]
	// ===== Balance / Collapse Lines =====

	void AppendBalanceCollapseLines(TArray<FString>& InOutLines, const FDebugOverlayBalanceCollapseViewData& InBalanceCollapseViewData)
	{
		const FBalanceDebugOverlayDetails& details = InBalanceCollapseViewData.Details;
		if (!details.bHasSnapshot) return;

		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Balance / Collapse]"));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Count: %s"), *details.CountText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Lifecycle: %s | Serial: %s"), *details.LifecycleText, *details.LifecycleSerialText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Loop Remaining: %s"), *details.LoopLifetimeText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Execution Down Remaining: %s"), *details.ExecutionDownLifetimeText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Collapse Pose: %s | Loop: %s | Execution Down: %s"), *details.CollapsePoseText, *details.CollapseLoopText, *details.ExecutionDownPoseText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Blocking: %s | Facing Suppressed: %s"), *details.LifecycleBlockingText, *details.FacingSuppressedText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Last Abort: %s"), *details.LastAbortText));
	}

	// [Execution Collaboration]
	// ===== Execution Collaboration Lines =====

	void AppendExecutionCollaborationLines(TArray<FString>& InOutLines, const FDebugOverlayExecutionCollaborationViewData& InExecutionCollaborationViewData)
	{
		const FExecutionCollaborationDebugOverlayDetails& details = InExecutionCollaborationViewData.Details;
		if (!details.bHasSnapshot) return;

		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Execution Collaboration]"));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Role: %s | State: %s | Outcome: %s"), *details.RoleText, *details.StateText, *details.OutcomeText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Partner: %s | Session: %s"), *details.PartnerText, *details.SessionText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Reservation: %s"), *details.ReservationText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Terminal: %s"), *details.TerminalText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Geometry: %s"), *details.GeometryText));
	}

	// [Combat Participation]
	// ===== Combat Participation Lines =====

	void AppendCombatParticipationLines(TArray<FString>& InOutLines, const FDebugOverlayCombatParticipationViewData& InCombatParticipationViewData)
	{
		const FCombatParticipationDebugOverlayDetails& details = InCombatParticipationViewData.FocusedEnemyDetails;
		if (!details.bHasSnapshot) return;

		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Combat Participation]"));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Role: %s | Admission: %s"), *details.RoleText, *details.AdmissionText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Evidence: %s"), *details.EvidenceText));
		if (!details.PerceptionLifetimeText.IsEmpty())
		{
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Perception Lifetime: %s"), *details.PerceptionLifetimeText));
		}
		if (!details.HitReactiveLifetimeText.IsEmpty())
		{
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("HitReactive Lifetime: %s"), *details.HitReactiveLifetimeText));
		}
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Target: %s | Assignment Revision: %s"), *details.TargetText, *details.AssignmentRevisionText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Protection: %s"), *details.RetentionText));
	}

	// [Recent Execution]
	// ===== Recent Execution Lines =====

	void AppendRecentExecutionBlockLines(TArray<FString>& InOutLines, const FDebugOverlayRecentExecutionViewData& InRecentExecutionViewData)
	{
		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, InRecentExecutionViewData.HeaderText);

		switch (InRecentExecutionViewData.State)
		{
		case EDebugOverlayRecentExecutionViewState::NotCaptured:
			AppendFormattedOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		case EDebugOverlayRecentExecutionViewState::NoActor:
			AppendFormattedOverlayLine(InOutLines, TEXT("N/A"));
			return;
		case EDebugOverlayRecentExecutionViewState::NoEvents:
			AppendFormattedOverlayLine(InOutLines, TEXT("NoEvents(Filter: Execution)"));
			return;
		case EDebugOverlayRecentExecutionViewState::Captured:
			AppendSummaryLines(InOutLines, InRecentExecutionViewData.SummaryText, EDebugOverlayCaptureState::Captured);
			return;
		default:
			AppendFormattedOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}
	}

	// [Death Lifecycle]
	// ===== Death Lifecycle Lines =====

	void AppendDeathLifecycleBlock(TArray<FString>& InOutLines, const FDebugOverlayDeathLifecycleViewData& InViewData)
	{
		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Death Lifecycle]"));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Health State: %s"), *InViewData.HealthStateText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Lifecycle: %s"), *InViewData.LifecycleText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Death Entry: %s"), *InViewData.DeathEntryText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Presentation: %s"), *InViewData.PresentationText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Fallback Timer: %s"), *InViewData.FallbackTimerText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Finalization: %s"), *InViewData.FinalizationText));
	}

	// [Current AI / Recent AI Event]
	// ===== AI Lines =====

	void AppendCurrentAIBlock(TArray<FString>& InOutLines, const FDebugOverlayCurrentAIViewData& InCurrentAIViewData)
	{
		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Current AI]"));

		if (!InCurrentAIViewData.bHasEnemy)
		{
			AppendFormattedOverlayLine(InOutLines, TEXT("NoTarget"));
			return;
		}

		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Controller: %s"), *InCurrentAIViewData.ControllerText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Pawn: %s"), *InCurrentAIViewData.PawnText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Target: %s"), *InCurrentAIViewData.TargetText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("IntentState: %s"), *InCurrentAIViewData.IntentStateText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("ReturnHome: %s"), *InCurrentAIViewData.ReturnHomeText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("UsePatrol: %s"), *InCurrentAIViewData.UsePatrolText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("HasLOS: %s"), *InCurrentAIViewData.HasLOSText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("DistanceToTarget: %s"), *InCurrentAIViewData.DistanceToTargetText));
		AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("IsCombatAction: %s"), *InCurrentAIViewData.IsCombatActionText));
	}

	void AppendRecentAIEventBlock(TArray<FString>& InOutLines, const FDebugOverlayRecentAIEventViewData& InRecentAIEventViewData)
	{
		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, TEXT("[Recent AI Event]"));

		switch (InRecentAIEventViewData.State)
		{
		case EDebugOverlayRecentAIEventViewState::NoTarget:
			AppendFormattedOverlayLine(InOutLines, TEXT("NoTarget"));
			return;
		case EDebugOverlayRecentAIEventViewState::NotCaptured:
			AppendFormattedOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		case EDebugOverlayRecentAIEventViewState::Stale:
			if (!InRecentAIEventViewData.TaskText.IsEmpty())
			{
				AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Task: %s"), *InRecentAIEventViewData.TaskText));
			}
			if (!InRecentAIEventViewData.ResultText.IsEmpty())
			{
				AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Result: %s"), *InRecentAIEventViewData.ResultText));
			}
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Stale Time: %ss"), *InRecentAIEventViewData.StaleAgeText));
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("RejectReason: %s"), *FormatValueOrNoneText(InRecentAIEventViewData.RejectReasonText)));
			return;
		case EDebugOverlayRecentAIEventViewState::Captured:
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Task: %s"), *InRecentAIEventViewData.TaskText));
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Result: %s"), *InRecentAIEventViewData.ResultText));
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("Age: %s"), *InRecentAIEventViewData.AgeText));
			AppendFormattedOverlayLine(InOutLines, FString::Printf(TEXT("RejectReason: %s"), *FormatValueOrNoneText(InRecentAIEventViewData.RejectReasonText)));
			return;
		default:
			AppendFormattedOverlayLine(InOutLines, TEXT("NotCaptured"));
			return;
		}
	}

	// [Panel_01]
	// ===== Main Panel Lines =====

	void AppendActorPanelLines(TArray<FString>& InOutLines, const FDebugOverlayActorPanelViewData& InActorPanelViewData)
	{
		AppendFormattedOverlayLine(InOutLines, TEXT(""));
		AppendFormattedOverlayLine(InOutLines, InActorPanelViewData.HeaderText);

		if (InActorPanelViewData.bIncludeFocus)
		{
			AppendFocusLines(InOutLines, InActorPanelViewData.Focus);
		}

		if (InActorPanelViewData.bAppendBlankBeforeStatus)
		{
			AppendFormattedOverlayLine(InOutLines, TEXT(""));
		}

		if (InActorPanelViewData.bIncludeStatus)
		{
			AppendActorStatusLines(InOutLines, InActorPanelViewData.Status);
		}

		if (InActorPanelViewData.bIncludeLocomotion)
		{
			AppendPlayerLocomotionLines(InOutLines, InActorPanelViewData.Locomotion);
		}

		if (InActorPanelViewData.bIncludeTargeting)
		{
			AppendPlayerTargetingLines(InOutLines, InActorPanelViewData.Targeting);
		}

		if (InActorPanelViewData.bIncludeBalanceCollapse)
		{
			AppendBalanceCollapseLines(InOutLines, InActorPanelViewData.BalanceCollapse);
		}

		if (InActorPanelViewData.bIncludeExecutionCollaboration)
		{
			AppendExecutionCollaborationLines(InOutLines, InActorPanelViewData.ExecutionCollaboration);
		}

		if (InActorPanelViewData.bIncludeCombatParticipation)
		{
			AppendCombatParticipationLines(InOutLines, InActorPanelViewData.CombatParticipation);
		}

		if (InActorPanelViewData.bIncludeDeathLifecycle)
		{
			AppendDeathLifecycleBlock(InOutLines, InActorPanelViewData.DeathLifecycle);
		}

		if (InActorPanelViewData.bIncludeRecentExecution)
		{
			AppendRecentExecutionBlockLines(InOutLines, InActorPanelViewData.RecentExecution);
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

	TArray<FString> BuildMainTextPanelLines(const FDebugOverlayViewData& InViewData)
	{
		TArray<FString> lines;
		if (InViewData.MainPanelTitle.IsEmpty()) return lines;

		lines.Reserve(32);
		AppendFormattedOverlayLine(lines, InViewData.MainPanelTitle);

		for (const FDebugOverlayActorPanelViewData& actorPanel : InViewData.ActorPanels)
		{
			AppendActorPanelLines(lines, actorPanel);
		}

		return lines;
	}

	// [Panel_02]
	// ===== EventLog Panel Lines =====

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
		AppendFormattedOverlayLine(lines, InViewData.EventLogPanelTitle);
		AppendFormattedOverlayLine(lines, TEXT(""));
		AppendFormattedOverlayLine(lines, FString::Printf(TEXT("[Event Log: %s]"), *InViewData.EventLog.FilterText));

		if (!InViewData.EventLog.bHasSnapshot)
		{
			AppendFormattedOverlayLine(lines, TEXT("NotCaptured"));
			return lines;
		}

		if (InViewData.EventLog.DisplayLimit == 0)
		{
			AppendFormattedOverlayLine(lines, FString::Printf(TEXT("NoEvents(Filter: %s Limit: 0)"), *InViewData.EventLog.FilterText));
			return lines;
		}

		if (InViewData.EventLog.Entries.IsEmpty())
		{
			AppendFormattedOverlayLine(lines, FString::Printf(TEXT("NoEvents(Filter: %s)"), *InViewData.EventLog.FilterText));
			return lines;
		}

		for (const FDebugOverlayEventLogEntryViewData& eventEntry : InViewData.EventLog.Entries)
		{
			AppendFormattedOverlayLine(lines, FormatEventLogEntryLine(eventEntry));
		}

		return lines;
	}

	// [Panel_03]
	// ===== World Summary Panel Lines =====

	void AppendRecentSummaryBlockLines(TArray<FString>& InOutLines, const FDebugOverlayRecentSummaryBlockViewData& InBlockViewData)
	{
		if (InBlockViewData.bAppendLeadingBlank)
		{
			AppendFormattedOverlayLine(InOutLines, TEXT(""));
		}

		AppendFormattedOverlayLine(InOutLines, InBlockViewData.HeaderText);
		if (InBlockViewData.bHasSnapshot)
		{
			AppendSummaryLines(InOutLines, InBlockViewData.SummaryText, InBlockViewData.CaptureState);
		}
		else
		{
			AppendFormattedOverlayLine(InOutLines, TEXT("NotCaptured"));
		}
	}

	TArray<FString> BuildWorldSummaryPanelLines(const FDebugOverlayViewData& InViewData)
	{
		TArray<FString> lines;
		lines.Reserve(16);
		AppendFormattedOverlayLine(lines, InViewData.WorldSummaryPanelTitle);
		AppendFormattedOverlayLine(lines, TEXT(""));
		AppendFormattedOverlayLine(lines, InViewData.WorldSummary.HeaderText);

		for (const FDebugOverlayRecentSummaryBlockViewData& summaryBlock : InViewData.WorldSummary.SummaryBlocks)
		{
			AppendRecentSummaryBlockLines(lines, summaryBlock);
		}

		if (InViewData.WorldSummary.bIncludeCombatParticipation)
		{
			AppendFormattedOverlayLine(lines, TEXT(""));
			AppendFormattedOverlayLine(lines, TEXT("[Combat Participation]"));
			for (const FString& summaryLine : InViewData.WorldSummary.CombatParticipation.WorldSummaryLines)
			{
				AppendFormattedOverlayLine(lines, summaryLine);
			}
		}

		return lines;
	}

	// ===== Text Panel Role Mapping =====

	EDebugOverlayTextLineRole ResolveTextLineRole(EDebugOverlayTextPanelRole InPanelRole, const FString& InLine)
	{
		if (InLine.StartsWith(TEXT("[Debug Overlay Panel_")))
		{
			return EDebugOverlayTextLineRole::PanelTitle;
		}

		if (InPanelRole != EDebugOverlayTextPanelRole::EventLog
			&& (InLine == TEXT("[Player]") || InLine == TEXT("[Enemy]") || InLine == TEXT("[World Summary]")))
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

// ===== Public API =====

FDebugOverlayTextPanels FDebugOverlayTextFormatter::Format(const FDebugOverlayViewData& InViewData)
{
	const TArray<FString> mainPanelLines = BuildMainTextPanelLines(InViewData);
	const TArray<FString> eventLogPanelLines = BuildEventLogPanelLines(InViewData);
	const TArray<FString> worldSummaryPanelLines = BuildWorldSummaryPanelLines(InViewData);

	FDebugOverlayTextPanels panels;
	panels.MainPanel = MakeTextPanel(EDebugOverlayTextPanelRole::Main, mainPanelLines);
	panels.EventLogPanel = MakeTextPanel(EDebugOverlayTextPanelRole::EventLog, eventLogPanelLines);
	panels.WorldSummaryPanel = MakeTextPanel(EDebugOverlayTextPanelRole::WorldSummary, worldSummaryPanelLines);
	return panels;
}
