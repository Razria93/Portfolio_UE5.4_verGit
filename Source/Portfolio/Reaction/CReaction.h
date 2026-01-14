#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "CReaction.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction : public UObject
{
	GENERATED_BODY()

protected:
	/* === Injection Objects === */
	class ACharacter* OwnerCharacter_Injected;
	TArray<FReactionData> ReactionDatas_Injected;

protected:
	/* === Cached Objects === */
	class UCStateComponent* StateComp_Cached;
	class UCReactionComponent* ReactionComp_Cached;

private:
	bool bBeginReaction;	// Reaction start triggered
	bool bIsReaction;		// Reaction is active

public:
	virtual void InitializeReaction(ACharacter* InOwnerCharacter, const TArray<FReactionData> InReactionDatas);
	virtual void Tick(float InDeltaTime) {};

public:
	virtual void PlayReaction();
	virtual void BeginPlayReaction();
	virtual void EndPlayReaction();
};
