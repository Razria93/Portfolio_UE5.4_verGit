#include "Controller/CPlayerController.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "Component/CPlayerFeedbackComponent.h"
#if !UE_BUILD_SHIPPING
#include "Core/Debug/CDebugOverlayTargetComponent.h"
#endif
#include "Type/CActionOrchestrationTypes.h"

#include "EngineUtils.h"

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
	DebugOverlayTargetComponent = CreateDefaultSubobject<UCDebugOverlayTargetComponent>(TEXT("DebugOverlayTarget"));
	check(DebugOverlayTargetComponent);
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
	TrySelectDebugOverlayNearestEnemy();
#endif
}

void ACPlayerController::DebugOverlayClearTarget()
{
#if !UE_BUILD_SHIPPING
	ClearDebugOverlayTarget();
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

bool ACPlayerController::TrySelectDebugOverlayNearestEnemy()
{
	if (!IsValid(DebugOverlayTargetComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectNearestTarget Result=TargetComponentMissing"));
		return false;
	}

	if (!IsValid(GetWorld()) || !IsValid(GetPawn()))
	{
		DebugOverlayTargetComponent->ClearDebugOverlayTarget();

		const FString summary = TEXT("NearestFailed | InvalidContext");
		RecordDebugOverlayNearestSelectionResult(summary);
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectNearestTarget Result=InvalidContext"));
		return false;
	}

	float closestDistance = 0.f;
	ACEnemy* closestEnemy = FindClosestDebugOverlayEnemy(closestDistance);
	if (!IsValid(closestEnemy))
	{
		DebugOverlayTargetComponent->ClearDebugOverlayTarget();

		const FString summary = FString::Printf(
			TEXT("NearestFailed | NoEnemy | Radius=%.0f"),
			DebugOverlayNearestTargetRadius);
		RecordDebugOverlayNearestSelectionResult(summary);
		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectNearestTarget Result=NoEnemy Radius=%.0f"), DebugOverlayNearestTargetRadius);
		return false;
	}

	if (closestDistance > DebugOverlayNearestTargetRadius)
	{
		DebugOverlayTargetComponent->ClearDebugOverlayTarget();

		const FString summary = FString::Printf(
			TEXT("NearestFailed | OutOfRange | Closest=%.0f | Radius=%.0f"),
			closestDistance,
			DebugOverlayNearestTargetRadius);
		RecordDebugOverlayNearestSelectionResult(summary);
		UE_LOG(
			LogTemp,
			Log,
			TEXT("DebugOverlaySelectNearestTarget Result=OutOfRange Closest=%.0f Radius=%.0f"),
			closestDistance,
			DebugOverlayNearestTargetRadius);
		return false;
	}

	DebugOverlayTargetComponent->SetDebugOverlayTarget(closestEnemy, EDebugOverlayTargetSource::Nearest);

	const FString summary = FString::Printf(
		TEXT("NearestSelected | Target=%s | Distance=%.0f | Radius=%.0f"),
		*GetNameSafe(closestEnemy),
		closestDistance,
		DebugOverlayNearestTargetRadius);
	RecordDebugOverlayNearestSelectionResult(summary);
	UE_LOG(
		LogTemp,
		Log,
		TEXT("DebugOverlaySelectNearestTarget Result=Selected Target=%s Distance=%.0f Radius=%.0f"),
		*GetNameSafe(closestEnemy),
		closestDistance,
		DebugOverlayNearestTargetRadius);
	return true;
}

void ACPlayerController::ClearDebugOverlayTarget()
{
	if (!IsValid(DebugOverlayTargetComponent)) return;

	DebugOverlayTargetComponent->ClearDebugOverlayTarget();
}

void ACPlayerController::RecordDebugOverlayNearestSelectionResult(const FString& InSummary) const
{
	if (!IsValid(DebugOverlayTargetComponent)) return;

	DebugOverlayTargetComponent->SetDebugOverlaySelectionSummary(InSummary);
}

ACEnemy* ACPlayerController::FindClosestDebugOverlayEnemy(float& OutDistance) const
{
	OutDistance = 0.f;

	UWorld* world = GetWorld();
	const APawn* pawn = GetPawn();
	if (!IsValid(world) || !IsValid(pawn)) return nullptr;

	const FVector origin = pawn->GetActorLocation();

	ACEnemy* closestEnemy = nullptr;
	float closestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<ACEnemy> it(world); it; ++it)
	{
		ACEnemy* enemy = *it;
		if (!IsValid(enemy)) continue;

		const float distanceSquared = FVector::DistSquared(origin, enemy->GetActorLocation());
		if (distanceSquared > closestDistanceSquared) continue;

		closestDistanceSquared = distanceSquared;
		closestEnemy = enemy;
	}

	if (!IsValid(closestEnemy)) return nullptr;

	OutDistance = FMath::Sqrt(closestDistanceSquared);
	return closestEnemy;
}

#endif
