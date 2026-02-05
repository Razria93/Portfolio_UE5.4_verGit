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
	UPROPERTY(Transient)
	class UAnimMontage* ActiveMontage_Cached;

	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	bool bInterruptible = false;		// Setted by AnimNotify

	UPROPERTY(Transient)
	bool bCancelable = false;			// Setted by AnimNotify

protected:
	uint32 Serial_CurrentPlay = 0;		// Serial of Current Play Reaction
	uint32 CachedSerial_ActivePlay = 0;	// Cached Serial of Active Play Raction

public:
	virtual void InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent);
	virtual void Tick(float InDeltaTime) {};

public:
	// Validate: Validate Object (Object Validation)
	virtual bool Validate(const FReactionData& InReactionData);

	// Initialize: Initialize State (bIsActive / bInterruptible / bCancelable)
	virtual bool Initialize(const FReactionData& InReactionData);

	// Begin: Execute (Play montage and Bind delegate)
	virtual bool Begin(const FReactionData& InReactionData);

	// Stop: Force Stop (Interrupt / Cancelled)
	virtual void Stop(EReactionStopReason InStopReason, const UCReaction* InNewReaction);

	// End: Clean Up
	virtual void End(bool bInterrupted);

public:
	UAnimMontage* GetActiveMontage() const { return ActiveMontage_Cached; }

public:
	bool IsActive() { return bIsActive; }
	bool IsInterruptibleNow() const { return bInterruptible; }
	bool IsCancelableNow() const { return bCancelable; }

public:
	// To Be Implement for override (this is Minimal)
	virtual bool WantToInterrupt(const FReactionQueryContext& InReactionQueryContext) const { return true; }
	virtual bool WantToCancel(const FReactionQueryContext& InReactionQueryContext) const { return true; }
	virtual bool AllowInterruptionBy(const FReactionQueryContext& InReactionQueryContext) const { return bInterruptible; }
	virtual bool AllowCancelBy(const FReactionQueryContext& InReactionQueryContext) const { return bCancelable; }

public:
	void SetInterruptible(bool bValue) { bInterruptible = bValue; }
	void SetCancelable(bool bValue) { bCancelable = bValue; }

public:
	// PrintInfo API
	void PrintReactionExecutorRuntimeInfo_Public() const;

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial);

private:
	void PrintReactionExecutorRuntimeInfo() const;
	void PrintStopReasonInfo(EReactionStopReason InStopReason, const UCReaction* InNewReaction) const;
};
