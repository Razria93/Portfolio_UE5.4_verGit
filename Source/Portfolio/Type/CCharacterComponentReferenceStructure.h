#pragma once

#include "CoreMinimal.h"

struct FCharacterComponentReferences
{
	class ACharacter* OwnerCharacter = nullptr;

	class UCMovementComponent* MovementComponent = nullptr;
	class UCWeaponComponent* WeaponComponent = nullptr;
	class UCStateComponent* StateComponent = nullptr;
	class UCHealthComponent* HealthComponent = nullptr;
	class UCDefenseComponent* DefenseComponent = nullptr;
	class UCObservableOverlayComponent* ObservableOverlayComponent = nullptr;

	class UCCombatSignalSourceComponent* CombatSignalSourceComponent = nullptr;
	class UCCombatSignalTargetComponent* CombatSignalTargetComponent = nullptr;

	class UCActionOrchestratorComponent* ActionOrchestratorComponent = nullptr;
	class UCReactionOrchestratorComponent* ReactionOrchestratorComponent = nullptr;

	class UCActionComponent* ActionComponent = nullptr;
	class UCReactionComponent* ReactionComponent = nullptr;

	class UCHitFeedbackComponent* HitFeedbackComponent = nullptr;
	class UCActionFeedbackComponent* ActionFeedbackComponent = nullptr;
	class UCReactionFeedbackComponent* ReactionFeedbackComponent = nullptr;
};
