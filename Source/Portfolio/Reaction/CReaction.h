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
	bool bInterruptibleNow = false;		// Setted by AnimNotify

	UPROPERTY(Transient)
	bool bCancelableNow = false;		// Setted by AnimNotify

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
	void SetInterruptibleNow(bool bValue) { bInterruptibleNow = bValue; }
	void SetCancelableNow(bool bValue) { bCancelableNow = bValue; }

public:
	bool IsActive() const { return bIsActive; }

public:
	virtual bool CanInterrupt() const { return true; }
	virtual bool CanBeInterrupted(const UCReaction* newReaction) const { return bInterruptibleNow; }
	virtual bool CanBeCanceled(const UCReaction* newReaction) const { return bCancelableNow; }

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* montage, bool bInterrupted, uint32 InSerial);

private:
	void ChangeMovementToImmovable(bool bCanMove);
	void ChangeStateToReaction();

public:
	void PrintReactionExecutorRuntimeInfo() const;
};
