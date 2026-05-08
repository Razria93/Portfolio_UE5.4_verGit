#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_ActionBase.h"
#include "CAnimNotifyState_ActionFeedback.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_ActionFeedback : public UCAnimNotifyState_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_ActionFeedback();

public:
	UPROPERTY(EditAnywhere, Category = "Feedback")
	FName TriggerKey = NAME_None;

public:
	FString GetNotifyName_Implementation() const override;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
