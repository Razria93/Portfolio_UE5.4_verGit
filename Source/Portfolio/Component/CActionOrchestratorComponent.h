#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CActionOrchestratorComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCActionOrchestratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCActionOrchestratorComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FActionRequestResult RequestMovementAction(const FMovementActionRequest& InActionRequest);
	FActionRequestResult RequestEquipmentAction(const FEquipmentActionRequest& InActionRequest);
	FActionRequestResult RequestCombatAction(const FCombatActionRequest& InActionRequest);
};
