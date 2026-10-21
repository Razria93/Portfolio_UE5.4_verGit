#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_CounterOpportunity.generated.h"

/** Point marker; eligibility belongs to the current PerfectDodge execution, not this asset. */
UCLASS(meta = (DisplayName = "Counter Opportunity (Point)"))
class PORTFOLIO_API UCAnimNotify_CounterOpportunity : public UAnimNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "CounterOpportunity")
	bool bOpen = true;
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
