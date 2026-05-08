#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_AdvanceCombo.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_AdvanceCombo : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()
	
public:
	UCAnimNotify_AdvanceCombo();

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
