#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_CommitExecution.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_CommitExecution : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_CommitExecution();

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
