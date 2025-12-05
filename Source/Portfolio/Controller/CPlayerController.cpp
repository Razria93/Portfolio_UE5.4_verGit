#include "Controller/CPlayerController.h"
#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &ACPlayerController::InputMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACPlayerController::InputMoveRight);

	InputComponent->BindAxis("LookYaw", this, &ACPlayerController::InputLookYaw);
	InputComponent->BindAxis("LookPitch", this, &ACPlayerController::InputLookPitch);

	InputComponent->BindAction("Walk", EInputEvent::IE_Pressed, this, &ACPlayerController::InputWalk);
	InputComponent->BindAction("Walk", EInputEvent::IE_Released, this, &ACPlayerController::InputRun);
}

void ACPlayerController::InputMoveForward(float inAxisValue)
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleMoveForward(inAxisValue);
}

void ACPlayerController::InputMoveRight(float inAxisValue)
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleMoveRight(inAxisValue);
}

void ACPlayerController::InputLookYaw(float inAxisValue)
{
	AddYawInput(inAxisValue);
}

void ACPlayerController::InputLookPitch(float inAxisValue)
{
	AddPitchInput(inAxisValue);
}

void ACPlayerController::InputWalk()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleWalk();
}

void ACPlayerController::InputRun()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleRun();
}
