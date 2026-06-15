#include "Component/CDefenseComponent.h"
#include "ProjectGlobal.h"

UCDefenseComponent::UCDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCDefenseComponent::BeginGuardIntent()
{
	bWantsGuarding = true;
}

void UCDefenseComponent::EndGuardIntent()
{
	bWantsGuarding = false;
}

void UCDefenseComponent::BeginGuardPose()
{
	bIsGuardingPose = true;
}

void UCDefenseComponent::EndGuardPose()
{
	bIsGuardingPose = false;
}

void UCDefenseComponent::OpenGuardWindow()
{
	bCanGuard = true;
}

void UCDefenseComponent::CloseGuardWindow()
{
	bCanGuard = false;
}

void UCDefenseComponent::OpenParryWindow()
{
	bCanParry = true;
}

void UCDefenseComponent::CloseParryWindow()
{
	bCanParry = false;
}

void UCDefenseComponent::ResetGuardState()
{
	bWantsGuarding = false;
	bIsGuardingPose = false;
	bCanGuard = false;
	bCanParry = false;
}

void UCDefenseComponent::HandleGuardInStarted()
{
	BeginGuardIntent();
	BeginGuardPose();

	CloseGuardWindow();
	OpenParryWindow();
	
	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardOutStarted()
{
	EndGuardIntent();
	EndGuardPose();

	CloseGuardWindow();
	CloseParryWindow();
	
	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardOutCompleted()
{
	ResetGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardInterrupted(EActionStopReason InStopReason)
{
	ResetGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::PrintGuardStateInfo() const
{
	FLog::Log(FString::Printf(
		TEXT("[Defense] WantsGuarding = %s | IsGuardingPose = %s | CanGuard = %s | CanParry = %s"),
		bWantsGuarding ? TEXT("true") : TEXT("false"),
		bIsGuardingPose ? TEXT("true") : TEXT("false"),
		bCanGuard ? TEXT("true") : TEXT("false"),
		bCanParry ? TEXT("true") : TEXT("false")));
}
