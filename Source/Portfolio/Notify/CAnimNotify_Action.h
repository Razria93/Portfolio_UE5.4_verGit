#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_Action.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_Action : public UCAnimNotify
{
	GENERATED_BODY()
	
public:
	UCAnimNotify_Action();

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
