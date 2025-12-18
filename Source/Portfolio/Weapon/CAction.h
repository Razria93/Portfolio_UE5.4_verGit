#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "CAction.generated.h"

UCLASS()
class PORTFOLIO_API UCAction : public UObject
{
	GENERATED_BODY()

protected:
	// Dependency Injection
	class ACharacter* OwnerCharacter_Injected;
	FActionData ActionData_Injected;

protected:
	// Cached
	class UCStateComponent* StateComp_Cached;

public:
	virtual void InitializeAction(ACharacter* InOwnerCharacter, FActionData InFActionData);
};
