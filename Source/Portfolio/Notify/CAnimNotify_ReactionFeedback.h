#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_ReactionFeedback.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_ReactionFeedback : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_ReactionFeedback();

public:
	UPROPERTY(EditAnywhere, Category = "Feedback")
	FName TriggerKey = NAME_None;

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
