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
	const FString bodyPath = TEXT("/Game/04_Montage/Dodge/M_PerfectDodgeOverlay");
	UMaterial* body = LoadObject<UMaterial>(nullptr, *bodyPath, nullptr, LOAD_NoWarn);
	if (!body && bApply)
	{
		body = NewObject<UMaterial>(CreatePackage(*bodyPath), TEXT("M_PerfectDodgeOverlay"), RF_Public | RF_Standalone);
		body->BlendMode = BLEND_Translucent;
		body->SetShadingModel(MSM_Unlit);
		bool needsCompile = false;
		body->SetMaterialUsage(needsCompile, MATUSAGE_SkeletalMesh);
		auto* color = NewObject<UMaterialExpressionConstant3Vector>(body);
		color->Constant = FLinearColor(3.f, 0.015f, 0.08f);
		auto* rim = NewObject<UMaterialExpressionFresnel>(body);
		rim->Exponent = 3.f;
		rim->BaseReflectFraction = 0.08f;
		body->GetExpressionCollection().AddExpression(color);
		body->GetExpressionCollection().AddExpression(rim);
		body->GetEditorOnlyData()->EmissiveColor.Expression = color;
		body->GetEditorOnlyData()->Opacity.Expression = rim;
		body->PostEditChange();
		if (!Save(body)) return 1;
	}
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
		const FString perfectPath = root + variant + TEXT("/M_PerfectDodge") + suffix;
		UAnimMontage* perfect = LoadObject<UAnimMontage>(nullptr, *perfectPath, nullptr, LOAD_NoWarn);
		if (!perfect && bApply)
		{
			UAnimMontage* dodge = LoadObject<UAnimMontage>(nullptr, *(root + variant + TEXT("/M_Dodge") + suffix));
			perfect = Copy(dodge, perfectPath);
			if (!perfect) return 1;
			perfect->Notifies.Reset();
			Complete(perfect, true, 1.3f);
			perfect->RefreshCacheData();
			if (!Save(perfect)) return 1;
		}
		if (!perfect) return 1;
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
			const FString sequencePath = TEXT("/Game/03_Animation/Sword/Stellar/DashAttack/A_CounterRush_Locked");
			UAnimSequence* sequence = LoadObject<UAnimSequence>(nullptr, *sequencePath, nullptr, LOAD_NoWarn);
			if (!sequence && bApply)
			{
				sequence = Copy(LoadObject<UAnimSequence>(nullptr, TEXT("/Game/03_Animation/Sword/Stellar/DashAttack/Attack_Sprint_Anim_Stellar")), sequencePath);
				if (!sequence) return 1;
				sequence->bEnableRootMotion = false;
				sequence->bForceRootLock = true;
				sequence->RootMotionRootLock = ERootMotionRootLock::AnimFirstFrame;
				if (!Save(sequence)) return 1;
			}
			if (!sequence || !sequence->bForceRootLock || sequence->bEnableRootMotion) return 1;
			// Dedicated playback owns all gameplay notifies on the montage, not the source sequence.
			if (bApply && !sequence->Notifies.IsEmpty())
			{
				sequence->Notifies.Reset();
				sequence->RefreshCacheData();
				if (!Save(sequence)) return 1;
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
				if (!Save(rush)) return 1;
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
				if (!Save(trails)) return 1;
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
				if (!Save(rush)) return 1;
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
		if (bApply && !Save(bp)) return 1;
		UE_LOG(LogTemp, Display, TEXT("DodgeAssets %s: %s"), *variant, bApply ? TEXT("Applied") : TEXT("Verified persisted data"));
	}
	return 0;
}
