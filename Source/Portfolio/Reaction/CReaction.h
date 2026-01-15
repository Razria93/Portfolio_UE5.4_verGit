#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "CReaction.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCReaction : public UObject
{
	GENERATED_BODY()

protected:
	/* === Owner Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected;

	UPROPERTY(Transient)
	class UCReactionComponent* OwnerReactionComponent_Injected;

protected:
	/* === Cached Objects === */
	class UCStateComponent* StateComp_Cached;
	class UCReactionComponent* ReactionComp_Cached;

private:
	bool bBeginReaction;	// Reaction start triggered
	bool bIsReaction;		// Reaction is active

public:
	virtual void InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent);
	virtual void Tick(float InDeltaTime) {};

public:
	// Policy
	virtual bool CanInterrupt(const FReactionData& InActiveReactionData, const FReactionData& InNewReactionData) { return false; };
	virtual bool CanBeInterrupted(const FReactionData& InActiveReactionData, const FReactionData& InNewReactionData) { return false; };

public:
	// Excution
	virtual bool PlayReaction(const FReactionData& reactionData);
	virtual void Stop(EReactionStopReason InStopReason, const UCReaction* InNewReaction);

public:
	// AnimNotify Entry API
	virtual void OnAnimNotify_ReactionBegin() {};
	virtual void OnAnimNotify_ReactionEnd() {};
};
