#include "Component/CReactionComponent.h"
#include "ProjectGlobal.h"

#include "Type/CWeaponStructure.h"

UCReactionComponent::UCReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	BuildReactionContainer();
	PrintReactionContainerInfo();
}

void UCReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCReactionComponent::BuildReactionContainer()
{
	ReactionContainer.Reset();

	for (FReactionData reactionData : ReactionDatas)
	{
		if (!reactionData.IsValidMinimal()) continue;

		FReactionKey reactionKey;
		reactionKey.ApplyDamageSpecKey = reactionData.ApplyDamageSpecKey;
		reactionKey.ReactionType = reactionData.ReactionType;

		ReactionContainer.Add(reactionKey, reactionData);
	}
}

void UCReactionComponent::PrintReactionContainerInfo() const
{
	FLog::Log(TEXT("==== ReactionContainer Info ====="));
	
	const int32 count = ReactionContainer.Num();
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Count"), count));

	if (count <= 0)
	{
		FLog::Log(TEXT("[ReactionContainer is empty]"));
		FLog::Log(TEXT("================================="));
		return;
	}

	FLog::Log(TEXT("----------- Pair Info -----------"));

	int32 index = 0;
	for (const TPair<FReactionKey, FReactionData>& pair : ReactionContainer)
	{
		const FReactionKey& key = pair.Key;
		const FReactionData& value = pair.Value;

		const FApplyDamageSpecKey& applyDamageSpecKey = key.ApplyDamageSpecKey;
		const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);

		FLog::Log(FString::Printf(TEXT("[%s: %d]"), TEXT("PairIndex"), index++));

		FLog::Log(TEXT("---- Key ----"));
		
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.AttachmentType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.EquipmentType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(key.ReactionType)));

		FLog::Log(TEXT("--- Value ---"));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Montage"), *GetNameSafe(value.Montage)));
		FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("PlayRate"), value.PlayRate));
		FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("bCanMove"), value.bCanMove ? 1 : 0));
		FLog::Log(TEXT("---------------------------------"));
	}
	
	FLog::Log(TEXT("================================="));
}