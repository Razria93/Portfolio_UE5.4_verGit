#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "Type/CReactionFeedbackStructure.h"
#include "Type/CReactionOrchestrationStructure.h"
#include "CReaction.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCReaction : public UObject
{
	GENERATED_BODY()

protected:
	/* === Runtime State === */
	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	bool bWantInterrupt = false;

	UPROPERTY(Transient)
	bool bWantCancel = false;

	UPROPERTY(Transient)
	bool bAllowInterrupt = false;

	UPROPERTY(Transient)
	bool bAllowCancel = false;

protected:
	uint32 Serial_CurrentPlay = 0;		// Serial of Current Play Reaction
	uint32 CachedSerial_ActivePlay = 0;	// Cached Serial of Active Play Reaction

protected:
	UPROPERTY(Transient)
	FReactionDataKey ActiveDataKey_Cached = FReactionDataKey();

	UPROPERTY(Transient)
	FReactionData ActiveData_Cached = FReactionData();

	UPROPERTY(Transient)
	class UAnimMontage* ActiveMontage_Cached = nullptr;

	UPROPERTY(Transient)
	EReactionStopReason LastStopReason_Cached = EReactionStopReason::None;

protected:
	/* === Injection Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* OwnerReactionComp_Injected = nullptr;

protected:
	UPROPERTY(Transient)
	class UCReactionFeedbackComponent* ReactionFeedbackComp_Cached = nullptr;

public:
	virtual void Initialize(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComp);
	virtual void Tick(float InDeltaTime) {};

public:
	bool IsActive() const { return bIsActive; }

protected:
	bool IsWantInterruptNow() const { return bWantInterrupt; }
	bool IsWantCancelNow() const { return bWantCancel; }

	bool IsAllowInterruptNow() const { return bAllowInterrupt; }
	bool IsAllowCancelNow() const { return bAllowCancel; }

protected:
	void SetWantInterrupt(bool bEnable) { bWantInterrupt = bEnable; }
	void SetWantCancel(bool bEnable) { bWantCancel = bEnable; }

	void SetAllowInterrupt(bool bEnable) { bAllowInterrupt = bEnable; }
	void SetAllowCancel(bool bEnable) { bAllowCancel = bEnable; }

public:
	const FReactionDataKey& GetActiveDataKey() const { return ActiveDataKey_Cached; }
	const FReactionData& GetActiveData() const { return ActiveData_Cached; }

public:
	virtual EExecutionDecision ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const;

public:
	virtual bool Start(const FReactionData& InData);
	virtual void Stop(EReactionStopReason InStopReason);
	virtual void Complete();

protected:
	virtual void ClearRuntime();

protected:
	virtual bool PlayMontage(const FReactionData& InData);
	virtual void StopMontage(float InBlendOutTime = 0.1f);

protected:
	virtual bool BindMontageEndDelegate();

public:
	void HandleNotifyCommand(EReactionNotifyCommand InCommand);

protected:
	virtual void HandleSpecificNotifyCommand(EReactionNotifyCommand InCommand);

public:
	void HandleNotifyFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None);

public:
	virtual bool WantIntervention(const FExecutionInterventionQuery& InQuery) const;
	virtual bool AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const;

protected:
	void RequestFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;
	virtual FReactionFeedbackRequest BuildFeedbackRequest(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial);
	bool CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const;

public:
	void PrintReactionExecutorRuntimeInfo_Public() const;

private:
	void PrintReactionExecutorRuntimeInfo() const;

	void PrintStopReasonInfo(EReactionStopReason InStopReason) const;
	void PrintIgnoredStopReasonInfo() const;
};
