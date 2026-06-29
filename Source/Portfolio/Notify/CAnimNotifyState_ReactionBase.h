#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState.h"
#include "Type/CWeaponStructure.h"
#include "CAnimNotifyState_ReactionBase.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCAnimNotifyState_ReactionBase : public UCAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_ReactionBase();

protected:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	EReactionType TriggerReactionType = EReactionType::All;

protected:
	bool CanProcessReactionNotify(const class UCReactionComponent* InReactionComp) const;

protected:
	class UCReactionComponent* GetReactionComponent(USkeletalMeshComponent* InMeshComp) const;
};
