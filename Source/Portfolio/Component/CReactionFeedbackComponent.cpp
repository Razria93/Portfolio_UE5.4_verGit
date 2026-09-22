#include "Component/CReactionFeedbackComponent.h"

#include "ProjectGlobal.h"

#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "Type/CReactionFeedbackTypes.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CWeaponComponent.h"
#include "Weapon/CWeaponActor.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

// Helper
namespace
{
	namespace ReactionFeedbackScore
	{
		constexpr int32 ReactionExact	= 10000;
		constexpr int32 WeaponExact		= 1000;
		constexpr int32 ActionExact		= 100;
		constexpr int32 IndexExact		= 10;
	}
}

UCReactionFeedbackComponent::UCReactionFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PerfectBodyEnvelope.Hold = .4f;
	PerfectBodyEnvelope.FadeOut = .22f;
}

// Component Reference

void UCReactionFeedbackComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	ClearPerfectDodgePresentation();
	LastPerfectResultSerial = 0;
	OwnerCharacter_Injected = InReferences.OwnerCharacter;

	ValidateRequiredComponentReferences();
}

bool UCReactionFeedbackComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Entry

void UCReactionFeedbackComponent::PlayFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest)
{
	if (!CanPlayReactionFeedback(InReactionFeedbackRequest)) return;

	FCombatFeedbackDebug::RecordReactionFeedbackRequestAcceptedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest);
	FCombatFeedbackProfiling::RecordReactionFeedbackRequest();

	if (FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("RuntimeLODSkipEnemyFeedback"));
		FCombatFeedbackProfiling::RecordReactionFeedbackSkipped();
		return;
	}

	ExecuteVFXFeedbacks(InReactionFeedbackRequest);
	ExecuteSFXFeedbacks(InReactionFeedbackRequest);
}

void UCReactionFeedbackComponent::ClearRuntimeFeedback()
{
	// Success cues use their own lifetimes, not the reaction's immunity lifetime.
}

void UCReactionFeedbackComponent::PlayPerfectDodgeSuccess(AActor* InAttacker, uint64 InResultSerial)
{
	if (!IsValid(OwnerCharacter_Injected) || InResultSerial == 0 || InResultSerial == LastPerfectResultSerial) return;
	ClearPerfectDodgePresentation();
	LastPerfectResultSerial = InResultSerial;
	PerfectAttacker = InAttacker;
	const double now = FPlatformTime::Seconds();
	if (IsValid(PerfectBodyMaterial) && BodyEnvelopeState.Start(PerfectBodyEnvelope, now))
	{
		TArray<UMeshComponent*> parts;
		parts.Add(OwnerCharacter_Injected->GetMesh());
		TArray<USkeletalMeshComponent*> candidates;
		OwnerCharacter_Injected->GetComponents(candidates);
		for (USkeletalMeshComponent* part : candidates)
			if (part->GetSkinnedAsset() && (PerfectBodyParts.Contains(part->GetFName())
				|| (!PerfectBodyPartTag.IsNone() && part->ComponentHasTag(PerfectBodyPartTag)))) parts.AddUnique(part);
		BodyOverlay.Start(this, parts, PerfectBodyMaterial, GlowWeightParameter);
	}
	const auto* weaponComp = OwnerCharacter_Injected->FindComponentByClass<UCWeaponComponent>();
	GlowWeapon = weaponComp ? weaponComp->GetWeaponActor() : nullptr;
	if (GlowWeapon.IsValid() && IsValid(CounterWeaponMaterial) && WeaponEnvelopeState.Start(CounterWeaponEnvelope, now, true))
	{
		TArray<UMeshComponent*> parts;
		GlowWeapon->GetComponents(parts);
		if (WeaponOverlay.Start(this, parts, CounterWeaponMaterial, GlowWeightParameter)) WeaponOpportunitySerial = InResultSerial;
		else RestoreCounterWeapon();
	}
}

