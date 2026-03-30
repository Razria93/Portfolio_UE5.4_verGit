#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_Equip.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_Equip : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_Equip();

private:
	FString GetNotifyName_Implementation() const override; // UAnimNotify::GetNotifyName override
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override; // UAnimNotify::Notify override
};
