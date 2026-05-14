#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_EnterDeadState.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_EnterDeadState : public UCAnimNotify
{
	GENERATED_BODY()
	
public:
	UCAnimNotify_EnterDeadState();

private:
	FString GetNotifyName_Implementation() const override;

private:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