void UCReactionFeedbackComponent::StartPerfectDodgeSlow(const FCombatPresentationEnvelope& InEnvelope,
	float InDilation, bool bInPlayer, bool bInAttacker)
{
	if (!IsValid(OwnerCharacter_Injected) || !InEnvelope.IsValid() || InEnvelope.Duration() <= 0.
		|| !FMath::IsFinite(InDilation) || InDilation < 0.f || InDilation > 1.f || (!bInPlayer && !bInAttacker)) return;
	if (auto* subsystem = GetWorld() ? GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>() : nullptr)
	{
		subsystem->CancelPerfectDodgeSlow(PerfectSlowSerial);
		PerfectSlowSerial = subsystem->RequestPerfectDodgeSlow(bInPlayer ? OwnerCharacter_Injected : nullptr,
			bInAttacker ? PerfectAttacker.Get() : nullptr, InEnvelope, InDilation);
	}
}

void UCReactionFeedbackComponent::StartPerfectDodgeVignette(const FCombatPresentationEnvelope& InEnvelope, float InPeakIntensity)
{
	if (!IsValid(OwnerCharacter_Injected) || !OwnerCharacter_Injected->IsLocallyControlled()
		|| !InEnvelope.IsValid() || InEnvelope.Duration() <= 0. || !FMath::IsFinite(InPeakIntensity)) return;
	RestorePerfectScreen();
	PerfectCamera = OwnerCharacter_Injected->FindComponentByClass<UCameraComponent>();
	if (!PerfectCamera.IsValid()) return;
	auto& settings = PerfectCamera->PostProcessSettings;
	PreviousVignette = LastWrittenVignette = settings.VignetteIntensity;
	bPreviousVignetteOverride = settings.bOverride_VignetteIntensity;
	PeakVignette = FMath::Clamp(InPeakIntensity, 0.f, 1.f);
	settings.bOverride_VignetteIntensity = true;
	ScreenEnvelopeState.Start(InEnvelope, FPlatformTime::Seconds());
}

void UCReactionFeedbackComponent::TickComponent(float InDeltaTime, ELevelTick InTickType, FActorComponentTickFunction* InTickFunction)
{
	Super::TickComponent(InDeltaTime, InTickType, InTickFunction);
	const UCHealthComponent* health = IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->FindComponentByClass<UCHealthComponent>() : nullptr;
	if (!IsValid(health) || !health->IsAlive()) { ClearPerfectDodgePresentation(); return; }
	const double now = FPlatformTime::Seconds();
	BodyOverlay.SetWeight(BodyEnvelopeState.Evaluate(now));
	if (BodyEnvelopeState.IsFinished(now)) RestorePerfectBody();
	if (WeaponOpportunitySerial != 0)
	{
		const auto* weaponComp = OwnerCharacter_Injected->FindComponentByClass<UCWeaponComponent>();
		const auto* defense = OwnerCharacter_Injected->FindComponentByClass<UCDefenseComponent>();
		if (!GlowWeapon.IsValid() || !weaponComp || weaponComp->GetWeaponActor() != GlowWeapon.Get())
			RestoreCounterWeapon();
		else
		{
			if (!bWeaponTransferredToAttack && (!defense || !defense->IsCounterRushPresentationPending(WeaponOpportunitySerial)))
				ReleaseCounterWeapon(WeaponOpportunitySerial);
			WeaponOverlay.SetWeight(WeaponEnvelopeState.Evaluate(now));
			if (WeaponEnvelopeState.IsFinished(now)) RestoreCounterWeapon();
		}
	}
	if (PerfectCamera.IsValid())
	{
		auto& settings = PerfectCamera->PostProcessSettings;
		if (!settings.bOverride_VignetteIntensity || !FMath::IsNearlyEqual(settings.VignetteIntensity, LastWrittenVignette))
		{
			// An external camera effect took ownership. Stop writing rather than fighting it.
			PerfectCamera.Reset();
			ScreenEnvelopeState.Reset();
		}
		else if (ScreenEnvelopeState.IsFinished(now)) RestorePerfectScreen();
		else
		{
			LastWrittenVignette = FMath::Lerp(PreviousVignette, PeakVignette, ScreenEnvelopeState.Evaluate(now));
			settings.VignetteIntensity = LastWrittenVignette;
		}
	}
}

