#include "Action/CAction_LightAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"

#include "Type/CWeaponStructure.h"
#include "Type/CStateStructure.h"

void UCAction_LightAttack::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

bool UCAction_LightAttack::PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached) || !IsValid(WeaponComp_Cached)) return false;
	if (WeaponComp_Cached->CheckCurAttachmentType(EAttachmentType::Unarmed)) return false;
	if (!StateComp_Cached->CheckCurExecutionState(EExecutionState::Idle)) return false;
	if (!IsValid(ActionDatas_Injected[0].Montage)) return false;

	if(!Super::PlayAction()) return false;		// bIsAction = true

	ActionDatas_Injected[0].BeginPlayMontage(OwnerCharacter_Injected);
	return true;
}

void UCAction_LightAttack::BeginPlayAction()
{
	Super::BeginPlayAction();	// bBeginAction = true
}

void UCAction_LightAttack::EndPlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	Super::EndPlayAction();	// bIsAction, bBeginAction = false

	ActionDatas_Injected[0].EndPlayMontage(OwnerCharacter_Injected);
}