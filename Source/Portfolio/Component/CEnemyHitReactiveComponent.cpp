#include "Component/CEnemyHitReactiveComponent.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CEnemyCombatParticipationComponent.h"
#include "Component/CHealthComponent.h"
#include "Controller/CAIController.h"
#include "Interface/TargetContextProvider.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatParticipationTypes.h"
#include "Type/CCombatSignalTargetTypes.h"

#include "GameFramework/Pawn.h"

UCEnemyHitReactiveComponent::UCEnemyHitReactiveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ===== Component Reference =====

void UCEnemyHitReactiveComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	UnbindCombatSignalTarget();

	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	HealthComp_Injected = InReferences.HealthComponent;
	CombatSignalTargetComp_Injected = InReferences.CombatSignalTargetComponent;
	CombatParticipationComp_Injected = InReferences.EnemyCombatParticipationComponent;

	BindCombatSignalTarget();
}

// ===== Lifecycle =====

void UCEnemyHitReactiveComponent::BeginPlay()
{
	Super::BeginPlay();

	BindCombatSignalTarget();
}

void UCEnemyHitReactiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindCombatSignalTarget();

	Super::EndPlay(EndPlayReason);
}

// ===== Combat Signal =====

void UCEnemyHitReactiveComponent::BindCombatSignalTarget()
{
	if (!IsValid(CombatSignalTargetComp_Injected)) return;

	CombatSignalTargetComp_Injected->OnCombatSignalTargetAccepted.RemoveAll(this);
	CombatSignalTargetComp_Injected->OnCombatSignalTargetAccepted.AddUObject(this, &UCEnemyHitReactiveComponent::HandleCombatSignalTargetAccepted);
}

void UCEnemyHitReactiveComponent::UnbindCombatSignalTarget()
{
	if (!IsValid(CombatSignalTargetComp_Injected)) return;

	CombatSignalTargetComp_Injected->OnCombatSignalTargetAccepted.RemoveAll(this);
}

void UCEnemyHitReactiveComponent::HandleCombatSignalTargetAccepted(const FCombatSignalTargetPacket& InPacket)
{
	if (InPacket.ResultSerial == 0 || InPacket.ResultSerial <= LastAcceptedResultSerial) return;
	LastAcceptedResultSerial = InPacket.ResultSerial;

	if (!IsEligibleHitReactiveResult(InPacket)) return;

	AActor* combatantTarget = ResolveCombatantTarget(InPacket);
	if (!IsValid(combatantTarget)) return;

	FCombatParticipationEvidenceContext evidenceContext;
	const ITargetContextProvider* targetProvider = Cast<ITargetContextProvider>(combatantTarget);
	if (!targetProvider) return;

	evidenceContext.TargetPriority = targetProvider->GetTargetPriority();
	evidenceContext.DistanceToTarget = FVector::Dist(OwnerCharacter_Injected->GetActorLocation(), combatantTarget->GetActorLocation());

	CombatParticipationComp_Injected->ReportEvidence(ECombatParticipationSource::HitReactive, combatantTarget, evidenceContext);
}

// ===== Evidence =====

bool UCEnemyHitReactiveComponent::IsEligibleHitReactiveResult(const FCombatSignalTargetPacket& InPacket) const
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()) return false;
	if (!IsValid(CombatParticipationComp_Injected)) return false;
	if (!InPacket.Result.bAccepted || InPacket.Context.TargetActor != OwnerCharacter_Injected) return false;
	if (InPacket.Result.DeadState_After != EDeadState::Alive) return false;

	if (InPacket.Result.DefenseOutcome == EDamageDefenseOutcome::Guard || InPacket.Result.DefenseOutcome == EDamageDefenseOutcome::Parry) return true;
	return InPacket.Result.bShouldCommitDamage && InPacket.Result.CommittedDamage > KINDA_SMALL_NUMBER;
}

AActor* UCEnemyHitReactiveComponent::ResolveCombatantTarget(const FCombatSignalTargetPacket& InPacket) const
{
	if (AActor* combatant = ResolveEligibleCombatant(InPacket.Context.SourceActor)) return combatant;

	if (IsValid(InPacket.Context.Instigator))
	{
		if (AActor* combatant = ResolveEligibleCombatant(InPacket.Context.Instigator->GetPawn())) return combatant;
	}

	if (AActor* combatant = ResolveEligibleCombatant(InPacket.Context.DamageCauser)) return combatant;

	if (IsValid(InPacket.Context.DamageCauser))
	{
		if (AActor* combatant = ResolveEligibleCombatant(InPacket.Context.DamageCauser->GetInstigator())) return combatant;
		if (AActor* combatant = ResolveEligibleCombatant(InPacket.Context.DamageCauser->GetOwner())) return combatant;
	}

	return nullptr;
}

AActor* UCEnemyHitReactiveComponent::ResolveEligibleCombatant(AActor* InCandidate) const
{
	if (!IsValid(InCandidate) || InCandidate == OwnerCharacter_Injected) return nullptr;
	if (!Cast<ITargetContextProvider>(InCandidate)) return nullptr;

	const ACEnemy* ownerEnemy = Cast<ACEnemy>(OwnerCharacter_Injected);
	const ACAIController* aiController = ownerEnemy ? Cast<ACAIController>(ownerEnemy->GetController()) : nullptr;
	if (!IsValid(aiController) || aiController->GetTeamAttitudeTowards(*InCandidate) != ETeamAttitude::Hostile) return nullptr;

	return InCandidate;
}
