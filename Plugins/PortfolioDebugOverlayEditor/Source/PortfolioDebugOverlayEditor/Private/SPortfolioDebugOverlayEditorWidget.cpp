#include "SPortfolioDebugOverlayEditorWidget.h"

#include "FPortfolioDebugOverlayEditorCVarAccess.h"
#include "FPortfolioDebugOverlayEditorFocusCommandBridge.h"

#include "Math/UnrealMathUtility.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateColor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SPortfolioDebugOverlayEditorWidget"

namespace CVarAccess = PortfolioDebugOverlayEditorCVarAccess;
namespace FocusCommandBridge = PortfolioDebugOverlayEditorFocusCommandBridge;

// ===== Construct =====

void SPortfolioDebugOverlayEditorWidget::Construct(const FArguments& InArgs)
{
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("All")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("Execution")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("Combat")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("AI")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("Balance")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("Death")));
	RefreshEventLogFilterSelection();
	LastFocusCommandStatus = LOCTEXT("FocusCommandNotRun", "Last Command: None");

	ChildSlot
	[
		SNew(SBorder)
		.Padding(10.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PanelDescription", "Session-only controls for Portfolio Debug Overlay CVars. Values are not saved to config."))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					MakeTopLevelSection(LOCTEXT("OverlayOptionsTitle", "Overlay Options"), MakeOverlayOptionsSection())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("MainPanelSectionsTitle", "Main Panel Sections"))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text_Lambda([]()
						{
							return CVarAccess::HasMainPanelSectionCVars()
								? LOCTEXT("MainPanelSectionsDescription", "Select the actor sections and detail blocks shown in Panel_01.")
								: LOCTEXT("MainPanelSectionsUnavailable", "Main panel section CVars are unavailable. Start the game module or PIE if needed.");
						})
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						.Padding(8.f)
						[
							MakeMainPanelSections()
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeTopLevelSection(LOCTEXT("WorldSummarySectionsTitle", "World Summary Sections"), MakeWorldSummarySections())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeTargetingDisplayOptionsSection()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeMovementDisplayOptionsSection()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeBalanceDisplayOptionsSection()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeExecutionCollaborationDisplayOptionsSection()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeCombatParticipationDisplayOptionsSection()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeFocusOptionsSection()
				]
			]
		]
	];
}

// ===== Section Layout =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTopLevelSection(const FText& InTitle, const TSharedRef<SWidget>& InContent) const
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(InTitle)
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				InContent
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeSectionCard(const FText& InTitle, const TSharedRef<SWidget>& InContent) const
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(InTitle)
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				InContent
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeOverlayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("EnabledLabel", "Enabled"), LOCTEXT("EnabledHelp", "Draw the debug overlay HUD."), CVarAccess::GetEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("CollectLabel", "Collect"), LOCTEXT("CollectHelp", "Collect future debug overlay snapshots and events."), CVarAccess::GetCollectCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeEventLogFilterRow()]
		+ SVerticalBox::Slot().AutoHeight()[MakeEventLogLimitRow()]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("HideNoiseEventsLabel", "Hide Noise Events"), LOCTEXT("HideNoiseEventsHelp", "Hide reject/ignore noise from the EventLog display."), CVarAccess::GetHideNoiseEventsCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("HideCollisionWindowEventsLabel", "Hide Collision Window Events"), LOCTEXT("HideCollisionWindowEventsHelp", "Hide collision window lifecycle events from the EventLog display."), CVarAccess::GetHideCollisionWindowEventsCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DiagnosticLoggingTitle", "Diagnostic Logging"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("DeathContractAuditLogLabel", "Death Contract Audit Log"), LOCTEXT("DeathContractAuditLogHelp", "Also write Death lifecycle contract violations to the Output Log."), CVarAccess::GetDeathLifecycleAuditCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CVarStatusTitle", "CVar Status"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()[MakeRefreshRow()];
}

