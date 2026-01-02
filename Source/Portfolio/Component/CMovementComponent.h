#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMovementComponent.generated.h"

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
	/* === Editor Settings === */
	UPROPERTY(EditAnywhere, Category = "Speed")
	TMap<ESpeedType, float> SpeedMap;

private:
	float CurrentSpeed = 0.f;
	float CurrentDirection = 0.f;

	bool bCanMove = true;
	bool bIsFalling = false;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;
	class UCharacterMovementComponent* CharacterMovementComp_Cached;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Getter === */
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	FORCEINLINE float GetCurrentDirection() const { return CurrentDirection; }

public:
	/* === Setter === */
	FORCEINLINE void SetStop() { bCanMove = false; }
	FORCEINLINE void SetMove() { bCanMove = true; }

public:
	/* === Check / Query === */
	FORCEINLINE bool IsFalling() const { return bIsFalling; }

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