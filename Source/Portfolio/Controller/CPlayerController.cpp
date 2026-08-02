#include "Controller/CPlayerController.h"

#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"
#include "Component/CPlayerFeedbackComponent.h"
#if !UE_BUILD_SHIPPING
#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Core/Debug/FDebugOverlayFocusResolver.h"
#endif
#include "Type/CActionOrchestrationTypes.h"

#if !UE_BUILD_SHIPPING
namespace
{
	static constexpr float DebugOverlayNearestTargetRadius = 3000.f;
}
#endif

ACPlayerController::ACPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerFeedbackComponent = CreateDefaultSubobject<UCPlayerFeedbackComponent>(TEXT("PlayerFeedback"));
	check(PlayerFeedbackComponent);

#if !UE_BUILD_SHIPPING
	DebugOverlayFocusComponent = CreateDefaultSubobject<UCDebugOverlayFocusComponent>(TEXT("DebugOverlayTarget"));
	check(DebugOverlayFocusComponent);
#endif
}

// Lifecycle

void ACPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IsValid(PlayerFeedbackComponent))
	{
		PlayerFeedbackComponent->InitializeReferences(this);
	}
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

	InputComponent->BindAction("Sword", EInputEvent::IE_Pressed, this, &ACPlayerController::PressSwordToggle);
	InputComponent->BindAction("ComboAction", EInputEvent::IE_Pressed, this, &ACPlayerController::PressComboAction);
	InputComponent->BindAction("Guard", EInputEvent::IE_Pressed, this, &ACPlayerController::PressGuard);
	InputComponent->BindAction("Guard", EInputEvent::IE_Released, this, &ACPlayerController::ReleaseGuard);
	InputComponent->BindAction("Dodge", EInputEvent::IE_Pressed, this, &ACPlayerController::PressDodge);
}

// Debug Overlay Exec

void ACPlayerController::DebugOverlaySelectNearestTarget()
{
#if !UE_BUILD_SHIPPING
	TryFocusDebugOverlayNearestEnemy();
#endif
}

void ACPlayerController::DebugOverlayClearTarget()
{
#if !UE_BUILD_SHIPPING
	ClearDebugOverlayFocus();
#endif
}

void ACPlayerController::DebugOverlaySelectActorTarget(const FString& ActorName)
{
#if !UE_BUILD_SHIPPING
	TryFocusDebugOverlayActorTarget(ActorName);
#endif
}

// Look Input

void ACPlayerController::InputLookYaw(float InAxisValue)
{
	AddYawInput(InAxisValue);
}

void ACPlayerController::InputLookPitch(float InAxisValue)
{
	AddPitchInput(InAxisValue);
}

// Move Input

void ACPlayerController::InputMoveForward(float InAxisValue)
{
	CachedMoveAxis2D.Y = InAxisValue;
}

void ACPlayerController::InputMoveRight(float InAxisValue)
{
	CachedMoveAxis2D.X = InAxisValue;
}

// Movement Dispatch

void ACPlayerController::FlushMoveInput()
{
	if (CachedMoveAxis2D.IsNearlyZero()) return;

	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleMove(CachedMoveAxis2D);
}

// Action Input

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

void ACPlayerController::PressGuard()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::Guard, EActionIntentEvent::Started);
}

void ACPlayerController::ReleaseGuard()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::Guard, EActionIntentEvent::Completed);
}

void ACPlayerController::PressDodge()
{
	ACPlayer* player = Cast<ACPlayer>(GetPawn());
	if (!IsValid(player)) return;

	FActionRequestResult result = player->HandleCombatAction(ECombatActionIntent::Dodge);
}

#if !UE_BUILD_SHIPPING

bool ACPlayerController::TryFocusDebugOverlayNearestEnemy()
{
	if (!IsValid(DebugOverlayFocusComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectNearestTarget Result: TargetComponentMissing"));
		return false;
	}

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveNearestEnemy(
		GetWorld(),
		GetPawn(),
		DebugOverlayNearestTargetRadius);

	ApplyDebugOverlayFocusResolveResult(result);

	switch (result.Status)
	{
	case EDebugOverlayFocusResolveStatus::InvalidContext:
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectNearestTarget Result: InvalidContext"));
		return false;
	case EDebugOverlayFocusResolveStatus::NoEnemy:
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectNearestTarget Result: NoEnemy | Radius: %.0f"), result.Radius);
		return false;
	case EDebugOverlayFocusResolveStatus::OutOfRange:
		UE_LOG(
			LogTemp,
			Log,
			TEXT("DebugOverlaySelectNearestTarget Result: OutOfRange | Closest: %.0f | Radius: %.0f"),
			result.Distance,
			result.Radius);
		return false;
	case EDebugOverlayFocusResolveStatus::Selected:
		UE_LOG(
			LogTemp,
			Log,
			TEXT("DebugOverlaySelectNearestTarget Result: Selected | Target: %s | Distance: %.0f | Radius: %.0f"),
			*result.ActorName,
			result.Distance,
			result.Radius);
		return true;
	default:
		return false;
	}
}

bool ACPlayerController::TryFocusDebugOverlayActorTarget(const FString& InActorName)
{
	const FString actorName = InActorName.TrimStartAndEnd();

	if (!IsValid(DebugOverlayFocusComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectActorTarget Result: TargetComponentMissing | Name: %s"), *actorName);
		return false;
	}

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveActorEnemy(
		GetWorld(),
		GetPawn(),
		actorName);

	ApplyDebugOverlayFocusResolveResult(result);

	switch (result.Status)
	{
	case EDebugOverlayFocusResolveStatus::InvalidContext:
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectActorTarget Result: InvalidContext | Name: %s"), *result.ActorName);
		return false;
	case EDebugOverlayFocusResolveStatus::NoActorName:
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectActorTarget Result: NoActorName"));
		return false;
	case EDebugOverlayFocusResolveStatus::NoActor:
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectActorTarget Result: NoActor | Name: %s"), *result.ActorName);
		return false;
	case EDebugOverlayFocusResolveStatus::NotEnemy:
		UE_LOG(
			LogTemp,
			Log,
			TEXT("DebugOverlaySelectActorTarget Result: NotEnemy | Target: %s | Class: %s"),
			*result.ActorName,
			*result.ClassName);
		return false;
	case EDebugOverlayFocusResolveStatus::Selected:
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectActorTarget Result: Selected | Target: %s"), *result.ActorName);
		return true;
	default:
		return false;
	}
}

void ACPlayerController::ClearDebugOverlayFocus()
{
	if (!IsValid(DebugOverlayFocusComponent)) return;

	DebugOverlayFocusComponent->ClearDebugOverlayFocus();
	DebugOverlayFocusComponent->ClearDebugOverlayFocusCommandResult();
}

void ACPlayerController::ApplyDebugOverlayFocusResolveResult(const FDebugOverlayFocusResolveResult& InResult) const
{
	if (!IsValid(DebugOverlayFocusComponent)) return;

	if (InResult.Status == EDebugOverlayFocusResolveStatus::Selected)
	{
		DebugOverlayFocusComponent->SetDebugOverlayFocus(InResult.FocusActor.Get(), InResult.FocusSource);
	}
	else
	{
		DebugOverlayFocusComponent->ClearDebugOverlayFocus();
	}

	RecordDebugOverlayFocusCommandResult(InResult.SummaryText);
}

void ACPlayerController::RecordDebugOverlayFocusCommandResult(const FString& InSummary) const
{
	if (!IsValid(DebugOverlayFocusComponent)) return;

	DebugOverlayFocusComponent->SetDebugOverlayFocusCommandResult(InSummary);
}

#endif
