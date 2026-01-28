#include "Component/CAIBehaviorComponent.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

UCAIBehaviorComponent::UCAIBehaviorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCAIBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCAIBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCAIBehaviorComponent::Initialize(UBlackboardComponent* InBlackboardComponent)
{
	if (!IsValid(InBlackboardComponent)) return false;

	BlackboardComp_Injected = InBlackboardComponent;
	return true;
}