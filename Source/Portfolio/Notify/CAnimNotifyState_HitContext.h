#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_ActionBase.h"
#include "CAnimNotifyState_HitContext.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_HitContext : public UCAnimNotifyState_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_HitContext();

public:
	FString GetNotifyName_Implementation() const override;

public:
	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};