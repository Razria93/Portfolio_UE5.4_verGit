#include "Audit/CDodgeAssetsCommandlet.h"
#include "Action/CAction_CounterRush.h"
#include "Reaction/CReaction_PerfectDodge.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Component/CDefenseComponent.h"
#include "Notify/CAnimNotify_CounterOpportunity.h"
#include "Notify/CAnimNotify_BodyGlow.h"
#include "MontageSectionPath.h"
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
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/Skeleton.h"
#include "NiagaraSystem.h"
#include "Components/SkeletalMeshComponent.h"

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
	if (FParse::Param(*Params, TEXT("BodyGlowPoints")))
	{
		const bool bVerify = FParse::Param(*Params, TEXT("Verify"));
		if (bApply && bVerify) return 1;
		float requestedStart = .01f, requestedEnd = .3f;
		const bool hasStart = FParse::Value(*Params, TEXT("StartTime="), requestedStart);
		const bool hasEnd = FParse::Value(*Params, TEXT("EndTime="), requestedEnd);
		auto* bp = LoadObject<UBlueprint>(nullptr, TEXT("/Game/01_Character/01_Player/BP_CPlayer_Stellar"));
		auto* cdo = bp && bp->GeneratedClass ? Cast<AActor>(bp->GeneratedClass->GetDefaultObject()) : nullptr;
		auto* reactions = cdo ? cdo->FindComponentByClass<UCReactionComponent>() : nullptr;
		auto* feedback = cdo ? cdo->FindComponentByClass<UCReactionFeedbackComponent>() : nullptr;
		if (!reactions || !feedback) return 1;
		const auto& legacy = Field<FCombatPresentationEnvelope>(feedback, TEXT("PerfectBodyEnvelope"));
		if (!legacy.IsValid()) return 1;
		UE_LOG(LogTemp, Display, TEXT("BodyGlow legacy REAL seconds: delay=%.3f in=%.3f hold=%.3f out=%.3f. Do not copy hold into montage seconds."),
			legacy.Delay, legacy.FadeIn, legacy.Hold, legacy.FadeOut);
		struct FPointPlan { UAnimMontage* Montage; float Start; float End; bool AddStart; bool AddEnd; };
		TArray<FPointPlan> plans;
		TSet<UAnimMontage*> seen;
		for (const auto& row : Field<TArray<FReactionData>>(reactions, TEXT("ReactionDatas")))
		{
			if (row.ReactionDataKey.ReactionType != EReactionType::PerfectDodge) continue;
			auto* montage = row.Montage;
			if (!montage) return 1;
			const auto path = ExecutionMontageAudit::BuildSectionPath(montage, row.StartSectionName, row.PlayRate);
			if (!path.Error.IsEmpty() || !path.HasLinearTimeline()) return 1;
			float terminal = montage->GetPlayLength();
			for (const auto& event : montage->Notifies)
			{
				UE_LOG(LogTemp, Display, TEXT("BodyGlow current %s time=%.3f end=%.3f class=%s reachable=%d playRate=%.3f"),
					*montage->GetPathName(), event.GetTime(), event.GetEndTriggerTime(),
					*GetNameSafe(event.Notify ? event.Notify->GetClass() : (event.NotifyStateClass ? event.NotifyStateClass->GetClass() : nullptr)),
					path.ContainsTime(montage, event.GetTime()), row.PlayRate * montage->RateScale);
				if (!path.ContainsTime(montage, event.GetTime())) continue;
				if (Cast<UCAnimNotify_CompleteReaction>(event.Notify)) terminal = FMath::Min(terminal, event.GetTime());
				if (const auto* slow = Cast<UCAnimNotify_PerfectDodgePresentation>(event.Notify); slow && slow->Cue == EPerfectDodgePresentationCue::Slow)
					UE_LOG(LogTemp, Display, TEXT("BodyGlow slow context %s montageTime=%.3f player=%d dilation=%.3f realEnvelope=%.3f/%.3f/%.3f/%.3f playRate=%.3f"),
						*montage->GetPathName(), event.GetTime(), slow->bAffectPlayer, slow->SlowDilation,
						slow->Envelope.Delay, slow->Envelope.FadeIn, slow->Envelope.Hold, slow->Envelope.FadeOut, row.PlayRate * montage->RateScale);
			}
			FPointPlan plan{montage, requestedStart, requestedEnd, true, true};
			for (const auto& event : montage->Notifies)
			{
				const auto* notify = Cast<UCAnimNotify_BodyGlow>(event.Notify);
				if (!notify) continue;
				if (!FMath::IsFinite(notify->PeakWeight) || notify->PeakWeight < 0.f || notify->PeakWeight > 1.f
					|| !FMath::IsFinite(notify->FadeIn) || notify->FadeIn < 0.f
					|| !FMath::IsFinite(notify->FadeOut) || notify->FadeOut < 0.f
					|| !FMath::IsFinite(notify->SafetyTimeout) || notify->SafetyTimeout <= 0.f) return 1;
				bool& absent = notify->bStart ? plan.AddStart : plan.AddEnd;
				if (!absent) return 1;
				absent = false;
				(notify->bStart ? plan.Start : plan.End) = event.GetTime();
			}
			if (!FMath::IsFinite(plan.Start) || !FMath::IsFinite(plan.End) || plan.Start <= 0.f
				|| plan.End <= plan.Start || plan.End >= terminal
				|| !path.ContainsTime(montage, plan.Start) || !path.ContainsTime(montage, plan.End))
			{
				UE_LOG(LogTemp, Error, TEXT("BodyGlow: %s requires reachable 0 < Start < End < completion %.3f."), *montage->GetPathName(), terminal);
				return 1;
			}
			if ((plan.AddStart || plan.AddEnd) && (bVerify || (bApply && (!hasStart || !hasEnd))))
			{
				UE_LOG(LogTemp, Error, TEXT("BodyGlow: missing points; Apply requires explicit -StartTime= and -EndTime= after reviewing slow context."));
				return 1;
			}
			UE_LOG(LogTemp, Display, TEXT("BodyGlow plan %s Start=%.3f End=%.3f add=%d/%d%s"), *montage->GetPathName(), plan.Start, plan.End,
				plan.AddStart, plan.AddEnd, (!hasStart || !hasEnd) ? TEXT(" (missing argument defaults are preview only)") : TEXT(""));
			if (!seen.Contains(montage)) { plans.Add(plan); seen.Add(montage); }
		}
		if (plans.IsEmpty()) return 1;
		if (!bApply) return 0;
		// All current montage paths have passed preflight. Only add missing points; never retime user points.
		for (const auto& plan : plans)
		{
			if (!plan.AddStart && !plan.AddEnd) continue;
			for (bool start : {true, false})
			{
				if (!(start ? plan.AddStart : plan.AddEnd)) continue;
				auto* notify = NewObject<UCAnimNotify_BodyGlow>(plan.Montage);
				notify->bStart = start;
				notify->PeakWeight = 1.f;
				notify->FadeIn = legacy.FadeIn;
				notify->FadeOut = legacy.FadeOut;
				notify->SafetyTimeout = 8.f;
				auto& event = plan.Montage->Notifies.AddDefaulted_GetRef();
				event.Notify = notify;
				event.NotifyName = FName(*notify->GetNotifyName());
				event.Link(plan.Montage, start ? plan.Start : plan.End);
				event.TriggerWeightThreshold = 0.f;
			}
			plan.Montage->RefreshCacheData();
			if (!DodgeAssets::Save(plan.Montage)) return 1;
		}
		return 0;
	}
	if (FParse::Param(*Params, TEXT("CounterOpportunityWindow")))
	{
		const bool bVerify = FParse::Param(*Params, TEXT("Verify"));
		if (bApply && bVerify) return 1;
		auto* bp = LoadObject<UBlueprint>(nullptr, TEXT("/Game/01_Character/01_Player/BP_CPlayer_Stellar"));
		auto* cdo = bp && bp->GeneratedClass ? Cast<AActor>(bp->GeneratedClass->GetDefaultObject()) : nullptr;
		auto* reactions = cdo ? cdo->FindComponentByClass<UCReactionComponent>() : nullptr;
		auto* defense = cdo ? cdo->FindComponentByClass<UCDefenseComponent>() : nullptr;
		if (!reactions || !defense) return 1;
		auto& tuning = Field<FDefenseDodgeTuning>(defense, TEXT("DodgeTuning"));
		TMap<UAnimMontage*, float> missingOpens;
		int32 rows = 0;
		// Preflight every configured path before changing any object or saving any package.
		for (const auto& row : Field<TArray<FReactionData>>(reactions, TEXT("ReactionDatas")))
		{
			if (row.ReactionDataKey.ReactionType != EReactionType::PerfectDodge) continue;
			++rows;
			auto* montage = row.Montage;
			if (!montage) return 1;
			const auto path = ExecutionMontageAudit::BuildSectionPath(montage, row.StartSectionName, row.PlayRate);
			if (!path.Error.IsEmpty() || !path.HasLinearTimeline())
			{
				UE_LOG(LogTemp, Error, TEXT("CounterOpportunity: %s requires manual section-path review."), *montage->GetPathName());
				return 1;
			}
			float terminal = montage->GetPlayLength();
			for (const auto& event : montage->Notifies)
				if (path.ContainsTime(montage, event.GetTime()) && Cast<UCAnimNotify_CompleteReaction>(event.Notify))
					terminal = FMath::Min(terminal, event.GetTime());
			int32 opens = 0;
			float opening = tuning.CounterConnectStart;
			for (const auto& event : montage->Notifies)
			{
				const auto* notify = Cast<UCAnimNotify_CounterOpportunity>(event.Notify);
				if (!notify) continue;
				if (!FMath::IsFinite(event.GetTime()) || event.GetTime() < 0.f || event.GetTime() >= terminal
					|| !path.ContainsTime(montage, event.GetTime())) return 1;
				if (notify->bOpen) { ++opens; opening = event.GetTime(); }
			}
			if (opens > 1 || !FMath::IsFinite(opening) || opening < 0.f || opening >= terminal
				|| !path.ContainsTime(montage, opening))
			{
				UE_LOG(LogTemp, Error, TEXT("CounterOpportunity: %s opening %.3f must be reachable before completion %.3f."), *montage->GetPathName(), opening, terminal);
				return 1;
			}
			for (const auto& event : montage->Notifies)
				if (const auto* notify = Cast<UCAnimNotify_CounterOpportunity>(event.Notify); notify && !notify->bOpen && event.GetTime() <= opening) return 1;
			if (!opens) missingOpens.Add(montage, opening);
			UE_LOG(LogTemp, Display, TEXT("CounterOpportunity: %s open=%.3f existing=%d terminal=%.3f"), *montage->GetPathName(), opening, opens, terminal);
		}
		if (!rows) return 1;
		if (bVerify) return tuning.bUseMontageCounterWindow && missingOpens.IsEmpty() ? 0 : 1;
		if (!bApply) return 0;
		for (const auto& pair : missingOpens)
		{
			auto* notify = NewObject<UCAnimNotify_CounterOpportunity>(pair.Key);
			notify->bOpen = true;
			auto& event = pair.Key->Notifies.AddDefaulted_GetRef();
			event.Notify = notify;
			event.NotifyName = FName(*notify->GetNotifyName());
			event.Link(pair.Key, pair.Value);
			event.TriggerWeightThreshold = 0.f;
			pair.Key->RefreshCacheData();
			if (!DodgeAssets::Save(pair.Key)) return 1;
		}
		// Enable only after every required marker has been persisted successfully.
		if (!tuning.bUseMontageCounterWindow)
		{
			tuning.bUseMontageCounterWindow = true;
			if (!DodgeAssets::Save(bp)) return 1;
		}
		return 0;
	}
	// Narrow, explicit migration. Does not touch montage, Niagara or combo settings.
	if (FParse::Param(*Params, TEXT("CounterTrailAttachment")))
	{
		auto* mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/06_Weapon/Asset/SciFi_Sword/scifi_sword"));
		auto* data = LoadObject<UWeaponTrailDataAsset>(nullptr, TEXT("/Game/09_DataAsset/DA_WeaponTrail_SciFiSword"));
		if (!mesh || !mesh->GetSkeleton() || !data) return 1;
		auto* skeleton = mesh->GetSkeleton();
		auto* current = data->TrailDefinitions.FindByPredicate([](const FWeaponTrailDefinition& D) { return D.TriggerKey == TEXT("CounterRush_Trail"); });
		// Validation follows current authoring, not the source asset used by the migration.
		if (!bApply)
		{
			if (!current || current->NiagaraSystems.IsEmpty()) return 1;
			if (current->RelativeLocation.ContainsNaN() || current->RelativeRotation.ContainsNaN()
				|| (current->bAttachToSocket && (current->AttachmentSocket.IsNone() || !mesh->FindSocket(current->AttachmentSocket))))
			{
				UE_LOG(LogTemp, Error, TEXT("CounterTrail: invalid attachment socket or relative transform."));
				return 1;
			}
			if (current->bOverrideRibbonWidth && (current->RibbonWidthParameter.IsNone()
				|| !FMath::IsFinite(current->RibbonWidth) || current->RibbonWidth <= 0.f)) return 1;
			for (UNiagaraSystem* system : current->NiagaraSystems)
			{
				if (!system) return 1;
				if (current->bOverrideRibbonWidth)
				{
					TArray<FNiagaraVariable> parameters;
					system->GetExposedParameters().GetParameters(parameters);
					if (!parameters.ContainsByPredicate([current](const FNiagaraVariable& Parameter)
						{ return Parameter.GetName() == current->RibbonWidthParameter && Parameter.GetType() == FNiagaraTypeDefinition::GetFloatDef(); }))
					{
						UE_LOG(LogTemp, Error, TEXT("CounterTrail: %s requires exposed float %s."), *system->GetPathName(), *current->RibbonWidthParameter.ToString());
						return 1;
					}
				}
			}
			UE_LOG(LogTemp, Display, TEXT("CounterTrail: current systems, attachment and width settings verified (read-only)."));
			return 0;
		}
		// This is a one-time Scifi migration; it must never reset arbitrary current art settings.
		const auto* start = mesh->FindSocket(TEXT("TrailStart"));
		const auto* end = mesh->FindSocket(TEXT("TrailEnd"));
		auto* entry = data->TrailDefinitions.FindByPredicate([](const FWeaponTrailDefinition& D) { return D.TriggerKey == TEXT("CounterRush_Trail"); });
		if (!start || !end || start->BoneName != end->BoneName || !entry
			|| entry->NiagaraSystems.Num() != 1 || !entry->NiagaraSystems[0]
			|| entry->NiagaraSystems[0]->GetName() != TEXT("NS_SlashTrail_Scifi_Sample")) return 1;
		const FVector span = end->RelativeLocation - start->RelativeLocation;
		if (span.IsNearlyZero()) return 1;
		const auto& ref = mesh->GetRefSkeleton();
		int32 bone = ref.FindBoneIndex(start->BoneName);
		if (bone == INDEX_NONE) return 1;
		FTransform boneToMesh = ref.GetRefBonePose()[bone];
		while ((bone = ref.GetParentIndex(bone)) != INDEX_NONE)
			boneToMesh = boneToMesh * ref.GetRefBonePose()[bone];
		// Clip the existing guide span to the mesh bounds; preserve the original sockets.
		const FBox bounds = mesh->GetBounds().GetBox();
		const FVector startMesh = boneToMesh.TransformPosition(start->RelativeLocation);
		const FVector spanMesh = boneToMesh.TransformVector(span);
		double endFraction = 1.;
		for (int32 axis = 0; axis < 3; ++axis)
		{
			if (FMath::Abs(spanMesh[axis]) > SMALL_NUMBER)
			{
				const double edge = spanMesh[axis] > 0 ? bounds.Max[axis] : bounds.Min[axis];
				endFraction = FMath::Min(endFraction, (edge - startMesh[axis]) / spanMesh[axis]);
			}
		}
		if (endFraction <= 0.) return 1;
		const FVector fittedSpan = span * endFraction;
		const FVector center = start->RelativeLocation + fittedSpan * .5;
		auto* weaponClass = LoadClass<AActor>(nullptr, TEXT("/Game/06_Weapon/BP_CWeaponActor_SciFi_Sword.BP_CWeaponActor_SciFi_Sword_C"));
		auto* weaponDefault = weaponClass ? weaponClass->GetDefaultObject<AActor>() : nullptr;
		auto* weaponMesh = weaponDefault ? weaponDefault->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
		if (!weaponMesh) return 1;
		const float fittedWidth = (boneToMesh.TransformVector(fittedSpan) * weaponMesh->GetRelativeScale3D()).Size();
		UE_LOG(LogTemp, Display, TEXT("CounterTrail fitted width=%f default mesh scale=%s"), fittedWidth, *weaponMesh->GetRelativeScale3D().ToString());
		UE_LOG(LogTemp, Display, TEXT("CounterTrail reference: start=%s end=%s center=%s meshBounds=%s"),
			*boneToMesh.TransformPosition(start->RelativeLocation).ToString(),
			*boneToMesh.TransformPosition(end->RelativeLocation).ToString(),
			*boneToMesh.TransformPosition(center).ToString(), *mesh->GetBounds().BoxExtent.ToString());
		auto* socket = skeleton->FindSocket(TEXT("FX_CounterTrail"));
		if (bApply)
		{
			// Preserve a socket already authored by the user.
			if (!socket)
			{
				socket = NewObject<USkeletalMeshSocket>(skeleton, NAME_None, RF_Transactional);
				socket->SocketName = TEXT("FX_CounterTrail");
				socket->BoneName = start->BoneName;
				socket->RelativeLocation = center;
				socket->RelativeRotation = FRotationMatrix::MakeFromZ(span).Rotator();
				socket->RelativeScale = FVector::OneVector;
				skeleton->Sockets.Add(socket);
				if (!DodgeAssets::Save(skeleton)) return 1;
			}
			entry->bAttachToSocket = true;
			entry->AttachmentSocket = socket->SocketName;
			entry->RelativeLocation = FVector::ZeroVector;
			entry->RelativeRotation = FRotator::ZeroRotator;
			entry->bOverrideRibbonWidth = true;
			entry->RibbonWidth = fittedWidth;
			if (!DodgeAssets::Save(data)) return 1;
		}
		if (FParse::Param(*Params, TEXT("Verify")) && (!socket || !entry->bAttachToSocket
			|| entry->AttachmentSocket != TEXT("FX_CounterTrail"))) return 1;
		return 0;
	}
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
				if (!DodgeAssets::Save(montage)) return 1;
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
