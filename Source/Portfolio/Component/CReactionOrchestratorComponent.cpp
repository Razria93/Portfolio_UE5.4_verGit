#include "Component/CReactionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"

#include "Reaction/CReaction.h"

UCReactionOrchestratorComponent::UCReactionOrchestratorComponent()
{
}

void UCReactionOrchestratorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	check(HealthComp_Cached);

	ReactionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCReactionComponent>();
	check(ReactionComp_Cached);
}

FReactionRequestResult UCReactionOrchestratorComponent::RequestReaction(const FDamageReactionRequest& InRequest)
{
	EReactionRequestRejectReason rejectReason = EReactionRequestRejectReason::None;

	if (!CanAcceptReactionRequest(rejectReason))
	{
		FReactionOrchestrationResult result;

		result.Decision = EReactionOrchestrationDecision::Reject;
		result.RejectReason = rejectReason;

		return BuildRequestResult(result);
	}

	FReactionContext context;
	EReactionType reactionType = EReactionType::None;

	if (!ResolveReactionContext(InRequest, context, reactionType, rejectReason))
	{
		FReactionOrchestrationResult result;

		result.Decision = EReactionOrchestrationDecision::Reject;
		result.RejectReason = rejectReason;
		result.ReactionType = reactionType;

		return BuildRequestResult(result);
	}

	FReactionExecutionPolicy policy;
	if (!ResolveReactionPolicy(context, reactionType, policy, rejectReason))
	{
		FReactionOrchestrationResult result;

		result.Decision = EReactionOrchestrationDecision::Reject;
		result.RejectReason = rejectReason;
		result.ReactionType = reactionType;

		return BuildRequestResult(result);
	}

	FReactionOrchestrationQuery query = BuildOrchestrationQuery(InRequest.IntentSource, reactionType, context, policy);
	FReactionOrchestrationResult result = OrchestrateQuery(query);

	if (result.IsAccepted() && !DispatchReactionDecision(result, rejectReason))
	{
		result.Decision = EReactionOrchestrationDecision::Reject;
		result.RejectReason = rejectReason;
	}

	return BuildRequestResult(result);
}

bool UCReactionOrchestratorComponent::CanAcceptReactionRequest(EReactionRequestRejectReason& OutRejectReason) const
{
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Cached))
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidOwner;
		return false;
	}

	if (!IsValid(ReactionComp_Cached) || !IsValid(HealthComp_Cached))
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidComponent;
		return false;
	}

	return true;
}

bool UCReactionOrchestratorComponent::ResolveReactionContext(const FDamageReactionRequest& InRequest, FReactionContext& OutContext, EReactionType& OutType, EReactionRequestRejectReason& OutRejectReason) const
{
	OutContext = FReactionContext();
	OutType = EReactionType::None;
	OutRejectReason = EReactionRequestRejectReason::None;

	if (InRequest.IntentSource != EReactionIntentSource::TakeDamage)
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidRequest;
		return false;
	}

	const FTakeDamageResult& damageResult = InRequest.TakeDamagePacket.Result;

	if (!damageResult.bAccepted)
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidDamageResult;
		return false;
	}

	OutType = ResolveReactionType(damageResult);
	if (OutType == EReactionType::None)
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionTypeNotFound;
		return false;
	}

	FReactionData reactionData;
	if (!ResolveReactionData(damageResult.ApplyDamageSpecKey, OutType, reactionData))
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionDataNotFound;
		return false;
	}

	UCReaction* reactionExecutor = ResolveReactionExecutor(reactionData);
	if (!IsValid(reactionExecutor))
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionExecutorNotFound;
		return false;
	}

	OutContext.ReactionData = reactionData;
	OutContext.ReactionExecutor = reactionExecutor;

	return true;
}

