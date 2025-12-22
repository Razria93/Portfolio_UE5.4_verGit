#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_PreInput.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_PreInput : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_PreInput();

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
