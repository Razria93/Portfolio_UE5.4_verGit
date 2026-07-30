#include "Controller/CPlayerController.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "Component/CPlayerFeedbackComponent.h"
#if !UE_BUILD_SHIPPING
#include "Core/Debug/CDebugOverlayTargetComponent.h"
#endif
#include "Type/CActionOrchestrationTypes.h"

#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	static constexpr float DebugOverlayTargetTraceDistance = 5000.f;
	static constexpr float DebugOverlayNearestTargetRadius = 1500.f;
	static constexpr float DebugOverlayTargetTraceDebugLifetime = 2.0f;

	TAutoConsoleVariable<int32> CVarDebugOverlayTargetTraceDebug(
		TEXT("Portfolio.DebugOverlay.TargetTraceDebug"),
		0,
		TEXT("Draw and log debug overlay target trace diagnostics."),
		ECVF_Default);

	bool ShouldDebugOverlayTargetTraceDebug()
	{
		return CVarDebugOverlayTargetTraceDebug.GetValueOnGameThread() != 0;
	}

	void DrawDebugOverlayTargetTrace(UWorld* InWorld, const FVector& InStart, const FVector& InEnd, const FHitResult& InHitResult, const FColor& InColor)
	{
		if (!ShouldDebugOverlayTargetTraceDebug() || !IsValid(InWorld)) return;

		DrawDebugLine(InWorld, InStart, InEnd, InColor, false, DebugOverlayTargetTraceDebugLifetime, 0, 2.0f);
		if (InHitResult.bBlockingHit)
		{
			DrawDebugSphere(InWorld, InHitResult.ImpactPoint, 16.0f, 12, InColor, false, DebugOverlayTargetTraceDebugLifetime);
		}
	}

	void LogDebugOverlayTargetTrace(const FString& InSummary)
	{
		if (!ShouldDebugOverlayTargetTraceDebug()) return;

		UE_LOG(LogTemp, Log, TEXT("DebugOverlaySelectTarget %s"), *InSummary);
	}
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

void ACPlayerController::DebugOverlaySelectTarget()
{
#if !UE_BUILD_SHIPPING
	TrySelectDebugOverlayTargetFromView();
#endif
}

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

bool ACPlayerController::TrySelectDebugOverlayTargetFromView()
{
	if (!IsValid(DebugOverlayTargetComponent)) return false;

	ACEnemy* targetEnemy = FindDebugOverlayEnemyFromView();
	if (!IsValid(targetEnemy))
	{
		DebugOverlayTargetComponent->ClearDebugOverlayTarget();
		return false;
	}

	DebugOverlayTargetComponent->SetDebugOverlayTarget(targetEnemy, EDebugOverlayTargetSource::Trace);
	return true;
}

bool ACPlayerController::TrySelectDebugOverlayNearestEnemy()
{
	if (!IsValid(DebugOverlayTargetComponent)) return false;

	ACEnemy* targetEnemy = FindNearestDebugOverlayEnemy();
	if (!IsValid(targetEnemy))
	{
		DebugOverlayTargetComponent->ClearDebugOverlayTarget();
		return false;
	}

	DebugOverlayTargetComponent->SetDebugOverlayTarget(targetEnemy, EDebugOverlayTargetSource::Nearest);
	return true;
}

void ACPlayerController::ClearDebugOverlayTarget()
{
	if (!IsValid(DebugOverlayTargetComponent)) return;

	DebugOverlayTargetComponent->ClearDebugOverlayTarget();
}

ACEnemy* ACPlayerController::FindDebugOverlayEnemyFromView()
{
	UWorld* world = GetWorld();
	if (!IsValid(world)) return nullptr;

	FVector viewLocation = FVector::ZeroVector;
	FRotator viewRotation = FRotator::ZeroRotator;
	GetPlayerViewPoint(viewLocation, viewRotation);

	const FVector traceStart = viewLocation;
	const FVector traceEnd = traceStart + viewRotation.Vector() * DebugOverlayTargetTraceDistance;

	FCollisionQueryParams queryParams(SCENE_QUERY_STAT(DebugOverlayTargetTrace), false);
	if (const APawn* pawn = GetPawn())
	{
		queryParams.AddIgnoredActor(pawn);
	}

	FHitResult hitResult;
	const bool bHit = world->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECC_Visibility, queryParams);
	if (!bHit)
	{
		const FString summary = FString::Printf(TEXT("Miss Distance=%.0f"), DebugOverlayTargetTraceDistance);
		if (IsValid(DebugOverlayTargetComponent))
		{
			DebugOverlayTargetComponent->RecordDebugOverlayTraceSummary(summary);
		}

		DrawDebugOverlayTargetTrace(world, traceStart, traceEnd, hitResult, FColor::Red);
		LogDebugOverlayTargetTrace(summary);
		return nullptr;
	}

	AActor* hitActor = hitResult.GetActor();
	ACEnemy* hitEnemy = Cast<ACEnemy>(hitActor);
	if (!IsValid(hitEnemy))
	{
		const FString summary = FString::Printf(
			TEXT("HitNonEnemy Actor=%s Class=%s"),
			*GetNameSafe(hitActor),
			*GetNameSafe(IsValid(hitActor) ? hitActor->GetClass() : nullptr));
		if (IsValid(DebugOverlayTargetComponent))
		{
			DebugOverlayTargetComponent->RecordDebugOverlayTraceSummary(summary);
		}

		DrawDebugOverlayTargetTrace(world, traceStart, traceEnd, hitResult, FColor::Yellow);
		LogDebugOverlayTargetTrace(summary);
		return nullptr;
	}

	const FString summary = FString::Printf(TEXT("HitEnemy Actor=%s"), *GetNameSafe(hitEnemy));
	if (IsValid(DebugOverlayTargetComponent))
	{
		DebugOverlayTargetComponent->RecordDebugOverlayTraceSummary(summary);
	}

	DrawDebugOverlayTargetTrace(world, traceStart, traceEnd, hitResult, FColor::Green);
	LogDebugOverlayTargetTrace(summary);
	return hitEnemy;
}

ACEnemy* ACPlayerController::FindNearestDebugOverlayEnemy() const
{
	UWorld* world = GetWorld();
	const APawn* pawn = GetPawn();
	if (!IsValid(world) || !IsValid(pawn)) return nullptr;

	const FVector origin = pawn->GetActorLocation();
	const float maxDistanceSquared = FMath::Square(DebugOverlayNearestTargetRadius);

	ACEnemy* nearestEnemy = nullptr;
	float nearestDistanceSquared = maxDistanceSquared;

	for (TActorIterator<ACEnemy> it(world); it; ++it)
	{
		ACEnemy* enemy = *it;
		if (!IsValid(enemy)) continue;

		const float distanceSquared = FVector::DistSquared(origin, enemy->GetActorLocation());
		if (distanceSquared > nearestDistanceSquared) continue;

		nearestDistanceSquared = distanceSquared;
		nearestEnemy = enemy;
	}

	return nearestEnemy;
}

#endif
