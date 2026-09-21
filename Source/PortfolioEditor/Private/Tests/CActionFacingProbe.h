#pragma once

#include "CoreMinimal.h"
#include "Action/CAction_ComboAttack.h"
#include "Character/Player/CPlayer.h"
#include "CActionFacingProbe.generated.h"

UCLASS(Transient)
class ACActionFacingCharacterProbe : public ACPlayer
{
	GENERATED_BODY()

protected:
	void PostInitializeComponents() override { ACharacter::PostInitializeComponents(); }
};

UCLASS(Transient)
class UCActionFacingProbe : public UCAction_ComboAttack
{
	GENERATED_BODY()

public:
	bool bFailPlayback = false;
	bool bFailBinding = false;
	bool bCompleteDuringPlayback = false;
	TFunction<void()> DuringPlayback;

protected:
	bool PlayMontage(const FActionData& InData) override
	{
		const bool bPlayed = !bFailPlayback;
		if (bCompleteDuringPlayback) Complete();
		TFunction<void()> callback = MoveTemp(DuringPlayback);
		if (callback) callback();
		return bPlayed;
	}
	bool BindMontageEndDelegate() override { return !bFailBinding; }
	void StopMontage(float InBlendOutTime) override {}
};
