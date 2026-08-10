#include "Controller/CPlayerController.h"

#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"
#include "Component/CPlayerFeedbackComponent.h"
#include "Component/CTargetLockAssistComponent.h"
#include "Component/CTargetingComponent.h"
#if !UE_BUILD_SHIPPING
#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Core/Debug/FDebugOverlayFocusRuntimeHelper.h"
#endif
#include "Type/CActionOrchestrationTypes.h"

namespace
{
	// Runtime Pawn Helper
	ACPlayer* ResolveControlledPlayer(APlayerController* InController)
	{
		if (!IsValid(InController)) return nullptr;
		return Cast<ACPlayer>(InController->GetPawn());
	}
}

ACPlayerController::ACPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerFeedbackComponent = CreateDefaultSubobject<UCPlayerFeedbackComponent>(TEXT("PlayerFeedback"));
	check(PlayerFeedbackComponent);

	TargetingComponent = CreateDefaultSubobject<UCTargetingComponent>(TEXT("Targeting"));
	check(TargetingComponent);

	TargetLockAssistComponent = CreateDefaultSubobject<UCTargetLockAssistComponent>(TEXT("TargetLockAssist"));
	check(TargetLockAssistComponent);

#if !UE_BUILD_SHIPPING
	DebugOverlayFocusComponent = CreateDefaultSubobject<UCDebugOverlayFocusComponent>(TEXT("DebugOverlayFocus"));
	check(DebugOverlayFocusComponent);
#endif
}

// ===== Debug Overlay Exec =====

void ACPlayerController::DebugOverlaySelectNearestFocus()
{
#if !UE_BUILD_SHIPPING
	FDebugOverlayFocusRuntimeHelper::TryFocusNearestFocus(
		DebugOverlayFocusComponent,
		GetWorld(),
		GetPawn(),
		FDebugOverlayFocusRuntimeHelper::GetNearestFocusRadius());
#endif
}

void ACPlayerController::DebugOverlaySelectOutlinerFocus(const FString& ActorName)
{
#if !UE_BUILD_SHIPPING
	FDebugOverlayFocusRuntimeHelper::TryFocusOutlinerFocus(
		DebugOverlayFocusComponent,
		GetWorld(),
		GetPawn(),
		ActorName);
#endif
}

void ACPlayerController::DebugOverlaySelectRecentCombatFocus()
{
#if !UE_BUILD_SHIPPING
	FDebugOverlayFocusRuntimeHelper::TryFocusRecentCombatFocus(
		DebugOverlayFocusComponent,
		GetWorld(),
		GetPawn(),
		FDebugOverlayFocusRuntimeHelper::GetNearestFocusRadius());
#endif
}

void ACPlayerController::DebugOverlaySelectPlayerTargetFocus()
{
#if !UE_BUILD_SHIPPING
	FDebugOverlayFocusRuntimeHelper::TryFocusPlayerTarget(DebugOverlayFocusComponent, this);
#endif
}

void ACPlayerController::DebugOverlayClearFocus()
{
#if !UE_BUILD_SHIPPING
	FDebugOverlayFocusRuntimeHelper::ClearFocus(DebugOverlayFocusComponent);
#endif
}

// ===== Lifecycle =====

void ACPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IsValid(PlayerFeedbackComponent))
	{
		PlayerFeedbackComponent->InitializeReferences(this);
	}

	if (IsValid(TargetingComponent))
	{
		TargetingComponent->InitializeReferences(this);
	}

	if (IsValid(TargetLockAssistComponent))
	{
		TargetLockAssistComponent->InitializeReferences(this, TargetingComponent);
		TargetLockAssistComponent->SetControlledPlayer(ResolveControlledPlayer(this));
	}
}

void ACPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsValid(TargetLockAssistComponent)) return;

	TargetLockAssistComponent->SetControlledPlayer(Cast<ACPlayer>(InPawn));
}

void ACPlayerController::OnUnPossess()
{
	if (IsValid(TargetLockAssistComponent))
	{
		TargetLockAssistComponent->ClearControlledPlayer();
	}

	Super::OnUnPossess();
}

void ACPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	FlushMoveInput();

#if !UE_BUILD_SHIPPING
	FDebugOverlayFocusRuntimeHelper::UpdateFocusRecentCombatFocus(
		DebugOverlayFocusComponent,
		GetWorld(),
		GetPawn(),
		FDebugOverlayFocusRuntimeHelper::GetNearestFocusRadius());
	FDebugOverlayFocusRuntimeHelper::UpdateFocusPlayerTarget(DebugOverlayFocusComponent, this);
#endif
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

	InputComponent->BindAction("Sword", EInputEvent::IE_Pressed, this, &ACPlayerController::PressSwordToggle);
	InputComponent->BindAction("ComboAction", EInputEvent::IE_Pressed, this, &ACPlayerController::PressComboAction);
	InputComponent->BindAction("Guard", EInputEvent::IE_Pressed, this, &ACPlayerController::PressGuard);
	InputComponent->BindAction("Guard", EInputEvent::IE_Released, this, &ACPlayerController::ReleaseGuard);
	InputComponent->BindAction("Dodge", EInputEvent::IE_Pressed, this, &ACPlayerController::PressDodge);

	InputComponent->BindAction("TargetLock", EInputEvent::IE_Pressed, this, &ACPlayerController::PressTargetLock);
	InputComponent->BindAction("TargetSwitchLeft", EInputEvent::IE_Pressed, this, &ACPlayerController::PressTargetSwitchLeft);
	InputComponent->BindAction("TargetSwitchRight", EInputEvent::IE_Pressed, this, &ACPlayerController::PressTargetSwitchRight);
}

// ===== Look Input =====

void ACPlayerController::InputLookYaw(float InAxisValue)
{
	if (IsValid(TargetLockAssistComponent) && TargetLockAssistComponent->ShouldSuppressLookInput()) return;

	AddYawInput(InAxisValue);
}

void ACPlayerController::InputLookPitch(float InAxisValue)
{
	if (IsValid(TargetLockAssistComponent) && TargetLockAssistComponent->ShouldSuppressLookInput()) return;

	AddPitchInput(InAxisValue);
}

// ===== Move Input =====

void ACPlayerController::InputMoveForward(float InAxisValue)
{
	CachedMoveAxis2D.Y = InAxisValue;
}

void ACPlayerController::InputMoveRight(float InAxisValue)
{
	CachedMoveAxis2D.X = InAxisValue;
}

// ===== Movement Dispatch =====

void ACPlayerController::FlushMoveInput()
{
	if (CachedMoveAxis2D.IsNearlyZero()) return;

	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleMove(CachedMoveAxis2D);
}

// ===== Action Input =====

void ACPlayerController::PressWalk()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleWalk();
}

void ACPlayerController::ReleaseWalk()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleRun();
}

void ACPlayerController::PressJump()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleJump();
}

void ACPlayerController::ReleaseJump()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleStopJump();
}

void ACPlayerController::PressSwordToggle()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleEquipmentAction(EEquipmentActionIntent::Toggle);
}

void ACPlayerController::PressComboAction()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::ComboAttack);
}

void ACPlayerController::PressGuard()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::Guard, EActionIntentEvent::Started);
}

void ACPlayerController::ReleaseGuard()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::Guard, EActionIntentEvent::Completed);
}

void ACPlayerController::PressDodge()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::Dodge);
}

// ===== Targeting =====

void ACPlayerController::PressTargetLock()
{
	if (!IsValid(TargetingComponent)) return;

	TargetingComponent->ToggleTargetLock();
}

void ACPlayerController::PressTargetSwitchLeft()
{
	if (!IsValid(TargetingComponent)) return;

	TargetingComponent->SwitchTarget(ETargetSwitchDirection::Left);
}

void ACPlayerController::PressTargetSwitchRight()
{
	if (!IsValid(TargetingComponent)) return;

	TargetingComponent->SwitchTarget(ETargetSwitchDirection::Right);
}
