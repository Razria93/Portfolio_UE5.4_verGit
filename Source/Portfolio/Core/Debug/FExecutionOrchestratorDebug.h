#pragma once

#include "CoreMinimal.h"
#include "Type/CActionOrchestrationTypes.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "Type/CExecutionTypes.h"

class PORTFOLIO_API FExecutionOrchestratorDebug
{
public:
	// Gate
	static bool ShouldAuditActionRequest();
	static bool ShouldAuditReactionRequest();
	static bool ShouldPrintExecutionOrchestratorDebug();

public:
	// Shared Diagnostic Hook
	static void RecordInvalidActiveParticipantsForAudit(const AActor* InOwnerActor, const TCHAR* InSystem);

public:
	// Action Diagnostic Hook
	static void RecordActionExecutionResultForAudit(const AActor* InOwnerActor, const FActionExecutionResult& InResult, const TCHAR* InEvent);
	static void RecordActionRequestResultForAudit(const AActor* InOwnerActor, const FActionRequestResult& InResult, const TCHAR* InEvent);

public:
	// Action Debug Dump
	static void PrintActionExecutionDebug(const AActor* InOwnerActor, const FExecutionDecisionQuery& InQuery, const FActionExecutionResult& InResult);

public:
	// Reaction Diagnostic Hook
	static void RecordReactionExecutionResultForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent);
	static void RecordReactionRequestResultForAudit(const AActor* InOwnerActor, const FReactionRequestResult& InResult, const TCHAR* InEvent);

public:
	// Reaction Debug Dump
	static void PrintReactionExecutionDebug(const AActor* InOwnerActor, const FExecutionDecisionQuery& InQuery, const FReactionExecutionResult& InResult);
};
