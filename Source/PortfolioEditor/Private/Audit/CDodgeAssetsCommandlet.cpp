#include "Audit/CDodgeAssetsCommandlet.h"
#include "Action/CAction_CounterRush.h"
#include "Reaction/CReaction_PerfectDodge.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Notify/CAnimNotify_CompleteAction.h"
#include "Notify/CAnimNotify_CompleteReaction.h"
#include "Notify/CAnimNotify_PerfectDodgePresentation.h"
#include "Notify/CAnimNotify_WeaponGlow.h"
#include "Notify/CAnimNotifyState_HitContext.h"
#include "Notify/CAnimNotifyState_Collision.h"
#include "Notify/CAnimNotifyState_ActionFeedback.h"
#include "DataAsset/CWeaponTrailDataAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

namespace DodgeAssets
{
	template<typename T> T& Field(UObject* InObject, const TCHAR* InName)
	{
		FProperty* property = FindFProperty<FProperty>(InObject->GetClass(), InName);
		check(property);
		return *property->ContainerPtrToValuePtr<T>(InObject);
	}
	bool Save(UObject* InAsset)
	{
		InAsset->MarkPackageDirty();
		FSavePackageArgs args;
		args.TopLevelFlags = RF_Public | RF_Standalone;
		const FString file = FPackageName::LongPackageNameToFilename(InAsset->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
		return UPackage::SavePackage(InAsset->GetOutermost(), InAsset, *file, args);
	}
	template<typename T> T* Copy(T* InSource, const FString& InPath)
	{
		if (!InSource || FPackageName::DoesPackageExist(InPath)) return nullptr;
		T* asset = DuplicateObject<T>(InSource, CreatePackage(*InPath), *FPackageName::GetLongPackageAssetName(InPath));
		asset->SetFlags(RF_Public | RF_Standalone);
		return asset;
	}
	void Complete(UAnimMontage* InMontage, bool bPerfect, float InTime)
	{
		FAnimNotifyEvent& event = InMontage->Notifies.AddDefaulted_GetRef();
		if (bPerfect)
		{
			auto* notify = NewObject<UCAnimNotify_CompleteReaction>(InMontage);
			Field<EReactionType>(notify, TEXT("TriggerReactionType")) = EReactionType::PerfectDodge;
			event.Notify = notify;
		}
		else
		{
			auto* notify = NewObject<UCAnimNotify_CompleteAction>(InMontage);
			Field<EActionType>(notify, TEXT("TriggerActionType")) = EActionType::CounterRush;
			Field<int32>(notify, TEXT("TriggerActionIndex")) = 0;
			event.Notify = notify;
		}
		event.Link(InMontage, InTime);
		event.TriggerWeightThreshold = 0.f;
	}
	template<typename T> void Window(UAnimMontage* InMontage, float InStart, float InEnd)
	{
		T* notify = NewObject<T>(InMontage);
		Field<EActionType>(notify, TEXT("TriggerActionType")) = EActionType::CounterRush;
		Field<int32>(notify, TEXT("TriggerActionIndex")) = 0;
		FAnimNotifyEvent& event = InMontage->Notifies.AddDefaulted_GetRef();
		event.NotifyStateClass = notify;
		event.Link(InMontage, InStart);
		event.SetDuration(InEnd - InStart);
		event.EndLink.Link(InMontage, InEnd);
		event.TriggerWeightThreshold = 0.f;
	}
}

UCDodgeAssetsCommandlet::UCDodgeAssetsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UCDodgeAssetsCommandlet::Main(const FString& Params)
{
	using namespace DodgeAssets;
	const bool bApply = FParse::Param(*Params, TEXT("Apply"));
	// Resolve the BP's actual montage references: user-renamed assets are authoritative.
	if (FParse::Param(*Params, TEXT("WeaponGlowOnly")))
	{
		TMap<UAnimMontage*, bool> montages;
		for (const FString suffix : {FString(), FString(TEXT("_Stellar"))})
		{
			auto* bp = LoadObject<UBlueprint>(nullptr, *(TEXT("/Game/01_Character/01_Player/BP_CPlayer") + suffix));
			auto* cdo = bp && bp->GeneratedClass ? Cast<AActor>(bp->GeneratedClass->GetDefaultObject()) : nullptr;
			if (!cdo) return 1;
			auto* reactions = cdo->FindComponentByClass<UCReactionComponent>();
			auto* actions = cdo->FindComponentByClass<UCActionComponent>();
			if (!reactions || !actions) return 1;
			for (const auto& row : Field<TArray<FReactionData>>(reactions, TEXT("ReactionDatas")))
				if (row.ReactionDataKey.ReactionType == EReactionType::PerfectDodge && row.Montage) montages.Add(row.Montage, false);
			for (const auto& row : Field<TArray<FActionData>>(actions, TEXT("ActionDatas")))
				if (row.ActionDataKey.ActionType == EActionType::CounterRush && row.Montage) montages.Add(row.Montage, true);
		}
		if (montages.IsEmpty()) return 1;
		for (const auto& pair : montages)
		{
			auto* montage = pair.Key;
			const bool rush = pair.Value;
			float complete = montage->GetPlayLength();
			float collisionEnd = -1.f;
			for (const auto& event : montage->Notifies)
			{
				UE_LOG(LogTemp, Display, TEXT("GlowAudit %s time=%.3f end=%.3f notify=%s"), *montage->GetPathName(), event.GetTime(), event.GetEndTriggerTime(), *GetNameSafe(event.Notify ? static_cast<UObject*>(event.Notify.Get()) : static_cast<UObject*>(event.NotifyStateClass.Get())));
				if (rush ? bool(Cast<UCAnimNotify_CompleteAction>(event.Notify)) : bool(Cast<UCAnimNotify_CompleteReaction>(event.Notify))) complete = FMath::Min(complete, event.GetTime());
				if (Cast<UCAnimNotifyState_Collision>(event.NotifyStateClass)) collisionEnd = FMath::Max(collisionEnd, event.GetEndTriggerTime());
			}
			if (complete <= .03f) return 1;
			const float end = rush && collisionEnd > 0.f ? FMath::Min(collisionEnd, complete - .01f) : complete - .01f;
			bool changed = false;
			for (EWeaponGlowCue cue : {EWeaponGlowCue::Start, EWeaponGlowCue::End})
			{
				if (rush && cue == EWeaponGlowCue::Start) continue; // Keep transferred playback and original deadline.
				int32 count = 0;
				for (const auto& event : montage->Notifies)
					if (auto* notify = Cast<UCAnimNotify_WeaponGlow>(event.Notify); notify && notify->Cue == cue)
					{
						if (notify->bCounterRush != rush) return 1;
						++count;
					}
				if (count > 1) return 1;
				if (!count && bApply)
				{
					auto* notify = NewObject<UCAnimNotify_WeaponGlow>(montage);
					notify->Cue = cue;
					notify->bCounterRush = rush;
					auto& event = montage->Notifies.AddDefaulted_GetRef();
					event.Notify = notify;
					event.NotifyName = FName(*notify->GetNotifyName());
					event.Link(montage, cue == EWeaponGlowCue::Start ? .01f : end);
					event.TriggerWeightThreshold = 0.f;
					changed = true;
				}
				if (!count && !bApply && FParse::Param(*Params, TEXT("Verify"))) return 1;
			}
			UE_LOG(LogTemp, Display, TEXT("GlowMontage %s rush=%d length=%.3f plannedEnd=%.3f changed=%d"), *montage->GetPathName(), rush, montage->GetPlayLength(), end, changed);
			if (changed)
			{
				montage->RefreshCacheData();
				if (!Save(montage)) return 1;
			}
		}
		return 0;
	}
	// Narrow authoring path: preserve all other user-edited BP and montage settings.
	if (FParse::Param(*Params, TEXT("HairOnly")))
	{
		UBlueprint* bp = LoadObject<UBlueprint>(nullptr, TEXT("/Game/01_Character/01_Player/BP_CPlayer_Stellar"));
		AActor* cdo = bp && bp->GeneratedClass ? Cast<AActor>(bp->GeneratedClass->GetDefaultObject()) : nullptr;
		auto* feedback = cdo ? cdo->FindComponentByClass<UCReactionFeedbackComponent>() : nullptr;
		if (!feedback) return 1;
		auto& parts = Field<TArray<FName>>(feedback, TEXT("PerfectBodyParts"));
		if (bApply)
		{
			parts.AddUnique(TEXT("BANG"));
			parts.AddUnique(TEXT("HAIR"));
			if (!DodgeAssets::Save(bp)) return 1;
		}
		for (const FName part : parts) UE_LOG(LogTemp, Display, TEXT("PerfectBodyPart=%s"), *part.ToString());
		return parts.Contains(TEXT("BANG")) && parts.Contains(TEXT("HAIR")) ? 0 : 1;
	}
	if (bApply && !FParse::Param(*Params, TEXT("LegacyBootstrapApply")))
	{
		UE_LOG(LogTemp, Error, TEXT("Full bootstrap overwrites presentation defaults. Use narrow authoring modes, or explicitly opt in with -LegacyBootstrapApply."));
		return 1;
	}
	const FString bodyPath = TEXT("/Game/CharacterOverlay/Materials/Material_Instance/MI_Fresnel");
	UMaterialInterface* body = LoadObject<UMaterialInterface>(nullptr, *bodyPath, nullptr, LOAD_NoWarn);
	if (!body) return 1;
	const FString root = TEXT("/Game/04_Montage/Dodge/");
	for (const FString variant : { FString(TEXT("Default")), FString(TEXT("Stellar")) })
	{
		const bool bStellar = variant == TEXT("Stellar");
		const FString suffix = bStellar ? TEXT("_Stellar") : TEXT("");
		UBlueprint* bp = LoadObject<UBlueprint>(nullptr, *(TEXT("/Game/01_Character/01_Player/BP_CPlayer") + suffix));
		AActor* cdo = bp && bp->GeneratedClass ? Cast<AActor>(bp->GeneratedClass->GetDefaultObject()) : nullptr;
		if (!cdo) return 1;
		auto* reactions = cdo->FindComponentByClass<UCReactionComponent>();
		auto* actions = cdo->FindComponentByClass<UCActionComponent>();
		auto* source = cdo->FindComponentByClass<UCCombatSignalSourceComponent>();
		auto* feedback = cdo->FindComponentByClass<UCReactionFeedbackComponent>();
		if (!reactions || !actions || !source || !feedback) return 1;
		if (bApply) Field<UMaterialInterface*>(feedback, TEXT("PerfectBodyMaterial")) = body;
		else if (Field<UMaterialInterface*>(feedback, TEXT("PerfectBodyMaterial")) != body) return 1;
		if (bApply)
		{
			Field<UMaterialInterface*>(feedback, TEXT("CounterWeaponMaterial")) = body;
			if (bStellar)
			{
				auto& parts = Field<TArray<FName>>(feedback, TEXT("PerfectBodyParts"));
				for (FName part : {FName(TEXT("HEAD")), FName(TEXT("NECK")), FName(TEXT("BANG")), FName(TEXT("HAIR"))}) parts.AddUnique(part);
			}
		}
		else
		{
			if (Field<UMaterialInterface*>(feedback, TEXT("CounterWeaponMaterial")) != body) return 1;
			const auto& parts = Field<TArray<FName>>(feedback, TEXT("PerfectBodyParts"));
			if (bStellar && (!parts.Contains(TEXT("HEAD")) || !parts.Contains(TEXT("NECK")))) return 1;
		}
		const FString montageSuffix = bStellar ? TEXT("_B_Root_Stellar") : TEXT("");
		const FString perfectPath = root + variant + TEXT("/M_PerfectDodge") + montageSuffix;
		UAnimMontage* perfect = LoadObject<UAnimMontage>(nullptr, *perfectPath, nullptr, LOAD_NoWarn);
		if (!perfect && bApply)
		{
			UAnimMontage* dodge = LoadObject<UAnimMontage>(nullptr, *(root + variant + TEXT("/M_Dodge") + montageSuffix));
			perfect = Copy(dodge, perfectPath);
			if (!perfect) return 1;
			perfect->Notifies.Reset();
			Complete(perfect, true, 1.3f);
			perfect->RefreshCacheData();
			if (!DodgeAssets::Save(perfect)) return 1;
		}
		if (!perfect) return 1;
		bool addedPresentation = false;
		for (EPerfectDodgePresentationCue cue : { EPerfectDodgePresentationCue::Slow, EPerfectDodgePresentationCue::Vignette })
		{
			int32 count = 0;
			for (const FAnimNotifyEvent& event : perfect->Notifies)
				if (const auto* notify = Cast<UCAnimNotify_PerfectDodgePresentation>(event.Notify); notify && notify->Cue == cue)
				{
					++count;
					if (!notify->Envelope.IsValid() || notify->Envelope.Duration() <= 0.
						|| event.GetTime() < 0.f || event.GetTime() >= perfect->GetPlayLength()) return 1;
				}
			if (count > 1 || (count == 0 && !bApply)) return 1;
			if (count == 0)
			{
				auto* notify = NewObject<UCAnimNotify_PerfectDodgePresentation>(perfect);
				notify->Cue = cue;
				const bool slow = cue == EPerfectDodgePresentationCue::Slow;
				notify->Envelope.FadeIn = slow ? .03f : .06f;
				notify->Envelope.Hold = slow ? .12f : .14f;
				notify->Envelope.FadeOut = slow ? .07f : .2f;
				auto& event = perfect->Notifies.AddDefaulted_GetRef();
				event.Notify = notify;
				event.Link(perfect, .01f);
				event.TriggerWeightThreshold = 0.f;
				addedPresentation = true;
			}
		}
		if (addedPresentation)
		{
			perfect->RefreshCacheData();
			if (!DodgeAssets::Save(perfect)) return 1;
		}
		auto& reactionRows = Field<TArray<FReactionData>>(reactions, TEXT("ReactionDatas"));
		const FReactionData* found = reactionRows.FindByPredicate([](const FReactionData& InData)
			{ return InData.ReactionDataKey.ReactionType == EReactionType::PerfectDodge; });
		if (bApply && !found)
		{
			FReactionData row;
			row.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
			row.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
			row.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
			row.Priority = 100;
			row.Montage = perfect;
			reactionRows.Add(row);
		}
		else if (!found || found->Montage != perfect || found->ReactionExecutorKey != UCReaction_PerfectDodge::StaticClass()) return 1;
		if (bStellar)
		{
			const FString sequencePath = TEXT("/Game/03_Animation/Counter/CounterRush_Locked");
			UAnimSequence* sequence = LoadObject<UAnimSequence>(nullptr, *sequencePath, nullptr, LOAD_NoWarn);
			// Source was deliberately removed during asset consolidation. Never recreate stale paths.
			if (!sequence || !sequence->bForceRootLock || sequence->bEnableRootMotion) return 1;
			// Dedicated playback owns all gameplay notifies on the montage, not the source sequence.
			if (bApply && !sequence->Notifies.IsEmpty())
			{
				sequence->Notifies.Reset();
				sequence->RefreshCacheData();
				if (!DodgeAssets::Save(sequence)) return 1;
			}
			if (!sequence->Notifies.IsEmpty()) return 1;
			const FString rushPath = root + TEXT("Stellar/M_CounterRush_Stellar");
			UAnimMontage* rush = LoadObject<UAnimMontage>(nullptr, *rushPath, nullptr, LOAD_NoWarn);
			if (!rush && bApply)
			{
				rush = Copy(perfect, rushPath);
				if (!rush || rush->SlotAnimTracks.IsEmpty()) return 1;
				rush->Notifies.Reset();
				rush->SlotAnimTracks.SetNum(1);
				FAnimSegment segment;
				segment.SetAnimReference(sequence, true);
				segment.LoopingCount = 1;
				rush->SlotAnimTracks[0].AnimTrack.AnimSegments = {segment};
				Field<float>(rush, TEXT("SequenceLength")) = sequence->GetPlayLength();
				rush->CompositeSections.Reset();
				rush->AddAnimCompositeSection(TEXT("Default"), 0.f);
				Window<UCAnimNotifyState_HitContext>(rush, 0.27f, 0.8f);
				Window<UCAnimNotifyState_Collision>(rush, 0.3f, 0.7f);
				Complete(rush, false, 1.5f);
				rush->RefreshCacheData();
				if (!DodgeAssets::Save(rush)) return 1;
			}
			if (!rush) return 1;
			UWeaponTrailDataAsset* trails = LoadObject<UWeaponTrailDataAsset>(nullptr, TEXT("/Game/09_DataAsset/DA_WeaponTrail_ScifiSword"));
			if (!trails) return 1;
			const FName rushTrail = TEXT("CounterRush_Trail");
			const auto* originalTrail = trails->TrailDefinitions.FindByPredicate([](const FWeaponTrailDefinition& InDefinition)
				{ return InDefinition.TriggerKey == TEXT("ScifiSword_Trail"); });
			if (!originalTrail || originalTrail->NiagaraSystems.IsEmpty()) return 1;
			const bool hasTrail = trails->TrailDefinitions.ContainsByPredicate([rushTrail](const FWeaponTrailDefinition& InDefinition)
				{ return InDefinition.TriggerKey == rushTrail; });
			if (!hasTrail && bApply)
			{
				FWeaponTrailDefinition definition = *originalTrail;
				definition.TriggerKey = rushTrail;
				definition.ActionFeedbackMatchKey.ActionType = EActionType::CounterRush;
				definition.ActionFeedbackMatchKey.ActionIndex = 0;
				trails->TrailDefinitions.Add(definition);
				if (!DodgeAssets::Save(trails)) return 1;
			}
			else if (!hasTrail) return 1;
			const bool hasTrailWindow = rush->Notifies.ContainsByPredicate([rushTrail](const FAnimNotifyEvent& InEvent)
			{
				const auto* notify = Cast<UCAnimNotifyState_ActionFeedback>(InEvent.NotifyStateClass);
				return notify && notify->TriggerKey == rushTrail;
			});
			if (!hasTrailWindow && bApply)
			{
				Window<UCAnimNotifyState_ActionFeedback>(rush, 0.02f, 0.75f);
				CastChecked<UCAnimNotifyState_ActionFeedback>(rush->Notifies.Last().NotifyStateClass)->TriggerKey = rushTrail;
				rush->RefreshCacheData();
				if (!DodgeAssets::Save(rush)) return 1;
			}
			else if (!hasTrailWindow) return 1;
			auto& actionRows = Field<TArray<FActionData>>(actions, TEXT("ActionDatas"));
			const FActionData* existing = actionRows.FindByPredicate([](const FActionData& InData)
				{ return InData.ActionDataKey.ActionType == EActionType::CounterRush; });
			if (bApply && !existing)
			{
				FActionData row;
				row.ActionDataKey.ActionType = EActionType::CounterRush;
				row.ActionDataKey.ActionIndex = 0;
				row.ActionExecutorKey = UCAction_CounterRush::StaticClass();
				row.Priority = 100;
				row.Montage = rush;
				actionRows.Add(row);
			}
			else if (!existing || existing->Montage != rush) return 1;
			FDamageSpecKey key;
			key.WeaponType = EWeaponType::Sword;
			key.ActionType = EActionType::CounterRush;
			key.ActionIndex = 0;
			auto& specs = Field<TMap<FDamageSpecKey, FDamageSpec>>(source, TEXT("DamageSpecContainer"));
			if (bApply && !specs.Contains(key))
			{
				FDamageSpec spec;
				spec.BaseDamage = 20.f;
				specs.Add(key, spec);
			}
			else if (!specs.Contains(key)) return 1;
		}
		if (bApply && !DodgeAssets::Save(bp)) return 1;
		UE_LOG(LogTemp, Display, TEXT("DodgeAssets %s: %s"), *variant, bApply ? TEXT("Applied") : TEXT("Verified persisted data"));
	}
	return 0;
}
