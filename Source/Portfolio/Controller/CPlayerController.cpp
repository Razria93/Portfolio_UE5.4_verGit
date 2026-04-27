#include "Controller/CPlayerController.h"
#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"

#include "Component/CPlayerFeedbackComponent.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

#include "Type/CActionOrchestrationStructure.h"

ACPlayerController::ACPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerFeedbackComponent = CreateDefaultSubobject<UCPlayerFeedbackComponent>(TEXT("PlayerFeedback"));
	check(PlayerFeedbackComponent);
}

void ACPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	FlushMoveInput();
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
	InputComponent->BindAction("Sword", EInputEvent::IE_Pressed, this, &ACPlayerController::PressSwordToggle);
}

void ACPlayerController::InputLookYaw(float inAxisValue)
{
	AddYawInput(inAxisValue);
}

void ACPlayerController::InputLookPitch(float inAxisValue)
{
	AddPitchInput(inAxisValue);
}

void ACPlayerController::InputMoveForward(float InAxisValue)
{
	CachedMoveAxis2D.Y = InAxisValue;
}

void ACPlayerController::InputMoveRight(float InAxisValue)
{
	CachedMoveAxis2D.X = InAxisValue;
}

void ACPlayerController::FlushMoveInput()
{
	if (CachedMoveAxis2D.IsNearlyZero()) return;

	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleMove(CachedMoveAxis2D);
}

void ACPlayerController::PressWalk()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;
	
	FActionRequestResult result = player->HandleWalk();
}

void ACPlayerController::ReleaseWalk()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleRun();
}

void ACPlayerController::PressJump()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;
	
	FActionRequestResult result = player->HandleJump();
}

void ACPlayerController::ReleaseJump()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;
	
	FActionRequestResult result = player->HandleStopJump();
}

void ACPlayerController::PressSwordToggle()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleEquipmentAction(EEquipmentActionIntent::Toggle);
}

void ACPlayerController::PressComboAction()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;
	
	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::ComboAttack);
}
