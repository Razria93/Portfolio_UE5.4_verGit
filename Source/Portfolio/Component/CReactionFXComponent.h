#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CReactionFXComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCReactionFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCReactionFXComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
