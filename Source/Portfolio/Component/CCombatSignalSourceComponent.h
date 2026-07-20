#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CCombatSignalStructure.h"
#include "Type/CWeaponStructure.h"
#include "CCombatSignalSourceComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatSignalSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCombatSignalSourceComponent();

private:
	UPROPERTY(EditAnywhere)
	TMap<FDamageSpecKey, FDamageSpec> DamageSpecContainer;	// TODO: Seperate DataAsset (DB)

private:
	TMap<FCombatSignalHitWindowKey, TSet<TWeakObjectPtr<AActor>>> DamagedTargetContainer;

private:
	/* === Injected Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// HitWindow
	void NotifyHitWindowOpened(AActor* InDamageCauser, int32 InHitWindowId);
	void NotifyHitWindowClosed(AActor* InDamageCauser, int32 InHitWindowId);

public:
	// Entry
	void RequestCombatSignalSource(const FHitContext& InHitContext);
	bool RequestCombatSignalCue(AActor* InTargetActor, FName InCueTag, const FVector& InCueLocation = FVector::ZeroVector, const FVector& InDirection = FVector::ZeroVector, AActor* InSignalCauser = nullptr);

public:
	// Entry for AI
	bool RequestAICombatSignalCue(FName InCueTag);

private:
	void ProcessCombatSignalSource(const FHitContext& InHitContext);

private:
	// Profiling
	bool ShouldSkipEnemyHitProcessingForProfiling() const;
	bool IsEnemyHitProcessingProfilingTarget() const;

private:
	// Receive
	bool ValidateRequest(const FHitContext& InHitContext) const;
	FCombatSignalSourcePayload BuildPayload(const FHitContext& InHitContext) const;
	FCombatSignalSourceContext BuildContext(const FCombatSignalSourcePayload& InCombatSignalSourcePayload) const;

private:
	// Resolve
	bool ValidateContext(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	bool CanSendCombatSignal(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	void ResolveSourceDamageSpec(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	void ComputeSourceDamage(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	FCombatSignalSourceResult BuildResult(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;

private:
	// Send
	void CommitCombatSignalSource(FCombatSignalSourceContext& InOutCombatSignalSourceContext);
	float SendDamageToTarget(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;
	bool SendCueSignal(const FCombatSignal& InCombatSignal) const;

private:
	// Cache
	void CacheDamagedTargetInWindow(const FCombatSignalSourceContext& InCombatSignalSourceContext);

private:
	// Hit Helper
	FCombatSignalHitWindowKey BuildHitWindowKey(const FHitContext& InHitContext) const;
	FDamageSpecKey BuildSpecKey(const FHitContext& InHitContext) const;
	AController* ResolveInstigatorController(AActor* InAttacker, AActor* InDamageCauser) const;
	bool IsDuplicateHit(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;
	bool IsFriendlyTarget(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;

private:
	// Cue Helper
	AActor* ResolveCueTargetActor() const;
	FCombatSignal BuildCueSignal(AActor* InTargetActor, FName InCueTag, const FVector& InCueLocation, const FVector& InDirection, AActor* InSignalCauser) const;
	bool ValidateCueSignal(const FCombatSignal& InCombatSignal) const;

};
