#include "Component/CEnemyTargetSelectionComponent.h"

#include "Component/CCombatTargetComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatTargetTypes.h"

void UCEnemyTargetSelectionComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	CombatTargetComponent_Injected = InReferences.CombatTargetComponent;
}

FEnemyTargetSelectionResult UCEnemyTargetSelectionComponent::RequestSelectCombatTarget(AActor* InCandidate, ECombatTargetChangeReason InReason)
{
	FEnemyTargetSelectionResult result;
	if (!IsValid(CombatTargetComponent_Injected))
	{
		result.RejectReason = EEnemyTargetSelectionRejectReason::MissingCombatTargetComponent;
		return result;
	}
	if (!IsValid(InCandidate))
	{
		result.RejectReason = EEnemyTargetSelectionRejectReason::InvalidCandidate;
		return result;
	}
	if (InCandidate == GetOwner())
	{
		result.RejectReason = EEnemyTargetSelectionRejectReason::OwnerActor;
		return result;
	}

	const FCombatTargetSnapshot snapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();
	if (snapshot.TargetActor == InCandidate)
	{
		result.Decision = EEnemyTargetSelectionDecision::Unchanged;
		result.CommittedTarget = snapshot.TargetActor;
		result.Revision = snapshot.Revision;
		return result;
	}

	CombatTargetComponent_Injected->RequestSetCombatTarget(InCandidate, InReason);
	const FCombatTargetSnapshot committedSnapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();

	result.Decision = EEnemyTargetSelectionDecision::Committed;
	result.CommittedTarget = committedSnapshot.TargetActor;
	result.Revision = committedSnapshot.Revision;
	return result;
}

FEnemyTargetSelectionResult UCEnemyTargetSelectionComponent::RequestClearCombatTarget(ECombatTargetChangeReason InReason)
{
	FEnemyTargetSelectionResult result;
	if (!IsValid(CombatTargetComponent_Injected))
	{
		result.RejectReason = EEnemyTargetSelectionRejectReason::MissingCombatTargetComponent;
		return result;
	}
	if (!CombatTargetComponent_Injected->HasCombatTarget())
	{
		result.RejectReason = EEnemyTargetSelectionRejectReason::NoCurrentTarget;
		return result;
	}

	CombatTargetComponent_Injected->RequestClearCombatTarget(InReason);
	const FCombatTargetSnapshot snapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();

	result.Decision = EEnemyTargetSelectionDecision::Cleared;
	result.CommittedTarget = snapshot.TargetActor;
	result.Revision = snapshot.Revision;
	return result;
}
