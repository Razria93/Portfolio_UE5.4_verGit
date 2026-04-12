#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CReactionFXComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCReactionFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCReactionFXComponent();

private:
	UPROPERTY(EditAnywhere)
	float HitStopTime = 0.f;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere)
	class USoundBase* HitSound = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class AActor* OwnerActor_Cached = nullptr;

	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	bool CanPlayDamageFeedback(const FTakeDamageResult& InTakeDamageResult) const;
	void PlayDamageFeedback(const FTakeDamageResult& InTakeDamageResult);

private:
	void PlayHitStop(const FTakeDamageResult& InTakeDamageResult);
	void PlayHitVFX(const FTakeDamageResult& InTakeDamageResult);
	void PlayHitSound(const FTakeDamageResult& InTakeDamageResult);
	void PlayCameraShake(const FTakeDamageResult& InTakeDamageResult);
};
