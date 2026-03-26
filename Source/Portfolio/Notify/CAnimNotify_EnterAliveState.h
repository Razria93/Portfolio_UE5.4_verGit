#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_EnterAliveState.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_EnterAliveState : public UCAnimNotify
{
	GENERATED_BODY()
	
public:
	UCAnimNotify_EnterAliveState();

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
