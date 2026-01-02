#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FActionTypeChanged, class ACharacter*, InOwnerCharacter, EActionType, InPrevActionType, EActionType, InNewActionType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCActionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	// === ActionData ======================================= //

private:
	UPROPERTY(EditAnywhere, Category = "ActionData")
	EActionType ActionType;

private:
	UPROPERTY(EditAnywhere, Category = "ActionData")
	TSubclassOf<class UCAction> ActionClass;

private:
	UPROPERTY(EditAnywhere, Category = "ActionData")
	TArray<FActionData> ActionDatas;

	// ====================================================== //

public:
	UPROPERTY(Transient)
	TMap<EActionType, class UCAction*> ActionContainer;

private:
	/* === State === */
	EActionType CurrentActionType_Cached;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;

public:
	FActionTypeChanged OnActionTypeChanged;

public:	
	UCActionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Getter === */
	class UCAction* GetAction(EActionType InNewActionType);

public:
	/* === Getter === */
	FORCEINLINE EActionType GetCurActionType() { return CurrentActionType_Cached; }

public:
	/* === Setter === */
	void SetIdleMode();
	void SetComboAttackMode();

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurActionType(EActionType InNewActionType) { return CurrentActionType_Cached == InNewActionType; }

public:
	void PlayAction();

private:
	void ChangeActionMode(EActionType InNewActionType);
	void ChangeActionType(EActionType InNewActionType);

private:
	bool CreateAction(AActor* InOwnerCharacter, EActionType InActionType, TSubclassOf<UCAction> InActionClass, const TArray<FActionData> InActionDatas);
};
