#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;

protected:
	void InputMoveForward(float InAxisValue);
	void InputMoveRight(float InAxisValue);
	
	void InputLookYaw(float InAxisValue);
	void InputLookPitch(float InAxisValue);

	void Press_Walk();
	void Release_Walk();

	void Press_Jump();
	void Release_Jump();
};