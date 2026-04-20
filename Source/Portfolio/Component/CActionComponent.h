#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FActionTypeChanged, class ACharacter*, InOwnerCharacter, EActionType, InPrevActionType, EActionType, InNewActionType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCActionComponent();

	// === ActionData ======================================= //
private:
	UPROPERTY(EditAnywhere, Category = "Action|Type")
	EActionType ActionType;

	UPROPERTY(EditAnywhere, Category = "Action|Class")
	TSubclassOf<class UCAction> ActionClass;

	UPROPERTY(EditAnywhere, Category = "Action|Data")
	TArray<FActionData> ActionDatas;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<EActionType, class UCAction*> ActionContainer;

private:
	/* === State === */
	UPROPERTY(Transient)
	EActionType CurrentActionType_Cached;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

public:
	FActionTypeChanged OnActionTypeChanged;

protected:
	void BeginPlay() override;

public:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurrentActionType(EActionType InNewActionType) const { return CurrentActionType_Cached == InNewActionType; }

public:
	/* === Getter === */
	FORCEINLINE EActionType GetCurrentActionType() const { return CurrentActionType_Cached; }

public:
	class UCAction* GetCurrentAction() const;

public:
	/* === Try Start API === */
	bool TryStartAction(EActionType InActionType);

private:
	void ChangeActionType(EActionType InNewActionType);

private:
	bool CreateAction(AActor* InOwnerCharacter, EActionType InActionType, TSubclassOf<UCAction> InActionClass, const TArray<FActionData> InActionDatas);
};
