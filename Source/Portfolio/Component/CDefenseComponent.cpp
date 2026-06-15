#include "Component/CDefenseComponent.h"
#include "ProjectGlobal.h"

UCDefenseComponent::UCDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCDefenseComponent::PrintGuardingInfo() const
{
	FLog::Log(FString::Printf(TEXT("[Defense] IsGuarding = %s"), bIsGuarding ? TEXT("true") : TEXT("false")));
}
