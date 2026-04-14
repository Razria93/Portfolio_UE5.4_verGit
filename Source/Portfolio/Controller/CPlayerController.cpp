#include "Controller/CPlayerController.h"
#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"

#include "Component/CPlayerFeedbackComponent.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

ACPlayerController::ACPlayerController()
{
	PlayerFeedbackComponent = CreateDefaultSubobject<UCPlayerFeedbackComponent>(TEXT("PlayerFeedback"));
	check(PlayerFeedbackComponent);
}

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &ACPlayerController::InputMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACPlayerController::InputMoveRight);

	InputComponent->BindAxis("LookYaw", this, &ACPlayerController::InputLookYaw);
	InputComponent->BindAxis("LookPitch", this, &ACPlayerController::InputLookPitch);

	InputComponent->BindAction("Walk", EInputEvent::IE_Pressed, this, &ACPlayerController::PressWalk);
	InputComponent->BindAction("Walk", EInputEvent::IE_Released, this, &ACPlayerController::ReleaseWalk);

	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACPlayerController::PressJump);
	InputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACPlayerController::ReleaseJump);

	InputComponent->BindAction("ComboAction", EInputEvent::IE_Pressed, this, &ACPlayerController::PressComboAction);
	InputComponent->BindAction("Sword", EInputEvent::IE_Pressed, this, &ACPlayerController::PressSword);
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

void ACPlayerController::PressWalk()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleWalk();
}

void ACPlayerController::ReleaseWalk()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleRun();
}

void ACPlayerController::PressJump()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleJump();
}

void ACPlayerController::ReleaseJump()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
		player->HandleStopJump();
}

void ACPlayerController::PressComboAction()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
	{
		player->HandleComboAction();
	}
}

void ACPlayerController::PressSword()
{
	if (ACPlayer* player = Cast<ACPlayer>(GetPawn()))
	{
		player->HandleSword();
	}
}
