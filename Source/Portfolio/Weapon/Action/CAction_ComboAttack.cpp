#include "Weapon/Action/CAction_ComboAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"

#include "Type/CWeaponStructure.h"
#include "Type/CStateStructure.h"

void UCAction_ComboAttack::InitializeAction(ACharacter* InOwnerCharacter, const TArray<FActionData> InActionDatas)
{
	Super::InitializeAction(InOwnerCharacter, InActionDatas);

	Index = 0;
	
	bEnablePreInput = false;
	bExistPreInput = false;
}

void UCAction_ComboAttack::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

void UCAction_ComboAttack::PlayAction()
{
	// [Re invocation] Convert 're-invoked PlayAction()' into 'buffered pre-input'
	if (bEnablePreInput)
	{
		bEnablePreInput = false;	// Enabled by CAnimNotify_ComboEnable
		bExistPreInput = true;		// Mark pre-input for next combo step
		
		return;
	}

	// [First invocation] Validate execution conditions & Execute first combo action
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached) || !IsValid(WeaponComp_Cached)) return;
	if (WeaponComp_Cached->CheckCurType(EWeaponType::Unarmed)) return;
	if (!StateComp_Cached->CheckCurType(EStateType::Idle)) return;
	if (ActionDatas_Injected.Num() <= 0) return;

	Super::PlayAction();		// bIsAction = true

	if (!IsValid(ActionDatas_Injected[Index].Montage)) return;
	ActionDatas_Injected[Index].Begin_PlayMontage(OwnerCharacter_Injected);
}

void UCAction_ComboAttack::Begin_PlayAction()
{
	Super::Begin_PlayAction();	// bBeginAction = true
}

void UCAction_ComboAttack::End_PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	Super::End_PlayAction();	// bIsAction, bBeginAction = false

	if (!IsValid(ActionDatas_Injected[Index].Montage)) return;
	ActionDatas_Injected[Index].End_PlayMontage(OwnerCharacter_Injected);

	Index = 0;

	bEnablePreInput = false;
	bExistPreInput = false;
}

void UCAction_ComboAttack::Next_PlayAction()
{
	if (bExistPreInput)
	{
		Super::Next_PlayAction();

		bExistPreInput = false;

		Index++;
		if ((int32)Index >= ActionDatas_Injected.Num()) return;

		if (!IsValid(ActionDatas_Injected[Index].Montage)) return;
		ActionDatas_Injected[Index].Begin_PlayMontage(OwnerCharacter_Injected);
	}
}

void UCAction_ComboAttack::OnAttachmentCollisionEnabled()
{
	// TODO
}

void UCAction_ComboAttack::OnAttachmentCollisionDisabled()
{
	// TODO
}

void UCAction_ComboAttack::OnAttachmentBeginOverlap(AActor* attackerActor, AActor* damageCauser, UShapeComponent* attackCollision, AActor* targetActor, UPrimitiveComponent* hitComponent)
{
	// TODO
}

void UCAction_ComboAttack::OnAttachmentEndOverlap(AActor* InAttackerActor, AActor* InTargetActor)
{
	// TODO
}
