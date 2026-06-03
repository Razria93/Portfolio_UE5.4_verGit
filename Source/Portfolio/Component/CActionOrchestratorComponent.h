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
	class UCHealthComponent* HealthComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Cached = nullptr;

protected:
	void BeginPlay() override;

public:
	FActionRequestResult RequestMovementAction(const FMovementActionRequest& InIncomingRequest);
	FActionRequestResult RequestEquipmentAction(const FEquipmentActionRequest& InIncomingRequest);
	FActionRequestResult RequestCombatAction(const FCombatActionRequest& InIncomingRequest);

private:
	bool CanAcceptActionRequest(EActionRequestRejectReason& OutRejectReason) const;

private:
	bool ResolveEquipmentActionCandidate(const FEquipmentActionRequest& InIncomingRequest, FActionCandidate& OutIncomingCandidate, EActionRequestRejectReason& OutRejectReason) const;
	bool ResolveCombatActionCandidate(const FCombatActionRequest& InIncomingRequest, FActionCandidate& OutIncomingCandidate, EActionRequestRejectReason& OutRejectReason) const;

private:
	FActionRequestResult ExecuteActionCandidate(const FActionCandidate& InIncomingCandidate);

private:
	bool ResolveActionContext(const FActionCandidate& InIncomingCandidate, FActionExecutionContext& OutIncomingContext, EActionRequestRejectReason& OutRejectReason) const;

	// Inner API
	bool ResolveActionData(const FActionDataKey& InIncomingDataKey, FActionData& OutIncomingData) const;
	class UCAction* ResolveActionExecutor(const FActionData& InIncomingData) const;

private:
	FExecutionDecisionQuery BuildDecisionQuery(const FActionExecutionContext& InIncomingContext) const;

	// Inner API
	FExecutionSnapshot BuildSnapshot() const;
	FExecutionParticipant BuildIncomingActionParticipant(const FActionExecutionContext& InIncomingContext) const;
	FExecutionParticipant BuildActiveExecutionParticipant() const;

private:
	FExecutionDecisionResult BuildDecisionResult(const FExecutionDecisionQuery& InQuery, EActionRequestRejectReason& OutRejectReason) const;

private:
	FActionExecutionResult BuildActionExecutionResult(const FActionExecutionContext& InContext, const FExecutionDecisionResult& InDecisionResult, EActionRequestRejectReason InRejectReason) const;

private:
	void ResolveExecutionApplyMode(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const;

	// Inner API
	void ResolveInterventionDirective(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const;

	bool BuildInterventionQuery(const FExecutionDecisionQuery& InQuery, EExecutionStopReason InStopReason, FExecutionInterventionQuery& OutQuery) const;
	bool BuildInterventionDirective(const FExecutionInterventionQuery& InQuery, EExecutionStopSource InStopSource, EExecutionAfterStopAction InAfterStopAction, FExecutionInterventionDirective& OutDirective) const;

private:
	FActionRequestResult DispatchActionDecision(const FActionExecutionResult& InResult);

private:
	EActionRequestResultType ConvertDecisionToResultType(const FActionExecutionResult& InResult) const;
	FActionRequestResult BuildActionRequestResult(EActionRequestResultType InResultType, EActionRequestRejectReason InRejectReason = EActionRequestRejectReason::None) const;

private:
	void PrintActionRequestResult(const FActionRequestResult& InResult) const;
};
