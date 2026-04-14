#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACPlayerController();

private:
	UPROPERTY(VisibleAnywhere)
	class UCPlayerFeedbackComponent* PlayerFeedbackComponent = nullptr;

protected:
	virtual void SetupInputComponent() override;

protected:
	void InputMoveForward(float InAxisValue);
	void InputMoveRight(float InAxisValue);

	void InputLookYaw(float InAxisValue);
	void InputLookPitch(float InAxisValue);

	void PressWalk();
	void ReleaseWalk();

	void PressJump();
	void ReleaseJump();

	void PressComboAction();

	void PressSword();
};
