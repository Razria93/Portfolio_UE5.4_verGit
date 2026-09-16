#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_WeaponSocketTransformTransitionBase.h"
#include "CAnimNotifyState_UnequipSocketTransformTransition.generated.h"

UCLASS(meta = (DisplayName = "Unequip (Socket Transform Transition)"))
class PORTFOLIO_API UCAnimNotifyState_UnequipSocketTransformTransition : public UCAnimNotifyState_WeaponSocketTransformTransitionBase
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_UnequipSocketTransformTransition();
};
