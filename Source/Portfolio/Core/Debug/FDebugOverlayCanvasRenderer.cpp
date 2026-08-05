#include "Core/Debug/FDebugOverlayCanvasRenderer.h"

#include "Core/Debug/CDebugOverlayHUD.h"
#include "Core/Debug/FDebugOverlayTextPanelTypes.h"

#include "Engine/Canvas.h"

namespace
{
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
	static constexpr float DebugOverlayInteractionPanelWidth = 520.f;
	static const FLinearColor DebugOverlayBackgroundColor(0.f, 0.f, 0.f, 0.72f);
	static const FLinearColor DebugOverlayPlayerHeaderColor(0.02f, 0.20f, 0.78f, 0.68f);
	static const FLinearColor DebugOverlayEnemyHeaderColor(0.78f, 0.06f, 0.04f, 0.68f);
	static const FLinearColor DebugOverlayDefaultHeaderColor(0.24f, 0.24f, 0.24f, 0.72f);

	struct FDebugOverlayRightPanelGeometry
	{
		float EventLogBackgroundX = 0.f;
		float EventLogBackgroundY = 0.f;
		float EventLogAvailableWidth = 0.f;
		float EventLogAvailableHeight = 0.f;
		float InteractionBackgroundX = 0.f;
		float InteractionBackgroundWidth = 0.f;
		bool bCanDrawInteractionPanel = false;
	};

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

	FDebugOverlayRightPanelGeometry CalculateRightPanelGeometry(
		const UCanvas* InCanvas,
		float InLeftPanelBackgroundX,
		float InLeftPanelBackgroundWidth,
		float InTopBackgroundY,
		bool bInHasInteractionLines)
	{
		FDebugOverlayRightPanelGeometry geometry;
		if (!InCanvas) return geometry;

		geometry.EventLogBackgroundX = InLeftPanelBackgroundX + InLeftPanelBackgroundWidth + DebugOverlayPanelGap;
		geometry.EventLogBackgroundY = InTopBackgroundY;

		const float rightPanelAvailableWidth = FMath::Max(0.f, InCanvas->SizeX - geometry.EventLogBackgroundX - DebugOverlayRightMargin);
		geometry.bCanDrawInteractionPanel = bInHasInteractionLines
			&& rightPanelAvailableWidth >= DebugOverlayMinEventLogPanelWidth + DebugOverlayPanelGap + DebugOverlayInteractionPanelWidth;
		geometry.InteractionBackgroundWidth = geometry.bCanDrawInteractionPanel ? DebugOverlayInteractionPanelWidth : 0.f;
		geometry.InteractionBackgroundX = geometry.bCanDrawInteractionPanel
			? InCanvas->SizeX - DebugOverlayRightMargin - geometry.InteractionBackgroundWidth
			: 0.f;
		geometry.EventLogAvailableWidth = geometry.bCanDrawInteractionPanel
			? FMath::Max(0.f, geometry.InteractionBackgroundX - DebugOverlayPanelGap - geometry.EventLogBackgroundX)
			: rightPanelAvailableWidth;
		geometry.EventLogAvailableHeight = FMath::Max(0.f, InCanvas->SizeY - geometry.EventLogBackgroundY - DebugOverlayBottomMargin);

		return geometry;
	}

	FLinearColor GetPanelHeaderColor(const FDebugOverlayTextLine& InLine)
	{
		if (InLine.Text == TEXT("[Player]")) return DebugOverlayPlayerHeaderColor;
		if (InLine.Text == TEXT("[Enemy]")) return DebugOverlayEnemyHeaderColor;
		return DebugOverlayDefaultHeaderColor;
	}

	void DrawOverlayPanel(
		ACDebugOverlayHUD& InHud,
		const FDebugOverlayTextPanel& InPanel,
		float InTextX,
		float InTextY,
		float InBackgroundX,
		float InBackgroundY,
		float InBackgroundWidth,
		float InBackgroundHeight)
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

			InHud.DrawText(line.Text, FLinearColor::White, InTextX, y, nullptr, DebugOverlayFontScale, false);
			y += DebugOverlayLineHeight;
			if (bDrawHeader)
			{
				y += DebugOverlayHeaderBottomPadding;
			}
		}
	}
}

