#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
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

	UPROPERTY(VisibleAnywhere)
	class UCAIBehaviorComponent* AIBehaviorComp;

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
	bool InitializePerception();
	bool InitializeBlackBoard();
	bool InitializeBehaviorTree();

private:
	bool InitializeBlackBoardComponent();
	bool InitializeAIBehaviorComponent();

private:
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<class AActor*>& InUpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionUpdated(class AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnTargetPerceptionForgotten(class AActor* Actor);

private:
	void PrintPerceptionUpdatedSummary(const TArray<class AActor*>& UpdatedActors) const;
	void PrintTargetPerceptionUpdatedSummary(class AActor* Actor, const FAIStimulus& Stimulus) const;
	void PrintTargetPerceptionForgotten(AActor* Actor) const;
};
