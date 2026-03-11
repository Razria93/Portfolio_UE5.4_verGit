#pragma once

#include "CoreMinimal.h"
#include "CWorldSubSystemStructure.generated.h"

UENUM(BlueprintType)
enum class EEngageRole : uint8
{
	None,
	Engage,
	Alert
};

USTRUCT(BlueprintType)
struct FEngageRequestContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class ACAIController* RequestController = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	int TargetPriority = INT_MAX;

	UPROPERTY(Transient)
	float LastSeenTime = 0.f;

	UPROPERTY(Transient)
	float DistanceToTarget = 0.f;

public:
	FEngageRequestContext() = default;
};