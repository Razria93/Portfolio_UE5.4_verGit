#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Type/CWorldSubSystemStructure.h"
#include "CWorldSubsystem_CombatEngage.generated.h"

UCLASS()
class PORTFOLIO_API UCWorldSubsystem_CombatEngage : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	int32 MaxEngagersPerTarget = 2;

	UPROPERTY()
	float RebuildInterval = 0.1f;

private:
	float ElapsedTime = 0.f;

private:
	UPROPERTY()
	TMap<class ACAIController*, FCombatRequestContext> RequestContainer;

	UPROPERTY()
	TMap<class ACAIController*, FCombatAssignmentContext> AssignmentContainer;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

public:
	FCombatAssignmentContext GetAssignment(const class ACAIController* InCAIController) const;

public:
	void SubmitRequest(const FCombatRequestContext& InCombatRequestContext);
	void RebuildAssignments();

private:
	void PrintEngageContext(const ACAIController* InCAIController, const AActor* InActor, const int& InPriority, const int& InIndex, const float& InDistance, const ECombatRole& InCombatRole) const;
};
