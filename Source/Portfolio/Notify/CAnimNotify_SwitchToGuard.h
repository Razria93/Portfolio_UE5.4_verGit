#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_SwitchToGuard.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_SwitchToGuard : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_SwitchToGuard();

public:
	FString GetNotifyName_Implementation() const override;

public:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
