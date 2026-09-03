#include "Core/Debug/FDebugOverlaySettingsRegistry.h"

#include <initializer_list>

namespace
{
	FDebugOverlaySettingDefinition MakeBool(const TCHAR* InCategory, const TCHAR* InCVarName, const TCHAR* InDisplayName, const TCHAR* InHelpText, const TCHAR* InDefaultValue, const TCHAR* InParentGate = nullptr)
	{
		FDebugOverlaySettingDefinition definition;
		definition.Category = FName(InCategory);
		definition.CVarName = InCVarName;
		definition.DisplayName = InDisplayName;
		definition.HelpText = InHelpText;
		definition.Type = EDebugOverlaySettingType::Bool;
		definition.DefaultValue = InDefaultValue;
		definition.ParentGateCVarName = InParentGate ? InParentGate : TEXT("");
		return definition;
	}

	FDebugOverlaySettingDefinition MakeInt(const TCHAR* InCategory, const TCHAR* InCVarName, const TCHAR* InDisplayName, const TCHAR* InHelpText, const TCHAR* InDefaultValue, const float InMinValue, const float InMaxValue)
	{
		FDebugOverlaySettingDefinition definition = MakeBool(InCategory, InCVarName, InDisplayName, InHelpText, InDefaultValue);
		definition.Type = EDebugOverlaySettingType::Int;
		definition.MinValue = InMinValue;
		definition.MaxValue = InMaxValue;
		return definition;
	}

	FDebugOverlaySettingDefinition MakeFloat(const TCHAR* InCategory, const TCHAR* InCVarName, const TCHAR* InDisplayName, const TCHAR* InHelpText, const TCHAR* InDefaultValue, const float InMinValue, const float InMaxValue)
	{
		FDebugOverlaySettingDefinition definition = MakeBool(InCategory, InCVarName, InDisplayName, InHelpText, InDefaultValue);
		definition.Type = EDebugOverlaySettingType::Float;
		definition.MinValue = InMinValue;
		definition.MaxValue = InMaxValue;
		return definition;
	}

	FDebugOverlaySettingDefinition MakeEnum(const TCHAR* InCategory, const TCHAR* InCVarName, const TCHAR* InDisplayName, const TCHAR* InHelpText, const TCHAR* InDefaultValue, std::initializer_list<FDebugOverlayEnumOption> InOptions)
	{
		FDebugOverlaySettingDefinition definition = MakeBool(InCategory, InCVarName, InDisplayName, InHelpText, InDefaultValue);
		definition.Type = EDebugOverlaySettingType::Enum;
		definition.EnumOptions = InOptions;
		return definition;
	}
}

const TArray<FDebugOverlaySettingsCategory>& FDebugOverlaySettingsRegistry::GetCategories()
{
	static const TArray<FDebugOverlaySettingsCategory> categories =
	{
		{ TEXT("Overlay"), TEXT("Overlay Options"), TEXT("HUD visibility, event capture, and Event Log display settings.") },
		{ TEXT("CharacterDetails"), TEXT("Character Details Sections"), TEXT("Select the Player and Enemy detail blocks shown in Character Details.") },
		{ TEXT("WorldSummary"), TEXT("World Summary Sections"), TEXT("Select the detail blocks shown in World Summary.") },
		{ TEXT("Targeting"), TEXT("Targeting Display Options"), TEXT("Targeting runtime diagnostics and world visualization.") },
		{ TEXT("Movement"), TEXT("Movement Display Options"), TEXT("Movement runtime diagnostics and world visualization.") },
		{ TEXT("Balance"), TEXT("Balance / Collapse Display Options"), TEXT("Balance and Collapse runtime diagnostics and world visualization.") },
		{ TEXT("Facing"), TEXT("Combat Target Facing Display Options"), TEXT("Facing runtime diagnostics, transition capture, and audit logging.") },
		{ TEXT("ExecutionSession"), TEXT("Execution Session Display Options"), TEXT("Execution Session diagnostics and world visualization.") },
		{ TEXT("CombatParticipation"), TEXT("Combat Participation Display Options"), TEXT("Combat participation diagnostics and world visualization.") },
		{ TEXT("Focus"), TEXT("Focus Settings"), TEXT("Settings used by Debug Overlay focus selection.") },
		{ TEXT("Diagnostics"), TEXT("Diagnostic Logging"), TEXT("Output Log diagnostics independent from HUD visibility and event capture.") },
	};

	return categories;
}

