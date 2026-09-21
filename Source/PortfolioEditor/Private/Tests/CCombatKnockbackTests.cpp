#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/CCombatKnockbackProbe.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/CPlayer.h"
#include "Component/CMovementComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CDefenseComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include <limits>

namespace CombatKnockbackTest
{
	template<typename T> T& Field(UObject* Object, const TCHAR* Name)
	{
		FProperty* property = FindFProperty<FProperty>(Object->GetClass(), Name);
		check(property);
		return *property->ContainerPtrToValuePtr<T>(Object);
	}

	struct FFixture
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		ACPlayer* Player = World->SpawnActor<ACPlayer>();
		UCMovementComponent* Movement = Player->FindComponentByClass<UCMovementComponent>();
		UCReactionComponent* Reaction = Player->FindComponentByClass<UCReactionComponent>();
		UCharacterMovementComponent* CharacterMovement = Player->GetCharacterMovement();
		FCharacterComponentReferences References;

		FFixture()
		{
			References.OwnerCharacter = Player;
			References.CharacterMovementComponent = CharacterMovement;
			References.MovementComponent = Movement;
			References.HealthComponent = Player->GetHealthComp();
			References.StateComponent = Player->FindComponentByClass<UCStateComponent>();
			References.BalanceComponent = Player->FindComponentByClass<UCBalanceComponent>();
			References.ActionComponent = Player->FindComponentByClass<UCActionComponent>();
			References.ReactionComponent = Reaction;
			References.ReactionFeedbackComponent = Player->FindComponentByClass<UCReactionFeedbackComponent>();
			References.ObservableOverlayComponent = Player->FindComponentByClass<UCObservableOverlayComponent>();
			Player->GetHealthComp()->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
			References.StateComponent->InitializeReferences(References);
			Movement->InitializeReferences(References);
			Reaction->InitializeReferences(References);
			if (!Movement->IsRegistered()) Movement->RegisterComponent();
			if (!CharacterMovement->IsRegistered()) CharacterMovement->RegisterComponent();
			CharacterMovement->Activate(true);
			CharacterMovement->bRunPhysicsWithNoController = true;
			CharacterMovement->SetMovementMode(MOVE_Walking);
		}

		~FFixture() { World->DestroyWorld(false); }

		void TickMovement(float Delta = 1.f / 60.f)
		{
			static_cast<UActorComponent*>(Movement)->TickComponent(Delta, LEVELTICK_All, &Movement->PrimaryComponentTick);
			CharacterMovement->TickComponent(Delta, LEVELTICK_All, &CharacterMovement->PrimaryComponentTick);
		}

		UBoxComponent* AddBox(FVector Position, FVector Extent)
		{
			AActor* actor = World->SpawnActor<AActor>();
			UBoxComponent* box = NewObject<UBoxComponent>(actor);
			actor->SetRootComponent(box);
			box->SetBoxExtent(Extent);
			box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			box->SetCollisionObjectType(ECC_WorldStatic);
			box->SetCollisionResponseToAllChannels(ECR_Block);
			box->RegisterComponent();
			actor->SetActorLocation(Position);
			return box;
		}

