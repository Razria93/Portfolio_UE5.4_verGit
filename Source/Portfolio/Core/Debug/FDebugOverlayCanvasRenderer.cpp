#include "Core/Debug/FDebugOverlayCanvasRenderer.h"

#include "Core/Debug/CDebugOverlayHUD.h"
#include "Core/Debug/FDebugOverlayTextPanelTypes.h"

#include "Engine/Canvas.h"

namespace
{
	// ===== Constants =====

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
	static constexpr float DebugOverlayWorldSummaryPanelWidth = 520.f;
	static const FLinearColor DebugOverlayBackgroundColor(0.f, 0.f, 0.f, 0.72f);
	static const FLinearColor DebugOverlayPlayerHeaderColor(0.02f, 0.20f, 0.78f, 0.68f);
	static const FLinearColor DebugOverlayEnemyHeaderColor(0.78f, 0.06f, 0.04f, 0.68f);
	static const FLinearColor DebugOverlayDefaultHeaderColor(0.24f, 0.24f, 0.24f, 0.72f);

	// ===== Layout Structs =====

	struct FDebugOverlayPanelRect
	{
		float X = 0.f;
		float Y = 0.f;
		float Width = 0.f;
		float Height = 0.f;
	};

	struct FDebugOverlayRightPanelLayout
	{
		FDebugOverlayPanelRect EventLogRect;
		FDebugOverlayPanelRect WorldSummaryRect;
		bool bCanDrawWorldSummaryPanel = false;
	};

	// ===== Line Metrics =====

	bool IsOverlayHeaderLine(const FDebugOverlayTextLine& InLine)
	{
		return InLine.Role == EDebugOverlayTextLineRole::PanelHeader
			|| InLine.Role == EDebugOverlayTextLineRole::EventLogHeader;
	}

	float GetOverlayLineHeight(const FDebugOverlayTextLine& InLine)
	{
		return DebugOverlayLineHeight
			+ (IsOverlayHeaderLine(InLine) ? DebugOverlayHeaderBottomPadding : 0.f);
	}

	float CalculateOverlayLinesHeight(const FDebugOverlayTextPanel& InPanel)
	{
		float height = 0.f;
		for (const FDebugOverlayTextLine& line : InPanel.Lines)
		{
			height += GetOverlayLineHeight(line);
		}

		return height;
	}

	int32 CalculateVisibleOverlayLineCount(const FDebugOverlayTextPanel& InPanel, float InMaxTextHeight)
	{
		float usedHeight = 0.f;
		for (int32 lineIndex = 0; lineIndex < InPanel.Lines.Num(); ++lineIndex)
		{
			const FDebugOverlayTextLine& line = InPanel.Lines[lineIndex];
			const float lineHeight = GetOverlayLineHeight(line);

			if (usedHeight + lineHeight > InMaxTextHeight)
			{
				return lineIndex;
			}

			usedHeight += lineHeight;
		}

		return InPanel.Lines.Num();
	}

	FDebugOverlayTextPanel MakeVisibleOverlayPanel(const FDebugOverlayTextPanel& InPanel, float InMaxTextHeight)
	{
		const int32 visibleLineCount = CalculateVisibleOverlayLineCount(InPanel, InMaxTextHeight);

		FDebugOverlayTextPanel visiblePanel(InPanel.Role);
		visiblePanel.Lines.Reserve(visibleLineCount);
		for (int32 lineIndex = 0; lineIndex < visibleLineCount; ++lineIndex)
		{
			visiblePanel.Lines.Add(InPanel.Lines[lineIndex]);
		}

		return visiblePanel;
	}

	// ===== Panel Layout =====

