#include "Component/CReactionFXComponent.h"
#include "ProjectGlobal.h"

UCReactionFXComponent::UCReactionFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCReactionFXComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCReactionFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

