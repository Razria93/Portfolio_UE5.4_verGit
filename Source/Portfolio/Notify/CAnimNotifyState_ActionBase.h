#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState.h"
#include "Type/CWeaponStructure.h"
#include "CAnimNotifyState_ActionBase.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCAnimNotifyState_ActionBase : public UCAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_ActionBase();

protected:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	EActionType TriggerActionType = EActionType::All;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	int32 TriggerActionIndex = INDEX_NONE;

protected:
	bool CanProcessActionNotify(const class UCActionComponent* InActionComp) const;

protected:
	class UCActionComponent* GetActionComponent(USkeletalMeshComponent* InMeshComp) const;
};
