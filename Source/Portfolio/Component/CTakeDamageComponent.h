#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CTakeDamageComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCTakeDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCTakeDamageComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
