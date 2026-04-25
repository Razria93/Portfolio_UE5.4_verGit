#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Type/CWeaponStructure.h"
#include "CAnimNotifyState.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState();

protected:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	EActionType TriggerActionType = EActionType::Max;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	int32 TriggerActionIndex = INDEX_NONE;

protected:
	bool CanProcessActionNotify(const class UCAction* InCurrentAction) const;
};
