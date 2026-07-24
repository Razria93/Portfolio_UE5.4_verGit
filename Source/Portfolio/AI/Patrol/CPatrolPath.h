#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Type/CAITypes.h"
#include "CPatrolPath.generated.h"

UCLASS()
class PORTFOLIO_API ACPatrolPath : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPatrolPath();

public:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = "Patrol")
	TArray<class ACPatrolPoint*> PatrolPoints;

public:
	UFUNCTION()
	int32 Num() const { return PatrolPoints.Num(); }

	UFUNCTION()
	bool GetPointData(int32 InIndex, FPatrolPointData& OutPatrolPointData) const;
};
