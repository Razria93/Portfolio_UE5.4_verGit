#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_ActionBase.h"
#include "CAnimNotifyState_ChainWindow.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_ChainWindow : public UCAnimNotifyState_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_ChainWindow();

public:
	FString GetNotifyName_Implementation() const override;

public:
	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