	FDebugOverlayRightPanelLayout CalculateRightPanelLayout(const UCanvas* InCanvas, float InLeftPanelBackgroundX, float InLeftPanelBackgroundWidth, float InTopBackgroundY, bool bInHasLeftPanel, bool bInHasWorldSummaryLines)
	{
		FDebugOverlayRightPanelLayout layout;
		if (!InCanvas) return layout;

		layout.EventLogRect.X = InLeftPanelBackgroundX + (bInHasLeftPanel ? InLeftPanelBackgroundWidth + DebugOverlayPanelGap : 0.f);
		layout.EventLogRect.Y = InTopBackgroundY;
		layout.EventLogRect.Height = FMath::Max(0.f, InCanvas->SizeY - layout.EventLogRect.Y - DebugOverlayBottomMargin);

		const float rightPanelAvailableWidth = FMath::Max(0.f, InCanvas->SizeX - layout.EventLogRect.X - DebugOverlayRightMargin);

		layout.bCanDrawWorldSummaryPanel = bInHasWorldSummaryLines && rightPanelAvailableWidth >= DebugOverlayWorldSummaryPanelWidth;

		layout.WorldSummaryRect.Width = layout.bCanDrawWorldSummaryPanel ? DebugOverlayWorldSummaryPanelWidth : 0.f;
		layout.WorldSummaryRect.X = layout.bCanDrawWorldSummaryPanel ? InCanvas->SizeX - DebugOverlayRightMargin - layout.WorldSummaryRect.Width : 0.f;
		layout.WorldSummaryRect.Y = layout.EventLogRect.Y;
		layout.WorldSummaryRect.Height = layout.EventLogRect.Height;

		layout.EventLogRect.Width = layout.bCanDrawWorldSummaryPanel ? FMath::Max(0.f, layout.WorldSummaryRect.X - DebugOverlayPanelGap - layout.EventLogRect.X) : rightPanelAvailableWidth;

		return layout;
	}

	// ===== Panel Style =====

	FLinearColor GetPanelHeaderColor(const FDebugOverlayTextLine& InLine)
	{
		if (InLine.Text == TEXT("[Player]")) return DebugOverlayPlayerHeaderColor;
		if (InLine.Text == TEXT("[Enemy]")) return DebugOverlayEnemyHeaderColor;
		return DebugOverlayDefaultHeaderColor;
	}

	FString TruncateOverlayTextToWidth(ACDebugOverlayHUD& InHud, const FString& InText, float InMaxWidth)
	{
		if (InText.IsEmpty() || InMaxWidth <= 0.f)
		{
			return FString();
		}

		float textWidth = 0.f;
		float textHeight = 0.f;
		InHud.GetTextSize(InText, textWidth, textHeight, nullptr, DebugOverlayFontScale);
		if (textWidth <= InMaxWidth)
		{
			return InText;
		}

		const FString ellipsis = TEXT("...");
		float ellipsisWidth = 0.f;
		InHud.GetTextSize(ellipsis, ellipsisWidth, textHeight, nullptr, DebugOverlayFontScale);
		if (ellipsisWidth > InMaxWidth)
		{
			return FString();
		}

		int32 visibleCharacterCount = InText.Len();
		while (visibleCharacterCount > 0)
		{
			const FString truncatedText = InText.Left(visibleCharacterCount) + ellipsis;
			InHud.GetTextSize(truncatedText, textWidth, textHeight, nullptr, DebugOverlayFontScale);
			if (textWidth <= InMaxWidth)
			{
				return truncatedText;
			}

			--visibleCharacterCount;
		}

		return ellipsis;
	}

	// ===== Panel Drawing =====

	void DrawOverlayPanel(ACDebugOverlayHUD& InHud, const FDebugOverlayTextPanel& InPanel, float InTextX, float InTextY, float InBackgroundX, float InBackgroundY, float InBackgroundWidth, float InBackgroundHeight, bool bInTruncateTextToPanel = false)
	{
		if (InBackgroundWidth > 0.f && InBackgroundHeight > 0.f)
		{
			InHud.DrawRect(DebugOverlayBackgroundColor, InBackgroundX, InBackgroundY, InBackgroundWidth, InBackgroundHeight);
		}

		float y = InTextY;
		for (const FDebugOverlayTextLine& line : InPanel.Lines)
		{
			const bool bDrawHeader = IsOverlayHeaderLine(line);

			if (bDrawHeader && InBackgroundWidth > 0.f)
			{
				InHud.DrawRect(GetPanelHeaderColor(line), InBackgroundX, y - 2.f, InBackgroundWidth, DebugOverlayLineHeight + 4.f);
			}

			const float maxTextWidth = FMath::Max(0.f, InBackgroundWidth - (InTextX - InBackgroundX) - DebugOverlayBackgroundPadding);
			const FString textToDraw = bInTruncateTextToPanel
				? TruncateOverlayTextToWidth(InHud, line.Text, maxTextWidth)
				: line.Text;
			InHud.DrawText(textToDraw, FLinearColor::White, InTextX, y, nullptr, DebugOverlayFontScale, false);
			y += DebugOverlayLineHeight;
			if (bDrawHeader)
			{
				y += DebugOverlayHeaderBottomPadding;
			}
		}
	}

	void DrawMainPanel(ACDebugOverlayHUD& InHud, const FDebugOverlayTextPanel& InPanel, const FDebugOverlayPanelRect& InRect)
	{
		DrawOverlayPanel(
			InHud,
			InPanel,
			DebugOverlayOriginX,
			DebugOverlayOriginY,
			InRect.X,
			InRect.Y,
			InRect.Width,
			InRect.Height);
	}

