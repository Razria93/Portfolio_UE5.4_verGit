#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Type/CActionFeedbackTypes.h"
#include "CWeaponTrailDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWeaponTrailDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	FActionFeedbackMatchKey ActionFeedbackMatchKey = FActionFeedbackMatchKey();

	UPROPERTY(EditAnywhere, Category = "Presentation")
	TArray<TObjectPtr<class UNiagaraSystem>> NiagaraSystems;
};

UCLASS(BlueprintType)
class PORTFOLIO_API UWeaponTrailDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon Trail", meta = (TitleProperty = "TriggerKey"))
	TArray<FWeaponTrailDefinition> TrailDefinitions;

public:
	const FWeaponTrailDefinition* FindTrailDefinition(FName InTriggerKey) const;
};
