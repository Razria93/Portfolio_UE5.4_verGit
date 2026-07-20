#pragma once

#include "CoreMinimal.h"

class AActor;
class UClass;
class UObject;

class PORTFOLIO_API FComponentReferenceDebug
{
public:
	// Gate
	static bool ShouldAuditComponentReference();

public:
	// Component Reference Diagnostic Hook
	static void RecordComponentReferenceRecoveredForAudit(const AActor* InOwnerActor, const UClass* InComponentClass, const UObject* InResolvedComponent);
};
