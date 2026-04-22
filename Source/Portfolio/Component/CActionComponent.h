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
	UPROPERTY(EditAnywhere, Category = "Action")
	TArray<FActionDefinition> ActionDefinitions;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<EActionType, class UCAction*> ActionContainer;

private:
	/* === State === */
	UPROPERTY(Transient)
	EActionType CurrentActionType = EActionType::Max;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

public:
	FActionTypeChanged OnActionTypeChanged;

protected:
	void BeginPlay() override;

public:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurrentActionType(EActionType InNewActionType) const { return CurrentActionType == InNewActionType; }

public:
	/* === Getter === */
	FORCEINLINE EActionType GetCurrentActionType() const { return CurrentActionType; }

public:
	class UCAction* GetCurrentAction() const;

private:
	bool CanStartAction() const;

public:
	bool StartAction(EActionType InActionType);
	void CompleteAction();

private:
	void EnterActionState(EActionType InActionType);
	void ExitActionState();

private:
	void ChangeActionType(EActionType InNewActionType);

private:
	bool CreateAction(ACharacter* InOwnerCharacter, const FActionDefinition& InActionDefinition);
};
