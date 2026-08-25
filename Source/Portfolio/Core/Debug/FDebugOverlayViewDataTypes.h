#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FBalanceDebug.h"
#include "Core/Debug/FCombatParticipationDebug.h"
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

enum class EDebugOverlayRecentExecutionViewState : uint8
{
	NotCaptured,
	NoActor,
	NoEvents,
	Captured,
};

struct FDebugOverlayRecentExecutionViewData
{
	EDebugOverlayRecentExecutionViewState State = EDebugOverlayRecentExecutionViewState::NotCaptured;
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
	FString MovementText;
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
	FString DeadInText;
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
	bool bIncludeCombatParticipation = false;
	FDebugOverlayCombatParticipationViewData CombatParticipation;
	bool bAppendBlankBeforeStatus = false;
	FDebugOverlayActorStatusViewData Status;
	FDebugOverlayRecentExecutionViewData RecentExecution;
	bool bIncludeDeathLifecycle = false;
	FDebugOverlayDeathLifecycleViewData DeathLifecycle;
	bool bIncludeRecentExecution = false;
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
	FString MainPanelTitle;
	TArray<FDebugOverlayActorPanelViewData> ActorPanels;
	FString EventLogPanelTitle;
	FDebugOverlayEventLogViewData EventLog;
	FString WorldSummaryPanelTitle;
	FDebugOverlayWorldSummaryViewData WorldSummary;
};