void FDebugOverlayCanvasRenderer::Draw(ACDebugOverlayHUD& InHud, UCanvas* InCanvas, const FDebugOverlayTextPanels& InTextPanels)
{
	const float backgroundX = FMath::Max(0.f, DebugOverlayOriginX - DebugOverlayBackgroundPadding);
	const float backgroundY = FMath::Max(0.f, DebugOverlayOriginY - DebugOverlayBackgroundPadding);
	const float availableWidth = InCanvas
		? FMath::Max(0.f, InCanvas->SizeX - backgroundX - DebugOverlayBackgroundPadding)
		: DebugOverlayBackgroundWidth;
	const float backgroundWidth = FMath::Min(DebugOverlayBackgroundWidth, availableWidth);
	const float backgroundHeight = CalculateOverlayLinesHeight(InTextPanels.MainPanel) + (DebugOverlayBackgroundPadding * 2.f);

	DrawOverlayPanel(
		InHud,
		InTextPanels.MainPanel,
		DebugOverlayOriginX,
		DebugOverlayOriginY,
		backgroundX,
		backgroundY,
		backgroundWidth,
		backgroundHeight);

	if (InCanvas && !InTextPanels.EventLogPanel.Lines.IsEmpty())
	{
		const FDebugOverlayRightPanelGeometry rightPanelGeometry = CalculateRightPanelGeometry(
			InCanvas,
			backgroundX,
			backgroundWidth,
			backgroundY,
			!InTextPanels.InteractionPanel.Lines.IsEmpty());
		const float maxEventLogTextHeight = FMath::Max(0.f, rightPanelGeometry.EventLogAvailableHeight - (DebugOverlayBackgroundPadding * 2.f));
		const FDebugOverlayTextPanel visibleEventLogPanel = MakeVisibleOverlayPanel(InTextPanels.EventLogPanel, maxEventLogTextHeight);

		const float eventLogBackgroundHeight = FMath::Min(
			CalculateOverlayLinesHeight(visibleEventLogPanel) + (DebugOverlayBackgroundPadding * 2.f),
			rightPanelGeometry.EventLogAvailableHeight);

		if (rightPanelGeometry.EventLogAvailableWidth >= DebugOverlayMinEventLogPanelWidth && eventLogBackgroundHeight > 0.f && !visibleEventLogPanel.Lines.IsEmpty())
		{
			DrawOverlayPanel(
				InHud,
				visibleEventLogPanel,
				rightPanelGeometry.EventLogBackgroundX + DebugOverlayBackgroundPadding,
				rightPanelGeometry.EventLogBackgroundY + DebugOverlayBackgroundPadding,
				rightPanelGeometry.EventLogBackgroundX,
				rightPanelGeometry.EventLogBackgroundY,
				rightPanelGeometry.EventLogAvailableWidth,
				eventLogBackgroundHeight);
		}

		if (rightPanelGeometry.bCanDrawInteractionPanel)
		{
			const float interactionAvailableHeight = FMath::Max(0.f, InCanvas->SizeY - rightPanelGeometry.EventLogBackgroundY - DebugOverlayBottomMargin);
			const float maxInteractionTextHeight = FMath::Max(0.f, interactionAvailableHeight - (DebugOverlayBackgroundPadding * 2.f));
			const FDebugOverlayTextPanel visibleInteractionPanel = MakeVisibleOverlayPanel(InTextPanels.InteractionPanel, maxInteractionTextHeight);

			const float interactionBackgroundHeight = FMath::Min(
				CalculateOverlayLinesHeight(visibleInteractionPanel) + (DebugOverlayBackgroundPadding * 2.f),
				interactionAvailableHeight);

			if (interactionBackgroundHeight > 0.f && !visibleInteractionPanel.Lines.IsEmpty())
			{
				DrawOverlayPanel(
					InHud,
					visibleInteractionPanel,
					rightPanelGeometry.InteractionBackgroundX + DebugOverlayBackgroundPadding,
					rightPanelGeometry.EventLogBackgroundY + DebugOverlayBackgroundPadding,
					rightPanelGeometry.InteractionBackgroundX,
					rightPanelGeometry.EventLogBackgroundY,
					rightPanelGeometry.InteractionBackgroundWidth,
					interactionBackgroundHeight);
			}
		}
	}
}
