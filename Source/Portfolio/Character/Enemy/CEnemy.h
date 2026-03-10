#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Type/CWeaponStructure.h"
#include "Type/CAIStructure.h"
#include "CEnemy.generated.h"

UCLASS()
class PORTFOLIO_API ACEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ACEnemy();

private:
	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	bool bUsePatrol;

	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	class ACPatrolPath* PatrolPath;

	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	EPatrolMode PatrolMode = EPatrolMode::None;

private:
	UPROPERTY(EditInstanceOnly, Category = "AI|Investigate")
	bool bUseInvestigate;

	UPROPERTY(EditInstanceOnly, Category = "AI|Investigate")
	float InvestigateDuration;

	UPROPERTY(EditInstanceOnly, Category = "AI|Investigate")
	int32 InvestigateMaxIndex;

private:
	UPROPERTY(EditInstanceOnly, Category = "AI|Chase")
	float ChaseOffsetRange;

	UPROPERTY(EditInstanceOnly, Category = "AI|Chase")
	float ChaseEnterBuffer;

	UPROPERTY(EditInstanceOnly, Category = "AI|Chase")
	float ChaseExitBuffer;

private:
	UPROPERTY(EditAnywhere, Category = "AI|Alert")
	bool bUseAlertStep;

	UPROPERTY(EditAnywhere, Category = "AI|Alert")
	float StepSideDistance;

	UPROPERTY(EditAnywhere, Category = "AI|Alert")
	float StepForwardDistance;

private:
	UPROPERTY(VisibleAnywhere)
	class UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere)
	class UCTakeDamageComponent* TakeDamageComponent;

	UPROPERTY(VisibleAnywhere)
	class UCHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere)
	class UCReactionComponent* ReactionComponent;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE bool GetbUsePatrol() const { return bUsePatrol; }
	FORCEINLINE ACPatrolPath* GetPatrolPath() const { return PatrolPath; }
	FORCEINLINE EPatrolMode GetPatrolMode() const { return PatrolMode; }

public:
	FORCEINLINE bool GetbUseInvestigate() const { return bUseInvestigate; }
	FORCEINLINE float GetInvestigateDuration() const { return InvestigateDuration; }
	FORCEINLINE int32 GetInvestigateMaxIndex() const { return InvestigateMaxIndex; }

public:
	FORCEINLINE float GetChaseOffsetDistance() const { return ChaseOffsetRange; }
	FORCEINLINE float GetChaseEnterBuffer() const { return ChaseEnterBuffer; }
	FORCEINLINE float GetChaseExitBuffer() const { return ChaseExitBuffer; }

public:
	FORCEINLINE bool GetbUseAlertStep() const { return bUseAlertStep; }
	FORCEINLINE float GetStepSideDistance() const { return StepSideDistance; }
	FORCEINLINE float GetStepForwardDistance() const { return StepForwardDistance; }

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
};
