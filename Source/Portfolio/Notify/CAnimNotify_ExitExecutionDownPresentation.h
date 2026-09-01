#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ReactionBase.h"
#include "CAnimNotify_ExitExecutionDownPresentation.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_ExitExecutionDownPresentation : public UCAnimNotify_ReactionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_ExitExecutionDownPresentation();
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
