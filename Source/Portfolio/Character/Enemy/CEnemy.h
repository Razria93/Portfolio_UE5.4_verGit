#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Type/CWeaponStructure.h"
#include "CEnemy.generated.h"

UCLASS()
class PORTFOLIO_API ACEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ACEnemy();

private:
	UPROPERTY(VisibleAnywhere)
	class UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere)
	class UCTakeDamageComponent* TakeDamageComponent;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
};