// ===== Main Panel Sections =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeMainPanelSections()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBoolCVarRow(LOCTEXT("PlayerPanelLabel", "Player"), LOCTEXT("PlayerPanelHelp", "Show or hide all Player detail blocks. Child selections are retained."), CVarAccess::GetPlayerPanelEnabledCVarName())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("PlayerStatusLabel", "Status"), LOCTEXT("PlayerStatusHelp", "Show Player state, action, reaction, health and movement."), CVarAccess::GetPlayerStatusEnabledCVarName(), CVarAccess::GetPlayerPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("PlayerLocomotionLabel", "Locomotion"), LOCTEXT("PlayerLocomotionHelp", "Show Player locomotion input details."), CVarAccess::GetPlayerLocomotionEnabledCVarName(), CVarAccess::GetPlayerPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("PlayerTargetingLabel", "Targeting"), LOCTEXT("PlayerTargetingHelp", "Show Player targeting score details."), CVarAccess::GetPlayerTargetingEnabledCVarName(), CVarAccess::GetPlayerPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("PlayerExecutionCollaborationLabel", "Execution Collaboration"), LOCTEXT("PlayerExecutionCollaborationHelp", "Show Player live Execution Collaboration state and start geometry."), CVarAccess::GetPlayerExecutionCollaborationEnabledCVarName(), CVarAccess::GetPlayerPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("PlayerRecentExecutionLabel", "Recent Execution"), LOCTEXT("PlayerRecentExecutionHelp", "Show the most recent Player execution event."), CVarAccess::GetPlayerRecentExecutionEnabledCVarName(), CVarAccess::GetPlayerPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBoolCVarRow(LOCTEXT("EnemyPanelLabel", "Enemy"), LOCTEXT("EnemyPanelHelp", "Show or hide all Enemy detail blocks. Child selections are retained."), CVarAccess::GetEnemyPanelEnabledCVarName())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyFocusLabel", "Focus"), LOCTEXT("EnemyFocusHelp", "Show selected Enemy focus details."), CVarAccess::GetEnemyFocusEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyStatusLabel", "Status"), LOCTEXT("EnemyStatusHelp", "Show Enemy state, action, reaction, health and movement."), CVarAccess::GetEnemyStatusEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyBalanceCollapseLabel", "Balance / Collapse"), LOCTEXT("EnemyBalanceCollapseHelp", "Show focused Enemy Balance count, Collapse lifecycle and derived gates."), CVarAccess::GetEnemyBalanceCollapseEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyExecutionCollaborationLabel", "Execution Collaboration"), LOCTEXT("EnemyExecutionCollaborationHelp", "Show focused Enemy live Execution Collaboration state."), CVarAccess::GetEnemyExecutionCollaborationEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyCombatParticipationLabel", "Combat Participation"), LOCTEXT("EnemyCombatParticipationHelp", "Show focused Enemy combat participation details."), CVarAccess::GetEnemyCombatParticipationEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyDeathLifecycleLabel", "Death Lifecycle"), LOCTEXT("EnemyDeathLifecycleHelp", "Show Enemy death lifecycle details."), CVarAccess::GetEnemyDeathLifecycleEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyRecentExecutionLabel", "Recent Execution"), LOCTEXT("EnemyRecentExecutionHelp", "Show the most recent Enemy execution event."), CVarAccess::GetEnemyRecentExecutionEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyCurrentAILabel", "Current AI"), LOCTEXT("EnemyCurrentAIHelp", "Show the focused Enemy AI state."), CVarAccess::GetEnemyCurrentAIEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(16.f, 0.f, 0.f, 0.f)[MakeMainPanelChildRow(LOCTEXT("EnemyRecentAIEventLabel", "Recent AI Event"), LOCTEXT("EnemyRecentAIEventHelp", "Show the most recent focused Enemy AI event."), CVarAccess::GetEnemyRecentAIEventEnabledCVarName(), CVarAccess::GetEnemyPanelEnabledCVarName())];
}

// ===== World Summary Sections =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeWorldSummarySections()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBoolCVarRow(LOCTEXT("WorldSummaryCombatParticipationLabel", "Combat Participation"), LOCTEXT("WorldSummaryCombatParticipationHelp", "Show target slot and participation summaries in Panel_03."), CVarAccess::GetWorldSummaryCombatParticipationEnabledCVarName())
		];
}

