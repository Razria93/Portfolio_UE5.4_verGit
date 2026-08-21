#include "Component/CEnemyCombatParticipationComponent.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"
#include "Controller/CAIController.h"
#include "System/Combat/CWorldSubsystem_CombatParticipation.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CEngageAssignmentTypes.h"

UCEnemyCombatParticipationComponent::UCEnemyCombatParticipationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ===== Component Reference =====

void UCEnemyCombatParticipationComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	UnbindCombatTargetComponent();
	CombatTargetComponent_Injected = InReferences.CombatTargetComponent;
	BindCombatTargetComponent();
}

void UCEnemyCombatParticipationComponent::SetAIController(ACAIController* InAIController)
{
	if (AIController_Injected == InAIController) return;

	ClearAIController();
	AIController_Injected = InAIController;
	BindParticipationSubsystem();
	SynchronizeParticipation();
}

void UCEnemyCombatParticipationComponent::ClearAIController()

{
	ClearAIController(true);
}

FCombatParticipationAppliedSnapshot UCEnemyCombatParticipationComponent::GetAppliedSnapshot() const
{
	return AppliedSnapshot;
}

bool UCEnemyCombatParticipationComponent::HasActiveEvidenceForTarget(const AActor* InTarget) const
{
	if (!IsValid(AIController_Injected) || !IsValid(InTarget)) return false;

	if (const UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		return subsystem->HasActiveEvidenceForParticipationPair(AIController_Injected, InTarget);
	}

	return false;
}

bool UCEnemyCombatParticipationComponent::TryGetCurrentEngageAssignment(FCombatTargetSnapshot& OutTargetSnapshot, int32& OutAssignmentRevision) const
{
	OutTargetSnapshot = FCombatTargetSnapshot();
	OutAssignmentRevision = 0;

	if (!AppliedSnapshot.IsAssigned() || AppliedSnapshot.CombatRole != ECombatRole::Engage) return false;
	if (!IsValid(CombatTargetComponent_Injected)) return false;

	const FCombatTargetSnapshot currentSnapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();
	if (!IsValid(currentSnapshot.TargetActor)) return false;
	if (currentSnapshot.TargetActor != AppliedSnapshot.TargetActor) return false;
	if (currentSnapshot.Revision != AppliedSnapshot.CombatTargetRevision) return false;

	OutTargetSnapshot = currentSnapshot;
	OutAssignmentRevision = AppliedSnapshot.AssignmentRevision;
	return true;
}

bool UCEnemyCombatParticipationComponent::AcquireParticipationAssignmentLock(const FCombatTargetSnapshot& InTargetSnapshot, const int32 InAssignmentRevision)
{
	if (!IsValid(AIController_Injected)) return false;

	FCombatTargetSnapshot currentSnapshot;
	int32 currentAssignmentRevision = 0;
	if (!TryGetCurrentEngageAssignment(currentSnapshot, currentAssignmentRevision)) return false;
	if (currentSnapshot.TargetActor != InTargetSnapshot.TargetActor) return false;
	if (currentSnapshot.Revision != InTargetSnapshot.Revision) return false;
	if (currentAssignmentRevision != InAssignmentRevision) return false;

	UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem();
	if (!IsValid(subsystem)) return false;

	FCombatParticipationAssignmentLock actionLock;
	actionLock.TargetActor = InTargetSnapshot.TargetActor;
	actionLock.CombatTargetRevision = InTargetSnapshot.Revision;
	actionLock.AssignmentRevision = InAssignmentRevision;
	if (!subsystem->AcquireAssignmentLock(AIController_Injected, actionLock)) return false;

	ActiveAssignmentLock = actionLock;
	return true;
}

void UCEnemyCombatParticipationComponent::ReleaseParticipationAssignmentLock()
{
	ActiveAssignmentLock = FCombatParticipationAssignmentLock();
	if (!IsValid(AIController_Injected)) return;

	if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		subsystem->ReleaseAssignmentLock(AIController_Injected);
	}
}

void UCEnemyCombatParticipationComponent::ReleaseParticipationForOwnerDeath()
{
	ClearAIController();
}

void UCEnemyCombatParticipationComponent::ClearAIController(const bool bReleaseCombatTarget)
{
	ActiveAssignmentLock = FCombatParticipationAssignmentLock();

	if (IsValid(AIController_Injected))
	{
		if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
		{
			subsystem->UnregisterParticipant(AIController_Injected);
		}
	}

	if (bReleaseCombatTarget && IsValid(CombatTargetComponent_Injected) && AppliedSnapshot.IsAssigned())
	{
		CombatTargetComponent_Injected->RequestClearCombatTargetIfCurrent(AppliedSnapshot.TargetActor, AppliedSnapshot.CombatTargetRevision, ECombatTargetChangeReason::ParticipationRevoked);
	}

	UnbindParticipationSubsystem();
	AIController_Injected = nullptr;
	LastAssignmentRevision = 0;
	ClearAppliedSnapshot();
}