		FReactionExecutionResult MakeResult(UCCombatKnockbackProbe* Probe, EReactionType Type = EReactionType::Hit)
		{
			FReactionExecutionResult result;
			result.Decision = EExecutionDecision::Accept;
			result.ApplyMode = EExecutionApplyMode::Start;
			FReactionExecutionContext& context = result.ResolvedContext;
			context.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
			context.ReactionDataKey.ReactionType = Type;
			context.ReactionDataKey.ReactionIndex = INDEX_NONE;
			context.ReactionData.ReactionDataKey = context.ReactionDataKey;
			context.ReactionData.ReactionExecutorKey = Probe->GetClass();
			context.ReactionData.Montage = NewObject<UAnimMontage>();
			context.ReactionExecutor = Probe;
			context.Knockback.Spec.Speed = 300.f;
			context.Knockback.Spec.Duration = 0.2f;
			context.Knockback.Direction = FVector::ForwardVector;
			return result;
		}
	};

	FCombatKnockbackContext Context()
	{
		FCombatKnockbackContext context;
		context.Spec.Speed = 300.f;
		context.Spec.Duration = 0.2f;
		context.Direction = FVector::ForwardVector;
		return context;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatKnockbackOwnershipTest, "Portfolio.Combat.Knockback.Ownership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatKnockbackOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace CombatKnockbackTest;
	FFixture fixture;
	FCombatKnockbackContext context = Context();
	TestFalse(TEXT("Default disabled"), fixture.Movement->StartKnockback(FCombatKnockbackContext(), 1));
	TestFalse(TEXT("Zero owner rejected"), fixture.Movement->StartKnockback(context, 0));
	context.Spec.Speed = std::numeric_limits<float>::infinity();
	TestFalse(TEXT("Infinite speed rejected"), fixture.Movement->StartKnockback(context, 1));
	context = Context();
	context.Direction.Z = 1.f;
	TestFalse(TEXT("Vertical direction rejected"), fixture.Movement->StartKnockback(context, 1));
	context = Context();
	TestTrue(TEXT("First owner starts"), fixture.Movement->StartKnockback(context, 10));
	TestTrue(TEXT("Replacement owner starts"), fixture.Movement->StartKnockback(context, 11));
	fixture.Movement->StopKnockback(10);
	TestTrue(TEXT("Stale stop preserves replacement"), fixture.Movement->IsKnockbackActive());
	TSharedPtr<FRootMotionSource_ConstantForce> unrelated = MakeShared<FRootMotionSource_ConstantForce>();
	unrelated->InstanceName = TEXT("UnrelatedTestMotion");
	unrelated->Duration = 2.f;
	unrelated->Force = FVector(0.f, 20.f, 0.f);
	const uint16 unrelatedID = fixture.CharacterMovement->ApplyRootMotionSource(unrelated);
	fixture.Movement->StopKnockback(11);
	TestFalse(TEXT("Owner stop clears knockback"), fixture.Movement->IsKnockbackActive());
	TestTrue(TEXT("Unrelated source retained"), fixture.CharacterMovement->GetRootMotionSourceByID(unrelatedID).IsValid());
	TestFalse(TEXT("Unrelated source not marked for removal"), unrelated->Status.HasFlag(ERootMotionSourceStatusFlags::MarkedForRemoval));
	TestTrue(TEXT("Reinitialization setup"), fixture.Movement->StartKnockback(context, 12));
	fixture.Movement->InitializeReferences(fixture.References);
	TestFalse(TEXT("Reinitialization clears motion"), fixture.Movement->IsKnockbackActive());
	fixture.CharacterMovement->SetMovementMode(MOVE_Falling);
	TestFalse(TEXT("Airborne start rejected"), fixture.Movement->StartKnockback(context, 12));
	fixture.CharacterMovement->SetMovementMode(MOVE_Walking);
	fixture.Player->GetHealthComp()->TryKill();
	TestFalse(TEXT("Dead start rejected"), fixture.Movement->StartKnockback(context, 13));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatKnockbackReactionTest, "Portfolio.Combat.Knockback.ReactionLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatKnockbackReactionTest::RunTest(const FString& Parameters)
{
	using namespace CombatKnockbackTest;
	FFixture fixture;
	UCCombatKnockbackProbe* probe = NewObject<UCCombatKnockbackProbe>(fixture.Reaction);
	probe->InitializeReferences(fixture.References);
	FReactionExecutionResult result = fixture.MakeResult(probe);
	fixture.Reaction->OnReactionTypeChanged.AddDynamic(probe, &UCCombatKnockbackProbe::CancelOnTypeChanged);
	TestFalse(TEXT("Type-change callback cancels start"), fixture.Reaction->ApplyReactionDecision(result));
	TestEqual(TEXT("Cancelled context does not invoke executor"), probe->StartCalls, 0);
	TestFalse(TEXT("Type-change cancellation has no movement"), fixture.Movement->IsKnockbackActive());
	fixture.Reaction->OnReactionTypeChanged.RemoveDynamic(probe, &UCCombatKnockbackProbe::CancelOnTypeChanged);
	probe->bFailStart = true;
	TestFalse(TEXT("Failed executor start"), fixture.Reaction->ApplyReactionDecision(result));
	TestFalse(TEXT("Failed start has no knockback"), fixture.Movement->IsKnockbackActive());
	probe->bFailStart = false;
	probe->bCompleteDuringStart = true;
	fixture.Reaction->ApplyReactionDecision(result);
	TestFalse(TEXT("Synchronous completion has no knockback"), fixture.Movement->IsKnockbackActive());
	probe->bCompleteDuringStart = false;
	TestTrue(TEXT("Hit starts"), fixture.Reaction->ApplyReactionDecision(result));
	TestTrue(TEXT("Hit starts knockback"), fixture.Movement->IsKnockbackActive());
	probe->Complete();
	TestFalse(TEXT("Completion stops knockback"), fixture.Movement->IsKnockbackActive());
	TestTrue(TEXT("Same executor can start next Hit"), fixture.Reaction->ApplyReactionDecision(result));
	TestTrue(TEXT("Cancellation succeeds"), fixture.Reaction->CancelActiveReactionForSystem());
	TestFalse(TEXT("Cancellation stops knockback"), fixture.Movement->IsKnockbackActive());
	for (EReactionType type : {EReactionType::BlockHit, EReactionType::Parry, EReactionType::CollapseHit,
		EReactionType::ExecutionStandard, EReactionType::ExecutionLethal, EReactionType::Dead})
	{
		result = fixture.MakeResult(probe, type);
		TestTrue(TEXT("Excluded reaction still starts"), fixture.Reaction->ApplyReactionDecision(result));
		TestFalse(TEXT("Excluded reaction has no knockback"), fixture.Movement->IsKnockbackActive());
		probe->Complete();
	}
	if (!fixture.Reaction->IsRegistered()) fixture.Reaction->RegisterComponent();
	fixture.Reaction->RegisterAllComponentTickFunctions(true);
	static_cast<UActorComponent*>(fixture.Reaction)->BeginPlay();
	result = fixture.MakeResult(probe);
	TestTrue(TEXT("EndPlay cleanup setup starts Hit"), fixture.Reaction->ApplyReactionDecision(result));
	TestTrue(TEXT("EndPlay cleanup setup has motion"), fixture.Movement->IsKnockbackActive());
	static_cast<UActorComponent*>(fixture.Reaction)->EndPlay(EEndPlayReason::Destroyed);
	TestFalse(TEXT("Reaction EndPlay clears movement"), fixture.Movement->IsKnockbackActive());
	TestFalse(TEXT("Reaction EndPlay resets active runtime"), fixture.Reaction->IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatKnockbackMovementTest, "Portfolio.Combat.Knockback.Movement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatKnockbackMovementTest::RunTest(const FString& Parameters)
{
	using namespace CombatKnockbackTest;
	FFixture fixture;
	fixture.AddBox(FVector(0.f, 0.f, -20.f), FVector(1000.f, 1000.f, 20.f));
	fixture.Player->SetActorLocation(FVector(0.f, 0.f, 100.f));
	for (int32 i = 0; i < 12; ++i) fixture.TickMovement();
	TestTrue(TEXT("Fixture settled on ground"), fixture.CharacterMovement->IsMovingOnGround());
	const FVector start = fixture.Player->GetActorLocation();
	TestTrue(TEXT("Grounded motion starts"), fixture.Movement->StartKnockback(Context(), 20));
	for (int32 i = 0; i < 30; ++i) fixture.TickMovement();
	TestTrue(TEXT("Character displaced horizontally"), fixture.Player->GetActorLocation().X > start.X + 20.f);
	TestTrue(TEXT("No unexpected lateral displacement"), FMath::IsNearlyEqual(fixture.Player->GetActorLocation().Y, start.Y, 0.1f));
	TestFalse(TEXT("Duration expires"), fixture.Movement->IsKnockbackActive());
	const FVector end = fixture.Player->GetActorLocation();
	for (int32 i = 0; i < 10; ++i) fixture.TickMovement();
	TestTrue(TEXT("No persistent residual movement"), FVector::Dist2D(end, fixture.Player->GetActorLocation()) < 0.1f);
	fixture.AddBox(FVector(end.X + 80.f, 0.f, 100.f), FVector(10.f, 200.f, 100.f));
	FCombatKnockbackContext context = Context();
	context.Spec.Duration = 1.f;
	TestTrue(TEXT("Wall-directed motion starts"), fixture.Movement->StartKnockback(context, 21));
	for (int32 i = 0; i < 75; ++i) fixture.TickMovement();
	TestTrue(TEXT("Wall prevents penetration"), fixture.Player->GetActorLocation().X < end.X + 70.f);
	TestTrue(TEXT("Death cleanup setup"), fixture.Movement->StartKnockback(context, 22));
	fixture.Player->GetHealthComp()->TryKill();
	fixture.TickMovement();
	TestFalse(TEXT("Death stops existing motion"), fixture.Movement->IsKnockbackActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatKnockbackPipelineTest, "Portfolio.Combat.Knockback.DamagePipeline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatKnockbackPipelineTest::RunTest(const FString& Parameters)
{
	using namespace CombatKnockbackTest;
	FFixture fixture;
	UCReactionOrchestratorComponent* orchestrator = fixture.Player->FindComponentByClass<UCReactionOrchestratorComponent>();
	UCCombatSignalTargetComponent* target = fixture.Player->FindComponentByClass<UCCombatSignalTargetComponent>();
	fixture.References.ReactionOrchestratorComponent = orchestrator;
	fixture.References.HitFeedbackComponent = fixture.Player->FindComponentByClass<UCHitFeedbackComponent>();
	fixture.References.DefenseComponent = fixture.Player->FindComponentByClass<UCDefenseComponent>();
	orchestrator->InitializeReferences(fixture.References);
	fixture.References.ObservableOverlayComponent->InitializeReferences(fixture.References);
	target->InitializeReferences(fixture.References);
	UCCombatKnockbackProbe* probe = NewObject<UCCombatKnockbackProbe>(fixture.Reaction);
	probe->InitializeReferences(fixture.References);
	FReactionExecutionResult prepared = fixture.MakeResult(probe);
	FReactionData data = prepared.ResolvedContext.ReactionData;
	Field<TMap<FReactionDataKey, FReactionData>>(fixture.Reaction, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	Field<TMap<UClass*, UCReaction*>>(fixture.Reaction, TEXT("ReactionExecutorMap")).Add(probe->GetClass(), probe);
	EReactionRequestRejectReason reason;
	const FVector initialPosition = fixture.Player->GetActorLocation();
	TestTrue(TEXT("Prepared Hit query is available"), orchestrator->QueryPreparedReactionAvailability(data.ReactionDataKey, false, reason));
	TestFalse(TEXT("Availability does not start reaction"), fixture.Reaction->IsActive());
	TestFalse(TEXT("Availability does not start movement"), fixture.Movement->IsKnockbackActive());
	TestTrue(TEXT("Availability preserves position"), fixture.Player->GetActorLocation().Equals(initialPosition));

	AActor* source = fixture.World->SpawnActor<ACharacter>();
	source->SetActorLocation(initialPosition - FVector(200.f, 0.f, 0.f));
	APlayerController* controller = fixture.World->SpawnActor<APlayerController>();
	FDefaultDamageEvent damage;
	damage.SourceActor = source;
	damage.TargetActor = fixture.Player;
	damage.DamageSpecKey.WeaponType = EWeaponType::Sword;
	damage.DamageSpecKey.ActionType = EActionType::ComboAttack;
	damage.DamageSpecKey.ActionIndex = 0;
	damage.DamageSpec.BaseDamage = 1.f;
	damage.DamageRequestAmount.RequestDamage = 1.f;
	damage.DamageSpec.Knockback = Context().Spec;
	data.ReactionDataKey.MatchMode = EReactionDataMatchMode::DamageSpec;
	data.ReactionDataKey.DamageSpecKey = damage.DamageSpecKey;
	Field<TMap<FReactionDataKey, FReactionData>>(fixture.Reaction, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	FCombatSignalTargetPacket accepted;
	target->OnCombatSignalTargetAccepted.AddLambda([&](const FCombatSignalTargetPacket& Packet) { accepted = Packet; });
	TestEqual(TEXT("Hit commits damage"), target->RequestCombatDamageTarget(1.f, damage, controller, source), 1.f);
	TestTrue(TEXT("Target resolved valid knockback"), accepted.Context.Knockback.IsValid());
	TestTrue(TEXT("Direction points away from source"), accepted.Context.Knockback.Direction.Equals(FVector::ForwardVector));
	FReactionExecutionContext active;
	TestTrue(TEXT("Damage starts reaction"), fixture.Reaction->GetActiveReactionContext(active));
	TestEqual(TEXT("Speed propagated to execution"), active.Knockback.Spec.Speed, damage.DamageSpec.Knockback.Speed);
	TestEqual(TEXT("Duration propagated to execution"), active.Knockback.Spec.Duration, damage.DamageSpec.Knockback.Duration);
	TestTrue(TEXT("Pipeline starts movement"), fixture.Movement->IsKnockbackActive());
	probe->Complete();
	fixture.CharacterMovement->SetMovementMode(MOVE_Falling);
	target->RequestCombatDamageTarget(1.f, damage, controller, source);
	TestFalse(TEXT("Airborne damage has no knockback context"), accepted.Context.Knockback.IsValid());
	TestFalse(TEXT("Airborne damage has no movement"), fixture.Movement->IsKnockbackActive());
	probe->Complete();
	target->OnCombatSignalTargetAccepted.Clear();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatKnockbackLaunchTest, "Portfolio.Combat.Knockback.ExternalLaunch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatKnockbackLaunchTest::RunTest(const FString& Parameters)
{
	using namespace CombatKnockbackTest;
	FFixture fixture;
	fixture.AddBox(FVector(0.f, 0.f, -20.f), FVector(1000.f, 1000.f, 20.f));
	fixture.Player->SetActorLocation(FVector(0.f, 0.f, 100.f));
	for (int32 i = 0; i < 12; ++i) fixture.TickMovement();
	FCombatKnockbackContext context = Context();
	context.Spec.Duration = 1.f;
	TestTrue(TEXT("Launch test starts grounded knockback"), fixture.Movement->StartKnockback(context, 30));
	fixture.TickMovement();
	TestTrue(TEXT("Movement component is active for Launch"), fixture.CharacterMovement->IsActive());
	fixture.Player->LaunchCharacter(FVector(200.f, 0.f, 300.f), true, true);
	TestTrue(TEXT("Launch queues velocity"), !fixture.CharacterMovement->PendingLaunchVelocity.IsZero());
	fixture.CharacterMovement->TickComponent(1.f / 60.f, LEVELTICK_All, &fixture.CharacterMovement->PrimaryComponentTick);
	TestTrue(TEXT("External launch enters Falling"), fixture.CharacterMovement->IsFalling());
	const FVector launchedVelocity = fixture.CharacterMovement->Velocity;
	TestTrue(TEXT("Launch has horizontal velocity before cleanup"), launchedVelocity.SizeSquared2D() > 0.f);
	static_cast<UActorComponent*>(fixture.Movement)->TickComponent(1.f / 60.f, LEVELTICK_All, &fixture.Movement->PrimaryComponentTick);
	TestFalse(TEXT("Airborne transition clears owned knockback"), fixture.Movement->IsKnockbackActive());
	TestTrue(TEXT("Cleanup preserves external airborne velocity"), fixture.CharacterMovement->Velocity.Equals(launchedVelocity));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatKnockbackFrameRateTest, "Portfolio.Combat.Knockback.FrameRate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatKnockbackFrameRateTest::RunTest(const FString& Parameters)
{
	using namespace CombatKnockbackTest;
	TArray<double> distances;
	for (int32 rate : {30, 60, 144})
	{
		FFixture fixture;
		fixture.AddBox(FVector(0.f, 0.f, -20.f), FVector(1000.f, 1000.f, 20.f));
		fixture.Player->SetActorLocation(FVector(0.f, 0.f, 100.f));
		for (int32 i = 0; i < 12; ++i) fixture.TickMovement();
		const double startX = fixture.Player->GetActorLocation().X;
		FCombatKnockbackContext context = Context();
		context.Spec.Duration = 0.5f;
		TestTrue(TEXT("Frame-rate motion starts"), fixture.Movement->StartKnockback(context, 40));
		for (int32 i = 0; i < rate; ++i) fixture.TickMovement(1.f / rate);
		distances.Add(fixture.Player->GetActorLocation().X - startX);
		TestFalse(TEXT("Frame-rate motion expires"), fixture.Movement->IsKnockbackActive());
		TestTrue(TEXT("Distance matches speed times duration within one 30Hz frame"), FMath::Abs(distances.Last() - 150.0) <= 10.1);
	}
	TestTrue(TEXT("30Hz and 144Hz displacement stay within one 30Hz frame"), FMath::Abs(distances[0] - distances[2]) <= 10.1);
	return true;
}
#endif