// ===== Targeting Debug =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTargetingDisplayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TargetingDisplayOptionsTitle", "Targeting Display Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasTargetingDisplayCVars()
					? LOCTEXT("TargetingCVarsAvailable", "Targeting Display CVars are available.")
					: LOCTEXT("TargetingCVarsUnavailable", "Targeting Display CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				MakeTargetingDebugSection()
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTargetingDebugSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingEnabledLabel", "Enabled"), LOCTEXT("TargetingEnabledHelp", "Enable targeting debug data and world visualization. Panel_01 targeting display requires this domain gate."), CVarAccess::GetTargetingEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingRangeLabel", "Range Sphere"), LOCTEXT("TargetingRangeHelp", "Draw the maximum targeting range around the viewpoint."), CVarAccess::GetTargetingDrawRangeSphereCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingSphereLabel", "Selected Target Sphere"), LOCTEXT("TargetingSphereHelp", "Draw a sphere around the current player target."), CVarAccess::GetTargetingDrawSelectedTargetSphereCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingLineLabel", "View Line"), LOCTEXT("TargetingLineHelp", "Draw a line from the viewpoint to the current player target."), CVarAccess::GetTargetingDrawViewLineCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingTextLabel", "World Debug Text"), LOCTEXT("TargetingTextHelp", "Draw distance, Dot and score at the current player target."), CVarAccess::GetTargetingDrawDebugTextCVarName())];
}

// ===== Movement Debug =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeMovementDisplayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MovementDisplayOptionsTitle", "Movement Display Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasMovementDisplayCVars()
					? LOCTEXT("MovementCVarsAvailable", "Movement Display CVars are available.")
					: LOCTEXT("MovementCVarsUnavailable", "Movement Display CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				MakeMovementDebugSection()
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeMovementDebugSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("MovementEnabledLabel", "Enabled"), LOCTEXT("MovementEnabledHelp", "Enable movement debug data and world visualization. Panel_01 locomotion display requires this domain gate."), CVarAccess::GetMovementEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("MovementVelocityLabel", "Velocity Arrow"), LOCTEXT("MovementVelocityHelp", "Draw the current world velocity direction and magnitude."), CVarAccess::GetMovementDrawVelocityCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("MovementInputLabel", "Last Input Arrow"), LOCTEXT("MovementInputHelp", "Draw the last movement input direction."), CVarAccess::GetMovementDrawInputCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("MovementFacingLabel", "Facing Arrow"), LOCTEXT("MovementFacingHelp", "Draw the current actor facing direction."), CVarAccess::GetMovementDrawFacingCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("MovementTextLabel", "World Debug Text"), LOCTEXT("MovementTextHelp", "Draw movement speed and direction near the player."), CVarAccess::GetMovementDrawDebugTextCVarName())];
}

// ===== Balance Debug =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeBalanceDisplayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BalanceDisplayOptionsTitle", "Balance / Collapse Display Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasBalanceDisplayCVars()
					? LOCTEXT("BalanceCVarsAvailable", "Balance and Collapse display CVars are available.")
					: LOCTEXT("BalanceCVarsUnavailable", "Balance and Collapse display CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				MakeBalanceDebugSection()
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeBalanceDebugSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("BalanceEnabledLabel", "Enabled"), LOCTEXT("BalanceEnabledHelp", "Enable Balance and Collapse debug data. Panel_01 Balance / Collapse requires this domain gate."), CVarAccess::GetBalanceEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("BalanceWorldTextLabel", "World Text"), LOCTEXT("BalanceWorldTextHelp", "Draw focused Enemy Balance count, lifecycle and derived gates in the world."), CVarAccess::GetBalanceDrawWorldTextCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("BalanceLifecycleBarLabel", "Count Segments"), LOCTEXT("BalanceLifecycleBarHelp", "Show threshold segments in the focused Enemy Balance world text."), CVarAccess::GetBalanceDrawLifecycleBarCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("BalanceAuditLabel", "Lifecycle Audit Log"), LOCTEXT("BalanceAuditHelp", "Also write Balance and Collapse lifecycle events to the Output Log."), CVarAccess::GetBalanceAuditCVarName())];
}

