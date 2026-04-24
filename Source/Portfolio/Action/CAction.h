#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "CAction.generated.h"

UCLASS(Abstract) // Base Action Class
class PORTFOLIO_API UCAction : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(Transient)
	bool bIsAction = false;

protected:
	UPROPERTY(Transient)
	TArray<FActionData> ActionDatas_Injected;

protected:
	/* === Injection Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

protected:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionFeedbackComponent* ActionFeedbackComp_Cached = nullptr;

public:
	virtual void InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData>& InActionDatas);
	virtual void Tick(float InDeltaTime) {};

public:
	EActionType GetActionType() const;

public:
	void SetActionType(EActionType InActionType);

public:
	/* === Action Arbitration === */
	virtual EActionExecutionDecision DecideExecution(const FActionExecutionQuery& InActionExecuteQuery) const;

public:
	virtual bool Start();
	virtual bool ApplyChain(const FActionExecutionQuery& InActionExecuteQuery);

public:
	virtual void Complete();

public:
	void PushHitContext();
	void ClearHitContext();

public:
	void RequestFeedback(EActionFeedbackTiming InActionFeedbackTiming, FName InTriggerKey = NAME_None) const;

protected:
	virtual FActionContext BuildActionContext() const;
	virtual FActionFeedbackRequest BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

public:
	/* === [IN] Custom Delgate Events === */
	// [Legacy delegate] CWeaponActor
	UFUNCTION()
	virtual void OnWeaponActorCollisionEnabled() {};

	UFUNCTION()
	virtual void OnWeaponActorCollisionDisabled() {};

	// [Legacy delegate] CWeaponActor
	UFUNCTION()
	virtual void OnWeaponActorBeginOverlap(AActor* InAttackerActor, AActor* InDamageCauser, UShapeComponent* InAttackCollision, AActor* InTargetActor, UPrimitiveComponent* InHitComponent, int32 InOtherBodyIndex, bool InbFromSweep, const FHitResult& InSweepResult) {};

	UFUNCTION()
	virtual void OnWeaponActorEndOverlap(AActor* InAttackerActor, AActor* InTargetActor) {};
};
