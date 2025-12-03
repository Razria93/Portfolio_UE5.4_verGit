#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPlayer.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayer : public ACharacter
{
	GENERATED_BODY()

  public:
	ACPlayer();

  private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UCameraComponent* CameraComp;

protected:
	virtual void BeginPlay() override;

  public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
