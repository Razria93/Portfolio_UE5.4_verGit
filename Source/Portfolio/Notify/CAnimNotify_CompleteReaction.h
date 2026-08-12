#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ReactionBase.h"
#include "CAnimNotify_CompleteReaction.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_CompleteReaction : public UCAnimNotify_ReactionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_CompleteReaction();

private:
	FString GetNotifyName_Implementation() const override;

private:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
