#include "Action/CAction_Execution.h"

#include "Component/CActionComponent.h"
#include "Core/Debug/FActionComponentDebug.h"

#include "GameFramework/Character.h"

// Execution Arbitration

FExecutionDecisionResult UCAction_Execution::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;
	if (!IsValid(OwnerCharacter_Injected) || !IsIncomingActionType(InQuery, EActionType::Execution) || !CanResolveIndependentRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Independent;
	return result;
}

// Execution Notify Bridge

void UCAction_Execution::HandleSpecificNotifyCommand(const EActionNotifyCommand InCommand)
{
	if (InCommand != EActionNotifyCommand::CommitExecution) return;

	if (!IsValid(ActionComp_Injected)
		|| !ActionComp_Injected->TryCommitActiveExecution(this, ActionRequestSerial_Cached))
	{
		FActionComponentDebug::RecordActionNotifyCommandIgnoredForAudit(OwnerCharacter_Injected, this, InCommand, TEXT("ExecutionCommitRejected"));
	}
}
