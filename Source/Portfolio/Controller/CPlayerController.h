#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACPlayerController();

private:
	// Components
	UPROPERTY(VisibleAnywhere)
	class UCPlayerFeedbackComponent* PlayerFeedbackComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCDebugOverlayFocusComponent* DebugOverlayFocusComponent = nullptr;

	// Cached input
	FVector2D CachedMoveAxis2D = FVector2D::ZeroVector;

public:

	// Debug Overlay Exec
	UFUNCTION(Exec)
	void DebugOverlaySelectNearestTarget();

	UFUNCTION(Exec)
	void DebugOverlaySelectOutlinerTarget(const FString& ActorName);

	UFUNCTION(Exec)
	void DebugOverlaySelectRecentCombatTarget();

	UFUNCTION(Exec)
	void DebugOverlayClearTarget();

protected:
	// Lifecycle
	virtual void PostInitializeComponents() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	// Look Input
	void InputLookYaw(float InAxisValue);
	void InputLookPitch(float InAxisValue);

	// Move Input
	void InputMoveForward(float InAxisValue);
	void InputMoveRight(float InAxisValue);

	// Movement Dispatch
	void FlushMoveInput();

	// Action Input
	void PressWalk();
	void ReleaseWalk();

	void PressJump();
	void ReleaseJump();

	void PressSwordToggle();
	void PressComboAction();
	void PressGuard();
	void ReleaseGuard();
	void PressDodge();
};
