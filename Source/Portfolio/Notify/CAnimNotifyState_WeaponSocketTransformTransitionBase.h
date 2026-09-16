#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_ActionBase.h"
#include "Type/CWeaponTypes.h"
#include "CAnimNotifyState_WeaponSocketTransformTransitionBase.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCAnimNotifyState_WeaponSocketTransformTransitionBase : public UCAnimNotifyState_ActionBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Socket Transform Transition", meta = (ClampMin = "0.0"))
	float BlendExponent = 2.f;

	EWeaponSocketSlot TargetSlot = EWeaponSocketSlot::None;
	EActionNotifyCommand CompletionCommand = EActionNotifyCommand::None;

private:
	struct FRuntimeState
	{
		TWeakObjectPtr<class UCWeaponComponent> WeaponComponent;
		uint32 TransitionHandle = 0;
		float ElapsedTime = 0.f;
		float TotalDuration = 0.f;
	};

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FRuntimeState> RuntimeStates;

protected:
	virtual FString GetNotifyName_Implementation() const override;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	virtual bool IsConfigurationValid() const;

protected:
	virtual bool BeginTransition(class UCWeaponComponent* InWeaponComp, uint32& OutTransitionHandle) const;
	bool UpdateTransition(class UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle, float InAlpha) const;
	bool CompleteTransition(class UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const;
	void CancelTransition(class UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const;

private:
	float CalculateAlpha(float InElapsedTime, float InTotalDuration) const;
};
