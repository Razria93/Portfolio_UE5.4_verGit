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

	FApplyDamageSpecKey applyDamageSpecKey;
	applyDamageSpecKey.AttachmentType = EAttachmentType::Sword;
	applyDamageSpecKey.EquipmentType = EEquipmentType::Default;
	applyDamageSpecKey.ActionType = EActionType::ComboAttack;
	applyDamageSpecKey.ActionIndex = 0;

	FReactionData reactionData;
	FindReaction(applyDamageSpecKey, EReactionType::Hit, reactionData);
}

void UCReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCReactionComponent::BuildReactionContainer()
{
	ReactionContainer.Reset();

	for (const FReactionData& reactionData : ReactionDatas)
	{
		if (!reactionData.IsValidMinimal()) continue;

		FReactionKey reactionKey;
		reactionKey.ApplyDamageSpecKey = reactionData.ApplyDamageSpecKey;
		reactionKey.ReactionType = reactionData.ReactionType;

		if (ReactionContainer.Contains(reactionKey))
		{
			FLog::Log(TEXT("[Duplicate key] Overwrite Value"));
		}

		ReactionContainer.Add(reactionKey, reactionData);
	}
}

void UCReactionComponent::BuildCandidateSpecKeys(const FApplyDamageSpecKey& InApplyDamageSpecKey, TArray<FApplyDamageSpecKey>& OutApplyDamageSpecKeys) const
{
	OutApplyDamageSpecKeys.Reset();

	// 1) 'Exact' Key
	OutApplyDamageSpecKeys.Add(InApplyDamageSpecKey);

	// 2) 'Index Any' Key
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 3) 'Action + Index Any' Key
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 4) 'Equipment + Action + Index Any' Key
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.EquipmentType = EEquipmentType::All;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 5) 'Attachment + Equipment + Action + Index Any' Key (All Any Key)
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.AttachmentType = EAttachmentType::All;
		candidateKey.EquipmentType = EEquipmentType::All;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}
}

bool UCReactionComponent::FindReaction(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData) const
{
	FLog::Log(TEXT("====== FindReaction Info ========"));

	// Debug
	PrintApplyDamageSpecKeyInfo(InApplyDamageSpecKey);

	TArray<FApplyDamageSpecKey> candidateKeys;
	BuildCandidateSpecKeys(InApplyDamageSpecKey, candidateKeys);

	for (const FApplyDamageSpecKey& candidateKey : candidateKeys)
	{
		FReactionKey reactionKey;
		reactionKey.ApplyDamageSpecKey = candidateKey;
		reactionKey.ReactionType = InReactionType;

		// Debug
		PrintReactionKeyInfo(reactionKey);

		if (const FReactionData* reactionDataPtr = ReactionContainer.Find(reactionKey))
		{
			OutReactionData = *reactionDataPtr;

			// Debug
			PrintReactionDataInfo(OutReactionData);

			FLog::Log(TEXT("================================="));
			return true;
		}
		else
		{
			FLog::Log(TEXT("------ ReactionData Info --------"));
			FLog::Log(TEXT("[Not Found]"));
			FLog::Log(TEXT("---------------------------------"));
		}
	}

	FLog::Log(TEXT("================================="));
	return false;
}

void UCReactionComponent::PrintReactionContainerInfo() const
{
	FLog::Log(TEXT("==== ReactionContainer Info ====="));
	
	const int32 count = ReactionContainer.Num();
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Count"), count));

	if (count <= 0)
	{
		FLog::Log(TEXT("[Is Empty]"));
		FLog::Log(TEXT("================================="));
		return;
	}

	FLog::Log(TEXT("=========== Pair Info ==========="));

	int32 index = 0;
	for (const TPair<FReactionKey, FReactionData>& pair : ReactionContainer)
	{
		const FReactionKey& key = pair.Key;
		const FReactionData& value = pair.Value;

		FLog::Log(FString::Printf(TEXT("[%s: %d]"), TEXT("PairIndex"), index++));
		
		PrintReactionKeyInfo(key);
		PrintReactionDataInfo(value);
		FLog::Log(TEXT("================================="));
	}
}

void UCReactionComponent::PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey) const
{
	const FString actionIndexText = (InApplyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(InApplyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("---- ApplyDamageSpecKey Info ----"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionKeyInfo(const FReactionKey& InReactionKey) const
{
	const FApplyDamageSpecKey& applyDamageSpecKey = InReactionKey.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("------- ReactionKey Info --------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InReactionKey.ReactionType)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataInfo(const FReactionData& InReactionData) const
{
	const FApplyDamageSpecKey& applyDamageSpecKey = InReactionData.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);
	
	FLog::Log(TEXT("------ ReactionData Info --------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InReactionData.ReactionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Montage"), *GetNameSafe(InReactionData.Montage)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("PlayRate"), InReactionData.PlayRate));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("bCanMove"), InReactionData.bCanMove ? 1 : 0));
	FLog::Log(TEXT("---------------------------------"));
}