#include "Audit/DodgeAssetValidation.h"
#include "Audit/MontageSectionPath.h"
#include "Action/CAction_CounterRush.h"
#include "Reaction/CReaction_PerfectDodge.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Component/CWeaponComponent.h"
#include "Weapon/CWeaponActor.h"
#include "DataAsset/CWeaponTrailDataAsset.h"
#include "Notify/CAnimNotify_PerfectDodgePresentation.h"
#include "Notify/CAnimNotifyState_ActionFeedback.h"
#include "Animation/AnimSequence.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "UObject/UnrealType.h"

namespace DodgeAssetValidation
{
	namespace
	{
		template<typename T> const T& ReadField(const UObject* Object, const TCHAR* Name)
		{
			const FProperty* Property = FindFProperty<FProperty>(Object->GetClass(), Name);
			check(Property);
			return *Property->ContainerPtrToValuePtr<T>(Object);
		}
		template<typename T> T* ReadObject(const UObject* Object, const TCHAR* Name)
		{
			const auto* Property = Object ? FindFProperty<FObjectPropertyBase>(Object->GetClass(), Name) : nullptr;
			return Property ? Cast<T>(Property->GetObjectPropertyValue_InContainer(Object)) : nullptr;
		}
		bool Fail(FString& Error, const FString& Reason)
		{
			Error = Reason;
			return false;
		}
	}

	bool ValidatePerfectReaction(const FReactionData& Row, FString& Error)
	{
		Error.Reset();
		if (!Row.ReactionExecutorKey || !Row.ReactionExecutorKey->IsChildOf(UCReaction_PerfectDodge::StaticClass()))
			return Fail(Error, TEXT("PerfectDodge requires a PerfectDodge reaction executor."));
		if (!IsValid(Row.Montage)) return Fail(Error, TEXT("PerfectDodge montage reference is missing."));
		const auto Path = ExecutionMontageAudit::BuildSectionPath(Row.Montage, Row.StartSectionName, Row.PlayRate);
		if (!Path.Error.IsEmpty()) return Fail(Error, Path.Error);
		if (!Path.HasLinearTimeline()) return Fail(Error, TEXT("PerfectDodge section path requires manual review (loop or non-linear timeline)."));
		for (const auto Cue : {EPerfectDodgePresentationCue::Slow, EPerfectDodgePresentationCue::Vignette})
		{
			int32 Count = 0;
			for (const auto& Event : Row.Montage->Notifies)
			{
				const auto* Notify = Cast<UCAnimNotify_PerfectDodgePresentation>(Event.Notify);
				if (!Notify || Notify->Cue != Cue || !Path.ContainsTime(Row.Montage, Event.GetTime())) continue;
				++Count;
				if (!Notify->Envelope.IsValid() || Notify->Envelope.Duration() <= 0.
					|| !FMath::IsFinite(Event.GetTime()) || Event.GetTime() < 0.f || Event.GetTime() >= Row.Montage->GetPlayLength())
					return Fail(Error, FString::Printf(TEXT("Invalid presentation cue %d in %s."), int32(Cue), *Row.Montage->GetPathName()));
			}
			if (Count != 1) return Fail(Error, FString::Printf(TEXT("Expected one reachable presentation cue %d, found %d in %s."), int32(Cue), Count, *Row.Montage->GetPathName()));
		}
		return true;
	}

	bool ValidateCounterRush(const FActionData& Row, const UWeaponTrailDataAsset* Trails, FString& Error)
	{
		Error.Reset();
		if (!Row.ActionExecutorKey || !Row.ActionExecutorKey->IsChildOf(UCAction_CounterRush::StaticClass()))
			return Fail(Error, TEXT("CounterRush requires a CounterRush action executor."));
		if (!IsValid(Row.Montage)) return Fail(Error, TEXT("CounterRush montage reference is missing."));
		const auto Path = ExecutionMontageAudit::BuildSectionPath(Row.Montage, Row.StartSectionName, Row.PlayRate);
		if (!Path.Error.IsEmpty()) return Fail(Error, Path.Error);
		if (!Path.HasLinearTimeline()) return Fail(Error, TEXT("CounterRush section path requires manual review (loop or non-linear timeline)."));
		int32 Segments = 0;
		for (const auto& Slot : Row.Montage->SlotAnimTracks)
			for (const auto& Segment : Slot.AnimTrack.AnimSegments)
			{
				const auto* Sequence = Cast<UAnimSequence>(Segment.GetAnimReference());
				if (!Sequence) return Fail(Error, TEXT("CounterRush segment requires a valid direct animation sequence; composite sources require manual review."));
				if (!Sequence->bForceRootLock || Sequence->bEnableRootMotion || !Sequence->Notifies.IsEmpty())
					return Fail(Error, FString::Printf(TEXT("CounterRush sequence %s requires root lock, root motion disabled, and no sequence notifies."), *Sequence->GetPathName()));
				++Segments;
			}
		if (!Segments) return Fail(Error, TEXT("CounterRush montage has no animation segments."));
		if (!IsValid(Trails)) return Fail(Error, TEXT("Configured weapon has no trail data asset."));
		FActionFeedbackMatchKey Request;
		Request.ActionType = Row.ActionDataKey.ActionType;
		Request.ActionIndex = Row.ActionDataKey.ActionIndex;
		bool bHasTrailWindow = false;
		for (const auto& Event : Row.Montage->Notifies)
		{
			const auto* Notify = Cast<UCAnimNotifyState_ActionFeedback>(Event.NotifyStateClass);
			if (!Notify || !Path.ContainsTime(Row.Montage, Event.GetTime())) continue;
			const auto Type = ReadField<EActionType>(Notify, TEXT("TriggerActionType"));
			const int32 Index = ReadField<int32>(Notify, TEXT("TriggerActionIndex"));
			if ((Type != EActionType::All && Type != Request.ActionType) || (Index != INDEX_NONE && Index != Request.ActionIndex)) continue;
			const auto* Definition = Trails->FindTrailDefinition(Notify->TriggerKey);
			if (!Definition) continue; // An action feedback window may belong to non-trail VFX/SFX.
			if (Notify->TriggerKey.IsNone() || Definition->ActionFeedbackMatchKey.CalculateMatchTier(Request) == EActionFeedbackMatchTier::None
				|| Definition->NiagaraSystems.IsEmpty() || !FMath::IsFinite(Event.GetDuration()) || Event.GetDuration() <= 0.f)
				return Fail(Error, FString::Printf(TEXT("Invalid or mismatched trail window %s in %s."), *Notify->TriggerKey.ToString(), *Row.Montage->GetPathName()));
			int32 Definitions = 0;
			for (const auto& Entry : Trails->TrailDefinitions) if (Entry.TriggerKey == Notify->TriggerKey) ++Definitions;
			if (Definitions != 1) return Fail(Error, TEXT("Duplicate trail trigger key in configured weapon data."));
			for (const auto& System : Definition->NiagaraSystems)
				if (!IsValid(System)) return Fail(Error, TEXT("CounterRush trail contains a missing Niagara system."));
			bHasTrailWindow = true;
		}
		return bHasTrailWindow || Fail(Error, TEXT("CounterRush has no reachable trail window matching the configured weapon data and action key."));
	}

