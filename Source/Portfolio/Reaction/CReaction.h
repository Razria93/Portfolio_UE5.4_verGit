#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "Type/CReactionFeedbackStructure.h"
#include "CReaction.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCReaction : public UObject
{
	GENERATED_BODY()

protected:
	/* === Runtime State === */
	UPROPERTY(Transient)
	bool bIsReaction = false;

	UPROPERTY(Transient)
	bool bInterruptible = false;	// Setted by AnimNotify

	UPROPERTY(Transient)
	bool bCancelable = false;		// Setted by AnimNotify

	UPROPERTY(Transient)
	FReactionData ActiveReactionData_Cached = FReactionData();

	UPROPERTY(Transient)
	class UAnimMontage* ActiveReactionMontage_Cached = nullptr;

	UPROPERTY(Transient)
	EReactionStopReason LastStopReason_Cached = EReactionStopReason::None;

protected:
	/* === Injection Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* OwnerReactionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionFeedbackComponent* ReactionFeedbackComp_Cached = nullptr;

protected:
	uint32 Serial_CurrentPlay = 0;		// Serial of Current Play Reaction
	uint32 CachedSerial_ActivePlay = 0;	// Cached Serial of Active Play Raction

public:
	virtual void Initialize(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent);
	virtual void Tick(float InDeltaTime) {};

public:
	virtual bool IsValidMinimal() const;

public:
	bool IsReaction() const { return bIsReaction; }

	UAnimMontage* GetActiveReactionMontage() const { return ActiveReactionMontage_Cached; }
	const FReactionData& GetActiveReactionData() const { return ActiveReactionData_Cached; }

protected:
	bool IsInterruptibleNow() const { return bInterruptible; }
	bool IsCancelableNow() const { return bCancelable; }

public:
	virtual bool Start(const FReactionData& InReactionData);
	virtual void Stop(EReactionStopReason InStopReason);

protected:
	virtual void FinishCompleted();
	virtual void FinishInterrupted();
	virtual void FinishCancelled();
	virtual void FinishAborted();

protected:
	virtual void Clear();

public:
	virtual void OnReactionControlWindowBegin(EReactionControlWindowType InReactionWindowType);
	virtual void OnReactionControlWindowEnd(EReactionControlWindowType InReactionWindowType);

	virtual void OnReactionFeedbackWindowBegin(FName InTriggerKey);
	virtual void OnReactionFeedbackWindowEnd(FName InTriggerKey);
	virtual void OnReactionFeedback(FName InTriggerKey);

public:
	virtual bool WantToInterrupt(const FReactionQueryContext& InReactionQueryContext) const;
	virtual bool WantToCancel(const FReactionQueryContext& InReactionQueryContext) const;
	virtual bool AllowInterruptionBy(const FReactionQueryContext& InReactionQueryContext) const;
	virtual bool AllowCancelBy(const FReactionQueryContext& InReactionQueryContext) const;

protected:
	void SetInterruptible(bool bEnable) { bInterruptible = bEnable; }
	void SetCancelable(bool bEnable) { bCancelable = bEnable; }

protected:
	void RequestFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;
	virtual FReactionFeedbackRequest BuildReactionFeedbackRequest(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

public:
	void PrintReactionExecutorRuntimeInfo_Public() const;

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial);

protected:
	bool CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const;

protected:
	virtual void OnMontageCompleted(UAnimMontage* InMontage, uint32 InSerial);
	virtual void OnMontageStopped(UAnimMontage* InMontage, uint32 InSerial);

private:
	void PrintReactionExecutorRuntimeInfo() const;

	void PrintStopReasonInfo(EReactionStopReason InStopReason) const;
	void PrintUnexpectedStopReasonInfo() const;
};
