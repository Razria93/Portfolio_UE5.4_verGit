#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatTargetTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CMovementTypes.h"
#include "Type/CReactionTypes.h"

class AAIController;
class AActor;

enum class EEnemyCombatTargetFacingPolicyState : uint8
{
	Uninitialized,
	Tracking,
	HoldForReaction,
	SuppressedDead,
	SuppressedBalance,
	NoCombatTarget,
	NoAIController,
};

enum class EEnemyCombatTargetFacingFocusDirective : uint8
{
	None,
	SetCombatTarget,
	ClearGameplayFocus,
	HoldCurrentFocus,
};

enum class EEnemyCombatTargetFacingRotationDirective : uint8
{
	None,
	ControllerDesired,
	OrientToMovement,
	HoldCurrentRotation,
};

struct FEnemyCombatTargetFacingRuntimeSnapshot
{
	bool bHasComponent = false;
	AActor* CombatTargetActor = nullptr;
	int32 CombatTargetRevision = 0;
	ECombatTargetChangeReason CombatTargetChangeReason = ECombatTargetChangeReason::None;
	AAIController* OwnerAIController = nullptr;
	AAIController* BoundAIController = nullptr;
	AActor* GameplayFocusActor = nullptr;
	EMovementRotationMode RotationMode = EMovementRotationMode::None;
	EReactionType ActiveReactionType = EReactionType::None;
	bool bIsReactionActive = false;
	bool bIsDead = false;
	bool bIsBalanceSuppressed = false;
	bool bDeferredSyncPending = false;
	bool bDeferredSyncQueued = false;
	bool bControllerBindingMatchesOwner = false;
	EEnemyCombatTargetFacingPolicyState PolicyState = EEnemyCombatTargetFacingPolicyState::Uninitialized;
	EEnemyCombatTargetFacingFocusDirective ExpectedFocusDirective = EEnemyCombatTargetFacingFocusDirective::None;
	EEnemyCombatTargetFacingRotationDirective ExpectedRotationDirective = EEnemyCombatTargetFacingRotationDirective::None;
	uint32 LastTransitionSequence = 0;
	float LastTransitionWorldTimeSeconds = 0.f;
	FString LastEventName;
	FString LastDecision;
};
