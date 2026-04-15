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
	EActionType ActionType;

protected:
	bool bBeginAction;	// Action start triggered
	bool bIsAction;		// Action is active

protected:
	/* === Injection Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	TArray<FActionData> ActionDatas_Injected;

protected:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionFeedbackComponent* ActionFeedbackComp_Cached = nullptr;

public:
	virtual void InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData> InActionDatas);
	virtual void Tick(float InDeltaTime) {};

public:
	EActionType GetActionType() const;

public:
	void SetActionType(EActionType InActionType);

public:
	virtual void PlayAction();
	virtual void BeginPlayAction();
	virtual void EndPlayAction();
	virtual void NextPlayAction() {};

public:
	virtual void NotifyActionTrailBegin();
	virtual void NotifyActionTrailEnd();

protected:
	void PushContextToAttachment(const FActionContext& InActionContext);
	void ClearContextToAttachment();

protected:
	void RequestPlayActionFeedback(const FActionContext& InActionContext, EActionFeedbackPhase InPhase);

public:
	/* === [IN] Custom Delgate Events === */
	// [Legacy delegate] CAttachment
	UFUNCTION()
	virtual void OnAttachmentCollisionEnabled() {};

	UFUNCTION()
	virtual void OnAttachmentCollisionDisabled() {};

	// [Legacy delegate] CAttachment
	UFUNCTION()
	virtual void OnAttachmentBeginOverlap(AActor* InAttackerActor, AActor* InDamageCauser, UShapeComponent* InAttackCollision, AActor* InTargetActor, UPrimitiveComponent* InHitComponent, int32 InOtherBodyIndex, bool InbFromSweep, const FHitResult& InSweepResult) {};

	UFUNCTION()
	virtual void OnAttachmentEndOverlap(AActor* InAttackerActor, AActor* InTargetActor) {};
};
