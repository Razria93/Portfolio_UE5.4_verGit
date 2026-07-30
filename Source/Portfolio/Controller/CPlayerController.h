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

public:
	// Debug Overlay Exec
	UFUNCTION(Exec)
	void DebugOverlaySelectTarget();

	UFUNCTION(Exec)
	void DebugOverlaySelectNearestTarget();

	UFUNCTION(Exec)
	void DebugOverlayClearTarget();

private:
	UPROPERTY(VisibleAnywhere)
	class UCPlayerFeedbackComponent* PlayerFeedbackComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCDebugOverlayTargetComponent* DebugOverlayTargetComponent = nullptr;

private:
	FVector2D CachedMoveAxis2D = FVector2D::ZeroVector;

protected:
	// Lifecycle
	virtual void PostInitializeComponents() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

protected:
	// Look Input
	void InputLookYaw(float InAxisValue);
	void InputLookPitch(float InAxisValue);

protected:
	// Move Input
	void InputMoveForward(float InAxisValue);
	void InputMoveRight(float InAxisValue);

protected:
	// Movement Dispatch
	void FlushMoveInput();

protected:
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

#if !UE_BUILD_SHIPPING
private:
	// Debug Overlay Target
	bool TrySelectDebugOverlayTargetFromView();
	bool TrySelectDebugOverlayNearestEnemy();
	void ClearDebugOverlayTarget();

	class ACEnemy* FindDebugOverlayEnemyFromView();
	class ACEnemy* FindNearestDebugOverlayEnemy() const;
#endif
};
