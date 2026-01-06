#include "Component/CTakeDamageComponent.h"

UCTakeDamageComponent::UCTakeDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCTakeDamageComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCTakeDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

