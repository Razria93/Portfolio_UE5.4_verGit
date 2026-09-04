#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ReactionBase.h"
#include "CAnimNotify_ResetBalanceLifecycle.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_ResetBalanceLifecycle : public UCAnimNotify_ReactionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_ResetBalanceLifecycle();
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
