#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_FinalizeEnemyDeath.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_FinalizeEnemyDeath : public UAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_FinalizeEnemyDeath();

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
