#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CDefenseComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCDefenseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCDefenseComponent();

private:
	UPROPERTY(Transient)
	bool bIsGuarding = false;

public:
	FORCEINLINE bool IsGuarding() const { return bIsGuarding; }
	FORCEINLINE void SetGuarding(bool bInGuarding) { bIsGuarding = bInGuarding; }

public:
	void PrintGuardingInfo() const;
};