	void DrawEventLogPanelIfVisible(ACDebugOverlayHUD& InHud, const FDebugOverlayTextPanel& InPanel, const FDebugOverlayPanelRect& InRect)
	{
		if (InPanel.Lines.IsEmpty() || InRect.Width < DebugOverlayMinEventLogPanelWidth) return;

		const float maxTextHeight = FMath::Max(0.f, InRect.Height - (DebugOverlayBackgroundPadding * 2.f));
		const FDebugOverlayTextPanel visiblePanel = MakeVisibleOverlayPanel(InPanel, maxTextHeight);
		const float backgroundHeight = FMath::Min(CalculateOverlayLinesHeight(visiblePanel) + (DebugOverlayBackgroundPadding * 2.f), InRect.Height);
		if (backgroundHeight <= 0.f || visiblePanel.Lines.IsEmpty()) return;

		DrawOverlayPanel(
			InHud,
			visiblePanel,
			InRect.X + DebugOverlayBackgroundPadding,
			InRect.Y + DebugOverlayBackgroundPadding,
			InRect.X,
			InRect.Y,
			InRect.Width,
			backgroundHeight,
			true);
	}

	void DrawWorldSummaryPanelIfVisible(ACDebugOverlayHUD& InHud, const FDebugOverlayTextPanel& InPanel, const FDebugOverlayPanelRect& InRect, bool bInCanDrawPanel)
	{
		if (!bInCanDrawPanel || InPanel.Lines.IsEmpty()) return;

		const float maxTextHeight = FMath::Max(0.f, InRect.Height - (DebugOverlayBackgroundPadding * 2.f));
		const FDebugOverlayTextPanel visiblePanel = MakeVisibleOverlayPanel(InPanel, maxTextHeight);
		const float backgroundHeight = FMath::Min(CalculateOverlayLinesHeight(visiblePanel) + (DebugOverlayBackgroundPadding * 2.f), InRect.Height);
		if (backgroundHeight <= 0.f || visiblePanel.Lines.IsEmpty()) return;

		DrawOverlayPanel(
			InHud,
			visiblePanel,
			InRect.X + DebugOverlayBackgroundPadding,
			InRect.Y + DebugOverlayBackgroundPadding,
			InRect.X,
			InRect.Y,
			InRect.Width,
			backgroundHeight);
	}
}

// ===== Public API =====

void FDebugOverlayCanvasRenderer::Draw(ACDebugOverlayHUD& InHud, UCanvas* InCanvas, const FDebugOverlayTextPanels& InTextPanels)
{
	const float backgroundX = FMath::Max(0.f, DebugOverlayOriginX - DebugOverlayBackgroundPadding);
	const float backgroundY = FMath::Max(0.f, DebugOverlayOriginY - DebugOverlayBackgroundPadding);
	const float availableWidth = InCanvas ? FMath::Max(0.f, InCanvas->SizeX - backgroundX - DebugOverlayBackgroundPadding) : DebugOverlayBackgroundWidth;
	const float backgroundWidth = FMath::Min(DebugOverlayBackgroundWidth, availableWidth);
	const bool bHasMainPanel = !InTextPanels.MainPanel.Lines.IsEmpty();
	const float backgroundHeight = bHasMainPanel ? CalculateOverlayLinesHeight(InTextPanels.MainPanel) + (DebugOverlayBackgroundPadding * 2.f) : 0.f;
	const FDebugOverlayPanelRect mainPanelRect = { backgroundX, backgroundY, bHasMainPanel ? backgroundWidth : 0.f, backgroundHeight };

	if (bHasMainPanel)
	{
		DrawMainPanel(InHud, InTextPanels.MainPanel, mainPanelRect);
	}

	if (!InCanvas) return;

	const FDebugOverlayRightPanelLayout rightPanelLayout = CalculateRightPanelLayout(InCanvas, mainPanelRect.X, mainPanelRect.Width, mainPanelRect.Y, bHasMainPanel, !InTextPanels.WorldSummaryPanel.Lines.IsEmpty());

	DrawEventLogPanelIfVisible(InHud, InTextPanels.EventLogPanel, rightPanelLayout.EventLogRect);
	DrawWorldSummaryPanelIfVisible(InHud, InTextPanels.WorldSummaryPanel, rightPanelLayout.WorldSummaryRect, rightPanelLayout.bCanDrawWorldSummaryPanel);
}
