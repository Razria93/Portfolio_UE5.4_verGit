#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CAnimNotifyState.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& InPayload) override;
	void BranchingPointNotifyTick(FBranchingPointNotifyPayload& InPayload, float InFrameDeltaTime) override;
	void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& InPayload) override;
};
