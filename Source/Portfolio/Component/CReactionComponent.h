#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CReactionComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	TArray<FReactionData> ReactionDatas;

	UPROPERTY(Transient)
	TMap<FReactionKey, FReactionData> ReactionContainer;

private:
	// bool bIsReaction = false;

private:
	// ACharacter* OwnerCharacter_Cached = nullptr;

public:
	UCReactionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void BuildReactionContainer();

private:
	void PrintReactionContainerInfo() const;
};
