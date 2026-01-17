#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "CReaction.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCReaction : public UObject
{
	GENERATED_BODY()

protected:
	/* === Owner Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected;

	UPROPERTY(Transient)
	class UCReactionComponent* OwnerReactionComp_Injected;

protected:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveMontage_Cached = nullptr;

	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	bool bInterruptible = false;		// Setted by AnimNotify

	UPROPERTY(Transient)
	bool bCancelable = false;		// Setted by AnimNotify

protected:
	uint32 Serial_CurrentPlay = 0;		// Serial of Current Play Reaction
	uint32 CachedSerial_ActivePlay = 0;	// Cached Serial of Active Play Raction

public:
	virtual void InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent);
	virtual void Tick(float InDeltaTime) {};

public:
	// Begin: Execute (Play montage and Bind delegate)
	virtual bool Begin(const FReactionData& reactionData);

	// Stop: Force Stop (Interrupt / Cancelled)
	virtual void Stop(EReactionStopReason InStopReason, const UCReaction* InNewReaction);

	// End: Clean Up
	virtual void End(bool bInterrupted);

public:
	UAnimMontage* GetActiveMontage() const { return ActiveMontage_Cached; }

public:
	bool IsActive() { return bIsActive; }

public:
	// To Be Implement for override (Must be Implement)
	virtual bool CanInterrupt() const { return true; }
	virtual bool CanCancel() const { return true; }

public:
	virtual bool CanBeInterrupted() const { return bInterruptible; }
	virtual bool CanBeCanceled() const { return bCancelable; }

public:
	void SetIsActive(bool bValue) { bIsActive = bValue; }
	void SetInterruptible(bool bValue) { bInterruptible = bValue; }
	void SetCancelable(bool bValue) { bCancelable = bValue; }

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial);

private:
	void UpdateMovementToImmovable();
	void UpdateStateToReaction();

public:
	void PrintReactionExecutorRuntimeInfo() const;
};
