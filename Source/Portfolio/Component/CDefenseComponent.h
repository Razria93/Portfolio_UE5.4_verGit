#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CWeaponStructure.h"
#include "Interface/ObservableOverlayPolicy.h"
#include "CDefenseComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCDefenseComponent : public UActorComponent, public IObservableOverlayPolicy
{
	GENERATED_BODY()

public:
	UCDefenseComponent();

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Injected = nullptr;

private:
	UPROPERTY(Transient)
	bool bCanStartGuard = true;

	UPROPERTY(Transient)
	bool bWantsGuarding = false;

	UPROPERTY(Transient)
	bool bIsGuardingPose = false;

	UPROPERTY(Transient)
	bool bCanGuard = false;

	UPROPERTY(Transient)
	bool bCanParry = false;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Query
	FORCEINLINE bool CanStartGuard() const { return bCanStartGuard; }
	FORCEINLINE bool WantsGuarding() const { return bWantsGuarding; }
	FORCEINLINE bool IsGuardingPose() const { return bIsGuardingPose; }
	FORCEINLINE bool CanGuard() const { return bCanGuard; }
	FORCEINLINE bool CanParry() const { return bCanParry; }
	FORCEINLINE bool HasGuardOverlay() const { return bIsGuardingPose || bCanGuard || bCanParry; }
	FORCEINLINE bool HasGuardRuntimeState() const { return !bCanStartGuard || bWantsGuarding || HasGuardOverlay(); }

public:
	// Overlay Policy
	void WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const override;
	bool CanApplyOverlayEvent(const FObservableOverlayEventContext& InContext) const override;
	bool ApplyOverlayEvent(const FObservableOverlayEventContext& InContext) override;
	bool CanApplyOverlayHandling(EObservableOverlayHandling InHandling) const override;
	bool ApplyOverlayHandling(EObservableOverlayHandling InHandling) override;

public:
	// Guard Event Entry
	void HandleGuardInputPressed();
	void HandleGuardInputReleased();

	void HandleGuardInStarted();
	void HandleGuardOutStarted();

	void HandleSwitchToGuard();
	void HandleAllowGuardStart();

	void HandleGuardLifecycleCompleted();
	void HandleGuardLifecycleInterrupted();

public:
	// Guard State Cleanup
	void ClearGuardState();
	void ClearGuardOverlay();
	void RestoreGuardOverlay();

private:
	// Guard State Primitive
	void AllowGuardStart();
	void BlockGuardStart();

	void BeginGuardIntent();
	void EndGuardIntent();

	void BeginGuardPose();
	void EndGuardPose();

	void OpenGuardWindow();
	void CloseGuardWindow();

	void OpenParryWindow();
	void CloseParryWindow();

private:
	// Movement Override
	void ApplyGuardMovementOverride();
	void ClearMovementOverride();

private:
	// Debug
	void PrintGuardStateInfo() const;
};
