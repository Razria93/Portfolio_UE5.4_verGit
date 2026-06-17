#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_AllowGuardStart.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_AllowGuardStart : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_AllowGuardStart();

public:
	FString GetNotifyName_Implementation() const override;

public:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
