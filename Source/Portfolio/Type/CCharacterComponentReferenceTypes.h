#pragma once

#include "CoreMinimal.h"

class ACharacter;
class UCActionComponent;
class UCActionFeedbackComponent;
class UCActionOrchestratorComponent;
class UCCombatSignalSourceComponent;
class UCCombatSignalTargetComponent;
class UCCombatTargetComponent;
class UCEnemyCombatTargetFacingComponent;
class UCEnemyCombatParticipationComponent;
class UCEnemyHitReactiveComponent;
class UCCharacterFeedbackComponent;
class UCDefenseComponent;
class UCExecutionCollaborationComponent;
class UCHealthComponent;
class UCHitFeedbackComponent;
class UCMovementComponent;
class UCObservableOverlayComponent;
class UCReactionComponent;
class UCReactionFeedbackComponent;
class UCReactionOrchestratorComponent;
class UCStateComponent;
class UCWeaponComponent;
class UCBalanceComponent;

// Runtime Context

struct FCharacterComponentReferences
{
	ACharacter* OwnerCharacter = nullptr;

	UCMovementComponent* MovementComponent = nullptr;
	UCWeaponComponent* WeaponComponent = nullptr;
	UCStateComponent* StateComponent = nullptr;
	UCHealthComponent* HealthComponent = nullptr;
	UCBalanceComponent* BalanceComponent = nullptr;
	UCDefenseComponent* DefenseComponent = nullptr;
	UCObservableOverlayComponent* ObservableOverlayComponent = nullptr;
	UCCombatTargetComponent* CombatTargetComponent = nullptr;
	UCExecutionCollaborationComponent* ExecutionCollaborationComponent = nullptr;
	UCEnemyCombatTargetFacingComponent* EnemyCombatTargetFacingComponent = nullptr;
	UCEnemyCombatParticipationComponent* EnemyCombatParticipationComponent = nullptr;
	UCEnemyHitReactiveComponent* EnemyHitReactiveComponent = nullptr;

	UCCombatSignalSourceComponent* CombatSignalSourceComponent = nullptr;
	UCCombatSignalTargetComponent* CombatSignalTargetComponent = nullptr;

	UCActionOrchestratorComponent* ActionOrchestratorComponent = nullptr;
	UCReactionOrchestratorComponent* ReactionOrchestratorComponent = nullptr;

	UCActionComponent* ActionComponent = nullptr;
	UCReactionComponent* ReactionComponent = nullptr;

	UCHitFeedbackComponent* HitFeedbackComponent = nullptr;
	UCActionFeedbackComponent* ActionFeedbackComponent = nullptr;
	UCReactionFeedbackComponent* ReactionFeedbackComponent = nullptr;
	UCCharacterFeedbackComponent* CharacterFeedbackComponent = nullptr;
};
