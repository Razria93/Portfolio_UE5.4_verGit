#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Type/CWorldSubSystemStructure.h"
#include "CPlayerController.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

private:
	void HandleCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest);

private:
	float ResolveCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest) const;

private:
	void PrintCameraShakeConsumeInfo(const FCameraShakeRequest& InCameraShakeRequest, float InFinalScale) const;
};
