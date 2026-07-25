#pragma once

#include "CoreMinimal.h"
#include "Type/CActionKeyTypes.h"
#include "Type/CExecutionRuleTypes.h"
#include "CActionDataTypes.generated.h"

// Data / Config

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Key")
	FActionDataKey ActionDataKey = FActionDataKey();

	UPROPERTY(EditAnywhere, Category = "Key")
	TSubclassOf<class UCAction> ActionExecutorKey = nullptr;

	UPROPERTY(EditAnywhere, Category = "Priority")
	int32 Priority = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Data")
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Data")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Data")
	FName StartSectionName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Data")
	bool bCanMove = false;

	UPROPERTY(EditAnywhere, Category = "Intervention|Want")
	TArray<FExecutionInterventionWantRule> WantInterventionRules;

	UPROPERTY(EditAnywhere, Category = "Intervention|Allow")
	TArray<FExecutionInterventionAllowRule> AllowInterventionRules;

public:
	FActionData() = default;

public:
	bool IsValidMinimal() const;
};

// Runtime Context

USTRUCT(BlueprintType)
struct FActionExecutionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionDataKey ActionDataKey = FActionDataKey();

	UPROPERTY(Transient)
	FActionData ActionData = FActionData();

	UPROPERTY(Transient)
	class UCAction* ActionExecutor = nullptr;

public:
	bool IsValidMinimal() const;
};
