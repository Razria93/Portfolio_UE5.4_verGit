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
	void OnLookYaw(float InAxisValue);
	void OnLookPitch(float InAxisValue);
};
