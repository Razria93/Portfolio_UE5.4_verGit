#include "Action/CAction_Execution.h"

#include "Component/CExecutionCollaborationComponent.h"
#include "Core/Debug/FActionComponentDebug.h"

#include "GameFramework/Character.h"

// Component Reference

void UCAction_Execution::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	Super::InitializeReferences(InReferences);
	ExecutionCollaborationComp_Injected = InReferences.ExecutionCollaborationComponent;
}

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

// Execution Notify

void UCAction_Execution::HandleSpecificNotifyCommand(const EActionNotifyCommand InCommand)
{
	if (InCommand != EActionNotifyCommand::CommitExecution) return;

	if (!IsValid(ExecutionCollaborationComp_Injected)
		|| !ExecutionCollaborationComp_Injected->HandleSourceExecutionCommit(ActionRequestSerial_Cached, ActiveData_Cached.StandardExecutionDamage))
	{
		FActionComponentDebug::RecordActionNotifyCommandIgnoredForAudit(OwnerCharacter_Injected, this, InCommand, TEXT("ExecutionCommitRejected"));
	}
}
