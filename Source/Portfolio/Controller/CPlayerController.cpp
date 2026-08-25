#include "Controller/CPlayerController.h"

#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"
#include "Component/CMovementComponent.h"
#include "Component/CPlayerFeedbackComponent.h"
#include "Component/CTargetHUDPresenterComponent.h"
#include "Component/CTargetLockAssistComponent.h"
#include "Component/CPlayerTargetSelectionComponent.h"
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

	PlayerTargetSelectionComponent = CreateDefaultSubobject<UCPlayerTargetSelectionComponent>(TEXT("Targeting"));
	check(PlayerTargetSelectionComponent);

	TargetLockAssistComponent = CreateDefaultSubobject<UCTargetLockAssistComponent>(TEXT("TargetLockAssist"));
	check(TargetLockAssistComponent);

	TargetHUDPresenterComponent = CreateDefaultSubobject<UCTargetHUDPresenterComponent>(TEXT("TargetHUDPresenter"));
	check(TargetHUDPresenterComponent);

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

	if (IsValid(PlayerTargetSelectionComponent))
	{
		PlayerTargetSelectionComponent->InitializeReferences(this);
	}

	if (IsValid(TargetLockAssistComponent))
	{
		TargetLockAssistComponent->InitializeReferences(this);
		TargetLockAssistComponent->SetControlledPlayer(ResolveControlledPlayer(this));
	}

	if (IsValid(TargetHUDPresenterComponent))
	{
		TargetHUDPresenterComponent->InitializeReferences(this);
	}

	SynchronizeCombatTargetReferences();
}

void ACPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ClearCombatTargetReferences();

	if (IsValid(TargetLockAssistComponent))
	{
		TargetLockAssistComponent->SetControlledPlayer(Cast<ACPlayer>(InPawn));
	}

	SynchronizeCombatTargetReferences();
	CachedMovementRotationMode = GetControlledPlayerMovementRotationMode();
	RefreshLocomotionGaitInput();
}

void ACPlayerController::OnUnPossess()
{
	ClearCombatTargetReferences();

	if (IsValid(TargetLockAssistComponent))
	{
		TargetLockAssistComponent->ClearControlledPlayer();
	}

	CachedMovementRotationMode = EMovementRotationMode::None;
	bWalkInputHeld = false;
	bSprintInputHeld = false;

	Super::OnUnPossess();
}

void ACPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	const EMovementRotationMode currentRotationMode = GetControlledPlayerMovementRotationMode();
	if (CachedMovementRotationMode != currentRotationMode)
	{
		CachedMovementRotationMode = currentRotationMode;
		RefreshLocomotionGaitInput();
	}

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
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &ACPlayerController::PressSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &ACPlayerController::ReleaseSprint);

	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACPlayerController::PressJump);
	InputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACPlayerController::ReleaseJump);

	InputComponent->BindAction("Sword", EInputEvent::IE_Pressed, this, &ACPlayerController::PressSwordToggle);
	InputComponent->BindAction("ComboAction", EInputEvent::IE_Pressed, this, &ACPlayerController::PressComboAction);
	InputComponent->BindAction("Guard", EInputEvent::IE_Pressed, this, &ACPlayerController::PressGuard);
	InputComponent->BindAction("Guard", EInputEvent::IE_Released, this, &ACPlayerController::ReleaseGuard);
	InputComponent->BindAction("Dodge", EInputEvent::IE_Pressed, this, &ACPlayerController::PressDodge);
	InputComponent->BindAction("Execution", EInputEvent::IE_Pressed, this, &ACPlayerController::PressExecution);

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

// ===== Locomotion Input Dispatch =====

void ACPlayerController::RefreshLocomotionGaitInput()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	player->HandleLocomotionGaitInput(bWalkInputHeld, bSprintInputHeld);
}

EMovementRotationMode ACPlayerController::GetControlledPlayerMovementRotationMode() const
{
	const ACPlayer* player = Cast<ACPlayer>(GetPawn());
	const UCMovementComponent* movementComp = IsValid(player) ? player->GetMovementComp() : nullptr;
	return IsValid(movementComp) ? movementComp->GetCurrentMovementRotationMode() : EMovementRotationMode::None;
}

// ===== Action Input =====

void ACPlayerController::PressWalk()
{
	bWalkInputHeld = true;
	RefreshLocomotionGaitInput();
}

void ACPlayerController::ReleaseWalk()
{
	bWalkInputHeld = false;
	RefreshLocomotionGaitInput();
}

void ACPlayerController::PressSprint()
{
	bSprintInputHeld = true;
	RefreshLocomotionGaitInput();
}

void ACPlayerController::ReleaseSprint()
{
	bSprintInputHeld = false;
	RefreshLocomotionGaitInput();
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

void ACPlayerController::PressExecution()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	if (!IsValid(player)) return;

	player->RequestExecutionForCurrentTarget();
}

// ===== Player Target Selection =====

void ACPlayerController::SynchronizeCombatTargetReferences()
{
	ACPlayer* player = ResolveControlledPlayer(this);
	UCCombatTargetComponent* combatTargetComponent = IsValid(player) ? player->GetCombatTargetComp() : nullptr;

	if (IsValid(PlayerTargetSelectionComponent))
	{
		PlayerTargetSelectionComponent->SetCombatTargetComponent(combatTargetComponent);
	}

	if (IsValid(TargetLockAssistComponent))
	{
		TargetLockAssistComponent->SetCombatTargetComponent(combatTargetComponent);
	}

	if (IsValid(TargetHUDPresenterComponent))
	{
		TargetHUDPresenterComponent->SetCombatTargetComponent(combatTargetComponent);
	}
}

void ACPlayerController::ClearCombatTargetReferences()
{
	if (IsValid(PlayerTargetSelectionComponent))
	{
		PlayerTargetSelectionComponent->SetCombatTargetComponent(nullptr);
	}

	if (IsValid(TargetLockAssistComponent))
	{
		TargetLockAssistComponent->SetCombatTargetComponent(nullptr);
	}

	if (IsValid(TargetHUDPresenterComponent))
	{
		TargetHUDPresenterComponent->SetCombatTargetComponent(nullptr);
	}
}

void ACPlayerController::PressTargetLock()
{
	if (!IsValid(PlayerTargetSelectionComponent)) return;

	PlayerTargetSelectionComponent->ToggleCombatTargetSelection();
}

void ACPlayerController::PressTargetSwitchLeft()
{
	if (!IsValid(PlayerTargetSelectionComponent)) return;

	PlayerTargetSelectionComponent->SelectAdjacentTarget(ETargetSwitchDirection::Left);
}

void ACPlayerController::PressTargetSwitchRight()
{
	if (!IsValid(PlayerTargetSelectionComponent)) return;

	PlayerTargetSelectionComponent->SelectAdjacentTarget(ETargetSwitchDirection::Right);
}
