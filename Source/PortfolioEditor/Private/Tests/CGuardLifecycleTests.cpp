#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/CPlayer.h"
#include "Action/CAction_Guard.h"
#include "Action/CAction_ComboAttack.h"
#include "Component/CActionComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CObservableOverlayComponent.h"

namespace GuardLifecycleTest
{
	// Native fixtures exercise request routing and health delegates without a map.
	template<typename T> T& Field(UObject* Object, const TCHAR* Name)
	{
		FProperty* property = FindFProperty<FProperty>(Object->GetClass(), Name);
		check(property);
		return *property->ContainerPtrToValuePtr<T>(Object);
	}
	template<typename T> T* Comp(AActor* Actor) { return Actor->FindComponentByClass<T>(); }
	FCharacterComponentReferences References(ACPlayer* Player)
	{
		FCharacterComponentReferences r;
		r.OwnerCharacter = Player;
		r.CharacterMovementComponent = Player->GetCharacterMovement();
		r.MovementComponent = Comp<UCMovementComponent>(Player);
		r.DefenseComponent = Comp<UCDefenseComponent>(Player);
		r.HitFeedbackComponent = Comp<UCHitFeedbackComponent>(Player);
		r.ExecutionCollaborationComponent = Comp<UCExecutionCollaborationComponent>(Player);
		r.HealthComponent = Comp<UCHealthComponent>(Player);
		r.StateComponent = Comp<UCStateComponent>(Player);
		r.WeaponComponent = Comp<UCWeaponComponent>(Player);
		r.ActionComponent = Comp<UCActionComponent>(Player);
		r.ActionOrchestratorComponent = Comp<UCActionOrchestratorComponent>(Player);
		r.ActionFeedbackComponent = Comp<UCActionFeedbackComponent>(Player);
		r.ReactionComponent = Comp<UCReactionComponent>(Player);
		r.ReactionOrchestratorComponent = Comp<UCReactionOrchestratorComponent>(Player);
		r.ReactionFeedbackComponent = Comp<UCReactionFeedbackComponent>(Player);
		r.CombatTargetComponent = Comp<UCCombatTargetComponent>(Player);
		r.CombatSignalTargetComponent = Comp<UCCombatSignalTargetComponent>(Player);
		r.CombatSignalSourceComponent = Comp<UCCombatSignalSourceComponent>(Player);
		r.BalanceComponent = Comp<UCBalanceComponent>(Player);
		r.ObservableOverlayComponent = Comp<UCObservableOverlayComponent>(Player);
		return r;
	}
	FActionExecutionContext Prepare(ACPlayer* Player, EActionType Type, int32 Index, UClass* Class)
	{
		UCActionComponent* component = Comp<UCActionComponent>(Player);
		UCAction* executor = NewObject<UCAction>(component, Class);
		executor->InitializeReferences(References(Player));
		FActionData data;
		data.ActionDataKey.ActionType = Type;
		data.ActionDataKey.ActionIndex = Index;
		data.ActionExecutorKey = Class;
		data.Montage = NewObject<UAnimMontage>();
		Field<TMap<FActionDataKey, FActionData>>(component, TEXT("ActionDataMap")).Add(data.ActionDataKey, data);
		Field<TMap<UClass*, UCAction*>>(component, TEXT("ActionExecutorMap")).Add(Class, executor);
		FActionExecutionContext context;
		component->FindPreparedActionContext(data.ActionDataKey, context);
		return context;
	}
}


