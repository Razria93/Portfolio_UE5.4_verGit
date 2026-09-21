#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "Tests/CActionFacingProbe.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/CPlayer.h"
#include "Component/CActionComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CWeaponComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include <limits>

namespace ActionFacingTest
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
		ACPlayer* Player = World->SpawnActor<ACActionFacingCharacterProbe>();
		ACharacter* Target = World->SpawnActor<ACharacter>();
		APlayerController* Controller = World->SpawnActor<APlayerController>();
		UCActionComponent* Action = Player->FindComponentByClass<UCActionComponent>();
		UCMovementComponent* Movement = Player->FindComponentByClass<UCMovementComponent>();
		UCCombatTargetComponent* CombatTarget = Player->FindComponentByClass<UCCombatTargetComponent>();
		UCActionFacingProbe* Probe = NewObject<UCActionFacingProbe>(Action);
		FCharacterComponentReferences References;

		FFixture()
		{
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			References.OwnerCharacter = Player;
			References.CharacterMovementComponent = Player->GetCharacterMovement();
			References.ActionComponent = Action;
			References.MovementComponent = Movement;
			References.CombatTargetComponent = CombatTarget;
			References.WeaponComponent = Player->FindComponentByClass<UCWeaponComponent>();
			References.StateComponent = Player->FindComponentByClass<UCStateComponent>();
			References.HealthComponent = Player->GetHealthComp();
			References.BalanceComponent = Player->FindComponentByClass<UCBalanceComponent>();
			References.ObservableOverlayComponent = Player->FindComponentByClass<UCObservableOverlayComponent>();
			References.CombatSignalSourceComponent = Player->FindComponentByClass<UCCombatSignalSourceComponent>();
			References.ActionOrchestratorComponent = Player->FindComponentByClass<UCActionOrchestratorComponent>();
			References.ReactionComponent = Player->FindComponentByClass<UCReactionComponent>();
			References.ActionFeedbackComponent = Player->FindComponentByClass<UCActionFeedbackComponent>();
			References.HealthComponent->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
			References.StateComponent->InitializeReferences(References);
			Movement->InitializeReferences(References);
			Action->InitializeReferences(References);
			Probe->InitializeReferences(References);
			Controller->Possess(Player);
			Controller->SetControlRotation(FRotator(0.f, -30.f, 0.f));
			Player->SetActorLocation(FVector::ZeroVector);
			Player->SetActorRotation(FRotator::ZeroRotator);
			Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			Target->SetActorLocation(FVector(200.f, 200.f, 0.f));
			CombatTarget->RequestSetCombatTarget(Target, ECombatTargetChangeReason::PlayerSelection);
		}

		~FFixture()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		FActionExecutionResult Result(uint32 Serial, int32 Index = 0)
		{
			FActionExecutionResult result;
			result.Decision = EExecutionDecision::Accept;
			result.ApplyMode = EExecutionApplyMode::Start;
			FActionExecutionContext& context = result.ResolvedContext;
			context.ActionDataKey.ActionType = EActionType::ComboAttack;
			context.ActionDataKey.ActionIndex = Index;
			context.ActionRequestSerial = Serial;
			context.ActionExecutor = Probe;
			context.ActionData.ActionDataKey = context.ActionDataKey;
			context.ActionData.ActionExecutorKey = Probe->GetClass();
			context.ActionData.Montage = NewObject<UAnimMontage>();
			context.ActionData.bFaceCombatTargetOnStart = true;
			return result;
		}

		void Tick()
		{
			static_cast<UActorComponent*>(Action)->TickComponent(1.f / 60.f, LEVELTICK_All, &Action->PrimaryComponentTick);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FActionFacingLifecycleTest, "Portfolio.Combat.ActionFacing.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FActionFacingLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace ActionFacingTest;
	FFixture f;
	TestTrue(TEXT("First attack starts"), f.Action->ApplyActionDecision(f.Result(1)));
	TestEqual(TEXT("No rotation inside playback/notify stack"), f.Player->GetActorRotation().Yaw, 0.0);
	f.Tick();
	TestTrue(TEXT("First attack faces character-to-target"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 45.f));
	TestTrue(TEXT("Camera control rotation unchanged"), FMath::IsNearlyZero(FMath::FindDeltaAngleDegrees(f.Controller->GetControlRotation().Yaw, -30.0)));
	f.Target->SetActorLocation(FVector(0.f, 250.f, 0.f));
	f.Tick();
	TestTrue(TEXT("No continuous tracking"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 45.f));
	f.Probe->OpenReserveChainWindow();
	FActionExecutionResult next = f.Result(2, 1);
	next.ApplyMode = EExecutionApplyMode::Reserve;
	TestTrue(TEXT("Combo reserved"), f.Action->ApplyActionDecision(next));
	f.Tick();
	TestTrue(TEXT("Reservation does not rotate"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 45.f));
	f.Probe->ConsumeChain();
	TestTrue(TEXT("Notify does not rotate previous frame motion"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 45.f));
	f.Action->HandleApplyActionStarted(f.Probe, 1);
	f.Tick();
	TestTrue(TEXT("Followup rotates; stale start cannot replace it"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 90.f));
	f.Probe->Complete();
	f.Player->SetActorRotation(FRotator::ZeroRotator);
	TestTrue(TEXT("New attack starts"), f.Action->ApplyActionDecision(f.Result(3)));
	f.Probe->Complete();
	f.Tick();
	TestEqual(TEXT("Completion discards pending rotation"), f.Player->GetActorRotation().Yaw, 0.0);
	TestTrue(TEXT("Cancellation setup"), f.Action->ApplyActionDecision(f.Result(4)));
	f.Action->CancelActiveActionForSystem();
	f.Tick();
	TestEqual(TEXT("Cancellation discards pending rotation"), f.Player->GetActorRotation().Yaw, 0.0);
	TestTrue(TEXT("Target removal setup"), f.Action->ApplyActionDecision(f.Result(5)));
	f.CombatTarget->RequestClearCombatTarget(ECombatTargetChangeReason::ManualClear);
	f.Tick();
	TestEqual(TEXT("Target removal discards pending rotation"), f.Player->GetActorRotation().Yaw, 0.0);
	f.Probe->Complete();
	f.CombatTarget->RequestSetCombatTarget(f.Target, ECombatTargetChangeReason::PlayerSelection);
	TestTrue(TEXT("Failed chain setup"), f.Action->ApplyActionDecision(f.Result(6)));
	f.Probe->OpenReserveChainWindow();
	TestTrue(TEXT("Failed chain reserved"), f.Probe->ReserveChain(f.Result(7, 1).ResolvedContext.ActionData, 7));
	f.Probe->bFailPlayback = true;
	f.Probe->ConsumeChain();
	f.Tick();
	TestEqual(TEXT("Failed followup discards previous pending rotation"), f.Player->GetActorRotation().Yaw, 0.0);
	f.Probe->bFailPlayback = false;
	TestTrue(TEXT("Reinitialization setup"), f.Action->ApplyActionDecision(f.Result(8)));
	f.Action->InitializeReferences(f.References);
	f.Tick();
	TestEqual(TEXT("Reinitialization discards pending rotation"), f.Player->GetActorRotation().Yaw, 0.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FActionFacingGatesTest, "Portfolio.Combat.ActionFacing.Gates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FActionFacingGatesTest::RunTest(const FString& Parameters)
{
	using namespace ActionFacingTest;
	FFixture f;
	for (int32 mode = 0; mode < 7; ++mode)
	{
		FActionExecutionResult result = f.Result(10 + mode);
		f.Probe->bFailPlayback = mode == 0;
		f.Probe->bFailBinding = mode == 1;
		f.Probe->bCompleteDuringPlayback = mode == 2;
		if (mode == 3) result.ResolvedContext.ActionData.bFaceCombatTargetOnStart = false;
		if (mode == 4) result.ResolvedContext.ActionData.StartFacingMaxAngle = 20.f;
		if (mode == 5) result.ResolvedContext.ActionData.StartFacingMaxDistance = 100.f;
		if (mode == 6) f.CombatTarget->RequestClearCombatTarget(ECombatTargetChangeReason::ManualClear);
		f.Action->ApplyActionDecision(result);
		f.Tick();
		TestEqual(FString::Printf(TEXT("Excluded case %d does not rotate"), mode), f.Player->GetActorRotation().Yaw, 0.0);
		f.Action->CancelActiveActionForSystem();
	}
	TestFalse(TEXT("Default opt-in disabled"), FActionData().bFaceCombatTargetOnStart);
	TestFalse(TEXT("Invalid angle"), f.Movement->TryFaceTarget(FVector(200.f, 200.f, 0.f), 450.f, std::numeric_limits<float>::infinity()));
	TestFalse(TEXT("Coincident target"), f.Movement->TryFaceTarget(f.Player->GetActorLocation(), 450.f, 90.f));
	f.Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	TestFalse(TEXT("Airborne rotation excluded"), f.Movement->TryFaceTarget(FVector(200.f, 200.f, 0.f), 450.f, 90.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FActionFacingPlayerInputTest, "Portfolio.Combat.ActionFacing.PlayerInputZeroSerial",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FActionFacingPlayerInputTest::RunTest(const FString& Parameters)
{
	using namespace ActionFacingTest;
	FFixture f;
	f.References.ActionOrchestratorComponent->InitializeReferences(f.References);
	f.References.ObservableOverlayComponent->InitializeReferences(f.References);
	Field<EWeaponType>(f.References.WeaponComponent, TEXT("CurrentWeaponType")) = EWeaponType::Sword;
	for (int32 index = 0; index < 4; ++index)
	{
		const FActionData data = f.Result(0, index).ResolvedContext.ActionData;
		Field<TMap<FActionDataKey, FActionData>>(f.Action, TEXT("ActionDataMap")).Add(data.ActionDataKey, data);
	}
	Field<TMap<UClass*, UCAction*>>(f.Action, TEXT("ActionExecutorMap")).Add(f.Probe->GetClass(), f.Probe);
	uint64 previousGeneration = 0;
	for (int32 index = 0; index < 4; ++index)
	{
		const double expectedYaw = 20.0 * (index + 1);
		f.Target->SetActorLocation(FRotator(0.0, expectedYaw, 0.0).Vector() * 250.0);
		if (index > 0) f.Probe->OpenReserveChainWindow();
		f.Player->HandleCombatAction(ECombatActionIntent::ComboAttack, EActionIntentEvent::Started);
		TestEqual(TEXT("Real player request retains zero external serial"), f.Action->GetActiveActionRequestSerial(), uint32(0));
		if (index > 0)
		{
			TestEqual(TEXT("Reservation does not advance generation"), f.Action->GetActiveActionGeneration(), previousGeneration);
			f.Tick();
			TestTrue(TEXT("Reservation does not rotate"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, expectedYaw - 20.0));
			f.Probe->ConsumeChain();
		}
		TestEqual(TEXT("Requested combo step actually starts"), f.Action->GetActiveActionIndex(), index);
		const uint64 currentGeneration = f.Action->GetActiveActionGeneration();
		TestTrue(TEXT("Each started step has a new nonzero generation"), currentGeneration > previousGeneration);
		f.Action->HandleApplyActionStarted(f.Probe, previousGeneration);
		f.Tick();
		TestTrue(TEXT("Zero-serial step faces target despite stale hook"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, expectedYaw));
		previousGeneration = currentGeneration;
	}
	f.Probe->Complete();
	TestEqual(TEXT("Completion invalidates active generation"), f.Action->GetActiveActionGeneration(), uint64(0));
	f.Player->SetActorRotation(FRotator::ZeroRotator);
	f.Player->HandleCombatAction(ECombatActionIntent::ComboAttack, EActionIntentEvent::Started);
	TestTrue(TEXT("New chain does not reuse ended generation"), f.Action->GetActiveActionGeneration() > previousGeneration);
	f.Action->HandleApplyActionStarted(f.Probe, previousGeneration);
	f.Tick();
	TestTrue(TEXT("Old chain hook cannot clear new facing"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 80.0));
	f.Probe->Complete();
	f.Player->SetActorRotation(FRotator::ZeroRotator);
	f.Probe->bCompleteDuringPlayback = true;
	f.Player->HandleCombatAction(ECombatActionIntent::ComboAttack, EActionIntentEvent::Started);
	f.Tick();
	TestEqual(TEXT("Zero-serial synchronous completion does not rotate"), f.Player->GetActorRotation().Yaw, 0.0);
	f.Probe->bCompleteDuringPlayback = false;
	f.Probe->bFailPlayback = true;
	f.Probe->DuringPlayback = [&f]()
	{
		f.Probe->Complete();
		f.Probe->bFailPlayback = false;
		f.Player->HandleCombatAction(ECombatActionIntent::ComboAttack, EActionIntentEvent::Started);
	};
	f.Player->HandleCombatAction(ECombatActionIntent::ComboAttack, EActionIntentEvent::Started);
	TestTrue(TEXT("Old failed start cannot clear replacement using same executor and serial"), f.Action->IsActive() && f.Probe->IsActive());
	f.Tick();
	TestTrue(TEXT("Replacement facing survives old failed playback"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 80.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FActionFacingRootMotionTest, "Portfolio.Combat.ActionFacing.RootMotion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FActionFacingRootMotionTest::RunTest(const FString& Parameters)
{
	using namespace ActionFacingTest;
	FFixture f;
	f.World->InitializeActorsForPlay(FURL());
	UClass* stellarClass = LoadClass<ACPlayer>(nullptr, TEXT("/Game/01_Character/01_Player/BP_CPlayer_Stellar.BP_CPlayer_Stellar_C"));
	UAnimMontage* source = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/04_Montage/Combat/Sword/Stellar/Combo/Attack_Combo_01_01_Anim_Stellar_Montage.Attack_Combo_01_01_Anim_Stellar_Montage"));
	if (!TestNotNull(TEXT("Stellar class"), stellarClass) || !TestNotNull(TEXT("Authored attack montage"), source)) return false;
	USkeletalMeshComponent* mesh = f.Player->GetMesh();
	mesh->SetSkeletalMesh(stellarClass->GetDefaultObject<ACPlayer>()->GetMesh()->GetSkeletalMeshAsset());
	mesh->SetAnimInstanceClass(stellarClass->GetDefaultObject<ACPlayer>()->GetMesh()->GetAnimClass());
	mesh->InitAnim(true);
	mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	mesh->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
	UAnimMontage* montage = DuplicateObject<UAnimMontage>(source, GetTransientPackage());
	montage->Notifies.Reset();
	montage->RefreshCacheData();
	AActor* floorActor = f.World->SpawnActor<AActor>();
	UBoxComponent* floor = NewObject<UBoxComponent>(floorActor);
	floorActor->SetRootComponent(floor);
	floor->SetBoxExtent(FVector(2000.f, 2000.f, 10.f));
	floor->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	floor->SetCollisionResponseToAllChannels(ECR_Block);
	floor->RegisterComponent();
	floorActor->SetActorLocation(FVector(0.f, 0.f, -10.f));
	f.Player->SetActorLocation(FVector(0.f, 0.f, 96.f));
	f.Target->SetActorLocation(FVector(200.f, 200.f, 96.f));
	f.Movement->SetMovementRotationMode(EMovementRotationMode::ControllerDesired);
	UCharacterMovementComponent* characterMovement = f.Player->GetCharacterMovement();
	characterMovement->Activate(true);
	characterMovement->SetMovementMode(MOVE_Walking);
	UCAction_ComboAttack* executor = NewObject<UCAction_ComboAttack>(f.Action);
	executor->InitializeReferences(f.References);
	FActionExecutionResult result = f.Result(100);
	result.ResolvedContext.ActionExecutor = executor;
	result.ResolvedContext.ActionData.ActionExecutorKey = executor->GetClass();
	result.ResolvedContext.ActionData.Montage = montage;
	TestTrue(TEXT("Real montage starts"), f.Action->ApplyActionDecision(result));
	TestTrue(TEXT("Animation root motion active"), f.Player->IsPlayingRootMotion());
	f.Tick();
	TestTrue(TEXT("Facing applied before movement"), FMath::IsNearlyEqual(f.Player->GetActorRotation().Yaw, 45.0));
	const FVector before = f.Player->GetActorLocation();
	for (int32 frame = 0; frame < 10; ++frame)
	{
		characterMovement->TickComponent(1.f / 60.f, LEVELTICK_All, &characterMovement->PrimaryComponentTick);
		AddInfo(FString::Printf(TEXT("Frame=%d Position=%.3f PlayingRoot=%d MovementRoot=%d Yaw=%.2f"), frame,
			mesh->GetAnimInstance()->Montage_GetPosition(montage), f.Player->IsPlayingRootMotion(),
			characterMovement->HasAnimRootMotion(), f.Player->GetActorRotation().Yaw));
	}
	TestTrue(TEXT("Actual root motion moves character"), FVector::Dist2D(before, f.Player->GetActorLocation()) > 0.1f);
	TestTrue(TEXT("Root motion does not return to camera yaw"), FMath::Abs(FMath::FindDeltaAngleDegrees(f.Player->GetActorRotation().Yaw, -30.0)) > 30.0);
	TestTrue(TEXT("Camera yaw unchanged"), FMath::IsNearlyZero(FMath::FindDeltaAngleDegrees(f.Controller->GetControlRotation().Yaw, -30.0)));
	AddInfo(FString::Printf(TEXT("Root motion displacement=%.2f, yaw=%.2f"), FVector::Dist2D(before, f.Player->GetActorLocation()), f.Player->GetActorRotation().Yaw));
	return true;
}
#endif
