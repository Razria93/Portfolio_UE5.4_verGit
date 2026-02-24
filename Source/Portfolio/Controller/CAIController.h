#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "CAIController.generated.h"

USTRUCT()
struct FTargetData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AActor* TargetActor = nullptr;

	UPROPERTY()
	int TargetPriority = INT_MAX;

	UPROPERTY()
	bool bHasLOS = false;

	UPROPERTY()
	float LastSeenTime = -1.0f;

	UPROPERTY()
	FVector LastKnownLocation = FVector::ZeroVector;

public:
	FTargetData() = default;
	FTargetData(const FTargetData&) = default;
	FTargetData& operator=(const FTargetData&) = default;

public:
	bool IsValidData() const
	{
		return IsValid(TargetActor) && TargetPriority != INT_MAX;
	}
};

UCLASS()
class PORTFOLIO_API ACAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float TargetMemoryTimeout = 3.0f;

	UPROPERTY(EditAnywhere)
	float AttackRangeEnter = 250.0f;

	UPROPERTY(EditAnywhere)
	float AttackRangeExit = 300.0f;

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
	// Blackboard Service API
	void UpdateBlackboardContext();

private:
	bool ValidateBlackboardKeys(const UBlackboardData* InBlackboardAsset) const;
	bool ValidateBlackboardKey(const UBlackboardData* InBlackboardAsset, const FName& InKeyName) const;

private:
	bool SelectTopPriority(FTargetData& OutTargetData);
	void CleanUpTargetDataMap(float InNowTime);

private:
	void PrintPerceptionUpdatedSummary(const TArray<class AActor*>& UpdatedActors) const;
	void PrintTargetPerceptionUpdatedSummary(class AActor* Actor, const FAIStimulus& Stimulus) const;
	void PrintTargetPerceptionForgotten(AActor* Actor) const;
	void PrintAllTargetData() const;
	void PrintTargetData(const FTargetData& InData) const;
};