void UCReactionFeedbackComponent::RestorePerfectBody()
{
	BodyOverlay.Restore();
	BodyEnvelopeState.Reset();
}

void UCReactionFeedbackComponent::TransferCounterWeaponToAttack(uint64 InOpportunitySerial)
{
	if (InOpportunitySerial != 0 && WeaponOpportunitySerial == InOpportunitySerial && !WeaponEnvelopeState.bReleased)
		bWeaponTransferredToAttack = true;
}

void UCReactionFeedbackComponent::ReleaseCounterWeapon(uint64 InOpportunitySerial)
{
	if (InOpportunitySerial != 0 && WeaponOpportunitySerial == InOpportunitySerial)
		WeaponEnvelopeState.Release(FPlatformTime::Seconds());
}

void UCReactionFeedbackComponent::RestoreCounterWeapon()
{
	WeaponOverlay.Restore();
	WeaponEnvelopeState.Reset();
	GlowWeapon.Reset();
	WeaponOpportunitySerial = 0;
	bWeaponTransferredToAttack = false;
}

void UCReactionFeedbackComponent::RestorePerfectScreen()
{
	if (PerfectCamera.IsValid() && PerfectCamera->PostProcessSettings.bOverride_VignetteIntensity
		&& FMath::IsNearlyEqual(PerfectCamera->PostProcessSettings.VignetteIntensity, LastWrittenVignette))
	{
		PerfectCamera->PostProcessSettings.VignetteIntensity = PreviousVignette;
		PerfectCamera->PostProcessSettings.bOverride_VignetteIntensity = bPreviousVignetteOverride;
	}
	PerfectCamera.Reset();
	ScreenEnvelopeState.Reset();
}

void UCReactionFeedbackComponent::ClearPerfectDodgePresentation()
{
	RestorePerfectBody();
	RestoreCounterWeapon();
	RestorePerfectScreen();
	if (PerfectSlowSerial != 0)
		if (auto* subsystem = GetWorld() ? GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>() : nullptr)
			subsystem->CancelPerfectDodgeSlow(PerfectSlowSerial);
	PerfectSlowSerial = 0;
	PerfectAttacker.Reset();
}

void UCReactionFeedbackComponent::EndPlay(const EEndPlayReason::Type InReason)
{
	ClearPerfectDodgePresentation();
	Super::EndPlay(InReason);
}

// Query

bool UCReactionFeedbackComponent::CanPlayReactionFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest) const
{
	if (InReactionFeedbackRequest.ReactionFeedbackTiming == EReactionFeedbackTiming::None)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidTiming"));
		return false;
	}
	if (InReactionFeedbackRequest.ReactionFeedbackTiming == EReactionFeedbackTiming::Max)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidTiming"));
		return false;
	}

	if (InReactionFeedbackRequest.ReactionFeedbackMatchKey.ReactionType == EReactionType::None)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidReactionType"));
		return false;
	}
	if (InReactionFeedbackRequest.ReactionFeedbackMatchKey.ReactionType == EReactionType::All)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidReactionType"));
		return false;
	}
	if (InReactionFeedbackRequest.ReactionFeedbackMatchKey.ReactionType == EReactionType::Max)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidReactionType"));
		return false;
	}

	return true;
}

// Matching