// ===== Execution Collaboration Debug =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeExecutionCollaborationDisplayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ExecutionCollaborationDisplayOptionsTitle", "Execution Collaboration Display Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasExecutionCollaborationDisplayCVars()
					? LOCTEXT("ExecutionCollaborationCVarsAvailable", "Execution Collaboration display CVars are available.")
					: LOCTEXT("ExecutionCollaborationCVarsUnavailable", "Execution Collaboration display CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				MakeExecutionCollaborationDebugSection()
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeExecutionCollaborationDebugSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("ExecutionCollaborationEnabledLabel", "Enabled"), LOCTEXT("ExecutionCollaborationEnabledHelp", "Enable Execution Collaboration debug data and visualization. Panel_01 live sections require this domain gate."), CVarAccess::GetExecutionCollaborationEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("ExecutionCollaborationStartGeometryLabel", "Start Geometry"), LOCTEXT("ExecutionCollaborationStartGeometryHelp", "Draw the Source start distance radius, facing angle boundaries and current Target link."), CVarAccess::GetExecutionCollaborationDrawStartGeometryCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("ExecutionCollaborationPairLinkLabel", "Pair Link"), LOCTEXT("ExecutionCollaborationPairLinkHelp", "Draw the active Source-to-Target collaboration link with lifecycle colors."), CVarAccess::GetExecutionCollaborationDrawPairLinkCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("ExecutionCollaborationWorldTextLabel", "World Text"), LOCTEXT("ExecutionCollaborationWorldTextHelp", "Draw the active Execution Collaboration session summary at the pair midpoint."), CVarAccess::GetExecutionCollaborationDrawWorldTextCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("ExecutionCollaborationAuditLabel", "Start / Lifecycle Audit Log"), LOCTEXT("ExecutionCollaborationAuditHelp", "Write Execution Collaboration input, start validation and lifecycle diagnostics to the Output Log."), CVarAccess::GetExecutionCollaborationAuditCVarName())];
}

// ===== Combat Participation Debug =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeCombatParticipationDisplayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CombatParticipationDisplayOptionsTitle", "Combat Participation Display Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasCombatParticipationDisplayCVars()
					? LOCTEXT("CombatParticipationCVarsAvailable", "Combat Participation Display CVars are available.")
					: LOCTEXT("CombatParticipationCVarsUnavailable", "Combat Participation Display CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				MakeCombatParticipationDebugSection()
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeCombatParticipationDebugSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("CombatParticipationEnabledLabel", "Enabled"), LOCTEXT("CombatParticipationEnabledHelp", "Enable Combat Participation debug data and world visualization. Panel_01 and Panel_03 require this domain gate."), CVarAccess::GetCombatParticipationEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("CombatParticipationWorldTextLabel", "World Text"), LOCTEXT("CombatParticipationWorldTextHelp", "Draw role, evidence, target and protection state above participating Enemy actors."), CVarAccess::GetCombatParticipationDrawWorldTextCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("CombatParticipationWorldRingLabel", "World Ring"), LOCTEXT("CombatParticipationWorldRingHelp", "Draw role and evidence color rings below participating Enemy actors."), CVarAccess::GetCombatParticipationDrawWorldRingCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("CombatParticipationHitReactiveEvidenceAnchorLabel", "HitReactive Evidence Anchor"), LOCTEXT("CombatParticipationHitReactiveEvidenceAnchorHelp", "Draw each live HitReactive Evidence anchor point, Target connection and active 2D radius."), CVarAccess::GetCombatParticipationDrawHitReactiveEvidenceAnchorCVarName())];
}

