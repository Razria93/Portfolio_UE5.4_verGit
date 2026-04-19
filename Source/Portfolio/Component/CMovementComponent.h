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
	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	TMap<ESpeedType, float> SpeedMap;

private:
	UPROPERTY(Transient)
	float CurrentSpeed = 0.f;
	
	UPROPERTY(Transient)
	float CurrentDirection = 0.f;

	UPROPERTY(Transient)
	bool bCanMove = true;
	
	UPROPERTY(Transient)
	bool bIsFalling = false;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

	UPROPERTY(Transient)
	class UCharacterMovementComponent* CharacterMovementComp_Cached;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Getter === */
	FORCEINLINE bool GetCanMove() const { return bCanMove; }
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

	void OnJump();
	void OnStopJump();

private:
	void SetSpeedType(ESpeedType InType);

private:
	void CalculateSpeed();
	void CalculateDirection();
};