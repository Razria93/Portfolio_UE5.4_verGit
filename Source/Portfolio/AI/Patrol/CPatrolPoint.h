#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPatrolPoint.generated.h"

UCLASS()
class PORTFOLIO_API ACPatrolPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPatrolPoint();

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	float ExtraWaitTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bFaceOnArrive = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	float FaceYaw = 0.f;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName PointTag = NAME_None;
};
