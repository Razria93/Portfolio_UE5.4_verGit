#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CEnemyHitReactiveComponent.generated.h"

class AActor;
class UCCombatSignalTargetComponent;
class UCEnemyCombatParticipationComponent;
class UCHealthComponent;
class UCReactionComponent;
struct FCharacterComponentReferences;
struct FCombatSignalTargetPacket;
struct FReactionExecutionLifecycleEvent;
struct FReactionRequestResult;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCEnemyHitReactiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCEnemyHitReactiveComponent();

private:
	// Component Reference
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	UCCombatSignalTargetComponent* CombatSignalTargetComp_Injected = nullptr;

	UPROPERTY(Transient)
	UCEnemyCombatParticipationComponent* CombatParticipationComp_Injected = nullptr;

	UPROPERTY(Transient)
	UCReactionComponent* ReactionComp_Injected = nullptr;

	// Runtime State
	uint64 LastAcceptedResultSerial = 0;
	TMap<uint64, TWeakObjectPtr<AActor>> PendingCombatantTargetByResultSerial;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Combat Signal
	void BindCombatSignalTarget();
	void UnbindCombatSignalTarget();
	void HandleCombatSignalTargetAccepted(const FCombatSignalTargetPacket& InPacket);
	void HandleCombatSignalTargetReactionResolved(const FCombatSignalTargetPacket& InPacket, const FReactionRequestResult& InResult);
	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);

	// Evidence
	bool IsEligibleHitReactiveResult(const FCombatSignalTargetPacket& InPacket) const;
	AActor* ResolveCombatantTarget(const FCombatSignalTargetPacket& InPacket) const;
	AActor* ResolveEligibleCombatant(AActor* InCandidate) const;
};
