#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CAnimNotifyState_DodgeWindow.generated.h"

UENUM()
enum class EDodgeWindowKind : uint8 { Invulnerability, Perfect };

/** Authoring interval only: combat queries the owning montage position, not Begin/End callbacks. */
UCLASS(meta = (DisplayName = "Dodge Judgment Window"))
class PORTFOLIO_API UCAnimNotifyState_DodgeWindow : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "DodgeWindow")
	EDodgeWindowKind Kind = EDodgeWindowKind::Invulnerability;
	FString GetNotifyName_Implementation() const override;
	static bool Contains(const class UAnimMontage* InMontage, EDodgeWindowKind InKind, float InPosition);
};
