#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCombatParticipationTypes.h"
#include "CEnemyCombatParticipationComponent.generated.h"

class ACAIController;
class UCCombatTargetComponent;
class UCWorldSubsystem_CombatParticipation;
struct FCharacterComponentReferences;
struct FEngageAssignmentContext;
struct FCombatParticipationChange;
struct FCombatParticipationEvidenceExhaustedEvent;
struct FCombatTargetChange;
struct FCombatTargetSnapshot;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCEnemyCombatParticipationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCEnemyCombatParticipationComponent();

private:
	// Component Reference
	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;

	UPROPERTY(Transient)
	ACAIController* AIController_Injected = nullptr;

	// Assignment State
	int32 LastAssignmentRevision = 0;
	FCombatParticipationAppliedSnapshot AppliedSnapshot;
	FCombatParticipationAssignmentLock ActiveAssignmentLock;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);
	void SetAIController(ACAIController* InAIController);
	void ClearAIController();

	// Evidence Ingress
	void ReportEvidence(ECombatParticipationSource InSource, AActor* InTarget, const FCombatParticipationEvidenceContext& InContext);
	void ReportHitReactiveEvidence(AActor* InTarget, const FCombatParticipationEvidenceContext& InContext, uint64 InResultSerial);

	// HitReactive Evidence Lifetime
	void StartHitReactiveEvidencePostReactionTTL(AActor* InTarget, uint64 InResultSerial);

	// Evidence Removal
	void WithdrawEvidence(ECombatParticipationSource InSource, AActor* InTarget, bool bAllowInvestigateHandoff = true);

	// Assignment Query
	FCombatParticipationAppliedSnapshot GetAppliedSnapshot() const;
	bool HasActiveEvidenceForTarget(const AActor* InTarget) const;
	bool TryGetCurrentEngageAssignment(FCombatTargetSnapshot& OutTargetSnapshot, int32& OutAssignmentRevision) const;

	// Assignment Lock
	bool AcquireParticipationAssignmentLock(const FCombatTargetSnapshot& InTargetSnapshot, int32 InAssignmentRevision);
	void ReleaseParticipationAssignmentLock();

	// Participation Release
	void SetParticipationSuppressed(bool bSuppressed);
	void SoftReleaseParticipationForOwner();
	void HardReleaseParticipationForOwnerDeath();

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Participation System
	UCWorldSubsystem_CombatParticipation* GetParticipationSubsystem() const;
	void BindParticipationSubsystem();
	void UnbindParticipationSubsystem();
	void BindCombatTargetComponent();
	void UnbindCombatTargetComponent();

	// Participation Assignment
	void SynchronizeParticipation();
	void HandleCombatParticipationChanged(ACAIController* InAIController, const FCombatParticipationChange& InChange);
	void HandleCombatParticipationEvidenceExhausted(const FCombatParticipationEvidenceExhaustedEvent& InEvent);
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);
	void ApplyParticipationAssignment(const FEngageAssignmentContext& InAssignment);
	void RecordAppliedSnapshot(const FEngageAssignmentContext& InAssignment, const FCombatTargetSnapshot& InCombatTargetSnapshot);
	void ClearAppliedSnapshot();
	void ClearAIController(bool bReleaseCombatTarget);
};
