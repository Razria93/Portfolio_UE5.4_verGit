#pragma once

#include "CoreMinimal.h"

class ACEnemy;
class APawn;
class UObject;
class UWorld;

struct FDebugOverlayViewData
{
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
