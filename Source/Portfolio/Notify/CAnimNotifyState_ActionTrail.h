#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState.h"
#include "CAnimNotifyState_ActionTrail.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_ActionTrail : public UCAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_ActionTrail();

public:
	FString GetNotifyName_Implementation() const override;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
