#pragma once

#include "CoreMinimal.h"
#include "Type/CEnemyCombatTargetFacingTypes.h"
#include "Type/CMovementTypes.h"

class AActor;
class AAIController;
class ACEnemy;

struct FEnemyCombatTargetFacingDebugSnapshot
{
	bool bHasSnapshot = false;
	FEnemyCombatTargetFacingRuntimeSnapshot Runtime;
};

struct FEnemyCombatTargetFacingDebugOverlayDetails
{
	bool bHasSnapshot = false;
	FString PolicyText;
	FString ConsistencyText;
	FString ControllerBindingText;
	FString CombatTargetText;
	FString GameplayFocusText;
	FString RotationText;
	FString ReactionText;
	FString DeferredText;
	FString ExpectedText;
	FString LastDecisionSequenceText;
	FString LastDecisionTimeText;
	FString LastDecisionTriggerText;
	FString LastDecisionResultText;
};

class PORTFOLIO_API FEnemyCombatTargetFacingDebug
{
public:
	// Display Snapshot / Presentation
	static bool IsEnabled();
	static FEnemyCombatTargetFacingDebugSnapshot BuildSnapshot(const ACEnemy* InEnemy);
	static FEnemyCombatTargetFacingDebugOverlayDetails BuildOverlayDetails(const FEnemyCombatTargetFacingDebugSnapshot& InSnapshot);

	// Gate
	static bool ShouldAuditCombatTargetFacing();

public:
	// Facing Decision Diagnostic Hook
	static void RecordFacingDecision(const class UCEnemyCombatTargetFacingComponent* InFacingComponent, EMovementRotationMode InPreviousRotationMode, const AActor* InPreviousGameplayFocusActor, const TCHAR* InEvent, const TCHAR* InDecision);
};
