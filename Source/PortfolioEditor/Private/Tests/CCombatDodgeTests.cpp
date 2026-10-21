#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/CActionFacingProbe.h"
#include "Tests/CMontageNotifyProbe.h"
#include "Action/CAction_Dodge.h"
#include "Action/CAction_CounterRush.h"
#include "Reaction/CReaction_PerfectDodge.h"
#include "Notify/CAnimNotify_PerfectDodgePresentation.h"
#include "Notify/CAnimNotify_WeaponGlow.h"
#include "Notify/CAnimNotify_BodyGlow.h"
#include "Notify/CAnimNotify_CounterOpportunity.h"
#include "Animation/ActiveMontageInstanceScope.h"
#include "Notify/CAnimNotifyState_DodgeWindow.h"
#include "Component/CReactionFeedbackComponent.h"
#include "NiagaraComponent.h"
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
#include "Weapon/CWeaponActor.h"
#include "Materials/MaterialInterface.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"
#include "HAL/PlatformProcess.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgeMontageWindowTest, "Portfolio.Combat.Dodge.MontageWindows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgeMontageWindowTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	if (!TestNotNull(TEXT("Montage"), f.Montage)) return false;
	auto& tuning = Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning"));
	TestFalse(TEXT("Existing assets retain legacy timing"), tuning.bUseMontageWindows);
	tuning.bUseMontageWindows = true;
	TestTrue(TEXT("Dodge starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	TestFalse(TEXT("Missing markers fail closed, no numeric fallback"), f.Defense->CanDodge());
	const auto add = [&f](EDodgeWindowKind kind, float start, float end)
	{
		auto* marker = NewObject<UCAnimNotifyState_DodgeWindow>(f.Montage);
		marker->Kind = kind;
		auto& event = f.Montage->Notifies.AddDefaulted_GetRef();
		event.NotifyStateClass = marker;
		event.Link(f.Montage, start);
		event.SetDuration(end - start);
		event.EndLink.Link(f.Montage, end);
	};
	add(EDodgeWindowKind::Invulnerability, .1f, .5f);
	add(EDodgeWindowKind::Perfect, 0.f, .25f);
	auto* anim = f.Player->GetMesh()->GetAnimInstance();
	anim->Montage_SetPosition(f.Montage, .05f);
	TestTrue(TEXT("Perfect marker outside immunity cannot grant defense"), f.Defense->ResolveDodgeContact() == EDamageDefenseOutcome::None);
	anim->Montage_SetPosition(f.Montage, .1f);
	TestTrue(TEXT("Start inclusive without NotifyBegin delivery"), f.Defense->CanDodge());
	TestTrue(TEXT("Perfect contact"), f.Defense->ResolveDodgeContact() == EDamageDefenseOutcome::PerfectDodge);
	TestTrue(TEXT("Success only once per execution"), f.Defense->ResolveDodgeContact() == EDamageDefenseOutcome::Dodge);
	anim->Montage_SetPosition(f.Montage, .5f);
	TestFalse(TEXT("End exclusive without NotifyEnd delivery"), f.Defense->CanDodge());
	TestFalse(TEXT("NaN rejected"), UCAnimNotifyState_DodgeWindow::Contains(f.Montage, EDodgeWindowKind::Perfect, std::numeric_limits<float>::quiet_NaN()));
	anim->Montage_SetPosition(f.Montage, .2f);
	f.Actions->CancelActiveActionForSystem();
	TestFalse(TEXT("Interrupt clears ownership inside interval"), f.Defense->CanDodge());
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
	auto* feedback = f.References.ReactionFeedbackComponent;
	feedback->InitializeReferences(f.References);
	UMaterialInterface* glow = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/CharacterOverlay/Materials/Material_Instance/MI_Fresnel"));
	if (!TestNotNull(TEXT("Glow material loaded"), glow)) return false;
	Field<UMaterialInterface*>(feedback, TEXT("PerfectBodyMaterial")) = glow;
	Field<UMaterialInterface*>(feedback, TEXT("CounterWeaponMaterial")) = glow;
	UClass* weaponClass = LoadClass<ACWeaponActor>(nullptr, TEXT("/Game/06_Weapon/BP_CWeaponActor_Sword.BP_CWeaponActor_Sword_C"));
	if (!TestNotNull(TEXT("Authored weapon class"), weaponClass)) return false;
	Field<ACWeaponActor*>(f.References.WeaponComponent, TEXT("WeaponActor")) = f.World->SpawnActor<ACWeaponActor>(weaponClass);
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
	TestTrue(TEXT("Presentation includes preparation before input opens"), f.Defense->IsCounterRushPresentationPending(f.LastPacket.ResultSerial));
	TestFalse(TEXT("Success waits for body Start marker"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("BodyEnvelopeState")).bRunning);
	TestFalse(TEXT("Success does not auto-start weapon"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("WeaponEnvelopeState")).bRunning);
	const uint64 perfectGeneration = f.References.ReactionComponent->GetActiveReactionGeneration();
	feedback->StartWeaponGlow(false, perfectGeneration, 1.f, .08f, .18f, 8.f);
	TestTrue(TEXT("Point Start sustains weapon"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("WeaponEnvelopeState")).bSustain);
	feedback->EndWeaponGlow(false, perfectGeneration + 1, 0.f);
	TestFalse(TEXT("Stale End ignored"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("WeaponEnvelopeState")).bReleased);
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
	TestTrue(TEXT("Completion without End releases glow"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("WeaponEnvelopeState")).bReleased);
	TestFalse(TEXT("Completion clears immunity"), f.Defense->IsPerfectDodgeActive());
	TestTrue(TEXT("Completion grants real-time grace"), f.Defense->GetCounterRushTarget() == f.Attacker);
	f.Attacker->SetActorLocation(FVector(1000.f, 0.f, 0.f));
	TestNull(TEXT("Out of range rejected"), f.Defense->GetCounterRushTarget());
	TestTrue(TEXT("Temporary distance does not flicker preparation glow"), f.Defense->IsCounterRushPresentationPending(opportunity));
	f.Attacker->SetActorLocation(FVector(250.f, 0.f, 0.f));
	f.Player->SetActorScale3D(FVector(1.25));
	f.Attacker->SetActorScale3D(FVector(2.));
	auto& rushTuning = Field<FCounterRushTuning>(f.Actions, TEXT("CounterRushTuning"));
	rushTuning.MaxTravelDistance = 75.f;
	rushTuning.ExtraGap = 17.f;
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
	const FCounterRushPlan committedPlan = rush->GetRushPlan();
	TestEqual(TEXT("Runtime plan uses scaled player capsule"), committedPlan.PlayerRadius, f.Player->GetCapsuleComponent()->GetScaledCapsuleRadius());
	TestEqual(TEXT("Runtime plan uses scaled target capsule"), committedPlan.TargetRadius, f.Attacker->GetCapsuleComponent()->GetScaledCapsuleRadius());
	TestEqual(TEXT("Component tuning supplies gap"), committedPlan.CenterSpacing, committedPlan.PlayerRadius + committedPlan.TargetRadius + 17.f);
	TestTrue(TEXT("Component tuning limits distance"), committedPlan.TravelDistance <= 75.f);
	f.Attacker->SetActorLocation(FVector(300.f, 200.f, 0.f));
	rush->Tick(0.f);
	TestTrue(TEXT("Rush does not retarget after commit"), rush->GetRushPlan().Target.Equals(committedPlan.Target));
	TestEqual(TEXT("Actual rush consumes once"), f.Defense->GetCounterRushSerial(), uint64(0));
	TestFalse(TEXT("Consumed opportunity no longer pending"), f.Defense->IsCounterRushPresentationPending(opportunity));
	feedback->StartWeaponGlow(true, f.Actions->GetActiveActionGeneration(), 1.f, .08f, .18f, 8.f);
	f.Actions->HandleActionCollisionWindowEnd();
	TestFalse(TEXT("Collision end does not release authored glow"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("WeaponEnvelopeState")).bReleased);
	feedback->EndWeaponGlow(false, perfectGeneration, 0.f);
	TestFalse(TEXT("Old reaction End cannot release rush glow"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("WeaponEnvelopeState")).bReleased);
	feedback->EndWeaponGlow(true, f.Actions->GetActiveActionGeneration(), .12f);
	TestTrue(TEXT("Point End releases weapon"), Field<FCombatPresentationEnvelopeState>(feedback, TEXT("WeaponEnvelopeState")).bReleased);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeaponGlowOwnershipTest, "Portfolio.Combat.Presentation.WeaponGlowOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWeaponGlowOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	auto* feedback = f.References.ReactionFeedbackComponent;
	feedback->InitializeReferences(f.References);
	feedback->CounterWeaponMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/CharacterOverlay/Materials/Material_Instance/MI_Fresnel"));
	UClass* weaponClass = LoadClass<ACWeaponActor>(nullptr, TEXT("/Game/06_Weapon/BP_CWeaponActor_Sword.BP_CWeaponActor_Sword_C"));
	if (!TestNotNull(TEXT("Weapon class"), weaponClass) || !TestNotNull(TEXT("Material"), feedback->CounterWeaponMaterial)) return false;
	Field<ACWeaponActor*>(f.References.WeaponComponent, TEXT("WeaponActor")) = f.World->SpawnActor<ACWeaponActor>(weaponClass);
	FReactionData data;
	data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
	data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
	data.Montage = DuplicateObject<UAnimMontage>(f.Montage, GetTransientPackage());
	Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = .18f;
	TestTrue(TEXT("Dodge starts"), f.Actions->ApplyActionDecision(f.MakeStart()));
	f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
	const uint64 reactionGeneration = f.References.ReactionComponent->GetActiveReactionGeneration();
	feedback->StartWeaponGlow(false, reactionGeneration, .7f, .08f, .18f, 8.f);
	TestTrue(TEXT("Glow started"), feedback->WeaponEnvelopeState.bRunning);
	const double deadline = feedback->WeaponSafetyDeadline;
	feedback->StartWeaponGlow(false, reactionGeneration, .7f, .08f, .18f, 100.f);
	TestEqual(TEXT("Repeated Start cannot extend deadline"), feedback->WeaponSafetyDeadline, deadline);
	f.Player->GetMesh()->GetAnimInstance()->Montage_SetPosition(data.Montage, .9f);
	const uint64 serial = f.Defense->GetCounterRushSerial();
	feedback->BeginWeaponGlowTransfer(serial);
	f.References.ReactionComponent->GetActiveReactionExecutor()->Complete();
	TestFalse(TEXT("Pending handoff survives old reaction cleanup"), feedback->WeaponEnvelopeState.bReleased);
	auto* rush = NewObject<UCAction_CounterRush>(f.Actions);
	rush->InitializeReferences(f.References);
	FActionExecutionResult result = f.MakeStart();
	result.ResolvedContext.ActionDataKey.ActionType = EActionType::CounterRush;
	result.ResolvedContext.ActionExecutor = rush;
	result.ResolvedContext.ActionData.ActionDataKey = result.ResolvedContext.ActionDataKey;
	result.ResolvedContext.ActionData.ActionExecutorKey = rush->GetClass();
	TestTrue(TEXT("Committed rush transfers glow"), f.Actions->ApplyActionDecision(result));
	TestTrue(TEXT("Action owns transferred glow"), feedback->bWeaponTransferredToAttack);
	TestFalse(TEXT("Handoff did not release"), feedback->WeaponEnvelopeState.bReleased);
	TestEqual(TEXT("Handoff preserves safety deadline"), feedback->WeaponSafetyDeadline, deadline);
	feedback->EndWeaponGlow(false, reactionGeneration, 0.f);
	TestFalse(TEXT("Late reaction End ignored"), feedback->WeaponEnvelopeState.bReleased);
	f.Actions->CancelActiveActionForSystem();
	TestTrue(TEXT("Rush cancellation releases"), feedback->WeaponEnvelopeState.bReleased);
	TestEqual(TEXT("Cancellation clears safety deadline"), feedback->WeaponSafetyDeadline, 0.);
	// Exercise timeout deterministically without sleeping or depending on game time dilation.
	TestTrue(TEXT("Next dodge"), f.Actions->ApplyActionDecision(f.MakeStart()));
	f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
	const uint64 next = f.References.ReactionComponent->GetActiveReactionGeneration();
	feedback->StartWeaponGlow(false, next, 1.f, 0.f, 0.f, 8.f);
	feedback->WeaponSafetyDeadline = FPlatformTime::Seconds() - 1.;
	feedback->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Missing End times out and restores"), feedback->WeaponEnvelopeState.bRunning);
	feedback->StartWeaponGlow(false, next, 1.f, 0.f, .18f, 8.f);
	feedback->BeginWeaponGlowTransfer(feedback->WeaponOpportunitySerial);
	feedback->EndWeaponGlowOwner(false, next);
	feedback->FinishWeaponGlowTransfer();
	TestTrue(TEXT("Failed handoff releases ended owner"), feedback->WeaponEnvelopeState.bReleased);
	feedback->RestoreCounterWeapon();
	feedback->StartWeaponGlow(false, next, 1.f, 0.f, .18f, 8.f);
	feedback->EndWeaponGlow(false, next, .25f);
	TestEqual(TEXT("Explicit End clears watchdog"), feedback->WeaponSafetyDeadline, 0.);
	TestEqual(TEXT("End chooses fade duration"), feedback->WeaponEnvelopeState.Profile.FadeOut, .25f);
	Field<ACWeaponActor*>(f.References.WeaponComponent, TEXT("WeaponActor")) = nullptr;
	feedback->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Unequip restores immediately"), feedback->WeaponEnvelopeState.bRunning);
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
	f.References.ReactionFeedbackComponent->InitializeReferences(f.References);
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
	TestFalse(TEXT("Success itself does not start vignette before Notify"),
		Field<FCombatPresentationEnvelopeState>(f.References.ReactionFeedbackComponent, TEXT("ScreenEnvelopeState")).bRunning);
	TestEqual(TEXT("Success itself does not start slow before Notify"), f.Player->CustomTimeDilation, 1.f);
	f.Controller->Possess(f.Player);
	float presentationTime = 0.f;
	int32 presentationCount = 0;
	for (const auto& event : data.Montage->Notifies)
		if (Cast<UCAnimNotify_PerfectDodgePresentation>(event.Notify))
		{
			presentationTime = FMath::Max(presentationTime, event.GetTime());
			++presentationCount;
		}
	TestEqual(TEXT("Authored slow and vignette markers exist"), presentationCount, 2);
	anim->AdvanceWeightedProbe(presentationTime + .02f);
	TestTrue(TEXT("Authored point Notify starts vignette"),
		Field<FCombatPresentationEnvelopeState>(f.References.ReactionFeedbackComponent, TEXT("ScreenEnvelopeState")).bRunning);
	FPlatformProcess::Sleep(.04f);
	f.World->GetSubsystem<UCWorldSubsystem_CombatFeedback>()->Tick(0.f);
	TestTrue(TEXT("Authored point Notify slows player"), f.Player->CustomTimeDilation < 1.f);
	TestTrue(TEXT("Authored point Notify slows remembered attacker"), f.Attacker->CustomTimeDilation < 1.f);
	for (int32 tick = 0; tick < 60; ++tick) anim->AdvanceWeightedProbe(1.f / 30.f);
	TestFalse(TEXT("Authored complete reaction notify exits success"), f.References.ReactionComponent->IsActive());
	TestFalse(TEXT("Authored completion clears perfect immunity"), f.Defense->IsPerfectDodgeActive());
	TestNotNull(TEXT("Authored completion grants opportunity"), f.Defense->GetCounterRushTarget());
	USkeletalMesh* stellar = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/01_Character/Asset/StellarGirlCeleste/Mesh/SK_Celeste_01"));
	UClass* playerClass = LoadClass<ACPlayer>(nullptr, TEXT("/Game/01_Character/01_Player/BP_CPlayer_Stellar.BP_CPlayer_Stellar_C"));
	ACPlayer* playerDefaults = playerClass ? Cast<ACPlayer>(playerClass->GetDefaultObject()) : nullptr;
	UCActionComponent* authoredActions = playerDefaults ? playerDefaults->FindComponentByClass<UCActionComponent>() : nullptr;
	if (!TestNotNull(TEXT("Authored action configuration"), authoredActions)) return false;
	UAnimMontage* rushMontage = nullptr;
	for (const FActionData& entry : Field<TArray<FActionData>>(authoredActions, TEXT("ActionDatas")))
	{
		if (entry.ActionDataKey.ActionType == EActionType::CounterRush
			&& entry.ActionExecutorKey && entry.ActionExecutorKey->IsChildOf(UCAction_CounterRush::StaticClass()))
		{
			rushMontage = entry.Montage;
			break;
		}
	}
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerfectVignetteEnvelopeTest, "Portfolio.Combat.Presentation.VignetteLifetime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPerfectVignetteEnvelopeTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	f.Controller->Possess(f.Player);
	auto* feedback = f.References.ReactionFeedbackComponent;
	feedback->InitializeReferences(f.References);
	auto* camera = f.Player->FindComponentByClass<UCameraComponent>();
	if (!TestNotNull(TEXT("Player camera"), camera)) return false;
	camera->PostProcessSettings.VignetteIntensity = .2f;
	camera->PostProcessSettings.bOverride_VignetteIntensity = false;
	FCombatPresentationEnvelope profile;
	profile.FadeIn = profile.FadeOut = 2.f;
	profile.Hold = 1.f;
	feedback->StartPerfectDodgeVignette(profile, .8f);
	TestEqual(TEXT("Start does not jump to peak"), camera->PostProcessSettings.VignetteIntensity, .2f);
	auto& state = Field<FCombatPresentationEnvelopeState>(feedback, TEXT("ScreenEnvelopeState"));
	state.StartedAt = FPlatformTime::Seconds() - 1.;
	static_cast<UActorComponent*>(feedback)->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Fade in reaches midpoint"), FMath::IsNearlyEqual(camera->PostProcessSettings.VignetteIntensity, .5f, .01f));
	state.StartedAt = FPlatformTime::Seconds() - 4.;
	static_cast<UActorComponent*>(feedback)->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Fade out reaches midpoint"), FMath::IsNearlyEqual(camera->PostProcessSettings.VignetteIntensity, .5f, .01f));
	state.StartedAt = FPlatformTime::Seconds() - 6.;
	static_cast<UActorComponent*>(feedback)->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestEqual(TEXT("Original intensity restored"), camera->PostProcessSettings.VignetteIntensity, .2f);
	TestFalse(TEXT("Original override restored"), camera->PostProcessSettings.bOverride_VignetteIntensity);
	feedback->StartPerfectDodgeVignette(profile, .8f);
	camera->PostProcessSettings.VignetteIntensity = .93f;
	static_cast<UActorComponent*>(feedback)->TickComponent(0.f, LEVELTICK_All, nullptr);
	feedback->InitializeReferences(f.References);
	TestEqual(TEXT("External vignette owner survives cleanup"), camera->PostProcessSettings.VignetteIntensity, .93f);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCounterRushFOVTest, "Portfolio.Combat.Presentation.RushFOV",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCounterRushFOVTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	FFixture f;
	f.Controller->Possess(f.Player);
	auto* feedback = f.References.ReactionFeedbackComponent;
	feedback->InitializeReferences(f.References);
	auto* camera = f.Player->FindComponentByClass<UCameraComponent>();
	if (!TestNotNull(TEXT("Player camera"), camera)) return false;
	camera->SetFieldOfView(90.f);
	feedback->BeginRushPresentation(41);
	const double start = feedback->RushLastRealTime;
	feedback->TickRushPresentation(start + 1.);
	TestEqual(TEXT("No actual movement means no FOV expansion"), camera->FieldOfView, 90.f);
	feedback->UpdateRushPresentation(41, 1.f);
	feedback->TickRushPresentation(start + 1.02);
	TestTrue(TEXT("FOV blends through midpoint"), FMath::IsNearlyEqual(camera->FieldOfView, 92.5f, .01f));
	feedback->TickRushPresentation(start + 1.04);
	TestEqual(TEXT("Full movement reaches configured offset"), camera->FieldOfView, 95.f);
	feedback->EndRushPresentation(40);
	TestFalse(TEXT("Stale execution cannot end current cue"), feedback->bRushPresentationReleased);
	feedback->UpdateRushPresentation(41, 0.f);
	feedback->TickRushPresentation(start + 1.16);
	TestTrue(TEXT("Blocked movement fades back"), FMath::IsNearlyEqual(camera->FieldOfView, 90.f));
	feedback->EndRushPresentation(41);
	feedback->TickRushPresentation(start + 1.3);
	TestEqual(TEXT("Finished cue releases serial"), feedback->RushPresentationSerial, uint64(0));
	feedback->BeginRushPresentation(42);
	feedback->UpdateRushPresentation(42, 1.f);
	feedback->TickRushPresentation(feedback->RushLastRealTime + 1.);
	camera->SetFieldOfView(77.f);
	feedback->TickRushPresentation(feedback->RushLastRealTime + 1.);
	feedback->InitializeReferences(f.References);
	TestEqual(TEXT("External FOV survives cleanup"), camera->FieldOfView, 77.f);
	feedback->BeginRushPresentation(43);
	feedback->UpdateRushPresentation(43, 1.f);
	feedback->TickRushPresentation(feedback->RushLastRealTime + 1.);
	feedback->InitializeReferences(f.References);
	TestEqual(TEXT("Owned FOV restored on reinitialization"), camera->FieldOfView, 77.f);
	feedback->RushFOVOffset = 0.f;
	feedback->BeginRushPresentation(44);
	TestEqual(TEXT("VFX lifetime does not require FOV"), feedback->RushPresentationSerial, uint64(44));
	TestFalse(TEXT("FOV disabled independently"), feedback->RushCamera.IsValid());
	auto* ownedVFX = NewObject<UNiagaraComponent>(f.Player);
	ownedVFX->SetupAttachment(f.Player->GetRootComponent());
	feedback->RushVFXComponent = ownedVFX;
	feedback->EndRushPresentation(44);
	TestNull(TEXT("VFX tail detaches from follow-up attack movement"), ownedVFX->GetAttachParent());
	const double released = feedback->RushReleasedAt;
	feedback->EndRushPresentation(44);
	TestEqual(TEXT("Repeated end does not extend tail"), feedback->RushReleasedAt, released);
	feedback->TickRushPresentation(released + .1);
	TestNotNull(TEXT("Owned VFX retained within tail budget"), feedback->RushVFXComponent);
	feedback->TickRushPresentation(released + .3);
	TestNull(TEXT("Owned VFX removed after tail budget"), feedback->RushVFXComponent);
	feedback->BeginRushPresentation(45);
	feedback->RushVFXComponent = NewObject<UNiagaraComponent>(f.Player);
	feedback->InitializeReferences(f.References);
	TestNull(TEXT("Reinitialization clears VFX immediately"), feedback->RushVFXComponent);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNormalDodgePlayerDeathTest, "Portfolio.Combat.Dodge.NormalPlayerDeathStopsRootMotion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNormalDodgePlayerDeathTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	for (int32 scenario = 0; scenario < 4; ++scenario)
	{
		FFixture f;
		if (!TestNotNull(TEXT("Fixture montage"), f.Montage)) return false;
		f.Player->DispatchBeginPlay();
		Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Reset();
		if (!TestTrue(TEXT("Normal dodge started"), f.Actions->ApplyActionDecision(f.MakeStart()))) return false;
		const uint64 generation = f.Actions->GetActiveActionGeneration();
		auto* anim = f.Player->GetMesh()->GetAnimInstance();
		auto* instance = anim->GetMontageInstanceForID(f.Dodge->GetActiveMontageInstanceId());
		if (!TestNotNull(TEXT("Owned normal dodge instance"), instance)) return false;
		f.Player->GetHealthComp()->TakeDamage(1.f);
		TestTrue(TEXT("Nonlethal health change preserves dodge"), f.Dodge->IsActive());
		TestEqual(TEXT("Nonlethal health change preserves generation"), f.Actions->GetActiveActionGeneration(), generation);
		if (scenario == 0)
		{
			f.Actions->CancelActiveActionForSystem();
			TestTrue(TEXT("Living cancellation retains normal montage blend-out"), instance->IsPlaying() && instance->IsStopped());
			continue;
		}
		if (scenario == 3) instance->Stop(FAlphaBlend(.4f), false);
		auto* movement = f.Player->GetCharacterMovement();
		const auto mode = movement->MovementMode;
		movement->Velocity = FVector(500.f, 120.f, -90.f);
		movement->RootMotionParams.Set(FTransform(FVector(10.f, 2.f, 0.f)));
		if (scenario == 2) f.Player->GetHealthComp()->TryKill();
		else
		{
			auto lethal = f.Damage();
			lethal.DamageSpec.BaseDamage = 1000.f;
			lethal.DamageRequestAmount.RequestDamage = 1000.f;
			lethal.DamageSpec.bCanDodge = false;
			f.Player->TakeDamage(1000.f, lethal, f.Controller, f.Attacker);
		}
		TestTrue(TEXT("Player died without a Dead reaction"), f.Player->GetHealthComp()->IsDead());
		TestFalse(TEXT("Dodge executor ends synchronously"), f.Dodge->IsActive());
		TestFalse(TEXT("Action is no longer active"), f.Actions->IsActive());
		TestEqual(TEXT("Action generation cleared"), f.Actions->GetActiveActionGeneration(), uint64(0));
		TestFalse(TEXT("Owned normal montage cannot advance, including blend-out"), instance->IsPlaying());
		TestFalse(TEXT("Pending root motion cleared"), movement->RootMotionParams.bHasRootMotion);
		TestTrue(TEXT("Horizontal momentum cleared"), FVector2D(movement->Velocity).IsNearlyZero());
		TestEqual(TEXT("Vertical fall velocity preserved"), movement->Velocity.Z, -90.0);
		TestEqual(TEXT("Movement mode preserved"), movement->MovementMode, mode);
		TestFalse(TEXT("Dodge immunity ended"), f.Defense->CanDodge());
		TestNull(TEXT("No counter opportunity created"), f.Defense->GetCounterRushTarget());
		TestFalse(TEXT("Repeated kill is harmless"), f.Player->GetHealthComp()->TryKill());
	}
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerfectDodgePlayerDeathTest, "Portfolio.Combat.Dodge.PlayerDeathStopsRootMotion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPerfectDodgePlayerDeathTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	// Alive interruption, lethal damage, direct kill, and death during an existing blend-out.
	for (int32 scenario = 0; scenario < 4; ++scenario)
	{
		FFixture f;
		if (!TestNotNull(TEXT("Fixture montage"), f.Montage)) return false;
		f.Player->DispatchBeginPlay();
		Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = .18f;
		FReactionData data;
		data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
		data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
		data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
		data.Montage = f.Montage;
		auto& reactions = Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap"));
		reactions.Reset(); // No Dead reaction may be required for movement cleanup.
		reactions.Add(data.ReactionDataKey, data);
		if (!TestTrue(TEXT("Dodge setup"), f.Actions->ApplyActionDecision(f.MakeStart()))) return false;
		f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
		auto* reaction = f.References.ReactionComponent->GetActiveReactionExecutor();
		if (!TestNotNull(TEXT("Perfect dodge started"), reaction)) return false;
		const uint64 generation = f.References.ReactionComponent->GetActiveReactionGeneration();
		auto* anim = f.Player->GetMesh()->GetAnimInstance();
		auto* instance = anim->GetMontageInstanceForID(reaction->GetActiveMontageInstanceId());
		if (!TestNotNull(TEXT("Owned montage instance"), instance)) return false;
		f.Player->GetHealthComp()->TakeDamage(1.f);
		TestTrue(TEXT("Nonlethal HP change preserves perfect dodge"), reaction->IsActive());
		if (scenario == 0)
		{
			f.References.ReactionComponent->CancelActiveReactionForSystem();
			TestTrue(TEXT("Alive interruption retains normal montage blend-out"), instance->IsPlaying());
			continue;
		}
		if (scenario == 3) instance->Stop(FAlphaBlend(.4f), false);
		auto* movement = f.Player->GetCharacterMovement();
		movement->Velocity = FVector(500.f, 120.f, -90.f);
		movement->RootMotionParams.Set(FTransform(FVector(10.f, 2.f, 0.f)));
		if (scenario == 2) f.Player->GetHealthComp()->TryKill();
		else
		{
			auto lethal = f.Damage();
			lethal.DamageSpec.BaseDamage = 1000.f;
			lethal.DamageRequestAmount.RequestDamage = 1000.f;
			lethal.DamageSpec.bCanDodge = false;
			f.Player->TakeDamage(1000.f, lethal, f.Controller, f.Attacker);
		}
		TestTrue(TEXT("Player died"), f.Player->GetHealthComp()->IsDead());
		TestFalse(TEXT("Reaction ends before damage call returns"), reaction->IsActive());
		TestTrue(TEXT("Stopped reaction generation invalidated"), f.References.ReactionComponent->GetActiveReactionGeneration() != generation);
		TestFalse(TEXT("Owned montage cannot advance after death, including blend-out"), instance->IsPlaying());
		TestFalse(TEXT("Pending movement root motion cleared"), movement->RootMotionParams.bHasRootMotion);
		TestTrue(TEXT("Horizontal momentum cleared"), FVector2D(movement->Velocity).IsNearlyZero());
		TestEqual(TEXT("Existing vertical fall velocity preserved"), movement->Velocity.Z, -90.0);
		TestFalse(TEXT("Perfect dodge state ended"), f.Defense->IsPerfectDodgeActive());
		TestNull(TEXT("No counter opportunity survives death"), f.Defense->GetCounterRushTarget());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCounterRushPlayerDeathTest, "Portfolio.Combat.CounterRush.PlayerDeathWithoutReaction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCounterRushPlayerDeathTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	for (const bool directKill : { false, true })
	{
		FFixture f;
		if (!TestNotNull(TEXT("Fixture montage"), f.Montage)) return false;
		// Exercise the player's real lifecycle subscription, not a direct call to the death handler.
		f.Player->DispatchBeginPlay();
		f.Attacker->SetActorLocation(FVector(400, 0, 0));
		Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = .18f;
		FReactionData data;
		data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
		data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
		data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
		data.Montage = f.Montage;
		auto& reactions = Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap"));
		reactions.Reset(); // Deliberately no Dead reaction: this was the failure condition.
		reactions.Add(data.ReactionDataKey, data);
		if (!TestTrue(TEXT("Dodge setup"), f.Actions->ApplyActionDecision(f.MakeStart()))) return false;
		f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
		if (!TestNotNull(TEXT("Perfect dodge reaction"), f.References.ReactionComponent->GetActiveReactionExecutor())) return false;
		f.Player->GetMesh()->GetAnimInstance()->Montage_SetPosition(data.Montage, .9f);
		f.References.ReactionComponent->GetActiveReactionExecutor()->Complete();

		auto* rush = NewObject<UCAction_CounterRush>(f.Actions);
		rush->InitializeReferences(f.References);
		auto request = f.MakeStart();
		request.ResolvedContext.ActionDataKey.ActionType = EActionType::CounterRush;
		request.ResolvedContext.ActionExecutor = rush;
		request.ResolvedContext.ActionData.ActionDataKey = request.ResolvedContext.ActionDataKey;
		request.ResolvedContext.ActionData.ActionExecutorKey = rush->GetClass();
		if (!TestTrue(TEXT("Rush started"), f.Actions->ApplyActionDecision(request))) return false;
		auto* movement = f.Player->GetCharacterMovement();
		auto source = movement->GetRootMotionSource(TEXT("CounterRush"));
		TestTrue(TEXT("Rush movement source installed"), source.IsValid());
		f.Player->GetHealthComp()->TakeDamage(1.f);
		TestTrue(TEXT("Nonlethal HP change preserves rush"), rush->IsActive());
		if (directKill)
			TestTrue(TEXT("Direct kill succeeds"), f.Player->GetHealthComp()->TryKill());
		else
		{
			auto lethal = f.Damage();
			lethal.DamageSpec.BaseDamage = 1000.f;
			lethal.DamageRequestAmount.RequestDamage = 1000.f;
			lethal.DamageSpec.bCanDodge = false;
			f.Player->TakeDamage(1000.f, lethal, f.Controller, f.Attacker);
		}
		TestTrue(TEXT("Player is dead"), f.Player->GetHealthComp()->IsDead());
		TestFalse(TEXT("Action ends synchronously without a world tick"), f.Actions->IsActive());
		TestFalse(TEXT("Rush executor stops synchronously"), rush->IsActive());
		TestEqual(TEXT("Action generation invalidated"), f.Actions->GetActiveActionGeneration(), uint64(0));
		TestTrue(TEXT("Rush source removed or marked for removal"), !source.IsValid()
			|| source->Status.HasFlag(ERootMotionSourceStatusFlags::MarkedForRemoval)
			|| !movement->GetRootMotionSource(TEXT("CounterRush")).IsValid());
		TestFalse(TEXT("Repeated kill is harmless"), f.Player->GetHealthComp()->TryKill());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCounterRushMovementTest, "Portfolio.Combat.CounterRush.MovementAndWall",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCounterRushMovementTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	for (bool blocked : {false, true})
	{
		FFixture f;
		if (!TestNotNull(TEXT("Fixture montage"), f.Montage)) return false;
		f.Attacker->SetActorLocation(FVector(400,0,0));
		Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = .18f;
		FReactionData data;
		data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
		data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
		data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
		data.Montage = f.Montage;
		Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
		TestTrue(TEXT("Dodge setup"), f.Actions->ApplyActionDecision(f.MakeStart()));
		f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
		f.Player->GetMesh()->GetAnimInstance()->Montage_SetPosition(data.Montage, .9f);
		f.References.ReactionComponent->GetActiveReactionExecutor()->Complete();
		auto& tuning = Field<FCounterRushTuning>(f.Actions, TEXT("CounterRushTuning"));
		tuning.MaxTravelDistance = 200.f;
		tuning.Duration = .25f;
		if (blocked)
		{
			AActor* wall = f.World->SpawnActor<AActor>();
			auto* box = NewObject<UBoxComponent>(wall);
			wall->SetRootComponent(box);
			box->SetBoxExtent(FVector(10,200,200));
			box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			box->SetCollisionObjectType(ECC_WorldStatic);
			box->SetCollisionResponseToAllChannels(ECR_Block);
			box->RegisterComponent();
			wall->SetActorLocation(FVector(120,0,0));
		}
		auto* movement = f.Player->GetCharacterMovement();
		movement->Activate(true);
		movement->bRunPhysicsWithNoController = true;
		movement->SetMovementMode(MOVE_Flying);
		f.Player->GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		auto* rush = NewObject<UCAction_CounterRush>(f.Actions);
		rush->InitializeReferences(f.References);
		auto request = f.MakeStart();
		request.ResolvedContext.ActionDataKey.ActionType = EActionType::CounterRush;
		request.ResolvedContext.ActionExecutor = rush;
		request.ResolvedContext.ActionData.ActionDataKey = request.ResolvedContext.ActionDataKey;
		request.ResolvedContext.ActionData.ActionExecutorKey = rush->GetClass();
		if (!TestTrue(TEXT("Rush started"), f.Actions->ApplyActionDecision(request))) return false;
		const FCounterRushPlan plan = rush->GetRushPlan();
		for (int32 i = 0; i < 30; ++i)
		{
			movement->TickComponent(1.f/60.f, LEVELTICK_All, &movement->PrimaryComponentTick);
			rush->Tick(1.f/60.f);
		}
		const float actual = FVector::Dist2D(plan.Origin, f.Player->GetActorLocation());
		TestTrue(TEXT("Movement happened"), actual > 1.f);
		if (blocked)
		{
			TestTrue(TEXT("Wall stops capsule before wall face"), f.Player->GetActorLocation().X + f.Player->GetCapsuleComponent()->GetScaledCapsuleRadius() <= 111.f);
			TestTrue(TEXT("Blocked displacement below planned travel"), actual < plan.TravelDistance - 10.f);
		}
		else TestTrue(TEXT("Open movement reaches capped endpoint without drift"), FMath::IsNearlyEqual(actual, plan.TravelDistance, 2.f));
	}
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCounterOpportunityOwnershipTest, "Portfolio.Combat.Dodge.CounterOpportunityOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCounterOpportunityOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	for (const bool authored : { false, true })
	for (const bool consume : { false, true })
	{
		FFixture f;
		auto& tuning = Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning"));
		tuning.PerfectEnd = .18f;
		tuning.CounterGraceSeconds = 0.f;
		tuning.bUseMontageCounterWindow = authored;
		FReactionData data;
		data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
		data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
		data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
		data.Montage = DuplicateObject<UAnimMontage>(f.Montage, GetTransientPackage());
		Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
		TestTrue(TEXT("Dodge begins"), f.Actions->ApplyActionDecision(f.MakeStart()));
		f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
		auto* reaction = f.References.ReactionComponent->GetActiveReactionExecutor();
		if (!TestNotNull(TEXT("Perfect reaction"), reaction)) return false;
		const uint64 generation = f.References.ReactionComponent->GetActiveReactionGeneration();
		const int32 instance = reaction->GetActiveMontageInstanceId();
		f.Player->GetMesh()->GetAnimInstance()->Montage_SetPosition(data.Montage, .9f);
		if (authored)
		{
			TestNull(TEXT("Numeric threshold cannot open authored opportunity"), f.Defense->GetCounterRushTarget());
			TestFalse(TEXT("Old generation rejected"), f.Defense->SetCounterOpportunityWindow(true, generation + 1, instance));
			TestFalse(TEXT("Old instance rejected"), f.Defense->SetCounterOpportunityWindow(true, generation, instance + 1));
			TestTrue(TEXT("Open current opportunity"), f.Defense->SetCounterOpportunityWindow(true, generation, instance));
			TestTrue(TEXT("Close current opportunity"), f.Defense->SetCounterOpportunityWindow(false, generation, instance));
			TestNull(TEXT("Closed opportunity unavailable"), f.Defense->GetCounterRushTarget());
			auto* marker = NewObject<UCAnimNotify_CounterOpportunity>();
			FAnimNotifyEventReference stale;
			stale.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(instance + 1);
			marker->Notify(f.Player->GetMesh(), data.Montage, stale);
			TestNull(TEXT("Stale point marker cannot reopen"), f.Defense->GetCounterRushTarget());
			FAnimNotifyEventReference current;
			current.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(instance);
			marker->Notify(f.Player->GetMesh(), data.Montage, current);
			TestNotNull(TEXT("Current point marker opens"), f.Defense->GetCounterRushTarget());
		}
		const uint64 serial = f.Defense->GetCounterRushSerial();
		TestTrue(TEXT("Opportunity exists"), serial != 0);
		TestTrue(TEXT("Reserve before interruption"), f.Defense->BeginCounterRushTransition());
		TestFalse(TEXT("Nested reservation rejected"), f.Defense->BeginCounterRushTransition());
		reaction->Stop(EReactionStopReason::Interrupted);
		TestEqual(TEXT("Zero grace preserves authorized handoff"), f.Defense->GetCounterRushSerial(), serial);
		if (consume)
		{
			TestTrue(TEXT("Reserved opportunity consumed"), f.Defense->ConsumeCounterRush(serial));
			TestFalse(TEXT("Cannot consume twice"), f.Defense->ConsumeCounterRush(serial));
		}
		f.Defense->EndCounterRushTransition();
		TestNull(TEXT("Nothing leaks after transaction"), f.Defense->GetCounterRushTarget());
		TestFalse(TEXT("Expired owner cannot reopen"), f.Defense->SetCounterOpportunityWindow(true, generation, instance));
	}
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBodyGlowOwnershipTest, "Portfolio.Combat.Presentation.BodyGlowOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBodyGlowOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace CombatDodgeTest;
	for (int32 scenario = 0; scenario < 6; ++scenario)
	{
		FFixture f;
		if (!TestNotNull(TEXT("Fixture montage"), f.Montage)) return false;
		auto* feedback = f.References.ReactionFeedbackComponent;
		feedback->InitializeReferences(f.References);
		auto* glow = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/CharacterOverlay/Materials/Material_Instance/MI_Fresnel"));
		if (!TestNotNull(TEXT("Body glow material"), glow)) return false;
		feedback->PerfectBodyMaterial = feedback->CounterWeaponMaterial = glow;
		Field<FDefenseDodgeTuning>(f.Defense, TEXT("DodgeTuning")).PerfectEnd = .18f;
		FReactionData data;
		data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
		data.ReactionDataKey.ReactionType = EReactionType::PerfectDodge;
		data.ReactionExecutorKey = UCReaction_PerfectDodge::StaticClass();
		data.Montage = DuplicateObject<UAnimMontage>(f.Montage, GetTransientPackage());
		Field<TMap<FReactionDataKey, FReactionData>>(f.References.ReactionComponent, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
		TestTrue(TEXT("Dodge begins"), f.Actions->ApplyActionDecision(f.MakeStart()));
		f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
		auto* reaction = f.References.ReactionComponent->GetActiveReactionExecutor();
		if (!TestNotNull(TEXT("Perfect reaction owns body cue"), reaction)) return false;
		const uint64 generation = f.References.ReactionComponent->GetActiveReactionGeneration();
		const int32 instance = reaction->GetActiveMontageInstanceId();
		TestFalse(TEXT("Success alone does not start body"), feedback->BodyEnvelopeState.bRunning);
		auto* marker = NewObject<UCAnimNotify_BodyGlow>();
		marker->FadeIn = 2.f;
		marker->FadeOut = .25f;
		marker->PeakWeight = .6f;
		marker->SafetyTimeout = 8.f;
		FAnimNotifyEventReference stale;
		stale.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(instance + 1);
		marker->Notify(f.Player->GetMesh(), data.Montage, stale);
		TestFalse(TEXT("Stale montage Start rejected"), feedback->BodyEnvelopeState.bRunning);
		FAnimNotifyEventReference current;
		current.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(instance);
		marker->Notify(f.Player->GetMesh(), data.Montage, current);
		TestTrue(TEXT("Current Start sustains body"), feedback->BodyEnvelopeState.bRunning && feedback->BodyEnvelopeState.bSustain);
		TestEqual(TEXT("Marker peak applied"), feedback->BodyPeakWeight, .6f);
		const double deadline = feedback->BodySafetyDeadline;
		const double started = feedback->BodyEnvelopeState.StartedAt;
		marker->Notify(f.Player->GetMesh(), data.Montage, current);
		TestEqual(TEXT("Duplicate Start preserves watchdog"), feedback->BodySafetyDeadline, deadline);
		TestEqual(TEXT("Duplicate Start preserves fade"), feedback->BodyEnvelopeState.StartedAt, started);
		feedback->EndBodyGlow(generation + 1, instance, 0.f);
		feedback->EndBodyGlow(generation, instance + 1, 0.f);
		TestFalse(TEXT("Stale End cannot release current body"), feedback->BodyEnvelopeState.bReleased);
		if (scenario == 0)
		{
			feedback->BodyEnvelopeState.StartedAt = FPlatformTime::Seconds() - 1.;
			marker->bStart = false;
			marker->Notify(f.Player->GetMesh(), data.Montage, current);
			TestTrue(TEXT("Point End releases partial fade"), feedback->BodyEnvelopeState.bReleased);
			TestTrue(TEXT("Release begins at current weight, not full peak"), FMath::IsNearlyEqual(feedback->BodyEnvelopeState.ReleasedWeight, .5f, .02f));
			TestEqual(TEXT("End clears safety deadline"), feedback->BodySafetyDeadline, 0.);
			marker->bStart = true;
			marker->Notify(f.Player->GetMesh(), data.Montage, current);
			TestTrue(TEXT("Duplicate Start cannot undo release"), feedback->BodyEnvelopeState.bReleased);
			feedback->BodyEnvelopeState.ReleasedAt = FPlatformTime::Seconds() - 1.;
			feedback->TickComponent(0.f, LEVELTICK_All, nullptr);
			TestNull(TEXT("Fade restores original empty overlay"), f.Player->GetMesh()->GetOverlayMaterial());
			marker->Notify(f.Player->GetMesh(), data.Montage, current);
			TestFalse(TEXT("Same execution cannot restart completed cue"), feedback->BodyEnvelopeState.bRunning);
		}
		else if (scenario == 1)
		{
			feedback->BodyEnvelopeState.Profile.FadeOut = 0.f;
			feedback->BodySafetyDeadline = FPlatformTime::Seconds() - 1.;
			feedback->TickComponent(0.f, LEVELTICK_All, nullptr);
			TestFalse(TEXT("Missing End expires and restores"), feedback->BodyEnvelopeState.bRunning);
			marker->Notify(f.Player->GetMesh(), data.Montage, current);
			TestFalse(TEXT("Timeout cannot be rearmed by duplicate marker"), feedback->BodyEnvelopeState.bRunning);
		}
		else if (scenario == 2)
		{
			f.References.ReactionComponent->CancelActiveReactionForSystem();
			TestTrue(TEXT("Reaction cancellation releases body"), feedback->BodyEnvelopeState.bReleased);
			TestTrue(TEXT("Successor dodge begins"), f.Actions->ApplyActionDecision(f.MakeStart()));
			f.Target->RequestCombatDamageTarget(10.f, f.Damage(), f.Controller, f.Attacker);
			auto* successor = f.References.ReactionComponent->GetActiveReactionExecutor();
			if (!TestNotNull(TEXT("Successor perfect reaction"), successor)) return false;
			const uint64 nextGeneration = f.References.ReactionComponent->GetActiveReactionGeneration();
			const int32 nextInstance = successor->GetActiveMontageInstanceId();
			feedback->StartBodyGlow(nextGeneration, nextInstance, 1.f, .08f, .22f, 8.f);
			TestTrue(TEXT("New execution can start after old owner ended"), feedback->BodyEnvelopeState.bRunning);
			feedback->EndBodyGlow(generation, instance, 0.f);
			feedback->EndBodyGlowOwner(generation);
			TestFalse(TEXT("Old owner End cannot release successor"), feedback->BodyEnvelopeState.bReleased);
			TestEqual(TEXT("Successor retains ownership"), feedback->BodyOwnerGeneration, nextGeneration);
		}
		else if (scenario == 3)
		{
			UClass* weaponClass = LoadClass<ACWeaponActor>(nullptr, TEXT("/Game/06_Weapon/BP_CWeaponActor_Sword.BP_CWeaponActor_Sword_C"));
			if (!TestNotNull(TEXT("Weapon class for transfer"), weaponClass)) return false;
			Field<ACWeaponActor*>(f.References.WeaponComponent, TEXT("WeaponActor")) = f.World->SpawnActor<ACWeaponActor>(weaponClass);
			feedback->StartWeaponGlow(false, generation, 1.f, .08f, .18f, 8.f);
			feedback->BeginWeaponGlowTransfer(f.LastPacket.ResultSerial);
			reaction->Complete();
			TestTrue(TEXT("Counter handoff releases body owner"), feedback->BodyEnvelopeState.bReleased);
			TestFalse(TEXT("Counter handoff preserves weapon independently"), feedback->WeaponEnvelopeState.bReleased);
			feedback->FinishWeaponGlowTransfer();
		}
		else if (scenario == 4)
		{
			f.References.HealthComponent->TryKill();
			feedback->TickComponent(0.f, LEVELTICK_All, nullptr);
			TestFalse(TEXT("Death clears body immediately"), feedback->BodyEnvelopeState.bRunning);
			TestNull(TEXT("Death restores mesh overlay"), f.Player->GetMesh()->GetOverlayMaterial());
		}
		else
		{
			// The isolated fixture does not begin the world; establish the native component lifecycle first.
			if (!feedback->IsRegistered()) feedback->RegisterComponent();
			feedback->RegisterAllComponentTickFunctions(true);
			feedback->BeginPlay();
			feedback->EndPlay(EEndPlayReason::Destroyed);
			TestFalse(TEXT("EndPlay clears body"), feedback->BodyEnvelopeState.bRunning);
			TestEqual(TEXT("EndPlay clears body watchdog"), feedback->BodySafetyDeadline, 0.);
			TestNull(TEXT("EndPlay restores mesh overlay"), f.Player->GetMesh()->GetOverlayMaterial());
		}
	}
	return true;
}
#endif
