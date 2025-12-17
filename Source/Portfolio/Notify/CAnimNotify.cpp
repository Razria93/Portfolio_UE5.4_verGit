#include "Notify/CAnimNotify.h"
#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"

UCAnimNotify::UCAnimNotify()
{
}

FString UCAnimNotify::MakeNotifyName(FString InName) const
{
	if (FlowType != EAnimNotifyFlow::Max)
	{
		UEnum* metaData = StaticEnum<EAnimNotifyFlow>();

		if (metaData)
		{
			FString flowName = metaData->GetNameStringByValue((int64)FlowType);
			return InName + "_" + flowName;
		}
	}

	return InName;
}

UCWeaponComponent* UCAnimNotify::GetWeaponComponent(USkeletalMeshComponent* MeshComp)
{
	if (!IsValid(MeshComp) || !IsValid(MeshComp->GetOwner()))
		return nullptr;

	UActorComponent* tempComp = MeshComp->GetOwner()->GetComponentByClass(UCWeaponComponent::StaticClass());

	if (!tempComp)
		return nullptr;

	UCWeaponComponent* weaponComp = Cast<UCWeaponComponent>(tempComp);

	if (!weaponComp)
		return nullptr;

	return weaponComp;
}