namespace GuardLifecycleTest
{
	struct FFixture
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		ACPlayer* Player = World->SpawnActor<ACPlayer>();
		FCharacterComponentReferences Refs = References(Player);
		FFixture()
		{
			Refs.HealthComponent->InitializeHealth(100.f,100.f,EMaxHPUpdatePolicy::ClampCurrent);
			Refs.StateComponent->InitializeReferences(Refs);
			Refs.MovementComponent->InitializeReferences(Refs);
			Refs.ActionComponent->InitializeReferences(Refs);
			Refs.ActionOrchestratorComponent->InitializeReferences(Refs);
			Refs.DefenseComponent->InitializeReferences(Refs);
			Refs.ObservableOverlayComponent->InitializeReferences(Refs);
			Field<EWeaponType>(Refs.WeaponComponent,TEXT("CurrentWeaponType")) = EWeaponType::Sword;
		}
		~FFixture() { World->DestroyWorld(false); }
		void HoldGuard()
		{
			Refs.DefenseComponent->HandleGuardInputPressed();
			Refs.DefenseComponent->HandleGuardInStarted();
			Refs.DefenseComponent->HandleSwitchToGuard();
		}
		void Active(const FActionExecutionContext& C)
		{
			Field<FActionData>(C.ActionExecutor,TEXT("ActiveData_Cached"))=C.ActionData;
			Field<EActionType>(Refs.ActionComponent,TEXT("ActiveActionType"))=C.ActionDataKey.ActionType;
			Field<int32>(Refs.ActionComponent,TEXT("ActiveActionIndex"))=C.ActionDataKey.ActionIndex;
			Field<FActionData>(Refs.ActionComponent,TEXT("ActiveActionData"))=C.ActionData;
			Field<UCAction*>(Refs.ActionComponent,TEXT("ActiveActionExecutor"))=C.ActionExecutor;
			Refs.StateComponent->SetActionState();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuardReleaseLifecycleTest, "Portfolio.Combat.Guard.ReleaseLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGuardReleaseLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace GuardLifecycleTest;
	for (const bool deferred : {false,true})
	{
		FFixture f;
		Prepare(f.Player,EActionType::Guard,GetGuardActionPhaseIndex(EGuardActionPhase::Out),UCAction_Guard::StaticClass());
		auto active=deferred
			? Prepare(f.Player,EActionType::Guard,GetGuardActionPhaseIndex(EGuardActionPhase::In),UCAction_Guard::StaticClass())
			: Prepare(f.Player,EActionType::ComboAttack,0,UCAction_ComboAttack::StaticClass());
		f.HoldGuard();f.Active(active);
		FCombatActionRequest release;release.IntentType=ECombatActionIntent::Guard;release.IntentEvent=EActionIntentEvent::Completed;
		const auto result=f.Refs.ActionOrchestratorComponent->RequestCombatAction(release);
		TestFalse(TEXT("Release ends input intent"),f.Refs.DefenseComponent->WantsGuarding());
		TestTrue(TEXT("Release does not replace the active action"),f.Refs.ActionComponent->GetActiveActionExecutor()==active.ActionExecutor);
		if(deferred)
		{
			TestTrue(TEXT("GuardIn release stays deferred"),result.IsDeferredResult());
			TestTrue(TEXT("Deferred release preserves pose until GuardOut"),f.Refs.DefenseComponent->IsGuardingPose());
		}
		else
		{
			TestFalse(TEXT("Attack still rejects GuardOut"),result.IsAccepted());
			TestFalse(TEXT("Rejected GuardOut clears all guard overlay flags"),f.Refs.DefenseComponent->HasGuardOverlay());
			TestFalse(TEXT("Rejected GuardOut restores movement gait"),Field<bool>(f.Refs.MovementComponent,TEXT("bHasMovementGaitOverride")));
			TestTrue(TEXT("Rejected GuardOut allows a future guard start"),f.Refs.DefenseComponent->CanStartGuard());
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuardDeathLifecycleTest, "Portfolio.Combat.Guard.DeathLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGuardDeathLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace GuardLifecycleTest;
	for (const bool directKill : {false,true})
	{
		FFixture f;f.HoldGuard();
		f.Refs.HealthComponent->TakeDamage(1.f);
		TestTrue(TEXT("Nonlethal health change preserves held guard"),f.Refs.DefenseComponent->WantsGuarding()&&f.Refs.DefenseComponent->CanGuard());
		if(directKill)f.Refs.HealthComponent->TryKill();
		else f.Refs.HealthComponent->TakeDamage(1000.f);
		TestFalse(TEXT("Death clears intent without a tick"),f.Refs.DefenseComponent->WantsGuarding());
		TestFalse(TEXT("Death clears pose and both defense windows without a tick"),f.Refs.DefenseComponent->HasGuardOverlay());
		TestFalse(TEXT("Death removes guard gait override"),Field<bool>(f.Refs.MovementComponent,TEXT("bHasMovementGaitOverride")));
		f.Refs.DefenseComponent->HandleSwitchToGuard();
		f.Refs.DefenseComponent->RestoreGuardOverlay();
		TestFalse(TEXT("Late switch/restore cannot revive cleared guard"),f.Refs.DefenseComponent->HasGuardOverlay());
		f.Refs.HealthComponent->TryKill();
		TestFalse(TEXT("Repeated death is harmless"),f.Refs.DefenseComponent->HasGuardOverlay());
	}
	return true;
}
#endif
