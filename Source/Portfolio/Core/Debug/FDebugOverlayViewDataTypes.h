#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlaySnapshotTypes.h"

class ACEnemy;
class APawn;
class UObject;
class UWorld;

enum class EDebugOverlayRecentAIEventViewState : uint8
{
	NoTarget,
	NotCaptured,
	NotMatched,
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
	FString StaggerText;
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

struct FDebugOverlayRecentAIEventViewData
{
	EDebugOverlayRecentAIEventViewState State = EDebugOverlayRecentAIEventViewState::NoTarget;
	FString SelectedPawnName;
	FString LastPawnName;
	FString TaskText;
	FString ResultText;
	FString AgeText;
	FString RejectReasonText;
	FString StaleAgeText;
};

struct FDebugOverlayActorPanelViewData
{
	FString HeaderText;
	TArray<FString> LinesBeforeStatus;
	bool bAppendBlankBeforeStatus = false;
	FDebugOverlayActorStatusViewData Status;
	FDebugOverlayRecentExecutionViewData RecentExecution;
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

struct FDebugOverlayInteractionViewData
{
	FString HeaderText;
	TArray<FDebugOverlayRecentSummaryBlockViewData> SummaryBlocks;
};

struct FDebugOverlayViewData
{
	FString MainPanelTitle;
	TArray<FDebugOverlayActorPanelViewData> ActorPanels;
	TArray<FString> MainPanelLines;
	FString EventLogPanelTitle;
	FDebugOverlayEventLogViewData EventLog;
	FString InteractionPanelTitle;
	FDebugOverlayInteractionViewData Interaction;
};

struct FDebugOverlayViewDataBuildContext
{
	UObject* WorldContextObject = nullptr;
	UWorld* World = nullptr;
	const APawn* ViewerPawn = nullptr;
	const ACEnemy* DisplayEnemy = nullptr;
	TArray<FString> EnemySourceLines;
};
