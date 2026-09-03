#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FBalanceDebug.h"
#include "Core/Debug/FCombatParticipationDebug.h"
#include "Core/Debug/FEnemyCombatTargetFacingDebug.h"
#include "Core/Debug/FExecutionCollaborationDebug.h"
#include "Core/Debug/FMovementDebug.h"
#include "Core/Debug/FDebugOverlaySnapshotTypes.h"
#include "Core/Debug/FTargetingDebug.h"

class ACEnemy;
class APawn;
class UObject;
class UWorld;

enum class EDebugOverlayRecentAIEventViewState : uint8
{
	NoTarget,
	NotCaptured,
	Stale,
	Captured,
};

enum class EDebugOverlayRecentActionReactionViewState : uint8
{
	NotCaptured,
	NoActor,
	NoEvents,
	Captured,
};

struct FDebugOverlayRecentActionReactionViewData
{
	EDebugOverlayRecentActionReactionViewState State = EDebugOverlayRecentActionReactionViewState::NotCaptured;
	FString HeaderText;
	FString SummaryText;
};

struct FDebugOverlayActorStatusViewData
{
	FString StateText;
	FString ActionText;
	FString ReactionText;
	FString HealthText;
	FString BalanceText;
	FString GuardText;
	FString MovementGaitRotationText;
	FString LocomotionPresentationStateText;
	FString RuntimeLODText;
};

struct FDebugOverlayCurrentAIViewData
{
	bool bHasEnemy = false;
	FString ControllerText;
	FString PawnText;
	FString TargetText;
	FString IntentStateText;
	FString ReturnHomeText;
	FString UsePatrolText;
	FString HasLOSText;
	FString DistanceToTargetText;
	FString IsCombatActionText;
};

struct FDebugOverlayDeathLifecycleViewData
{
	FString HealthStateText;
	FString LifecycleText;
	FString DeathEntryText;
	FString PresentationText;
	FString FallbackTimerText;
	FString FinalizationText;
};

struct FDebugOverlayRecentAIEventViewData
{
	EDebugOverlayRecentAIEventViewState State = EDebugOverlayRecentAIEventViewState::NoTarget;
	FString TaskText;
	FString ResultText;
	FString AgeText;
	FString RejectReasonText;
	FString StaleAgeText;
};

struct FDebugOverlayFocusViewData
{
	FString CurrentSourceText;
	FString CurrentActorNameText;
	FString FocusDriverText;
	FString RecentFocusStateText;
};

struct FDebugOverlayPlayerTargetingViewData
{
	FTargetingDebugOverlayDetails Details;
};

struct FDebugOverlayPlayerLocomotionViewData
{
	FMovementDebugOverlayDetails Details;
};

struct FDebugOverlayCombatParticipationViewData
{
	FCombatParticipationDebugOverlayDetails FocusedEnemyDetails;
	TArray<FString> WorldSummaryLines;
};

struct FDebugOverlayBalanceCollapseViewData
{
	FBalanceDebugOverlayDetails Details;
};

struct FDebugOverlayCombatTargetFacingViewData
{
	FEnemyCombatTargetFacingDebugOverlayDetails Details;
};

struct FDebugOverlayExecutionSessionViewData
{
	FExecutionCollaborationDebugOverlayDetails Details;
};

struct FDebugOverlayActorPanelViewData
{
	FString HeaderText;
	bool bIncludeFocus = false;
	FDebugOverlayFocusViewData Focus;
	bool bIncludeStatus = false;
	bool bIncludeTargeting = false;
	FDebugOverlayPlayerTargetingViewData Targeting;
	bool bIncludeLocomotion = false;
	FDebugOverlayPlayerLocomotionViewData Locomotion;
	bool bIncludeBalanceCollapse = false;
	FDebugOverlayBalanceCollapseViewData BalanceCollapse;
	bool bIncludeCombatTargetFacing = false;
	FDebugOverlayCombatTargetFacingViewData CombatTargetFacing;
	bool bIncludeExecutionSession = false;
	FDebugOverlayExecutionSessionViewData ExecutionSession;
	bool bIncludeCombatParticipation = false;
	FDebugOverlayCombatParticipationViewData CombatParticipation;
	bool bAppendBlankBeforeStatus = false;
	FDebugOverlayActorStatusViewData Status;
	FDebugOverlayRecentActionReactionViewData RecentActionReaction;
	bool bIncludeDeathLifecycle = false;
	FDebugOverlayDeathLifecycleViewData DeathLifecycle;
	bool bIncludeRecentActionReaction = false;
	bool bIncludeCurrentAI = false;
	FDebugOverlayCurrentAIViewData CurrentAI;
	bool bIncludeRecentAIEvent = false;
	FDebugOverlayRecentAIEventViewData RecentAIEvent;
};

struct FDebugOverlayEventLogEntryViewData
{
	FString CategoryText;
	FString EventNameText;
	FString SummaryText;
};

struct FDebugOverlayEventLogViewData
{
	bool bHasSnapshot = false;
	int32 DisplayLimit = 0;
	FString FilterText;
	FString ScopeText;
	FString SubjectText;
	bool bFocusedScopeWithoutSubject = false;
	TArray<FDebugOverlayEventLogEntryViewData> Entries;
};

struct FDebugOverlayRecentSummaryBlockViewData
{
	FString HeaderText;
	FString SummaryText;
	EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
	bool bHasSnapshot = false;
	bool bAppendLeadingBlank = false;
};

struct FDebugOverlayWorldSummaryViewData
{
	FString HeaderText;
	TArray<FDebugOverlayRecentSummaryBlockViewData> SummaryBlocks;
	bool bIncludeCombatParticipation = false;
	FDebugOverlayCombatParticipationViewData CombatParticipation;
};

struct FDebugOverlayViewData
{
	TArray<FDebugOverlayActorPanelViewData> ActorPanels;
	FDebugOverlayEventLogViewData EventLog;
	FDebugOverlayWorldSummaryViewData WorldSummary;
};
