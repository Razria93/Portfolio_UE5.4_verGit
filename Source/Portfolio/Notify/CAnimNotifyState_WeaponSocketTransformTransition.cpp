#include "Notify/CAnimNotifyState_WeaponSocketTransformTransition.h"

#include "Component/CWeaponComponent.h"

FString UCAnimNotifyState_WeaponSocketTransformTransition::GetNotifyName_Implementation() const
{
	const FString sourceName = SourceSocketName.IsNone() ? TEXT("Current") : SourceSocketName.ToString();
	const FString targetName = TargetSocketName.IsNone() ? TEXT("None") : TargetSocketName.ToString();
	return FString::Printf(TEXT("Weapon Socket: %s -> %s"), *sourceName, *targetName);
}

bool UCAnimNotifyState_WeaponSocketTransformTransition::IsConfigurationValid() const
{
	return !TargetSocketName.IsNone();
}

bool UCAnimNotifyState_WeaponSocketTransformTransition::BeginTransition(UCWeaponComponent* InWeaponComp, uint32& OutTransitionHandle) const
{
	return IsValid(InWeaponComp)
		&& InWeaponComp->BeginWeaponSocketTransformTransition(SourceSocketName, TargetSocketName, OutTransitionHandle);
}
