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
	// Lifecycle
	void BeginPlay() override;
	void OnPossess(class APawn* InPawn) override;
	void OnUnPossess() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// Config Setup
	bool InitializeSightConfig();

private:
	// Runtime Lifecycle
	bool InitializeControllerRuntime(class APawn* InPawn);
	void UninitializeControllerRuntime();

private:
	// Possession Runtime
	bool SetPossessionRuntimeState(class APawn* InPawn);
	void ResetPossessionRuntimeState();

	// Perception Binding
	bool BindPerceptionEvents();
	void UnbindPerceptionEvents();

	// Blackboard Setup
	bool SetupBlackboardComponent();

	// Blackboard Runtime Value
	bool InitializeBlackboardRuntimeValues();
	void ApplyInitialBlackboardValues(class UBlackboardComponent* InBlackboardComp, const class APawn* InOwnerPawn, TSet<FName>& OutPendingKeys) const;

	void ApplyFixedInitialBlackboardValue(class UBlackboardComponent* InBlackboardComp, const struct FAIBlackboardKeySpec& InKeySpec) const;
	void ApplyOwnerLocationInitialBlackboardValue(class UBlackboardComponent* InBlackboardComp, const class APawn* InOwnerPawn, const struct FAIBlackboardKeySpec& InKeySpec) const;
	void ApplyCustomBlackboardValues(class UBlackboardComponent* InBlackboardComp, const class APawn* InOwnerPawn, TSet<FName>& InOutPendingKeys) const;
	void SetCustomBlackboardBoolValue(class UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const struct FAIBlackboardKeySpec& InKeySpec, bool InValue) const;
	void SetCustomBlackboardIntValue(class UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const struct FAIBlackboardKeySpec& InKeySpec, int32 InValue) const;
	void SetCustomBlackboardFloatValue(class UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const struct FAIBlackboardKeySpec& InKeySpec, float InValue) const;
	void SetCustomBlackboardVectorValue(class UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const struct FAIBlackboardKeySpec& InKeySpec, const FVector& InValue) const;
	void SetCustomBlackboardEnumValue(class UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const struct FAIBlackboardKeySpec& InKeySpec, uint8 InValue) const;
	void SetCustomBlackboardObjectValue(class UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const struct FAIBlackboardKeySpec& InKeySpec, class UObject* InValue) const;

	void MarkCustomBlackboardKeyApplied(TSet<FName>& InOutPendingKeys, const struct FAIBlackboardKeySpec& InKeySpec) const;
	bool ValidateCustomBlackboardKeysApplied(const TSet<FName>& InPendingKeys) const;

	void ClearBlackboardRuntimeValues();

	// Behavior Tree Runtime
	bool StartBehaviorTreeRuntime();
	void StopBehaviorTreeRuntime();

private:
	// Perception Event Callback
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<class AActor*>& InUpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionUpdated(class AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnTargetPerceptionForgotten(class AActor* Actor);

public:
	// Query
	EPerceptionBuildResult BuildPerceptionContext(FTargetData& OutTargetData);

private:
	// Target Data
	void UpdateTargetDataMap();
	void ClearTargetDataMap();
	EPerceptionBuildResult SelectTopPriority(FTargetData& OutTargetData);

private:
	// Debug
	void PrintPerceptionUpdatedSummary(const TArray<class AActor*>& UpdatedActors) const;
	void PrintTargetPerceptionUpdatedSummary(class AActor* Actor, const FAIStimulus& Stimulus) const;
	void PrintTargetPerceptionForgotten(AActor* Actor) const;
	void PrintAllTargetData() const;
	void PrintTargetData(const FTargetData& InData) const;
};