const TArray<FDebugOverlaySettingDefinition>& FDebugOverlaySettingsRegistry::GetSettings()
{
	static const TArray<FDebugOverlaySettingDefinition> settings = []
	{
		TArray<FDebugOverlaySettingDefinition> result;
		result.Reserve(47);

		result.Add(MakeBool(TEXT("Overlay"), TEXT("Portfolio.DebugOverlay.HUDVisible"), TEXT("HUD Visible"), TEXT("Show the Debug Overlay HUD and world diagnostics."), TEXT("0")));
		result.Add(MakeBool(TEXT("Overlay"), TEXT("Portfolio.DebugOverlay.CaptureEnabled"), TEXT("Capture Enabled"), TEXT("Capture future Event Log entries and Actor histories."), TEXT("0")));
		result.Add(MakeEnum(TEXT("Overlay"), TEXT("Portfolio.DebugOverlay.EventLogFilter"), TEXT("Event Log Filter"), TEXT("Controls the displayed Event Log category."), TEXT("All"), { { TEXT("All"), TEXT("All") }, { TEXT("ActionReaction"), TEXT("Action / Reaction") }, { TEXT("ExecutionSession"), TEXT("Execution Session") }, { TEXT("Combat"), TEXT("Combat") }, { TEXT("AI"), TEXT("AI") }, { TEXT("Balance"), TEXT("Balance") }, { TEXT("Death"), TEXT("Death") }, { TEXT("Facing"), TEXT("Facing") } }));
		result.Add(MakeEnum(TEXT("Overlay"), TEXT("Portfolio.DebugOverlay.EventLogScope"), TEXT("Event Log Scope"), TEXT("World shows all events. Focused Enemy shows only events related to the selected Enemy."), TEXT("World"), { { TEXT("World"), TEXT("World") }, { TEXT("FocusedEnemy"), TEXT("Focused Enemy") } }));
		result.Add(MakeInt(TEXT("Overlay"), TEXT("Portfolio.DebugOverlay.EventLogLimit"), TEXT("Event Log Limit"), TEXT("Maximum Event Log lines to display."), TEXT("16"), 0.f, 32.f));
		result.Add(MakeBool(TEXT("Overlay"), TEXT("Portfolio.DebugOverlay.HideNoiseEvents"), TEXT("Hide Noise Events"), TEXT("Hide reject and ignore noise from the Event Log."), TEXT("0")));
		result.Add(MakeBool(TEXT("Overlay"), TEXT("Portfolio.DebugOverlay.HideCollisionWindowEvents"), TEXT("Hide Collision Window Events"), TEXT("Hide collision window lifecycle events from the Event Log."), TEXT("0")));

		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Player.Enabled"), TEXT("Player"), TEXT("Show or hide all Player detail blocks. Child selections are retained."), TEXT("1")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Player.Status.Enabled"), TEXT("Player: Status"), TEXT("Show Player state, action, reaction, health, and movement."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Player.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Player.Locomotion.Enabled"), TEXT("Player: Locomotion"), TEXT("Show Player locomotion input details."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Player.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Player.Targeting.Enabled"), TEXT("Player: Targeting"), TEXT("Show Player targeting score details."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Player.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Player.ExecutionSession.Enabled"), TEXT("Player: Execution Session"), TEXT("Show Player live Execution Session state."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Player.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Player.RecentActionReaction.Enabled"), TEXT("Player: Recent Action / Reaction"), TEXT("Show the most recent Player Action / Reaction decision."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Player.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled"), TEXT("Enemy"), TEXT("Show or hide all Enemy detail blocks. Child selections are retained."), TEXT("1")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.Focus.Enabled"), TEXT("Enemy: Focus"), TEXT("Show selected Enemy focus details."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.Status.Enabled"), TEXT("Enemy: Status"), TEXT("Show Enemy state, action, reaction, health, and movement."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.BalanceCollapse.Enabled"), TEXT("Enemy: Balance / Collapse"), TEXT("Show focused Enemy Balance count, Collapse lifecycle, and derived gates."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.ExecutionSession.Enabled"), TEXT("Enemy: Execution Session"), TEXT("Show focused Enemy live Execution Session state."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.CombatParticipation.Enabled"), TEXT("Enemy: Combat Participation"), TEXT("Show focused Enemy combat participation details."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.DeathLifecycle.Enabled"), TEXT("Enemy: Death Lifecycle"), TEXT("Show Enemy death lifecycle details."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.RecentActionReaction.Enabled"), TEXT("Enemy: Recent Action / Reaction"), TEXT("Show the most recent Enemy Action / Reaction decision."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.CurrentAI.Enabled"), TEXT("Enemy: Current AI"), TEXT("Show the focused Enemy AI state."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.RecentAIEvent.Enabled"), TEXT("Enemy: Recent AI Event"), TEXT("Show the most recent focused Enemy AI event."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));
		result.Add(MakeBool(TEXT("CharacterDetails"), TEXT("Portfolio.DebugOverlay.Enemy.CombatTargetFacing.Enabled"), TEXT("Enemy Facing Visible"), TEXT("Show the already-created focused Enemy Combat Target Facing policy, Gameplay Focus, and rotation consistency in Character Details."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Enemy.Enabled")));

		result.Add(MakeBool(TEXT("WorldSummary"), TEXT("Portfolio.DebugOverlay.WorldSummary.CombatParticipation.Enabled"), TEXT("Combat Participation"), TEXT("Show target slot and participation summaries in World Summary."), TEXT("1")));

		result.Add(MakeBool(TEXT("Targeting"), TEXT("Portfolio.DebugOverlay.Targeting.Enabled"), TEXT("Targeting Diagnostics Enabled"), TEXT("Enable targeting debug data and world visualization."), TEXT("0")));
		result.Add(MakeBool(TEXT("Targeting"), TEXT("Portfolio.DebugOverlay.Targeting.DrawRangeSphere"), TEXT("Range Sphere"), TEXT("Draw the maximum targeting range around the viewpoint."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Targeting.Enabled")));
		result.Add(MakeBool(TEXT("Targeting"), TEXT("Portfolio.DebugOverlay.Targeting.DrawSelectedTargetSphere"), TEXT("Selected Target Sphere"), TEXT("Draw a sphere around the current player target."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Targeting.Enabled")));
		result.Add(MakeBool(TEXT("Targeting"), TEXT("Portfolio.DebugOverlay.Targeting.DrawViewLine"), TEXT("View Line"), TEXT("Draw a line from the viewpoint to the current player target."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Targeting.Enabled")));
		result.Add(MakeBool(TEXT("Targeting"), TEXT("Portfolio.DebugOverlay.Targeting.DrawDebugText"), TEXT("World Debug Text"), TEXT("Draw distance, Dot, and score at the current player target."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Targeting.Enabled")));

		result.Add(MakeBool(TEXT("Movement"), TEXT("Portfolio.DebugOverlay.Movement.Enabled"), TEXT("Movement Diagnostics Enabled"), TEXT("Enable movement debug data and world visualization."), TEXT("0")));
		result.Add(MakeBool(TEXT("Movement"), TEXT("Portfolio.DebugOverlay.Movement.DrawVelocity"), TEXT("Velocity Arrow"), TEXT("Draw the current world velocity direction and magnitude."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Movement.Enabled")));
		result.Add(MakeBool(TEXT("Movement"), TEXT("Portfolio.DebugOverlay.Movement.DrawInput"), TEXT("Last Input Arrow"), TEXT("Draw the last movement input direction."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Movement.Enabled")));
		result.Add(MakeBool(TEXT("Movement"), TEXT("Portfolio.DebugOverlay.Movement.DrawFacing"), TEXT("Facing Arrow"), TEXT("Draw the current actor facing direction."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Movement.Enabled")));
		result.Add(MakeBool(TEXT("Movement"), TEXT("Portfolio.DebugOverlay.Movement.DrawDebugText"), TEXT("World Debug Text"), TEXT("Draw movement speed and direction near the player."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Movement.Enabled")));

		result.Add(MakeBool(TEXT("Balance"), TEXT("Portfolio.DebugOverlay.Balance.Enabled"), TEXT("Balance / Collapse Diagnostics Enabled"), TEXT("Enable Balance and Collapse debug data and world visualization."), TEXT("0")));
		result.Add(MakeBool(TEXT("Balance"), TEXT("Portfolio.DebugOverlay.Balance.DrawWorldText"), TEXT("World Text"), TEXT("Draw focused Enemy Balance count, lifecycle, and derived gates in the world."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Balance.Enabled")));
		result.Add(MakeBool(TEXT("Balance"), TEXT("Portfolio.DebugOverlay.Balance.DrawLifecycleBar"), TEXT("Count Segments"), TEXT("Show threshold segments in the focused Enemy Balance world text."), TEXT("1"), TEXT("Portfolio.DebugOverlay.Balance.Enabled")));

		result.Add(MakeBool(TEXT("Facing"), TEXT("Portfolio.DebugOverlay.CombatTargetFacing.Enabled"), TEXT("Facing Diagnostics Enabled"), TEXT("Create Combat Target Facing runtime snapshots and transition events."), TEXT("1")));

		result.Add(MakeBool(TEXT("ExecutionSession"), TEXT("Portfolio.DebugOverlay.ExecutionSession.Enabled"), TEXT("Execution Session Diagnostics Enabled"), TEXT("Enable Execution Session debug data and visualization."), TEXT("0")));
		result.Add(MakeBool(TEXT("ExecutionSession"), TEXT("Portfolio.DebugOverlay.ExecutionSession.DrawStartGeometry"), TEXT("Start Geometry"), TEXT("Draw source start distance, facing angle boundaries, and target link."), TEXT("1"), TEXT("Portfolio.DebugOverlay.ExecutionSession.Enabled")));
		result.Add(MakeBool(TEXT("ExecutionSession"), TEXT("Portfolio.DebugOverlay.ExecutionSession.DrawPairLink"), TEXT("Pair Link"), TEXT("Draw the active source-to-target session link."), TEXT("1"), TEXT("Portfolio.DebugOverlay.ExecutionSession.Enabled")));
		result.Add(MakeBool(TEXT("ExecutionSession"), TEXT("Portfolio.DebugOverlay.ExecutionSession.DrawWorldText"), TEXT("World Text"), TEXT("Draw the active Execution Session summary."), TEXT("1"), TEXT("Portfolio.DebugOverlay.ExecutionSession.Enabled")));

		result.Add(MakeBool(TEXT("CombatParticipation"), TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled"), TEXT("Combat Participation Diagnostics Enabled"), TEXT("Enable Combat Participation debug data and world visualization."), TEXT("0")));
		result.Add(MakeBool(TEXT("CombatParticipation"), TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldText"), TEXT("World Text"), TEXT("Draw role, evidence, target, and protection state above participating Enemy actors."), TEXT("1"), TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled")));
		result.Add(MakeBool(TEXT("CombatParticipation"), TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldRing"), TEXT("World Ring"), TEXT("Draw role and evidence color rings below participating Enemy actors."), TEXT("1"), TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled")));
		result.Add(MakeBool(TEXT("CombatParticipation"), TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawHitReactiveEvidenceAnchor"), TEXT("HitReactive Evidence Anchor"), TEXT("Draw live HitReactive Evidence anchor points, target connections, and active radii."), TEXT("0"), TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled")));

		result.Add(MakeFloat(TEXT("Focus"), TEXT("Portfolio.DebugOverlay.NearestFocusRadius"), TEXT("Focus Search Radius"), TEXT("Search radius used by Nearest and Recent Combat focus selection."), TEXT("3000"), 0.f, 20000.f));
		result.Add(MakeBool(TEXT("Focus"), TEXT("Portfolio.DebugOverlay.Focus.LiveSyncPlayerTarget"), TEXT("Live Sync Player Target"), TEXT("Update Player Target Focus continuously; disabling it freezes the last synced target."), TEXT("1")));

		result.Add(MakeBool(TEXT("Diagnostics"), TEXT("Portfolio.Debug.DeathLifecycleAudit"), TEXT("Death Contract Audit Log"), TEXT("Write Death lifecycle contract violations to the Output Log."), TEXT("0")));
		result.Add(MakeBool(TEXT("Diagnostics"), TEXT("Portfolio.Debug.BalanceAudit"), TEXT("Balance Lifecycle Audit Log"), TEXT("Write Balance and Collapse lifecycle events to the Output Log."), TEXT("0")));
		result.Add(MakeBool(TEXT("Diagnostics"), TEXT("Portfolio.Debug.CombatTargetFacingAudit"), TEXT("Facing Audit Log"), TEXT("Write Facing policy decisions and external Gameplay Focus clears to the Output Log."), TEXT("0")));
		result.Add(MakeBool(TEXT("Diagnostics"), TEXT("Portfolio.Debug.ExecutionSessionAudit"), TEXT("Execution Session Audit Log"), TEXT("Write Execution Session input, start validation, and lifecycle diagnostics to the Output Log."), TEXT("0")));

		return result;
	}();

	return settings;
}
