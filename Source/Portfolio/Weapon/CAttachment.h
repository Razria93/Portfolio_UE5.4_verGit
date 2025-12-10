#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAttachment.generated.h"

UCLASS()
class PORTFOLIO_API ACAttachment : public AActor
{
	GENERATED_BODY()

public:
	ACAttachment();

protected:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* Root;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
