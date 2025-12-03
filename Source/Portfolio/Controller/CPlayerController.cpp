#include "Controller/CPlayerController.h"
#include "ProjectGlobal.h"

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("LookYaw", this, &ACPlayerController::OnLookYaw);
	InputComponent->BindAxis("LookPitch", this, &ACPlayerController::OnLookPitch);
}

void ACPlayerController::OnLookYaw(float inAxisValue)
{
	AddYawInput(inAxisValue);
}

void ACPlayerController::OnLookPitch(float inAxisValue)
{
	AddPitchInput(inAxisValue);
}