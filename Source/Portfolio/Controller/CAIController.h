#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Type/CAIStructure.h"
#include "CAIController.generated.h"

UCLASS()
class PORTFOLIO_API ACAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float TargetMemoryTimeout = 3.0f;

protected:
	UPROPERTY(Transient)
	TMap<AActor*, FTargetData> TargetDataMap;

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
	UPROPERTY(Transient)
	class UAISenseConfig_Sight* SightConfig;

public:
	ACAIController();

protected:
	void BeginPlay() override;

protected:
	void OnPossess(class APawn* InPawn) override;
	void OnUnPossess() override;

protected:
	bool InitializeSightConfig();

private:
	bool InitializePerception();
	bool InitializeBlackBoard();
	bool InitializeBehaviorTree();

private:
	bool InitializeBlackBoardValue();

private:
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<class AActor*>& InUpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionUpdated(class AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnTargetPerceptionForgotten(class AActor* Actor);

public:
	EPerceptionBuildResult BuildPerceptionContext(FTargetData& OutTargetData);

private:
	bool ValidateBlackboardKeys(const UBlackboardData* InBlackboardAsset) const;
	bool ValidateBlackboardKey(const UBlackboardData* InBlackboardAsset, const FName& InKeyName) const;

private:
	void UpdateTargetDataMap();
	EPerceptionBuildResult SelectTopPriority(FTargetData& OutTargetData);

private:
	void PrintPerceptionUpdatedSummary(const TArray<class AActor*>& UpdatedActors) const;
	void PrintTargetPerceptionUpdatedSummary(class AActor* Actor, const FAIStimulus& Stimulus) const;
	void PrintTargetPerceptionForgotten(AActor* Actor) const;
	void PrintAllTargetData() const;
	void PrintTargetData(const FTargetData& InData) const;
};


