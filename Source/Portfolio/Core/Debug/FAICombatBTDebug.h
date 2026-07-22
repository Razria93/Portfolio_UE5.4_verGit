#pragma once

#include "CoreMinimal.h"
#include "Type/CAIStructure.h"
#include "Type/CActionOrchestrationStructure.h"
#include "Type/CWorldSubSystemStructure.h"

class AAIController;
class APawn;

class PORTFOLIO_API FAICombatBTDebug
{
public:
	// Gate
	static bool ShouldAuditAICombatBT();
	static bool ShouldAuditCanMoveDecorator();

public:
	// Can Move Decorator Diagnostic Hook
	static void RecordCanMoveDecoratorResultForAudit(const APawn* InOwnerPawn, bool bInCanMove);

public:
	// AI Context / Engage Assignment Diagnostic Hook
	static void RecordAIContextClearedForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordAIContextEngageAssignmentForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, ECombatRole InCombatRole, bool bInShouldEngage, const TCHAR* InEvent);

public:
	// Engage Context Gate Diagnostic Hook
	static void RecordEngageContextComputedForAudit(const APawn* InOwnerPawn, const FEngageContext& InContext, bool bInCooldownElapsed, bool bInCombatAction, bool bInActiveReaction);
	static void RecordEngageContextRejectedForAudit(const APawn* InOwnerPawn, const FEngageContext& InContext, const TCHAR* InEvent, const TCHAR* InReason);

public:
	// Combat Action Task Diagnostic Hook
	static void RecordCombatActionTaskSucceededForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, ECombatActionIntent InIntent, const FActionRequestResult& InResult, float InCooldown, float InNextActionTime);
	static void RecordCombatActionTaskRejectedForAudit(const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, ECombatActionIntent InIntent, const FActionRequestResult& InResult, const TCHAR* InReason);
};