	bool ValidatePlayer(AActor* Defaults, bool bRequireStellarSetup, FString& Error)
	{
		Error.Reset();
		if (!IsValid(Defaults)) return Fail(Error, TEXT("Player Blueprint has no valid actor CDO."));
		const auto* Reactions = Defaults->FindComponentByClass<UCReactionComponent>();
		const auto* Actions = Defaults->FindComponentByClass<UCActionComponent>();
		const auto* Source = Defaults->FindComponentByClass<UCCombatSignalSourceComponent>();
		const auto* Feedback = Defaults->FindComponentByClass<UCReactionFeedbackComponent>();
		if (!Reactions || !Actions || !Source || !Feedback) return Fail(Error, TEXT("Missing reaction/action/combat source/feedback component."));
		const auto* Body = ReadObject<UMaterialInterface>(Feedback, TEXT("PerfectBodyMaterial"));
		const auto* Weapon = ReadObject<UMaterialInterface>(Feedback, TEXT("CounterWeaponMaterial"));
		if (!IsValid(Body) || !IsValid(Weapon)) return Fail(Error, TEXT("PerfectBodyMaterial and CounterWeaponMaterial must reference valid materials."));
		UE_LOG(LogTemp, Display, TEXT("DodgeAssets current %s body=%s weapon=%s"), *Defaults->GetPathName(), *Body->GetPathName(), *Weapon->GetPathName());
		const auto& Parts = ReadField<TArray<FName>>(Feedback, TEXT("PerfectBodyParts"));
		if (bRequireStellarSetup && (!Parts.Contains(TEXT("HEAD")) || !Parts.Contains(TEXT("NECK"))))
			return Fail(Error, TEXT("Stellar body glow requires HEAD and NECK parts."));
		int32 PerfectRows = 0;
		for (const auto& Row : ReadField<TArray<FReactionData>>(Reactions, TEXT("ReactionDatas")))
		{
			if (Row.ReactionDataKey.ReactionType != EReactionType::PerfectDodge) continue;
			if (!ValidatePerfectReaction(Row, Error)) return false;
			++PerfectRows;
			UE_LOG(LogTemp, Display, TEXT("DodgeAssets current perfect montage=%s"), *Row.Montage->GetPathName());
		}
		if (!PerfectRows) return Fail(Error, TEXT("No configured PerfectDodge reaction rows."));
		int32 RushRows = 0;
		for (const auto& Row : ReadField<TArray<FActionData>>(Actions, TEXT("ActionDatas")))
		{
			if (Row.ActionDataKey.ActionType != EActionType::CounterRush) continue;
			const auto* Weapons = Defaults->FindComponentByClass<UCWeaponComponent>();
			const UClass* WeaponClass = ReadObject<UClass>(Weapons, TEXT("WeaponActorClass"));
			const UObject* WeaponDefaults = WeaponClass && WeaponClass->IsChildOf(ACWeaponActor::StaticClass()) ? WeaponClass->GetDefaultObject() : nullptr;
			const auto* Trails = ReadObject<UWeaponTrailDataAsset>(WeaponDefaults, TEXT("WeaponTrailDataAsset"));
			if (!ValidateCounterRush(Row, Trails, Error)) return false;
			FDamageSpecKey Key;
			Key.WeaponType = ReadField<EWeaponType>(Weapons, TEXT("WeaponActorClassKey"));
			Key.ActionType = Row.ActionDataKey.ActionType;
			Key.ActionIndex = Row.ActionDataKey.ActionIndex;
			if (!ReadField<TMap<FDamageSpecKey, FDamageSpec>>(Source, TEXT("DamageSpecContainer")).Contains(Key))
				return Fail(Error, FString::Printf(TEXT("Missing damage spec for configured weapon %d CounterRush index %d."), int32(Key.WeaponType), Key.ActionIndex));
			++RushRows;
			UE_LOG(LogTemp, Display, TEXT("DodgeAssets current rush montage=%s weaponClass=%s trails=%s"), *Row.Montage->GetPathName(), *WeaponClass->GetPathName(), *Trails->GetPathName());
		}
		if (bRequireStellarSetup && !RushRows) return Fail(Error, TEXT("No configured Stellar CounterRush action rows."));
		return true;
	}
}
