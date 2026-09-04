#pragma once

#include "CoreMinimal.h"

// ===== Text Roles =====

enum class EDebugOverlayTextLineRole : uint8
{
	Normal,
	PanelHeader,
	EventLogHeader,
	Warning,
};

enum class EDebugOverlayTextPanelRole : uint8
{
	Main,
	EventLog,
	WorldSummary,
};

// ===== Text Line =====

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

// ===== Text Panel =====

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

// ===== Text Panel Set =====

struct FDebugOverlayTextPanels
{
	FDebugOverlayTextPanel MainPanel;
	FDebugOverlayTextPanel EventLogPanel;
	FDebugOverlayTextPanel WorldSummaryPanel;

	FDebugOverlayTextPanels()
		: MainPanel(EDebugOverlayTextPanelRole::Main)
		, EventLogPanel(EDebugOverlayTextPanelRole::EventLog)
		, WorldSummaryPanel(EDebugOverlayTextPanelRole::WorldSummary)
	{
	}
};
