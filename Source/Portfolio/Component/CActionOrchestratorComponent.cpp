#include "Component/CActionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "Type/CActionOrchestrationStructure.h"

UCActionOrchestratorComponent::UCActionOrchestratorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCActionOrchestratorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCActionOrchestratorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FActionRequestResult UCActionOrchestratorComponent::RequestMovementAction(const FMovementActionRequest& InRequest)
{
	return FActionRequestResult();
}

FActionRequestResult UCActionOrchestratorComponent::RequestEquipmentAction(const FEquipmentActionRequest& InRequest)
{
	return FActionRequestResult();
}

FActionRequestResult UCActionOrchestratorComponent::RequestCombatAction(const FCombatActionRequest& InRequest)
{
	return FActionRequestResult();
}

