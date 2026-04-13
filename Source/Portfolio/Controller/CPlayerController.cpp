#include "Controller/CPlayerController.h"
#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

void ACPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* world = GetWorld())
	{
		if (UCWorldSubsystem_CombatFeedback* feedbackSubsystem = world->GetSubsystem<UCWorldSubsystem_CombatFeedback>())
		{
			feedbackSubsystem->OnCameraShakeRequested.AddUObject(this, &ACPlayerController::HandleCameraShakeRequest);
		}
	}
}

void ACPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* world = GetWorld())
	{
		if (UCWorldSubsystem_CombatFeedback* feedbackSubsystem = world->GetSubsystem<UCWorldSubsystem_CombatFeedback>())
		{
			feedbackSubsystem->OnCameraShakeRequested.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
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

void ACPlayerController::HandleCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest)
{
	// [NOTE] Only the local player controller.
	if (!IsLocalController()) return;
	if (!IsValid(InCameraShakeRequest.CameraShakeClass)) return;

	const float finalScale = ResolveCameraShakeRequest(InCameraShakeRequest);
	if (finalScale <= KINDA_SMALL_NUMBER) return;

	FLog::Log(TEXT("[ACPlayerController] Handle CameraShakeRequest"));
	PrintCameraShakeConsumeInfo(InCameraShakeRequest, finalScale);

	if (!IsValid(PlayerCameraManager))
	{
		FLog::Log(TEXT("[ACPlayerController] Invalid PlayerCameraManager."));
		return;
	}

	PlayerCameraManager->StartCameraShake(InCameraShakeRequest.CameraShakeClass, finalScale);
}

float ACPlayerController::ResolveCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest) const
{
	APawn* pawn = GetPawn();
	if (!IsValid(pawn))
	{
		// return Default
		return InCameraShakeRequest.CameraShakeBaseScale;
	}

	float policyMultiplier = 1.f;

	// [Policy] player hit = stronger, enemy hit = weaker
	if (IsValid(InCameraShakeRequest.TargetActor) && InCameraShakeRequest.TargetActor == pawn)
	{
		// [Hit] Enemy -> Player
		policyMultiplier = 1.0f;
	}
	else
	{
		// [Hit] Player -> Enemy
		policyMultiplier = 1.0f;
	}

	return InCameraShakeRequest.CameraShakeBaseScale * policyMultiplier;
}

void ACPlayerController::PrintCameraShakeConsumeInfo(const FCameraShakeRequest& InCameraShakeRequest, float InFinalScale) const
{
	FLog::Log(TEXT("====== CameraShake Consume ======"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CameraShakeClass"), *GetNameSafe(InCameraShakeRequest.CameraShakeClass)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InCameraShakeRequest.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.2f"), TEXT("FinalScale"), InFinalScale));
	FLog::Log(TEXT("================================="));
}
