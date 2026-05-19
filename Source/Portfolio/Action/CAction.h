#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
// #include "Type/CActionFeedbackStructure.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CAction.generated.h"

UCLASS(Abstract) // Base Action Class
class PORTFOLIO_API UCAction : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	bool bInterruptible = false;	// Setted by AnimNotify

	UPROPERTY(Transient)
	bool bCancelable = false;		// Setted by AnimNotify

protected:
	uint32 Serial_CurrentPlay = 0;		// Serial of Current Play Action
	uint32 CachedSerial_ActivePlay = 0;	// Cached Serial of Active Play Action

protected:
	UPROPERTY(Transient)
	FActionDataKey ActiveDataKey_Cached = FActionDataKey();

	UPROPERTY(Transient)
	FActionData ActiveData_Cached = FActionData();

	UPROPERTY(Transient)
	UAnimMontage* ActiveMontage_Cached = nullptr;

	UPROPERTY(Transient)
	EActionStopReason LastStopReason_Cached = EActionStopReason::None;

protected:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* OwnerActionComp_Injected = nullptr;

protected:
	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionFeedbackComponent* ActionFeedbackComp_Cached = nullptr;

public:
	virtual void InitializeAction(ACharacter* InOwnerCharacter, class UCActionComponent* InOwnerActionComp);
	virtual void Tick(float InDeltaTime) {}

public:
	bool IsActive() const { return bIsActive; }

protected:
	bool IsInterruptibleNow() const { return bInterruptible; }
	bool IsCancelableNow() const { return bCancelable; }

protected:
	void SetInterruptible(bool bEnable) { bInterruptible = bEnable; }
	void SetCancelable(bool bEnable) { bCancelable = bEnable; }

public:
	const FActionDataKey& GetActiveDataKey() const { return ActiveDataKey_Cached; }
	const FActionData& GetActiveData() const { return ActiveData_Cached; }

public:
	virtual EExecutionDecision ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const;

public:
	virtual bool Start(const FActionData& InData);
	virtual void Stop(EActionStopReason InStopReason);
	virtual void Complete();

public:
	virtual bool ReserveChain(const FActionData& InData);
	virtual void ConsumeChain();

protected:
	virtual void ClearRuntime();

protected:
	virtual bool PlayMontage(const FActionData& InData);
	virtual void StopMontage(float InBlendOutTime = 0.1f);

protected:
	virtual bool BindMontageEndDelegate();

public:
	void HandleNotifyCommand(EActionNotifyCommand InCommand);

protected:
	virtual void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand);

public:
	virtual void HandleNotifyFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None);

public:
	virtual bool WantIntervention(const FExecutionInterventionQuery& InQuery) const;
	virtual bool AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const;

protected:
	void RequestFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;
	virtual FActionFeedbackRequest BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

protected:
	void EmitActionEvent(EActionEventType InEventType, int32 InActionIndex = INDEX_NONE) const;

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial);
	bool CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const;

protected:
	// Action-only hit context
	void PushHitContext();
	void ClearHitContext();
	virtual FActionContext BuildActionContext() const;

public:
	// [Legacy delegate]
	UFUNCTION()
	virtual void OnWeaponActorCollisionEnabled() {};

	UFUNCTION()
	virtual void OnWeaponActorCollisionDisabled() {};

	UFUNCTION()
	virtual void OnWeaponActorBeginOverlap(AActor* InAttackerActor, AActor* InDamageCauser, UShapeComponent* InAttackCollision, AActor* InTargetActor, UPrimitiveComponent* InHitComponent, int32 InOtherBodyIndex, bool InbFromSweep, const FHitResult& InSweepResult) {};

	UFUNCTION()
	virtual void OnWeaponActorEndOverlap(AActor* InAttackerActor, AActor* InTargetActor) {};
};
