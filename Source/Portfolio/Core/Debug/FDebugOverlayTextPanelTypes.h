#pragma once

#include "CoreMinimal.h"

enum class EDebugOverlayTextLineRole : uint8
{
	Normal,
	PanelTitle,
	PanelHeader,
	EventLogHeader,
	Warning,
};

enum class EDebugOverlayTextPanelRole : uint8
{
	Main,
	EventLog,
	Interaction,
};

struct FDebugOverlayTextLine
{
	FString Text;
	FString FullText;
	EDebugOverlayTextLineRole Role = EDebugOverlayTextLineRole::Normal;

	FDebugOverlayTextLine() = default;

	FDebugOverlayTextLine(const FString& InText, EDebugOverlayTextLineRole InRole)
		: Text(InText)
		, FullText(InText)
		, Role(InRole)
	{
	}
};

struct FDebugOverlayTextPanel
{
	EDebugOverlayTextPanelRole Role = EDebugOverlayTextPanelRole::Main;
	TArray<FDebugOverlayTextLine> Lines;

	FDebugOverlayTextPanel() = default;

	explicit FDebugOverlayTextPanel(EDebugOverlayTextPanelRole InRole)
		: Role(InRole)
	{
	}
};

struct FDebugOverlayTextPanels
{
	FDebugOverlayTextPanel MainPanel;
	FDebugOverlayTextPanel EventLogPanel;
	FDebugOverlayTextPanel InteractionPanel;

	FDebugOverlayTextPanels()
		: MainPanel(EDebugOverlayTextPanelRole::Main)
		, EventLogPanel(EDebugOverlayTextPanelRole::EventLog)
		, InteractionPanel(EDebugOverlayTextPanelRole::Interaction)
	{
	}
};
