#pragma once

#include "CoreMinimal.h"

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
	TArray<FString> RecentExecutionLines;
	bool bIncludeCurrentAI = false;
	FDebugOverlayCurrentAIViewData CurrentAI;
	bool bIncludeRecentAIEvent = false;
	FDebugOverlayRecentAIEventViewData RecentAIEvent;
};

struct FDebugOverlayViewData
{
	FString MainPanelTitle;
	TArray<FDebugOverlayActorPanelViewData> ActorPanels;
	TArray<FString> MainPanelLines;
	TArray<FString> EventLogPanelLines;
	TArray<FString> InteractionPanelLines;
};

struct FDebugOverlayViewDataBuildContext
{
	UObject* WorldContextObject = nullptr;
	UWorld* World = nullptr;
	const APawn* ViewerPawn = nullptr;
	const ACEnemy* DisplayEnemy = nullptr;
	TArray<FString> EnemySourceLines;
};
