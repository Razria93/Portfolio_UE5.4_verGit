#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Hit.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Hit : public UCReaction
{
	GENERATED_BODY()
	
public:
	void InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent) override;
	void Tick(float InDeltaTime) override;

public:
	bool Begin(const FReactionData& reactionData) override;
};
