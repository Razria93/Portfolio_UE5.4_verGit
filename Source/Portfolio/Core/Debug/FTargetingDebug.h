#pragma once

#include "CoreMinimal.h"

class UWorld;
struct FTargetingEvaluation;

struct FTargetingDebugOverlayDetails
{
	bool bHasSnapshot = false;
	FString RuntimeTargetText;
	FString DistanceText;
	FString DotText;
	FString AngleScoreText;
	FString DistanceScoreText;
	FString FinalScoreText;
	FString RangeText;
	FString ViewConeText;
};

class PORTFOLIO_API FTargetingDebug
{
public:
	// Runtime Gate Query
	static bool IsEnabled();
	static bool ShouldDrawRangeSphere();
	static bool ShouldDrawSelectedTargetSphere();
	static bool ShouldDrawViewLine();
	static bool ShouldDrawDebugText();
	static bool ShouldShowOverlayDetails();
	static FTargetingDebugOverlayDetails BuildOverlayDetails(const FTargetingEvaluation& InEvaluation);

	// World Draw
	static void DrawWorldDebug(UWorld* InWorld, const FTargetingEvaluation& InEvaluation);
};
