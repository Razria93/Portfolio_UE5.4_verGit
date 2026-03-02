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

	UPROPERTY(EditInstanceOnly, Category = "Config")
	EPatrolMode PatrolMode = EPatrolMode::None;

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
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
};
