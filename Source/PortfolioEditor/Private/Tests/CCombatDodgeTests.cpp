#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/CActionFacingProbe.h"
#include "Tests/CMontageNotifyProbe.h"
#include "Action/CAction_Dodge.h"
#include "Action/CAction_CounterRush.h"
#include "Reaction/CReaction_PerfectDodge.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CWeaponComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include <limits>

namespace CombatDodgeTest
{
	template<typename T> T& Field(UObject* InObject, const TCHAR* InName)
	{
		FProperty* property = FindFProperty<FProperty>(InObject->GetClass(), InName);
		check(property);
		return *property->ContainerPtrToValuePtr<T>(InObject);
	}

	struct FFixture
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		ACPlayer* Player = World->SpawnActor<ACActionFacingCharacterProbe>();
		ACharacter* Attacker = World->SpawnActor<ACharacter>();
		APlayerController* Controller = World->SpawnActor<APlayerController>();
		UCActionComponent* Actions = Player->FindComponentByClass<UCActionComponent>();
		UCDefenseComponent* Defense = Player->FindComponentByClass<UCDefenseComponent>();
		UCCombatSignalTargetComponent* Target = Player->FindComponentByClass<UCCombatSignalTargetComponent>();
		UCAction_Dodge* Dodge = NewObject<UCAction_Dodge>(Actions);
		FCharacterComponentReferences References;
		FCombatSignalTargetPacket LastPacket;
		int32 AcceptedCount = 0;
		UAnimMontage* Montage = nullptr;

		FFixture()
		{
			References.OwnerCharacter = Player;
			References.CharacterMovementComponent = Player->GetCharacterMovement();
			References.ActionComponent = Actions;
			References.DefenseComponent = Defense;
			References.HealthComponent = Player->GetHealthComp();
			References.MovementComponent = Player->FindComponentByClass<UCMovementComponent>();
			References.StateComponent = Player->FindComponentByClass<UCStateComponent>();
			References.WeaponComponent = Player->FindComponentByClass<UCWeaponComponent>();
			References.ActionFeedbackComponent = Player->FindComponentByClass<UCActionFeedbackComponent>();
			References.ActionOrchestratorComponent = Player->FindComponentByClass<UCActionOrchestratorComponent>();
			References.ReactionComponent = Player->FindComponentByClass<UCReactionComponent>();
			References.ReactionFeedbackComponent = Player->FindComponentByClass<UCReactionFeedbackComponent>();
			References.ReactionOrchestratorComponent = Player->FindComponentByClass<UCReactionOrchestratorComponent>();
			References.CombatSignalSourceComponent = Player->FindComponentByClass<UCCombatSignalSourceComponent>();
			References.CombatTargetComponent = Player->FindComponentByClass<UCCombatTargetComponent>();
			References.ObservableOverlayComponent = Player->FindComponentByClass<UCObservableOverlayComponent>();
			References.BalanceComponent = Player->FindComponentByClass<UCBalanceComponent>();
			References.HitFeedbackComponent = Player->FindComponentByClass<UCHitFeedbackComponent>();
			References.HealthComponent->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
			References.StateComponent->InitializeReferences(References);
			References.MovementComponent->InitializeReferences(References);
			Actions->InitializeReferences(References);
			Defense->InitializeReferences(References);
			Field<FDefenseDodgeTuning>(Defense, TEXT("DodgeTuning")).PerfectEnd = 0.f;
			Dodge->InitializeReferences(References);
			References.ReactionComponent->InitializeReferences(References);
			References.ReactionOrchestratorComponent->InitializeReferences(References);
			Target->InitializeReferences(References);
			Target->OnCombatSignalTargetAccepted.AddLambda([this](const FCombatSignalTargetPacket& InPacket)
			{
				LastPacket = InPacket;
				++AcceptedCount;
			});
			Controller->Possess(Attacker);
			UCHealthComponent* attackerHealth = NewObject<UCHealthComponent>(Attacker);
			Attacker->AddInstanceComponent(attackerHealth);
			attackerHealth->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
			USkeletalMesh* mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/01_Character/Regacy/Mesh/SK_Mannequin.SK_Mannequin"));
			Player->GetMesh()->SetSkeletalMesh(mesh);
			Player->GetMesh()->SetAnimInstanceClass(UCMontageProbeAnimInstance::StaticClass());
			Player->GetMesh()->InitAnim(true);
			UAnimMontage* source = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/04_Montage/Dodge/Default/M_Dodge.M_Dodge"));
			if (source)
			{
				Montage = DuplicateObject<UAnimMontage>(source, GetTransientPackage());
				Montage->Notifies.Reset();
				Montage->RefreshCacheData();
			}
		}

