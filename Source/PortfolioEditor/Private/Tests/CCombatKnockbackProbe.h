#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "Component/CReactionComponent.h"
#include "Character/Player/CPlayer.h"
#include "CCombatKnockbackProbe.generated.h"

UCLASS(Transient)
class ACCombatKnockbackCharacterProbe : public ACPlayer
{
	GENERATED_BODY()

protected:
	void PostInitializeComponents() override { ACharacter::PostInitializeComponents(); }
};

UCLASS(Transient)
class UCCombatKnockbackProbe : public UCReaction
{
	GENERATED_BODY()

public:
	bool bFailStart = false;
	bool bCompleteDuringStart = false;
	int32 StartCalls = 0;

	UFUNCTION()
	void CancelOnTypeChanged(ACharacter* Character, EReactionType PreviousType, EReactionType NewType)
	{
		if (NewType == EReactionType::Hit)
			ReactionComp_Injected->HandleApplyReactionFinished(this, EReactionFinishReason::Interrupted);
	}

	virtual bool Start(const FReactionData& InData) override
	{
		++StartCalls;
		if (bFailStart) return false;
		bIsActive = true;
		if (bCompleteDuringStart) Complete();
		return true;
	}

	virtual void Complete() override
	{
		bIsActive = false;
		ReactionComp_Injected->HandleApplyReactionFinished(this, EReactionFinishReason::Completed);
	}

	virtual void Stop(EReactionStopReason InReason) override
	{
		bIsActive = false;
		ReactionComp_Injected->HandleApplyReactionFinished(this, EReactionFinishReason::Interrupted);
	}
};
