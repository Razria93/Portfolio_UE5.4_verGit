#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CActionFeedbackComponent.generated.h"

UENUM(BlueprintType)
enum class EActionFeedbackPhase : uint8
{
	None,
	ActionStart,
	AttackWindowBegin,
	AttackWindowEnd,
	ActionEnd
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCActionFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCActionFeedbackComponent();

private:
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ActionStartVFX = nullptr;

	UPROPERTY(EditAnywhere)
	class USoundBase* ActionStartSound = nullptr;

	UPROPERTY(EditAnywhere)
	bool bUseWeaponTrail = true;

private:
	UPROPERTY(Transient)
	class AActor* OwnerActor_Cached = nullptr;

	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

protected:
	virtual void BeginPlay() override;

public:
	void PlayActionFeedback(const FHitContext& InHitContext, EActionFeedbackPhase InActionFeedbackPhase);

private:
	void PlayActionStartVFX(const FHitContext& InHitContext);
	void PlayActionStartSound(const FHitContext& InHitContext);
	void PlayAttackWindowBeginFeedback(const FHitContext& InHitContext);
	void PlayAttackWindowEndFeedback(const FHitContext& InHitContext);

private:
	bool CanPlayActionFeedback(const FHitContext& InHitContext, EActionFeedbackPhase InActionFeedbackPhase) const;

private:
	FApplyDamageSpecKey BuildActionFeedbackSpecKey(const FHitContext& InHitContext) const;

private:
	void SetWeaponTrailEnabled(bool bEnable);
};
