#include "Component/CAIBehaviorComponent.h"

UCAIBehaviorComponent::UCAIBehaviorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCAIBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCAIBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