		~FFixture()
		{
			Target->OnCombatSignalTargetAccepted.Clear();
			Actions->CancelActiveActionForSystem();
			References.ReactionComponent->CancelActiveReactionForSystem();
			World->DestroyWorld(false);
		}

		FActionExecutionResult MakeStart()
		{
			FActionExecutionResult result;
			result.Decision = EExecutionDecision::Accept;
			result.ApplyMode = EExecutionApplyMode::Start;
			auto& context = result.ResolvedContext;
			context.ActionDataKey.ActionType = EActionType::Dodge;
			context.ActionDataKey.ActionIndex = 0;
			context.ActionExecutor = Dodge;
			context.ActionData.ActionDataKey = context.ActionDataKey;
			context.ActionData.ActionExecutorKey = Dodge->GetClass();
			context.ActionData.Montage = Montage;
			return result;
		}

		FDefaultDamageEvent Damage()
		{
			FDefaultDamageEvent damage;
			damage.SourceActor = Attacker;
			damage.TargetActor = Player;
			damage.DamageSpecKey = { };
			damage.DamageSpecKey.WeaponType = EWeaponType::Sword;
			damage.DamageSpecKey.ActionType = EActionType::ComboAttack;
			damage.DamageSpecKey.ActionIndex = 0;
			damage.DamageSpec.BaseDamage = 10.f;
			damage.DamageRequestAmount.RequestDamage = 10.f;
			return damage;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgeWindowBoundsTest, "Portfolio.Combat.Dodge.WindowBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgeWindowBoundsTest::RunTest(const FString& Parameters)
{
	FDefenseDodgeTuning tuning;
	tuning.InvulnerabilityStart = 0.1f;
	tuning.InvulnerabilityEnd = 0.5f;
	TestFalse(TEXT("Before start"), tuning.ContainsInvulnerabilityPosition(0.099f));
	TestTrue(TEXT("Start inclusive"), tuning.ContainsInvulnerabilityPosition(0.1f));
	TestTrue(TEXT("Inside"), tuning.ContainsInvulnerabilityPosition(0.499f));
	TestFalse(TEXT("End exclusive"), tuning.ContainsInvulnerabilityPosition(0.5f));
	TestFalse(TEXT("NaN rejected"), tuning.ContainsInvulnerabilityPosition(std::numeric_limits<float>::quiet_NaN()));
	tuning.InvulnerabilityEnd = 0.05f;
	TestFalse(TEXT("Reversed window rejected"), tuning.ContainsInvulnerabilityPosition(0.1f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgePlaybackOwnershipTest, "Portfolio.Combat.Dodge.PlaybackOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgePlaybackOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	if (!TestNotNull(TEXT("Dodge montage loaded"), f.Montage)) return false;
	TestFalse(TEXT("Idle has no immunity"), f.Defense->CanDodge());
	TestTrue(TEXT("Actual montage starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	const uint64 firstGeneration = f.Actions->GetActiveActionGeneration();
	TestTrue(TEXT("Playback opens window"), f.Defense->CanDodge());
	UAnimInstance* anim = f.Player->GetMesh()->GetAnimInstance();
	anim->Montage_SetPosition(f.Montage, 0.5f);
	TestFalse(TEXT("Actual montage end boundary closes window"), f.Defense->CanDodge());
	f.Dodge->Complete();
	TestFalse(TEXT("Completion removes immunity"), f.Defense->CanDodge());
	TestTrue(TEXT("Second execution starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	f.Defense->EndDodgeExecution(firstGeneration);
	TestTrue(TEXT("Old cleanup cannot clear successor"), f.Defense->CanDodge());
	anim->Montage_Stop(0.f, f.Montage);
	TestFalse(TEXT("Stopped animation cannot retain immunity"), f.Defense->CanDodge());
	f.Actions->CancelActiveActionForSystem();
	FActionExecutionResult failed = f.MakeStart();
	failed.ResolvedContext.ActionData.Montage = NewObject<UAnimMontage>();
	TestFalse(TEXT("Invalid playback fails"), f.Actions->ApplyActionDecision(failed));
	TestFalse(TEXT("Failed start cannot grant immunity"), f.Defense->CanDodge());
	FExecutionDecisionQuery query;
	query.Snapshot.ExecutionState = EExecutionState::Reaction;
	query.IncomingPart.bIsValid = true;
	query.IncomingPart.ParticipantDomain = EExecutionDomain::Action;
	query.IncomingPart.ActionContext = f.MakeStart().ResolvedContext;
	TestTrue(TEXT("Reaction escape explicitly rejected"), f.Dodge->ResolveExecutionDecision(query).Decision == EExecutionDecision::Reject);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgeDamagePolicyTest, "Portfolio.Combat.Dodge.DamagePolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgeDamagePolicyTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	TestTrue(TEXT("Dodge start"), f.Actions->ApplyActionDecision(f.MakeStart()));
	FDefaultDamageEvent damage = f.Damage();
	FDamageDeliveryReceipt receipt;
	damage.DeliveryReceipt = &receipt;
	damage.DamageSpec.bCanGuard = false;
	damage.DamageSpec.bCanParry = false;
	TestEqual(TEXT("Unblockable dodgeable attack avoided"), f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker), 0.f);
	TestTrue(TEXT("Zero HP loss is accepted delivery"), receipt.bAccepted);
	TestTrue(TEXT("Dodge outcome"), f.LastPacket.Result.DefenseOutcome == EDamageDefenseOutcome::Dodge);
	TestTrue(TEXT("No hit reaction"), f.LastPacket.Result.ReactionOutcome == EDamageReactionOutcome::None);
	TestFalse(TEXT("No knockback"), f.LastPacket.Context.Knockback.IsValid());
	damage.DamageSpec.bCanDodge = false;
	TestEqual(TEXT("Undodgeable attack deals damage"), f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker), 10.f);
	f.Actions->CancelActiveActionForSystem();
	Field<bool>(f.Defense, TEXT("bCanParry")) = true;
	Field<bool>(f.Defense, TEXT("bCanGuard")) = true;
	damage.DamageSpec.bCanParry = true;
	damage.DamageSpec.bCanGuard = true;
	TestEqual(TEXT("Parry avoids damage"), f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker), 0.f);
	TestTrue(TEXT("Guard mitigation cannot overwrite parry"), f.LastPacket.Result.DefenseOutcome == EDamageDefenseOutcome::Parry);
	damage.DamageSpec.bCanParry = false;
	TestEqual(TEXT("Guard only"), f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker), 5.f);
	damage.DamageSpec.bCanGuard = false;
	TestEqual(TEXT("Neither defense allowed"), f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker), 10.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgeContactConsumptionTest, "Portfolio.Combat.Dodge.ContactConsumption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgeContactConsumptionTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	UCCombatSignalSourceComponent* source = NewObject<UCCombatSignalSourceComponent>(f.Attacker);
	FCharacterComponentReferences references;
	references.OwnerCharacter = f.Attacker;
	source->InitializeReferences(references);
	FDefaultDamageEvent damage = f.Damage();
	Field<TMap<FDamageSpecKey, FDamageSpec>>(source, TEXT("DamageSpecContainer")).Add(damage.DamageSpecKey, damage.DamageSpec);
	AActor* weapon = f.World->SpawnActor<AActor>();
	weapon->SetOwner(f.Attacker);
	UBoxComponent* collision = NewObject<UBoxComponent>(weapon);
	FHitContext hit;
	hit.OverlapContext.OwnerActor = f.Attacker;
	hit.OverlapContext.DamageCauser = weapon;
	hit.OverlapContext.OverlappedComponent = collision;
	hit.OverlapContext.OverlapShape = collision;
	hit.OverlapContext.OtherActor = f.Player;
	hit.OverlapContext.OtherComponent = f.Player->GetCapsuleComponent();
	hit.OverlapContext.HitWindowId = 1;
	hit.WeaponContext.WeaponType = EWeaponType::Sword;
	hit.ActionDataKey.ActionType = EActionType::ComboAttack;
	hit.ActionDataKey.ActionIndex = 0;
	source->NotifyHitWindowOpened(weapon, 1);
	TestTrue(TEXT("Dodge starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	f.Target->OnCombatSignalTargetAccepted.AddLambda([&](const FCombatSignalTargetPacket&) { source->RequestCombatSignalSource(hit); });
	source->RequestCombatSignalSource(hit);
	TestEqual(TEXT("Reentrant duplicate prevented"), f.AcceptedCount, 1);
	f.Actions->CancelActiveActionForSystem();
	source->RequestCombatSignalSource(hit);
	TestEqual(TEXT("Accepted evasion consumes same attack beyond immunity"), f.AcceptedCount, 1);
	TestEqual(TEXT("Health preserved"), f.Player->GetHealthComp()->GetCurrentHP(), 100.f);
	source->NotifyHitWindowClosed(weapon, 1);
	source->RequestCombatSignalSource(hit);
	TestEqual(TEXT("Closed window rejects stale contact"), f.AcceptedCount, 1);
	hit.OverlapContext.HitWindowId = 2;
	source->NotifyHitWindowOpened(weapon, 2);
	source->RequestCombatSignalSource(hit);
	TestEqual(TEXT("New attack may hit"), f.AcceptedCount, 2);
	TestEqual(TEXT("New attack damage"), f.Player->GetHealthComp()->GetCurrentHP(), 90.f);
	f.Target->OnCombatSignalTargetAccepted.AddLambda([&](const FCombatSignalTargetPacket&)
	{
		source->NotifyHitWindowClosed(weapon, hit.OverlapContext.HitWindowId);
	});
	hit.OverlapContext.HitWindowId = 3;
	source->NotifyHitWindowOpened(weapon, 3);
	source->RequestCombatSignalSource(hit);
	source->RequestCombatSignalSource(hit);
	TestEqual(TEXT("Synchronous close cannot resurrect window"), f.AcceptedCount, 3);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerfectDodgeLifecycleTest, "Portfolio.Combat.Dodge.PerfectLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPerfectDodgeLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	if (!TestNotNull(TEXT("Montage loaded"), f.Montage)) return false;
	Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = 0.18f;
	FReactionData data;
	data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
	data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
	data.Montage = DuplicateObject<UAnimMontage>(f.Montage, GetTransientPackage());
	Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	TestTrue(TEXT("Dodge starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	const uint64 dodgeGeneration = f.Actions->GetActiveActionGeneration();
	FDefaultDamageEvent damage = f.Damage();
	TestEqual(TEXT("Perfect contact avoids damage"), f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker), 0.f);
	TestTrue(TEXT("Perfect outcome"), f.LastPacket.Result.DefenseOutcome == EDamageDefenseOutcome::PerfectDodge);
	TestTrue(TEXT("Dodge interrupted"), !f.Actions->IsActive());
	TestTrue(TEXT("Actual success reaction owns immunity"), f.Defense->IsPerfectDodgeActive());
	TestNull(TEXT("Counter connection not open at start"), f.Defense->GetCounterRushTarget());
	const uint64 perfectGeneration = f.References.ReactionComponent->GetActiveReactionGeneration();
	f.Defense->EndDodgeExecution(dodgeGeneration);
	TestTrue(TEXT("Old dodge cleanup preserves perfect immunity"), f.Defense->IsPerfectDodgeActive());
	f.Player->GetMesh()->GetAnimInstance()->Montage_SetPosition(data.Montage, 0.9f);
	TestTrue(TEXT("Counter remembers attacker"), f.Defense->GetCounterRushTarget() == f.Attacker);
	const uint64 opportunity = f.Defense->GetCounterRushSerial();
	TestEqual(TEXT("Query does not consume opportunity"), f.Defense->GetCounterRushSerial(), opportunity);
	f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker);
	TestTrue(TEXT("Later contact is evasion, not new success"), f.LastPacket.Result.DefenseOutcome == EDamageDefenseOutcome::Dodge);
	TestEqual(TEXT("No restart on contact"), f.References.ReactionComponent->GetActiveReactionGeneration(), perfectGeneration);
	f.References.ReactionComponent->GetActiveReactionExecutor()->Complete();
	TestFalse(TEXT("Completion clears immunity"), f.Defense->IsPerfectDodgeActive());
	TestTrue(TEXT("Completion grants real-time grace"), f.Defense->GetCounterRushTarget() == f.Attacker);
	f.Attacker->SetActorLocation(FVector(1000.f, 0.f, 0.f));
	TestNull(TEXT("Out of range rejected"), f.Defense->GetCounterRushTarget());
	f.Attacker->SetActorLocation(FVector(250.f, 0.f, 0.f));
	UCAction_CounterRush* rush = NewObject<UCAction_CounterRush>(f.Actions);
	rush->InitializeReferences(f.References);
	FActionExecutionResult rushResult = f.MakeStart();
	rushResult.ResolvedContext.ActionDataKey.ActionType = EActionType::CounterRush;
	rushResult.ResolvedContext.ActionExecutor = rush;
	rushResult.ResolvedContext.ActionData.ActionDataKey = rushResult.ResolvedContext.ActionDataKey;
	rushResult.ResolvedContext.ActionData.ActionExecutorKey = rush->GetClass();
	rushResult.ResolvedContext.ActionData.Montage = NewObject<UAnimMontage>();
	TestFalse(TEXT("Failed rush playback"), f.Actions->ApplyActionDecision(rushResult));
	TestEqual(TEXT("Failed rush preserves opportunity"), f.Defense->GetCounterRushSerial(), opportunity);
	rushResult.ResolvedContext.ActionData.Montage = f.Montage;
	TestTrue(TEXT("Rush starts during grace"), f.Actions->ApplyActionDecision(rushResult));
	TestEqual(TEXT("Actual rush consumes once"), f.Defense->GetCounterRushSerial(), uint64(0));
	TestFalse(TEXT("Rush inherits no invulnerability"), f.Defense->CanDodge() || f.Defense->IsPerfectDodgeActive());
	TestFalse(TEXT("Repeated request cannot reuse success"), f.Actions->ApplyActionDecision(rushResult));
	f.Actions->CancelActiveActionForSystem();
	TestTrue(TEXT("Next dodge starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker);
	f.Defense->EndPerfectDodgeExecution(perfectGeneration);
	TestTrue(TEXT("Stale success cleanup preserves successor"), f.Defense->IsPerfectDodgeActive());
	f.References.ReactionComponent->CancelActiveReactionForSystem();
	TestFalse(TEXT("Interrupt clears immunity"), f.Defense->IsPerfectDodgeActive());
	data.Montage = NewObject<UAnimMontage>();
	Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	TestTrue(TEXT("Dodge before failed success start"), f.Actions->ApplyActionDecision(f.MakeStart()));
	TestEqual(TEXT("Failed reaction never retroactively applies damage"), f.Target->RequestCombatDamageTarget(10.f, damage, f.Controller, f.Attacker), 0.f);
	TestFalse(TEXT("Failed reaction leaves no immunity"), f.Defense->IsPerfectDodgeActive());
	TestFalse(TEXT("Failed reaction leaves no pending authorization"), f.Defense->CanStartPerfectDodge(f.LastPacket.ResultSerial));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerfectDodgeCleanupTest, "Portfolio.Combat.Dodge.OpportunityCleanup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPerfectDodgeCleanupTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = .18f;
	FReactionData data;
	data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
	data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
	data.Montage = DuplicateObject<UAnimMontage>(f.Montage, GetTransientPackage());
	Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	auto startPerfect = [&]()
	{
		f.Actions->ApplyActionDecision(f.MakeStart());
		f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
		f.Player->GetMesh()->GetAnimInstance()->Montage_SetPosition(data.Montage, .9f);
	};
	UCHealthComponent* targetHealth = f.Attacker->FindComponentByClass<UCHealthComponent>();
	// Earlier listener revives synchronously; the death event must still invalidate the opportunity.
	targetHealth->OnDeadStateChanged.AddLambda([&](EDeadState, EDeadState Current)
	{
		if (Current == EDeadState::Dead) targetHealth->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::FillToMax);
	});
	startPerfect();
	TestNotNull(TEXT("Opportunity before target death"), f.Defense->GetCounterRushTarget());
	targetHealth->TryKill();
	TestTrue(TEXT("Target synchronously revived"), targetHealth->IsAlive());
	TestNull(TEXT("Death permanently invalidates old opportunity"), f.Defense->GetCounterRushTarget());
	TestTrue(TEXT("Target death does not cancel perfect immunity"), f.Defense->IsPerfectDodgeActive());
	f.References.ReactionComponent->CancelActiveReactionForSystem();
	Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).CounterGraceSeconds = 0.f;
	startPerfect();
	f.References.ReactionComponent->GetActiveReactionExecutor()->Complete();
	TestNull(TEXT("Zero grace expires at completion"), f.Defense->GetCounterRushTarget());
	startPerfect();
	f.Defense->InitializeReferences(f.References);
	TestFalse(TEXT("Reference reset clears immunity"), f.Defense->IsPerfectDodgeActive());
	TestNull(TEXT("Reference reset clears opportunity"), f.Defense->GetCounterRushTarget());
	f.References.ReactionComponent->CancelActiveReactionForSystem();
	startPerfect();
	f.References.HealthComponent->TryKill();
	f.References.HealthComponent->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::FillToMax);
	TestFalse(TEXT("Owner death then revive cannot restore immunity"), f.Defense->IsPerfectDodgeActive());
	TestNull(TEXT("Owner death then revive cannot restore opportunity"), f.Defense->GetCounterRushTarget());
	targetHealth->OnDeadStateChanged.Clear();
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgeAuthoredPlaybackTest, "Portfolio.Combat.Dodge.AuthoredPlayback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgeAuthoredPlaybackTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	UAnimMontage* normal = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/04_Montage/Dodge/Default/M_Dodge"));
	f.Montage = normal;
	TestTrue(TEXT("Original dodge starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	auto* anim = CastChecked<UCMontageProbeAnimInstance>(f.Player->GetMesh()->GetAnimInstance());
	for (int32 tick = 0; tick < 60; ++tick) anim->AdvanceWeightedProbe(1.f / 30.f);
	TestFalse(TEXT("Original completion notify exits dodge"), f.Actions->IsActive());
	TestFalse(TEXT("Original completion clears immunity"), f.Defense->CanDodge());
	Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = .18f;
	FReactionData data;
	data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
	data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
	data.Montage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/04_Montage/Dodge/Default/M_PerfectDodge"));
	if (!TestNotNull(TEXT("Authored perfect montage"), data.Montage)) return false;
	Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	TestTrue(TEXT("Dodge before authored perfect"), f.Actions->ApplyActionDecision(f.MakeStart()));
	f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
	TestTrue(TEXT("Authored perfect owns immunity"), f.Defense->IsPerfectDodgeActive());
	for (int32 tick = 0; tick < 60; ++tick) anim->AdvanceWeightedProbe(1.f / 30.f);
	TestFalse(TEXT("Authored complete reaction notify exits success"), f.References.ReactionComponent->IsActive());
	TestFalse(TEXT("Authored completion clears perfect immunity"), f.Defense->IsPerfectDodgeActive());
	TestNotNull(TEXT("Authored completion grants opportunity"), f.Defense->GetCounterRushTarget());
	USkeletalMesh* stellar = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/01_Character/Asset/StellarGirlCeleste/Mesh/SK_Celeste_01"));
	UAnimMontage* rushMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/04_Montage/Dodge/Stellar/M_CounterRush_Stellar"));
	if (!TestNotNull(TEXT("Stellar fixture mesh"), stellar) || !TestNotNull(TEXT("Authored rush montage"), rushMontage)) return false;
	f.Player->GetMesh()->SetSkeletalMesh(stellar);
	f.Player->GetMesh()->InitAnim(true);
	UCAction_CounterRush* rush = NewObject<UCAction_CounterRush>(f.Actions);
	rush->InitializeReferences(f.References);
	FActionExecutionResult rushResult = f.MakeStart();
	rushResult.ResolvedContext.ActionDataKey.ActionType = EActionType::CounterRush;
	rushResult.ResolvedContext.ActionExecutor = rush;
	rushResult.ResolvedContext.ActionData.ActionDataKey = rushResult.ResolvedContext.ActionDataKey;
	rushResult.ResolvedContext.ActionData.ActionExecutorKey = rush->GetClass();
	rushResult.ResolvedContext.ActionData.Montage = rushMontage;
	TestTrue(TEXT("Authored Stellar rush starts"), f.Actions->ApplyActionDecision(rushResult));
	anim = CastChecked<UCMontageProbeAnimInstance>(f.Player->GetMesh()->GetAnimInstance());
	for (int32 tick = 0; tick < 60; ++tick) anim->AdvanceWeightedProbe(1.f / 30.f);
	TestFalse(TEXT("Authored rush complete notify exits action"), f.Actions->IsActive());
	TestEqual(TEXT("Authored rush consumes opportunity"), f.Defense->GetCounterRushSerial(), uint64(0));
	return true;
}
#endif
