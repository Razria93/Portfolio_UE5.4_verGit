#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_CounterRush.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_CounterRush : public UCAction
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	class UCDefenseComponent* DefenseComp_Injected = nullptr;
	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Injected = nullptr;
	TSharedPtr<struct FRootMotionSource_ConstantForce> RushSource;
	uint64 OpportunitySerial = 0;
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "CounterRush", meta = (ClampMin = "0.01", Units = "s"))
	float RushDuration = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = "CounterRush", meta = (ClampMin = "0.0", Units = "cm"))
	float MaxTravelDistance = 450.f;
	UPROPERTY(EditDefaultsOnly, Category = "CounterRush", meta = (ClampMin = "0.0", Units = "cm"))
	float StopDistance = 120.f;

public:
	void InitializeReferences(const FCharacterComponentReferences& InReferences) override;
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
	bool Start(const FActionData& InData, uint32 InActionRequestSerial = 0) override;
	void CommitStarted();
	void EndCounterAttackPresentation();

protected:
	void CleanupRuntimeEffects() override;
	void ClearRuntime() override;
};