bool UCReactionFeedbackComponent::TryCalculateMatchScore(const FReactionFeedbackMatchKey& InDataKey, EReactionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FReactionFeedbackRequest& InReactionFeedbackRequest, int32& OutScore) const
{
	OutScore = 0;

	if (InDataTiming != InReactionFeedbackRequest.ReactionFeedbackTiming)
		return false;

	if (InDataTriggerKey != InReactionFeedbackRequest.TriggerKey)
		return false;

	const FReactionFeedbackMatchKey& requestKey = InReactionFeedbackRequest.ReactionFeedbackMatchKey;

	if (InDataKey.ReactionType == requestKey.ReactionType)
	{
		OutScore += ReactionFeedbackScore::ReactionExact;
	}
	else if (InDataKey.ReactionType != EReactionType::All)
	{
		return false;
	}

	if (InDataKey.DamageSpecKey.WeaponType == requestKey.DamageSpecKey.WeaponType)
	{
		OutScore += ReactionFeedbackScore::WeaponExact;
	}
	else if (InDataKey.DamageSpecKey.WeaponType != EWeaponType::All)
	{
		return false;
	}

	if (InDataKey.DamageSpecKey.ActionType == requestKey.DamageSpecKey.ActionType)
	{
		OutScore += ReactionFeedbackScore::ActionExact;
	}
	else if (InDataKey.DamageSpecKey.ActionType != EActionType::All)
	{
		return false;
	}

	if (InDataKey.DamageSpecKey.ActionIndex == requestKey.DamageSpecKey.ActionIndex)
	{
		OutScore += ReactionFeedbackScore::IndexExact;
	}
	else if (InDataKey.DamageSpecKey.ActionIndex != INDEX_NONE)
	{
		return false;
	}

	return true;
}

// Runtime Key / Playback Key

FReactionVFXPlaybackKey UCReactionFeedbackComponent::BuildReactionVFXPlaybackKey(const FReactionVFXFeedbackData& InReactionVFXFeedbackData) const
{
	FReactionVFXPlaybackKey playbackKey;

	playbackKey.VFXPlayType = InReactionVFXFeedbackData.VFXPlayType;
	playbackKey.VFX = InReactionVFXFeedbackData.VFX;
	playbackKey.SocketName = InReactionVFXFeedbackData.SocketName;
	playbackKey.RelativeLocation = InReactionVFXFeedbackData.RelativeLocation;
	playbackKey.RelativeRotation = InReactionVFXFeedbackData.RelativeRotation;
	playbackKey.RelativeScale = InReactionVFXFeedbackData.RelativeScale;

	return playbackKey;
}

FReactionSFXPlaybackKey UCReactionFeedbackComponent::BuildReactionSFXPlaybackKey(const FReactionSFXFeedbackData& InReactionSFXFeedbackData) const
{
	FReactionSFXPlaybackKey playbackKey;

	playbackKey.SFXPlayType = InReactionSFXFeedbackData.SFXPlayType;
	playbackKey.SFX = InReactionSFXFeedbackData.SFX;

	return playbackKey;
}

// Execution

void UCReactionFeedbackComponent::ExecuteVFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest)
{
	int32 bestScore = INDEX_NONE;
	TArray<const FReactionVFXFeedbackData*> matchedDatas;

	for (const FReactionVFXFeedbackData& data : VFXFeedbackDatas)
	{
		int32 matchScore = INDEX_NONE;

		if (!TryCalculateMatchScore(data.ReactionFeedbackMatchKey, data.ReactionFeedbackTiming, data.TriggerKey, InReactionFeedbackRequest, matchScore))
		{
			continue;
		}

		if (matchScore < bestScore) continue;

		// Preferred: replace matches with the new highest score.
		if (matchScore > bestScore)
		{
			bestScore = matchScore;
			matchedDatas.Reset();
			matchedDatas.Add(&data);
			continue;
		}

		// Tie: keep all matches with the current best score.
		matchedDatas.Add(&data);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("VFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordReactionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("VFX"), matchedDatas.Num());
	TSet<FReactionVFXPlaybackKey> playbackKeys;

	for (const FReactionVFXFeedbackData* matchedData : matchedDatas)
	{
		if (!matchedData)
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), nullptr, TEXT("InvalidMatchedData"));
			continue;
		}

		const FReactionVFXPlaybackKey playbackKey = BuildReactionVFXPlaybackKey(*matchedData);

		if (playbackKeys.Contains(playbackKey))
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), matchedData->VFX, TEXT("DuplicatePlaybackKey"));
			continue;
		}

		playbackKeys.Add(playbackKey);
		PlayReactionVFX(*matchedData);
	}
}

