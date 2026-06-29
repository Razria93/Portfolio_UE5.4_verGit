#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_HealthBase.h"
#include "CAnimNotify_EnterAliveState.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_EnterAliveState : public UCAnimNotify_HealthBase
{
	GENERATED_BODY()
	
public:
	UCAnimNotify_EnterAliveState();

private:
	FString GetNotifyName_Implementation() const override;

private:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
