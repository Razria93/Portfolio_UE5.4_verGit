#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_Equip.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_Equip : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_Equip();

private:
	FString GetNotifyName_Implementation() const override;

private:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