bool UCReactionOrchestratorComponent::ResolveReactionPolicy(const FReactionContext& InContext, EReactionType InType, FReactionExecutionPolicy& OutPolicy, EReactionRequestRejectReason& OutRejectReason) const
{
	OutPolicy = FReactionExecutionPolicy();
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!InContext.IsValidMinimal())
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidRequest;
		return false;
	}

	if (InType == EReactionType::None || InType == EReactionType::All || InType == EReactionType::Max)
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionPolicyNotFound;
		return false;
	}

	OutPolicy.Priority = InContext.ReactionData.Priority;
	OutPolicy.bCanInterrupt = true; // TODO: implement interrupt policy by CReaction

	if (InType == EReactionType::Dead)
	{
		OutPolicy.bCanInterrupt = true;
		OutPolicy.bForceInterrupt = true;
		OutPolicy.bIgnoreInterruptWindow = true;
		OutPolicy.Priority = TNumericLimits<int32>::Max();
	}

	return true;
}

EReactionType UCReactionOrchestratorComponent::ResolveReactionType(const FTakeDamageResult& InTakeDamageResult) const
{
	if (!InTakeDamageResult.bAccepted)
	{
		return EReactionType::None;
	}

	if (InTakeDamageResult.DeadState_Before == EDeadState::Alive && InTakeDamageResult.DeadState_After != EDeadState::Alive)
	{
		return EReactionType::Dead;
	}

	if (InTakeDamageResult.CommittedDamage > KINDA_SMALL_NUMBER && InTakeDamageResult.DeadState_Before == EDeadState::Alive && InTakeDamageResult.DeadState_After == EDeadState::Alive)
	{
		return EReactionType::Hit;
	}

	return EReactionType::None;
}

bool UCReactionOrchestratorComponent::ResolveReactionData(const FApplyDamageSpecKey& InSpecKey, EReactionType InType, FReactionData& OutData) const
{
	OutData = FReactionData();

	if (!IsValid(ReactionComp_Cached)) return false;
	if (InType == EReactionType::None || InType == EReactionType::All || InType == EReactionType::Max) return false;

	// Resolve Data
	return ReactionComp_Cached->ResolveReactionData(InSpecKey, InType, OutData);
}

UCReaction* UCReactionOrchestratorComponent::ResolveReactionExecutor(const FReactionData& InData) const
{
	if (!IsValid(ReactionComp_Cached)) return nullptr;
	if (!InData.IsValidMinimal()) return nullptr;

	// Resolve Executor
	return ReactionComp_Cached->ResolveReactionExecutor(InData);
}

FReactionOrchestrationQuery UCReactionOrchestratorComponent::BuildOrchestrationQuery(EReactionIntentSource InIntentSource, EReactionType InType, const FReactionContext& InContext, const FReactionExecutionPolicy& InPolicy) const
{
	FReactionOrchestrationQuery query;

	query.IntentSource = InIntentSource;
	query.IncomingType = InType;
	query.IncomingPolicy = InPolicy;
	query.IncomingContext = InContext;

	FReactionContext activeContext;
	if (IsValid(ReactionComp_Cached) && ReactionComp_Cached->IsActiveReaction() && ReactionComp_Cached->GetActiveReactionContext(activeContext))
	{
		query.ActiveContext = activeContext;
	}

	return query;
}

FReactionOrchestrationResult UCReactionOrchestratorComponent::OrchestrateQuery(const FReactionOrchestrationQuery& InQuery) const
{
	FReactionOrchestrationResult result;

	result.Decision = EReactionOrchestrationDecision::None;
	result.RejectReason = EReactionRequestRejectReason::None;
	result.ReactionType = InQuery.IncomingType;
	result.ReactionContext = InQuery.IncomingContext;

	if (!InQuery.IncomingContext.IsValidMinimal())
	{
		result.Decision = EReactionOrchestrationDecision::Reject;
		result.RejectReason = EReactionRequestRejectReason::InvalidRequest;
		return result;
	}

	if (!InQuery.ActiveContext.IsValidMinimal())
	{
		result.Decision = EReactionOrchestrationDecision::Start;
		return result;
	}

	EReactionRequestRejectReason rejectReason = EReactionRequestRejectReason::None;
	if (!CanInterruptActiveReaction(InQuery.ActiveContext, InQuery.IncomingContext, InQuery.IncomingPolicy, rejectReason))
	{
		result.Decision = rejectReason == EReactionRequestRejectReason::LowerPriority ? EReactionOrchestrationDecision::Ignore : EReactionOrchestrationDecision::Reject;
		result.RejectReason = rejectReason;
		return result;
	}

	result.Decision = EReactionOrchestrationDecision::Interrupt;
	return result;
}

