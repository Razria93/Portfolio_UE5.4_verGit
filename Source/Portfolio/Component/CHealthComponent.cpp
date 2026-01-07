#include "Component/CHealthComponent.h"

UCHealthComponent::UCHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

