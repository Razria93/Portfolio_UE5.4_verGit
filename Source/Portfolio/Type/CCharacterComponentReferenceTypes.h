#pragma once

#include "CoreMinimal.h"

class ACharacter;
class UCActionComponent;
class UCActionFeedbackComponent;
class UCActionOrchestratorComponent;
class UCCombatSignalSourceComponent;
class UCCombatSignalTargetComponent;
class UCCombatTargetComponent;
class UCCharacterFeedbackComponent;
class UCDefenseComponent;
class UCHealthComponent;
class UCHitFeedbackComponent;
class UCMovementComponent;
class UCObservableOverlayComponent;
class UCReactionComponent;
class UCReactionFeedbackComponent;
class UCReactionOrchestratorComponent;
class UCStateComponent;
class UCWeaponComponent;

// Runtime Context

struct FCharacterComponentReferences
{
	ACharacter* OwnerCharacter = nullptr;

	UCMovementComponent* MovementComponent = nullptr;
	UCWeaponComponent* WeaponComponent = nullptr;
	UCStateComponent* StateComponent = nullptr;
	UCHealthComponent* HealthComponent = nullptr;
	UCDefenseComponent* DefenseComponent = nullptr;
	UCObservableOverlayComponent* ObservableOverlayComponent = nullptr;
	UCCombatTargetComponent* CombatTargetComponent = nullptr;

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
