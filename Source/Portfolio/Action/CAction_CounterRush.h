#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "Type/CCounterRushTypes.h"
#include "Core/Debug/FCombatCounterRushDebug.h"
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
	FCounterRushTuning ActiveRushTuning;
	FCounterRushPlan RushPlan;
	FCombatCounterRushDebug RushDebug;
	bool bRushMovementActive = false;
	FVector LastPresentationLocation = FVector::ZeroVector;

public:
	void InitializeReferences(const FCharacterComponentReferences& InReferences) override;
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
	bool Start(const FActionData& InData, uint32 InActionRequestSerial = 0) override;
	void CommitStarted();
	void EndCounterAttackPresentation();
	const FCounterRushPlan& GetRushPlan() const { return RushPlan; }
	void Tick(float InDeltaTime) override;

protected:
	void CleanupRuntimeEffects() override;
	void ClearRuntime() override;
};
