#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMovementComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class ESpeedType : uint8
{
	Walk,
	Run,
	Sprint,
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMovementComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Speed")
	TMap<ESpeedType, float> SpeedMap;	// Init in Blueprint

private:
	class ACharacter* OwnerCharacter_Cached;
	class UCharacterMovementComponent* CharacterMovementComp_Cached;

private:
	float CurrentSpeed = 0.f;
	float CurrentDirection = 0.f;

	bool bCanMove = true;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	FORCEINLINE float GetCurrentDirection() const { return CurrentDirection; }

public:
	void OnMoveForward(float InValue);
	void OnMoveRight(float InValue);
	void OnWalk();
	void OnRun();
	void OnSprint();

private:
	void SetSpeedType(ESpeedType InType);

private:
	void CalculateSpeed();
	void CalculateDirection();
};