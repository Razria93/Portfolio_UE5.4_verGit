#include "Controller/CPlayerController.h"
#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &ACPlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACPlayerController::OnMoveRight);

	InputComponent->BindAxis("LookYaw", this, &ACPlayerController::OnLookYaw);
	InputComponent->BindAxis("LookPitch", this, &ACPlayerController::OnLookPitch);
}

void ACPlayerController::OnMoveForward(float inAxisValue)
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleMoveForward(inAxisValue);
}

void ACPlayerController::OnMoveRight(float inAxisValue)
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleMoveRight(inAxisValue);
}

void ACPlayerController::OnLookYaw(float inAxisValue)
{
	AddYawInput(inAxisValue);
}

void ACPlayerController::OnLookPitch(float inAxisValue)
{
	AddPitchInput(inAxisValue);
}