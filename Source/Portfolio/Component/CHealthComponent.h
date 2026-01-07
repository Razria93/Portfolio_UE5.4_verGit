#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CHealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCHealthComponent(); // TODO: Extend ResourceComponent

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