void UCReactionFeedbackComponent::ExecuteSFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest)
{
	int32 bestScore = INDEX_NONE;
	TArray<const FReactionSFXFeedbackData*> matchedDatas;

	for (const FReactionSFXFeedbackData& data : SFXFeedbackDatas)
	{
		int32 matchScore = INDEX_NONE;

		if (!TryCalculateMatchScore(data.ReactionFeedbackMatchKey, data.ReactionFeedbackTiming, data.TriggerKey, InReactionFeedbackRequest, matchScore))
		{
			continue;
		}

		if (matchScore < bestScore) continue;

		// Preferred: replace matches with the new highest score.
		if (matchScore > bestScore)
		{
			bestScore = matchScore;
			matchedDatas.Reset();
			matchedDatas.Add(&data);
			continue;
		}

		// Tie: keep all matches with the current best score.
		matchedDatas.Add(&data);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("SFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordReactionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("SFX"), matchedDatas.Num());
	TSet<FReactionSFXPlaybackKey> playbackKeys;

	for (const FReactionSFXFeedbackData* matchedData : matchedDatas)
	{
		if (!matchedData)
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), nullptr, TEXT("InvalidMatchedData"));
			continue;
		}

		const FReactionSFXPlaybackKey playbackKey = BuildReactionSFXPlaybackKey(*matchedData);

		if (playbackKeys.Contains(playbackKey))
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), matchedData->SFX, TEXT("DuplicatePlaybackKey"));
			continue;
		}

		playbackKeys.Add(playbackKey);
		PlayReactionSFX(*matchedData);
	}
}

// Playback

void UCReactionFeedbackComponent::PlayReactionVFX(const FReactionVFXFeedbackData& InReactionVFXFeedbackData)
{
	if (!IsValid(InReactionVFXFeedbackData.VFX))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("InvalidAsset"));
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("InvalidOwner"));
		return;
	}

	switch (InReactionVFXFeedbackData.VFXPlayType)
	{
	case EReactionVFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordReactionVFX();
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("SpawnOnce"));

		UNiagaraFunctionLibrary::SpawnSystemAttached(
			InReactionVFXFeedbackData.VFX,
			OwnerCharacter_Injected->GetMesh(),
			InReactionVFXFeedbackData.SocketName,
			InReactionVFXFeedbackData.RelativeLocation,
			InReactionVFXFeedbackData.RelativeRotation,
			InReactionVFXFeedbackData.RelativeScale,
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::None);

		return;
	}

	case EReactionVFXPlayType::Loop:
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("LoopNotImplemented"));
		return;
	}

	default:
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("UnsupportedPlayType"));
		return;
	}
}
void UCReactionFeedbackComponent::PlayReactionSFX(const FReactionSFXFeedbackData& InReactionSFXFeedbackData)
{
	if (!IsValid(InReactionSFXFeedbackData.SFX))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("InvalidAsset"));
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("InvalidOwner"));
		return;
	}

	switch (InReactionSFXFeedbackData.SFXPlayType)
	{
	case EReactionSFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordReactionSFX();
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("PlayOnce"));

		UGameplayStatics::PlaySoundAtLocation(
			this,
			InReactionSFXFeedbackData.SFX,
			OwnerCharacter_Injected->GetActorLocation());

		return;
	}

	case EReactionSFXPlayType::Loop:
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("LoopNotImplemented"));
		return;
	}

	default:
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("UnsupportedPlayType"));
		return;
	}
}
