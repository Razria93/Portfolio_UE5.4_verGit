#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "Type/CActionTypes.h"
#include "CAnimNotify_ActionBase.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCAnimNotify_ActionBase : public UCAnimNotify
{
	GENERATED_BODY()
	
public:
	UCAnimNotify_ActionBase();

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
