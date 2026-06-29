#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_ReactionBase.h"
#include "CAnimNotifyState_ReactionFeedback.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_ReactionFeedback : public UCAnimNotifyState_ReactionBase
{
	GENERATED_BODY()
	
public:
	UCAnimNotifyState_ReactionFeedback();

public:
	UPROPERTY(EditAnywhere, Category = "Feedback")
	FName TriggerKey = NAME_None;

public:
	FString GetNotifyName_Implementation() const override;

public:
	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
