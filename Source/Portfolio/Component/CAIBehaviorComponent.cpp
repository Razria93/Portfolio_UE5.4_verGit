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

	// [EngineAPI / UBlackboardComponent] GetKeyID
	// true  : returns 'a valid FKey'
	// false : returns 'FBlackboard::InvalidKey'
	const bool bHasAIStateTypeKey = InBlackboardComponent->GetKeyID(AIStateTypeKey) != FBlackboard::InvalidKey;
	const bool bHasTargetKey = InBlackboardComponent->GetKeyID(TargetActorKey) != FBlackboard::InvalidKey;

	// [Fail] Blackboard key mismatch: 'Blackboard' vs 'This'
	if (!bHasAIStateTypeKey || !bHasTargetKey)
	{
		FLog::Log(FString::Printf(TEXT("[Error] Missing Blackboard Keys : AIStateTypeKey = %s | TargetActorKey = %s"),
			*AIStateTypeKey.ToString(),
			*TargetActorKey.ToString()));

		BlackboardComp_Injected = nullptr;
		return false;
	}

	BlackboardComp_Injected = InBlackboardComponent;
	return true;
}