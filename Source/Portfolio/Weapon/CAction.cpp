#include "Weapon/CAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CStateComponent.h"

void UCAction::InitializeAction(ACharacter* InOwnerCharacter, FActionData InActionData)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	ActionData_Injected = InActionData;

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Injected->GetComponentByClass(UCStateComponent::StaticClass())); // TODO: Refactor Interface
	check(StateComp_Cached);
}