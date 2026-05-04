#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CReactionFeedbackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCReactionFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCReactionFeedbackComponent();

protected:
	void BeginPlay() override;
};
