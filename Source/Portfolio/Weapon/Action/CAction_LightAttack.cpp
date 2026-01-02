#include "Weapon/Action/CAction_LightAttack.h"
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

void UCAction_LightAttack::PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached) || !IsValid(WeaponComp_Cached)) return;
	if (WeaponComp_Cached->CheckCurAttachmentType(EAttachmentType::Unarmed)) return;
	if (!StateComp_Cached->CheckCurStateType(EStateType::Idle)) return;
	if (!IsValid(ActionDatas_Injected[0].Montage)) return;
	
	Super::PlayAction();		// bIsAction = true

	ActionDatas_Injected[0].Begin_PlayMontage(OwnerCharacter_Injected);
}

void UCAction_LightAttack::Begin_PlayAction()
{
	Super::Begin_PlayAction();	// bBeginAction = true
}

void UCAction_LightAttack::End_PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	Super::End_PlayAction();	// bIsAction, bBeginAction = false

	ActionDatas_Injected[0].End_PlayMontage(OwnerCharacter_Injected);
}