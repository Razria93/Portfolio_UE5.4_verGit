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

	// Opt-in: existing trails continue following the weapon mesh origin.
	UPROPERTY(EditAnywhere, Category = "Attachment")
	bool bAttachToSocket = false;

	UPROPERTY(EditAnywhere, Category = "Attachment", meta = (EditCondition = "bAttachToSocket"))
	FName AttachmentSocket = TEXT("FX_CounterTrail");

	UPROPERTY(EditAnywhere, Category = "Attachment")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Attachment")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Ribbon")
	bool bOverrideRibbonWidth = false;

	UPROPERTY(EditAnywhere, Category = "Ribbon", meta = (EditCondition = "bOverrideRibbonWidth"))
	FName RibbonWidthParameter = TEXT("User.Trail Width");

	// World-space width in cm, not automatically multiplied by mesh scale.
	UPROPERTY(EditAnywhere, Category = "Ribbon", meta = (EditCondition = "bOverrideRibbonWidth", ClampMin = "0.01"))
	float RibbonWidth = 35.f;
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
