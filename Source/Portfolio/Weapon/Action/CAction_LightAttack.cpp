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

	ActionDatas_Injected[0].BeginPlayMontage(OwnerCharacter_Injected);
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