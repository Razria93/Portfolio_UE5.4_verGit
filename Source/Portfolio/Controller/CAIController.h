#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CAIController.generated.h"

UCLASS()
class PORTFOLIO_API ACAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
	class UBlackboardData* BlackboardAsset;

	UPROPERTY(EditDefaultsOnly)
	class UBehaviorTree* BehaviorTreeAsset;

protected:
	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerceptionComp;

protected:
	UPROPERTY(Transient)
	class APawn* ControlledPawn_Cached;

public:
	ACAIController();

protected:
	void BeginPlay() override;

protected:
	void OnPossess(APawn* InPawn) override;
	void OnUnPossess() override;

private:
	void InitializeBlackboardAndTree();
};
