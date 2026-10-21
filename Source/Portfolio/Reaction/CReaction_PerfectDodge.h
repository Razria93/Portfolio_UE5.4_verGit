#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_PerfectDodge.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_PerfectDodge : public UCReaction
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	class UCDefenseComponent* DefenseComp_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences) override;

	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	bool Start(const FReactionData& InData) override;

	// Intervention
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
	bool AllowIntervention(const FExecutionInterventionQuery& InQuery) const override;

protected:
	void StopMontage(float InBlendOutTime) override;
};
