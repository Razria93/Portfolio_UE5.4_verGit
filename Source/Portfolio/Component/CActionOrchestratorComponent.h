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

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FActionRequestResult RequestMovementAction(const FMovementActionRequest& InActionRequest);
	FActionRequestResult RequestEquipmentAction(const FEquipmentActionRequest& InActionRequest);
	FActionRequestResult RequestCombatAction(const FCombatActionRequest& InActionRequest);

private:
	bool CanAcceptActionRequest(EActionRequestRejectReason& OutRejectReason) const;

	FActionRequestResult BuildExecutedResult(EActionType InExecutedActionType = EActionType::Max) const;
	FActionRequestResult BuildRejectedResult(EActionRequestRejectReason InRejectReason) const;
	FActionRequestResult BuildIgnoredResult() const;
};
