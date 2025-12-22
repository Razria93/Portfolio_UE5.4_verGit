#include "Notify/CAnimNotify_Action.h"
#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CAction.h"

UCAnimNotify_Action::UCAnimNotify_Action()
{
}

FString UCAnimNotify_Action::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Action");
}

void UCAnimNotify_Action::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = GetWeaponComponent(MeshComp);

	if (!weaponComp) return;

	UCAction* action = weaponComp->GetAction();

	if (!action) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: action->Begin_PlayAction(); return;
	case EAnimNotifyFlow::End: action->End_PlayAction(); return;
	case EAnimNotifyFlow::Next: action->Next_PlayAction(); return;
	}
}