// ===== CVar Rows =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeBoolCVarRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName, TFunction<bool()> InAdditionalEnabledPredicate) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(InLabel)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(InHelp)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SNew(SCheckBox)
			.IsEnabled_Lambda([InCVarName, InAdditionalEnabledPredicate]()
			{
				return CVarAccess::FindCVar(InCVarName) != nullptr
					&& (!InAdditionalEnabledPredicate || InAdditionalEnabledPredicate());
			})
			.IsChecked_Lambda([InCVarName]()
			{
				return CVarAccess::GetBool(InCVarName) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([InCVarName](ECheckBoxState InNewState)
			{
				CVarAccess::SetBool(InCVarName, InNewState == ECheckBoxState::Checked);
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([InCVarName]()
			{
				return CVarAccess::GetAvailabilityText(InCVarName);
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeMainPanelChildRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName, const TCHAR* InParentCVarName) const
{
	return MakeBoolCVarRow(InLabel, InHelp, InCVarName, [InParentCVarName]()
	{
		return CVarAccess::GetBool(InParentCVarName);
	});
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeEventLogFilterRow()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogFilterLabel", "EventLog Filter"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogFilterHelp", "Controls the displayed EventLog category."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SAssignNew(EventLogFilterComboBox, SComboBox<TSharedPtr<FString>>)
			.IsEnabled_Lambda([]()
			{
				return CVarAccess::FindCVar(CVarAccess::GetEventLogFilterCVarName()) != nullptr;
			})
			.OptionsSource(&EventLogFilterOptions)
			.InitiallySelectedItem(SelectedEventLogFilter)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
			{
				return SNew(STextBlock)
					.Text(FText::FromString(InItem.IsValid() ? *InItem : FString()));
			})
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> InSelection, ESelectInfo::Type)
			{
				if (InSelection.IsValid())
				{
					SelectedEventLogFilter = InSelection;
					CVarAccess::SetString(CVarAccess::GetEventLogFilterCVarName(), *InSelection);
				}
			})
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					const FString currentValue = CVarAccess::GetString(CVarAccess::GetEventLogFilterCVarName());
					if (!CVarAccess::FindCVar(CVarAccess::GetEventLogFilterCVarName()))
					{
						return LOCTEXT("EventLogFilterUnavailable", "Unavailable");
					}

					return CVarAccess::IsKnownEventLogFilter(currentValue)
						? FText::FromString(currentValue)
						: FText::FromString(FString::Printf(TEXT("Unknown (%s)"), *currentValue));
				})
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeEventLogLimitRow() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogLimitLabel", "EventLog Limit"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogLimitHelp", "Maximum EventLog lines to display. Range: 0-32."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SNew(SSpinBox<int32>)
			.IsEnabled_Lambda([]()
			{
				return CVarAccess::FindCVar(CVarAccess::GetEventLogLimitCVarName()) != nullptr;
			})
			.MinValue(0)
			.MaxValue(32)
			.MinSliderValue(0)
			.MaxSliderValue(32)
			.Value_Lambda([]()
			{
				return FMath::Clamp(CVarAccess::GetInt(CVarAccess::GetEventLogLimitCVarName()), 0, 32);
			})
			.OnValueChanged_Lambda([](int32 InValue)
			{
				CVarAccess::SetInt(CVarAccess::GetEventLogLimitCVarName(), FMath::Clamp(InValue, 0, 32));
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::GetAvailabilityText(CVarAccess::GetEventLogLimitCVarName());
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeNearestFocusRadiusRow() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NearestFocusRadiusLabel", "Focus Search Radius"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NearestFocusRadiusHelp", "Search radius used by Nearest and Recent Combat focus selection."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SNew(SSpinBox<float>)
			.IsEnabled_Lambda([]()
			{
				return CVarAccess::FindCVar(CVarAccess::GetNearestFocusRadiusCVarName()) != nullptr;
			})
			.MinValue(0.f)
			.MaxValue(20000.f)
			.MinSliderValue(0.f)
			.MaxSliderValue(10000.f)
			.Delta(50.f)
			.Value_Lambda([]()
			{
				return FMath::Clamp(CVarAccess::GetFloat(CVarAccess::GetNearestFocusRadiusCVarName()), 0.f, 20000.f);
			})
			.OnValueChanged_Lambda([](float InValue)
			{
				CVarAccess::SetFloat(CVarAccess::GetNearestFocusRadiusCVarName(), FMath::Clamp(InValue, 0.f, 20000.f));
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::GetAvailabilityText(CVarAccess::GetNearestFocusRadiusCVarName());
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

// ===== Status / Refresh =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeRefreshRow()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasOverlayCVars()
					? LOCTEXT("AllCVarsAvailable", "Debug Overlay CVars are available.")
					: LOCTEXT("SomeCVarsUnavailable", "Some Debug Overlay CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshButton", "Refresh"))
			.OnClicked_Lambda([this]()
			{
				RefreshEventLogFilterSelection();
				return FReply::Handled();
			})
		];
}

void SPortfolioDebugOverlayEditorWidget::RefreshEventLogFilterSelection()
{
	const FString currentValue = CVarAccess::GetString(CVarAccess::GetEventLogFilterCVarName());
	SelectedEventLogFilter.Reset();

	for (const TSharedPtr<FString>& option : EventLogFilterOptions)
	{
		if (option.IsValid() && option->Equals(currentValue, ESearchCase::IgnoreCase))
		{
			SelectedEventLogFilter = option;
			break;
		}
	}

	if (EventLogFilterComboBox.IsValid())
	{
		EventLogFilterComboBox->SetSelectedItem(SelectedEventLogFilter);
	}
}

// ===== Focus Options =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeFocusOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FocusOptionsTitle", "Focus Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasFocusCVars()
					? LOCTEXT("FocusCVarsAvailable", "Focus CVars are available.")
					: LOCTEXT("FocusCVarsUnavailable", "Focus CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeFocusSearchSettingsCard()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeManualFocusSelectionCard()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeRuntimeFocusSourcesCard()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeClearFocusCard()
		]
		;
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeFocusSearchSettingsCard() const
{
	return MakeSectionCard(
		LOCTEXT("FocusSearchSettingsGroupTitle", "Search Settings"),
		MakeNearestFocusRadiusRow());
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeManualFocusSelectionCard()
{
	return MakeSectionCard(
		LOCTEXT("ManualFocusSelectionGroupTitle", "Manual Selection"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NearestFocusSourceLabel", "Nearest"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NearestFocusSourceHelp", "Select the nearest eligible actor around the player."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectNearestFocusButton", "Select Nearest Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectNearestFocusCommand();
				return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OutlinerFocusSourceLabel", "Outliner"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OutlinerFocusSourceHelp", "Use the actor currently selected in the editor Outliner."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectOutlinerActorButton", "Select Outliner Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectOutlinerFocusCommand();
				return FReply::Handled();
			})
		]);
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeRuntimeFocusSourcesCard()
{
	return MakeSectionCard(
		LOCTEXT("RuntimeFocusSourcesGroupTitle", "Runtime Sources"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PlayerTargetFocusSourceLabel", "Player Target"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBoolCVarRow(
				LOCTEXT("TargetingLiveSyncLabel", "Live Sync Player Target"),
				LOCTEXT("TargetingLiveSyncHelp", "Update PlayerTarget Focus continuously; disabling it freezes the last synced target."),
				CVarAccess::GetFocusLiveSyncPlayerTargetCVarName())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectPlayerTargetFocusButton", "Select Player Target Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectPlayerTargetFocusCommand();
				return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RecentCombatFocusSourceLabel", "Recent Combat"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RecentCombatFocusSourceHelp", "Select the latest eligible actor recorded by the runtime combat context."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectRecentCombatFocusButton", "Select Recent Combat Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectRecentCombatFocusCommand();
				return FReply::Handled();
			})
		]);
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeClearFocusCard()
{
	return MakeSectionCard(
		LOCTEXT("ClearFocusGroupTitle", "Clear"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SButton)
			.Text(LOCTEXT("ClearFocusButton", "Clear Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteClearFocusCommand();
				return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { return LastFocusCommandStatus; })
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]);
}

#undef LOCTEXT_NAMESPACE
