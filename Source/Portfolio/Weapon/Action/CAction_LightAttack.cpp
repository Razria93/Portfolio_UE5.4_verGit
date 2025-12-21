#include "Weapon/Action/CAction_LightAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CStateComponent.h"

#include "Type/CStateStructure.h"

void UCAction_LightAttack::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

void UCAction_LightAttack::PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;
	if (!StateComp_Cached->CheckCurType(EStateType::Idle)) return;
	if (!IsValid(ActionData_Injected.Montage)) return;

	Super::PlayAction();		// bIsAction = true

	ActionData_Injected.Begin_PlayMontage(OwnerCharacter_Injected);
}

void UCAction_LightAttack::Begin_PlayAction()
{
	Super::Begin_PlayAction();	// bBeginAction = true
}

void UCAction_LightAttack::End_PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	Super::End_PlayAction();	// bIsAction, bBeginAction = false

	ActionData_Injected.End_PlayMontage(OwnerCharacter_Injected);
}