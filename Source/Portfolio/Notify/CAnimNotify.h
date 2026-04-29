#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Type/CWeaponStructure.h"
#include "CAnimNotify.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify();

protected:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	EActionType TriggerActionType = EActionType::Max;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	int32 TriggerActionIndex = INDEX_NONE;

protected:
	bool CanProcessActionNotify(const class UCAction* InCurrentAction) const;
};
