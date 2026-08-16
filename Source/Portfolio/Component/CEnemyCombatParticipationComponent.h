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
	FCombatParticipationActionLock ActiveActionLock;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);
	void SetAIController(ACAIController* InAIController);
	void ClearAIController();

	// Evidence Ingress
	void ReportEvidence(ECombatParticipationSource InSource, AActor* InTarget, const FCombatParticipationEvidenceContext& InContext);
	void WithdrawEvidence(ECombatParticipationSource InSource, AActor* InTarget);
	FCombatParticipationAppliedSnapshot GetAppliedSnapshot() const;
	bool AcquireCombatActionLock(const FCombatTargetSnapshot& InTargetSnapshot, int32 InAssignmentRevision);
	void ReleaseCombatActionLock();
	void ReleaseParticipationForOwnerDeath();

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
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);
	void ApplyParticipationAssignment(const FEngageAssignmentContext& InAssignment);
	void RecordAppliedSnapshot(const FEngageAssignmentContext& InAssignment, const FCombatTargetSnapshot& InCombatTargetSnapshot);
	void ClearAppliedSnapshot();
	void ClearAIController(bool bReleaseCombatTarget);
};
