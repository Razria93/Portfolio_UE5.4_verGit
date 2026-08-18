#include "Component/CEnemyCombatTargetFacingComponent.h"

#include "Component/CCombatTargetComponent.h"
#include "Component/CMovementComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatTargetTypes.h"

#include "AIController.h"

UCEnemyCombatTargetFacingComponent::UCEnemyCombatTargetFacingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCEnemyCombatTargetFacingComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.RemoveAll(this);
	}

	CombatTargetComponent_Injected = InReferences.CombatTargetComponent;
	MovementComponent_Injected = InReferences.MovementComponent;

	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.AddUObject(this, &UCEnemyCombatTargetFacingComponent::HandleCombatTargetChanged);
	}

	SynchronizeCombatTargetFacing();
}

void UCEnemyCombatTargetFacingComponent::SetAIController(AAIController* InAIController)
{
	if (AIController_Injected == InAIController)
	{
		SynchronizeCombatTargetFacing();
		return;
	}

	ClearCombatTargetFacing();
	AIController_Injected = InAIController;
	SynchronizeCombatTargetFacing();
}

void UCEnemyCombatTargetFacingComponent::ClearAIController()
{
	ClearCombatTargetFacing();
	AIController_Injected = nullptr;
}

void UCEnemyCombatTargetFacingComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.RemoveAll(this);
	}

	ClearAIController();
	CombatTargetComponent_Injected = nullptr;
	MovementComponent_Injected = nullptr;

	Super::EndPlay(InEndPlayReason);
}

void UCEnemyCombatTargetFacingComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	ApplyCombatTargetFacing(InChange.CurrentSnapshot);
}

void UCEnemyCombatTargetFacingComponent::SynchronizeCombatTargetFacing()
{
	const FCombatTargetSnapshot snapshot = IsValid(CombatTargetComponent_Injected) ? CombatTargetComponent_Injected->GetCombatTargetSnapshot() : FCombatTargetSnapshot();
	ApplyCombatTargetFacing(snapshot);
}

void UCEnemyCombatTargetFacingComponent::ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot)
{
	if (!IsValid(AIController_Injected) || !IsValid(InSnapshot.TargetActor))
	{
		ClearCombatTargetFacing();
		return;
	}

	AIController_Injected->SetFocus(InSnapshot.TargetActor, EAIFocusPriority::Gameplay);
	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::ControllerDesired);
	}
}

void UCEnemyCombatTargetFacingComponent::ClearCombatTargetFacing()
{
	if (IsValid(AIController_Injected))
	{
		AIController_Injected->ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::OrientToMovement);
	}
}
