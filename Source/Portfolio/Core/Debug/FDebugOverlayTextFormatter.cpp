#include "Core/Debug/FDebugOverlayTextFormatter.h"

#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

namespace
{
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
	FDebugOverlayTextPanels panels;
	panels.MainPanel = MakeTextPanel(EDebugOverlayTextPanelRole::Main, InViewData.MainPanelLines);
	panels.EventLogPanel = MakeTextPanel(EDebugOverlayTextPanelRole::EventLog, InViewData.EventLogPanelLines);
	panels.InteractionPanel = MakeTextPanel(EDebugOverlayTextPanelRole::Interaction, InViewData.InteractionPanelLines);
	return panels;
}