// ===== Evidence Ingress =====

void UCEnemyCombatParticipationComponent::ReportEvidence(ECombatParticipationSource InSource, AActor* InTarget, const FCombatParticipationEvidenceContext& InContext)
{
	if (!IsValid(InTarget) || InTarget == GetOwner()) return;
	if (!IsValid(AIController_Injected)) return;

	AIController_Injected->CancelInvestigateForNewCombatEvidence();

	if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		subsystem->ReportEvidence(AIController_Injected, InSource, InTarget, InContext);
	}
}

void UCEnemyCombatParticipationComponent::ReportHitReactiveEvidence(AActor* InTarget, const FCombatParticipationEvidenceContext& InContext, const uint64 InResultSerial)
{
	if (!IsValid(AIController_Injected) || !IsValid(InTarget) || InResultSerial == 0) return;

	AIController_Injected->CancelInvestigateForNewCombatEvidence();

	if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		subsystem->ReportHitReactiveEvidence(AIController_Injected, InTarget, InContext, InResultSerial);
	}
}

void UCEnemyCombatParticipationComponent::StartHitReactiveEvidencePostReactionTTL(AActor* InTarget, const uint64 InResultSerial)
{
	if (!IsValid(AIController_Injected) || !IsValid(InTarget) || InResultSerial == 0) return;

	if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		subsystem->StartHitReactiveEvidencePostReactionTTL(AIController_Injected, InTarget, InResultSerial);
	}
}

void UCEnemyCombatParticipationComponent::WithdrawEvidence(ECombatParticipationSource InSource, AActor* InTarget, const bool bAllowInvestigateHandoff)
{
	if (!IsValid(AIController_Injected) || !IsValid(InTarget)) return;

	if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		subsystem->WithdrawEvidence(AIController_Injected, InSource, InTarget, bAllowInvestigateHandoff);
	}
}

// ===== Participation Release =====

void UCEnemyCombatParticipationComponent::WithdrawAllEvidenceForOwner()
{
	if (!IsValid(AIController_Injected)) return;

	if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		subsystem->WithdrawAllEvidenceForParticipant(AIController_Injected);
	}
}

void UCEnemyCombatParticipationComponent::SetParticipationSuppressed(const bool bSuppressed)
{
	if (!IsValid(AIController_Injected)) return;

	if (UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem())
	{
		subsystem->SetParticipationSuppressed(AIController_Injected, bSuppressed);
	}

	if (!bSuppressed)
	{
		AIController_Injected->RefreshParticipationEvidenceFromPerception();
	}
}

// ===== Lifecycle =====

void UCEnemyCombatParticipationComponent::BeginPlay()
{
	Super::BeginPlay();
	BindParticipationSubsystem();
}

void UCEnemyCombatParticipationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindParticipationSubsystem();
	ClearAIController(false);
	UnbindCombatTargetComponent();

	Super::EndPlay(EndPlayReason);
}

// ===== Participation System =====

UCWorldSubsystem_CombatParticipation* UCEnemyCombatParticipationComponent::GetParticipationSubsystem() const
{
	UWorld* world = GetWorld();
	return IsValid(world) ? world->GetSubsystem<UCWorldSubsystem_CombatParticipation>() : nullptr;
}

void UCEnemyCombatParticipationComponent::BindParticipationSubsystem()
{
	UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem();
	if (!IsValid(subsystem)) return;

	subsystem->OnCombatParticipationChanged.RemoveAll(this);
	subsystem->OnCombatParticipationChanged.AddUObject(this, &UCEnemyCombatParticipationComponent::HandleCombatParticipationChanged);
	subsystem->OnCombatParticipationEvidenceExhausted.RemoveAll(this);
	subsystem->OnCombatParticipationEvidenceExhausted.AddUObject(this, &UCEnemyCombatParticipationComponent::HandleCombatParticipationEvidenceExhausted);
}

void UCEnemyCombatParticipationComponent::UnbindParticipationSubsystem()
{
	UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem();
	if (!IsValid(subsystem)) return;

	subsystem->OnCombatParticipationChanged.RemoveAll(this);
	subsystem->OnCombatParticipationEvidenceExhausted.RemoveAll(this);
}

void UCEnemyCombatParticipationComponent::BindCombatTargetComponent()
{
	if (!IsValid(CombatTargetComponent_Injected)) return;

	CombatTargetComponent_Injected->OnCombatTargetChanged.AddUObject(this, &UCEnemyCombatParticipationComponent::HandleCombatTargetChanged);
}

void UCEnemyCombatParticipationComponent::UnbindCombatTargetComponent()
{
	if (!IsValid(CombatTargetComponent_Injected)) return;

	CombatTargetComponent_Injected->OnCombatTargetChanged.RemoveAll(this);
}

