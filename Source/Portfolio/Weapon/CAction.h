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

private:
	bool bBeginAction;	// Action start triggered
	bool bIsAction;		// Action is active

public:
	virtual void InitializeAction(ACharacter* InOwnerCharacter, const TArray<FActionData> InActionDatas);
	virtual void Tick(float InDeltaTime) {};

public:
	virtual void PlayAction();
	virtual void Begin_PlayAction();
	virtual void End_PlayAction();
	virtual void Next_PlayAction() {};

public:
	/* === [IN] Custom Delgate Events === */
	// CAttachment
	UFUNCTION()
	virtual void OnAttachmentCollisionEnabled() {};

	UFUNCTION()
	virtual void OnAttachmentCollisionDisabled() {};

	// CAttachment
	UFUNCTION()
	virtual void OnAttachmentBeginOverlap(AActor* attackerActor, AActor* damageCauser, UShapeComponent* attackCollision, AActor* targetActor, UPrimitiveComponent* hitComponent) {};
	
	UFUNCTION()
	virtual void OnAttachmentEndOverlap(AActor* InAttackerActor, AActor* InTargetActor) {};
};
