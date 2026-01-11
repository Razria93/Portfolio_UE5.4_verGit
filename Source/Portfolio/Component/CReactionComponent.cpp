#include "Component/CReactionComponent.h"

UCReactionComponent::UCReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCReactionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

