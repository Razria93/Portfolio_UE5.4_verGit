#include "Weapon/CAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"

void UCAction::InitializeAction(ACharacter* InOwnerCharacter, const TArray<FActionData> InActionDatas)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	ActionDatas_Injected = InActionDatas;

	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerCharacter_Injected->GetComponentByClass(UCWeaponComponent::StaticClass()));	// TODO: Refactor Interface
	check(WeaponComp_Cached);

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Injected->GetComponentByClass(UCStateComponent::StaticClass()));		// TODO: Refactor Interface
	check(StateComp_Cached);
}

void UCAction::PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bIsAction = true;

	StateComp_Cached->SetActionMode();

	// NOTE: To be implemented detail by derived classes
}

void UCAction::Begin_PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bBeginAction = true;

	// NOTE: To be implemented detail by derived classes
}

void UCAction::End_PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bIsAction = false;
	bBeginAction = false;

	StateComp_Cached->SetIdleMode();

	// NOTE: To be implemented detail by derived classes
}
