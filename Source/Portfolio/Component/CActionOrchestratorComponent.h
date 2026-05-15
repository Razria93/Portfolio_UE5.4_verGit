#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CActionOrchestratorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCActionOrchestratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCActionOrchestratorComponent();

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

protected:
	void BeginPlay() override;

public:
	FActionRequestResult RequestMovementAction(const FMovementActionRequest& InRequest);
	FActionRequestResult RequestEquipmentAction(const FEquipmentActionRequest& InRequest);
	FActionRequestResult RequestCombatAction(const FCombatActionRequest& InRequest);

private:
	bool CanAcceptActionRequest(EActionRequestRejectReason& OutRejectReason) const;

private:
	bool ResolveEquipmentActionCandidate(const FEquipmentActionRequest& InRequest, FActionCandidate& OutCandidate, EActionRequestRejectReason& OutRejectReason) const;
	bool ResolveCombatActionCandidate(const FCombatActionRequest& InRequest, FActionCandidate& OutCandidate, EActionRequestRejectReason& OutRejectReason) const;

private:
	FActionRequestResult ExecuteActionCandidate(EActionIntentSource InSource, const FActionCandidate& InCandidate);

private:
	bool ResolveActionContext(const FActionCandidate& InCandidate, FActionResolvedContext& OutContext, EActionRequestRejectReason& OutRejectReason) const;
	bool ResolveActionData(const FActionDataKey& InDataKey, FActionData& OutData) const;
	class UCAction* ResolveActionExecutor(const FActionData& InData) const;

private:
	FActionLocalLevelQuery BuildLocalLevelQuery(const FActionResolvedContext& InIncoming) const;
	FActionLocalLevelResult ResolveLocalLevelResult(const FActionLocalLevelQuery& InLocalQuery) const;

private:
	bool ResolveActionPolicy(const FActionLocalLevelQuery& InLocalQuery, const FActionLocalLevelResult& InLocalResult, FActionResolvedPolicy& OutPolicy, EActionRequestRejectReason& OutRejectReason) const;

private:
	FActionOrchestrationLevelQuery BuildOrchestrationLevelQuery(EActionIntentSource InSource, const FActionLocalLevelQuery& InLocalQuery, const FActionLocalLevelResult& InLocalResult, const FActionResolvedPolicy& InPolicy) const;
	FActionOrchestrationLevelResult ResolveOrchestrationLevelResult(const FActionOrchestrationLevelQuery& InOrchestrationQuery) const;

private:
	void ResolveExecutionInterventionDirective(FActionOrchestrationLevelResult& InOutResult) const;

private:
	FActionRequestResult DispatchActionDecision(const FActionOrchestrationLevelResult& InResult);

private:
	EActionRequestResultType ConvertDecisionToResultType(EActionOrchestrationLevelDecision InDecision) const;
	FActionRequestResult BuildActionRequestResult(EActionRequestResultType InResultType, EActionRequestRejectReason InRejectReason = EActionRequestRejectReason::None) const;
};
