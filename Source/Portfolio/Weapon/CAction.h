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
	/* === Injection Objects === */
	class ACharacter* OwnerCharacter_Injected;
	TArray<FActionData> ActionDatas_Injected;

protected:
	/* === Cached Objects === */
	class UCWeaponComponent* WeaponComp_Cached;
	class UCStateComponent* StateComp_Cached;
	class UCActionComponent* ActionComp_Cached;

protected:
	EActionType ActionType;

private:
	bool bBeginAction;	// Action start triggered
	bool bIsAction;		// Action is active

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
	/* === [IN] Custom Delgate Events === */
	// [Regacy] CAttachment
	UFUNCTION()
	virtual void OnAttachmentCollisionEnabled() {};

	UFUNCTION()
	virtual void OnAttachmentCollisionDisabled() {};

	// [Regacy] CAttachment
	UFUNCTION()
	virtual void OnAttachmentBeginOverlap(AActor* InAttackerActor, AActor* InDamageCauser, UShapeComponent* InAttackCollision, AActor* InTargetActor, UPrimitiveComponent* InHitComponent, int32 InOtherBodyIndex, bool InbFromSweep, const FHitResult& InSweepResult) {};

	UFUNCTION()
	virtual void OnAttachmentEndOverlap(AActor* InAttackerActor, AActor* InTargetActor) {};
};
