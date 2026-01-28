#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CAIController.generated.h"

UCLASS()
class PORTFOLIO_API ACAIController : public AAIController
{
	GENERATED_BODY()

protected:
	/* --- Asset --- */
	UPROPERTY(EditDefaultsOnly)
	class UBlackboardData* BlackboardAsset;

	UPROPERTY(EditDefaultsOnly)
	class UBehaviorTree* BehaviorTreeAsset;

protected:
	/* --- Component --- */
	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerceptionComp;

protected:
	/* --- Cached --- */
	UPROPERTY(Transient)
	class APawn* ControlledPawn_Cached;

protected:
	/* --- Config --- */
	UPROPERTY()
	class UAISenseConfig_Sight* SightConfig;

public:
	ACAIController();

protected:
	void BeginPlay() override;

protected:
	void OnPossess(class APawn* InPawn) override;
	void OnUnPossess() override;

protected:
	virtual bool InitializeSightConfig();

private:
	bool InitializeBlackBoard();
	bool InitializeBehaviorTree();
};