bool UCReactionOrchestratorComponent::CanInterruptActiveReaction(const FReactionContext& InCurrentContext, const FReactionContext& InIncomingContext, const FReactionExecutionPolicy& InIncomingPolicy, EReactionRequestRejectReason& OutRejectReason) const
{
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!InIncomingContext.IsValidMinimal())
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidRequest;
		return false;
	}

	// Early Return
	if (!InCurrentContext.IsValidMinimal()) return true;

	const FReactionData& currentData = InCurrentContext.ReactionData;
	const FReactionData& incomingData = InIncomingContext.ReactionData;

	UCReaction* currentExecutor = InCurrentContext.ReactionExecutor;
	UCReaction* incomingExecutor = InIncomingContext.ReactionExecutor;

	// Early Return
	if (!IsValid(currentExecutor)) return true;

	if (!IsValid(incomingExecutor))
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionExecutorNotFound;
		return false;
	}

	if (!InIncomingPolicy.bCanInterrupt)
	{
		OutRejectReason = EReactionRequestRejectReason::IncomingCannotInterrupt;
		return false;
	}

	// [Policy] Higher value means higher priority.
	if (InIncomingPolicy.Priority < currentData.Priority)
	{
		OutRejectReason = EReactionRequestRejectReason::LowerPriority;
		return false;
	}

	FReactionQueryContext queryContext;

	queryContext.CurrentReactionExecutor = currentExecutor;
	queryContext.IncomingReactionExecutor = incomingExecutor;
	queryContext.CurrentReactionData = currentData;
	queryContext.IncomingReactionData = incomingData;

	if (!InIncomingPolicy.bIgnoreInterruptWindow && !currentExecutor->AllowInterruptionBy(queryContext))
	{
		OutRejectReason = EReactionRequestRejectReason::CurrentNotInterruptible;
		return false;
	}

	if (!InIncomingPolicy.bForceInterrupt && !incomingExecutor->WantToInterrupt(queryContext))
	{
		OutRejectReason = EReactionRequestRejectReason::IncomingCannotInterrupt;
		return false;
	}

	return true;
}

bool UCReactionOrchestratorComponent::DispatchReactionDecision(const FReactionOrchestrationResult& InResult, EReactionRequestRejectReason& OutRejectReason)
{
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!IsValid(ReactionComp_Cached))
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionDispatchFailed;
		return false;
	}

	if (!InResult.IsAccepted())
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionDispatchFailed;
		return false;
	}

	if (!ReactionComp_Cached->ApplyReactionDecision(InResult))
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionExecutionFailed;
		return false;
	}

	return true;
}

FReactionRequestResult UCReactionOrchestratorComponent::BuildRequestResult(const FReactionOrchestrationResult& InResult) const
{
	FReactionRequestResult result;

	result.RejectReason = InResult.RejectReason;
	result.ResolvedReactionType = InResult.ReactionType;

	switch (InResult.Decision)
	{
	case EReactionOrchestrationDecision::Start:
	{
		result.ResultType = EReactionRequestResultType::Started;
		break;
	}

	case EReactionOrchestrationDecision::Interrupt:
	{
		result.ResultType = EReactionRequestResultType::Interrupted;
		break;
	}

	case EReactionOrchestrationDecision::Ignore:
	{
		result.ResultType = EReactionRequestResultType::Ignored;
		break;
	}

	case EReactionOrchestrationDecision::Reject:
	{
		result.ResultType = EReactionRequestResultType::Rejected;
		break;
	}

	case EReactionOrchestrationDecision::None:
	default:
		result.ResultType = EReactionRequestResultType::None;
		break;
	}

	return result;
}
