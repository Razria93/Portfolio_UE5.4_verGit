#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_WeaponSocketTransformTransitionBase.h"
#include "CAnimNotifyState_EquipSocketTransformTransition.generated.h"

UCLASS(meta = (DisplayName = "Equip (Socket Transform Transition)"))
class PORTFOLIO_API UCAnimNotifyState_EquipSocketTransformTransition : public UCAnimNotifyState_WeaponSocketTransformTransitionBase
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_EquipSocketTransformTransition();
};
