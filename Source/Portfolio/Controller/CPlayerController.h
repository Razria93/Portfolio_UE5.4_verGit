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

private:
	FVector2D CachedMoveAxis2D = FVector2D::ZeroVector;

protected:
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

protected:
	void InputMoveForward(float InAxisValue);
	void InputMoveRight(float InAxisValue);
	
protected:
	void InputLookYaw(float InAxisValue);
	void InputLookPitch(float InAxisValue);

protected:
	void FlushMoveInput();

	void PressWalk();
	void ReleaseWalk();

	void PressJump();
	void ReleaseJump();

	void PressComboAction();

	void PressSword();
};
