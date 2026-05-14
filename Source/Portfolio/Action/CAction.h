#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CAction.generated.h"

UCLASS(Abstract) // Base Action Class
class PORTFOLIO_API UCAction : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	bool bIsActive = false;

protected:
	UPROPERTY(Transient)
	FActionDataKey ActiveDataKey_Cached = FActionDataKey();

	UPROPERTY(Transient)
	FActionData ActiveData_Cached = FActionData();

	UPROPERTY(Transient)
	UAnimMontage* ActiveMontage_Cached = nullptr;

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

public:
	const FActionDataKey& GetActiveDataKey() const { return ActiveDataKey_Cached; }
	const FActionData& GetActiveData() const { return ActiveData_Cached; }

public:
	// Action local rule
	virtual EActionLocalLevelDecision ResolveLocalLevelDecision(const FActionLocalLevelQuery& InQuery) const;

public:
	virtual bool Start(const FActionData& InData);
	virtual bool ApplyChain(const FActionData& InData);
	virtual void Stop(EActionStopReason InStopReason);
	virtual void Complete();

protected:
	virtual void ClearRuntime();

protected:
	virtual bool PlayMontage(const FActionData& InData);
	virtual void StopMontage(float InBlendOutTime = 0.1f);

public:
	void HandleNotifyCommand(EActionNotifyCommand InCommand);

public:
	void HandleNotifyFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None);

protected:
	virtual FActionContext BuildActionContext() const;
	virtual FActionFeedbackRequest BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

protected:
	// Action-only hit context
	void PushHitContext();
	void ClearHitContext();

protected:
	void RequestFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

protected:
	void EmitActionEvent(EActionEventType InEventType, int32 InActionIndex = INDEX_NONE) const;

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
