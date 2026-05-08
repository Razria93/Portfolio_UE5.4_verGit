#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_ActionFeedback.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_ActionFeedback : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()
	
public:
	UCAnimNotify_ActionFeedback();

public:
	UPROPERTY(EditAnywhere, Category = "Feedback")
	FName TriggerKey = NAME_None;

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