// ===== Participation Assignment =====

void UCEnemyCombatParticipationComponent::SynchronizeParticipation()
{
	UCWorldSubsystem_CombatParticipation* subsystem = GetParticipationSubsystem();
	if (!IsValid(subsystem) || !IsValid(AIController_Injected)) return;

	const FEngageAssignmentContext assignment = subsystem->GetAssignment(AIController_Injected);
	LastAssignmentRevision = assignment.AssignmentRevision;
	ApplyParticipationAssignment(assignment);
}

void UCEnemyCombatParticipationComponent::HandleCombatParticipationChanged(ACAIController* InAIController, const FCombatParticipationChange& InChange)
{
	if (InAIController != AIController_Injected) return;
	const FEngageAssignmentContext& assignment = InChange.CurrentAssignment;
	if (assignment.AssignmentRevision <= LastAssignmentRevision) return;

	LastAssignmentRevision = assignment.AssignmentRevision;
	ApplyParticipationAssignment(assignment);
}

void UCEnemyCombatParticipationComponent::HandleCombatParticipationEvidenceExhausted(const FCombatParticipationEvidenceExhaustedEvent& InEvent)
{
	if (InEvent.Participant != AIController_Injected || !InEvent.bWasAppliedCombatTarget) return;
	if (!IsValid(InEvent.TargetActor) || !IsValid(AIController_Injected)) return;

	if (AppliedSnapshot.IsAssigned() && AppliedSnapshot.TargetActor != InEvent.TargetActor) return;
	AIController_Injected->HandleCombatParticipationEvidenceExhausted(InEvent);
}

void UCEnemyCombatParticipationComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	if (!IsValid(ActiveAssignmentLock.TargetActor)) return;

	const FCombatTargetSnapshot& currentSnapshot = InChange.CurrentSnapshot;
	if (currentSnapshot.TargetActor == ActiveAssignmentLock.TargetActor
		&& currentSnapshot.Revision == ActiveAssignmentLock.CombatTargetRevision) return;

	ReleaseParticipationAssignmentLock();
}

void UCEnemyCombatParticipationComponent::ApplyParticipationAssignment(const FEngageAssignmentContext& InAssignment)
{
	if (!IsValid(CombatTargetComponent_Injected)) return;

	if (!ActiveAssignmentLock.Matches(InAssignment))
	{
		ActiveAssignmentLock = FCombatParticipationAssignmentLock();
	}

	if (InAssignment.IsValidAssignment())
	{
		const FCombatTargetSnapshot currentSnapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();
		if (currentSnapshot.TargetActor != InAssignment.TargetActor)
		{
			CombatTargetComponent_Injected->RequestSetCombatTarget(InAssignment.TargetActor, ECombatTargetChangeReason::ParticipationAssigned);
		}

		const FCombatTargetSnapshot appliedCombatTargetSnapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();
		if (appliedCombatTargetSnapshot.TargetActor == InAssignment.TargetActor)
		{
			RecordAppliedSnapshot(InAssignment, appliedCombatTargetSnapshot);
		}
		else
		{
			ClearAppliedSnapshot();
		}
		return;
	}

	if (AppliedSnapshot.IsAssigned())
	{
		CombatTargetComponent_Injected->RequestClearCombatTargetIfCurrent(AppliedSnapshot.TargetActor, AppliedSnapshot.CombatTargetRevision, ECombatTargetChangeReason::ParticipationRevoked);
	}

	const FCombatTargetSnapshot appliedCombatTargetSnapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();
	if (!IsValid(appliedCombatTargetSnapshot.TargetActor))
	{
		RecordAppliedSnapshot(InAssignment, appliedCombatTargetSnapshot);
	}
	else
	{
		ClearAppliedSnapshot();
	}
}

void UCEnemyCombatParticipationComponent::RecordAppliedSnapshot(const FEngageAssignmentContext& InAssignment, const FCombatTargetSnapshot& InCombatTargetSnapshot)
{
	AppliedSnapshot.TargetActor = InCombatTargetSnapshot.TargetActor;
	AppliedSnapshot.AssignmentRevision = InAssignment.AssignmentRevision;
	AppliedSnapshot.CombatTargetRevision = InCombatTargetSnapshot.Revision;
	AppliedSnapshot.CombatRole = InAssignment.IsValidAssignment() ? InAssignment.CombatRole : ECombatRole::None;
	AppliedSnapshot.EngageAdmission = InAssignment.IsValidAssignment() ? InAssignment.EngageAdmission : EEngageAdmissionKind::None;
	AppliedSnapshot.bIsApplied = true;
}

void UCEnemyCombatParticipationComponent::ClearAppliedSnapshot()
{
	AppliedSnapshot = FCombatParticipationAppliedSnapshot();
}
