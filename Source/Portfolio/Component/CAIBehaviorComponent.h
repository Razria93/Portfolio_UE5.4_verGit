#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CAIBehaviorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCAIBehaviorComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/* --- Key --- */
	UPROPERTY(EditAnywhere, Category = "Blackboard Key")
	FName AIStateTypeKey = "AIStateType";

	UPROPERTY(EditAnywhere, Category = "Blackboard Key")
	FName TargetActorKey = "TargetActor";

private:
	/* === Injected Objects === */
	class UBlackboardComponent* BlackboardComp_Injected;

public:
	UCAIBehaviorComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	bool Initialize(class UBlackboardComponent* InBlackboardComponent);
};
