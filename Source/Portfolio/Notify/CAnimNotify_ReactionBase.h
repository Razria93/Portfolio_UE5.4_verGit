#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "Type/CReactionTypes.h"
#include "CAnimNotify_ReactionBase.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCAnimNotify_ReactionBase : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_ReactionBase();

protected:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	EReactionType TriggerReactionType = EReactionType::All;

protected:
	bool CanProcessReactionNotify(const class UCReactionComponent* InReactionComp) const;

protected:
	class UCReactionComponent* GetReactionComponent(USkeletalMeshComponent* InMeshComp) const;
};
