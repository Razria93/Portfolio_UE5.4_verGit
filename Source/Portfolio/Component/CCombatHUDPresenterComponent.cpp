#include "Component/CCombatHUDPresenterComponent.h"
#include "Character/Player/CPlayer.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CHealthComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CTargetLockAssistComponent.h"
#include "Engine/World.h"
#include "UI/CCombatHUDWidget.h"
#include "GameFramework/PlayerController.h"

namespace
{
	FHUDResourceViewData ReadHealth(const UCHealthComponent* Health)
	{
		FHUDResourceViewData result;
		if (IsValid(Health) && Health->GetMaxHP() > 0.f)
		{
			result.Availability = EHUDResourceAvailability::Available;
			result.Current = Health->GetCurrentHP();
			result.Maximum = Health->GetMaxHP();
		}
		return result;
	}
}

// Construction

UCCombatHUDPresenterComponent::UCCombatHUDPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	WidgetClass = UCCombatHUDWidget::StaticClass();
}

// Component Reference

void UCCombatHUDPresenterComponent::SetControlledPlayer(ACPlayer* InPlayer)
{
	UnbindPlayerEvents();
	UnbindTarget();

	ResetParryPresentationRuntime();

	LockAssist = IsValid(GetOwner()) ? GetOwner()->FindComponentByClass<UCTargetLockAssistComponent>() : nullptr;

	CachePlayerReferences(bEndingPlay ? nullptr : InPlayer);
	SetComponentTickEnabled(Player.IsValid());

	BindPlayerEvents();
	RefreshTarget();
}

// Lifecycle

void UCCombatHUDPresenterComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* controller = Cast<APlayerController>(GetOwner());
	if (!IsValid(controller) || !controller->IsLocalController()) return;

	if (WidgetClass)
	{
		Widget = CreateWidget<UCCombatHUDWidget>(controller, WidgetClass);
		if (Widget) Widget->AddToViewport(WidgetZOrder);
	}

	SetControlledPlayer(Cast<ACPlayer>(controller->GetPawn()));
}

void UCCombatHUDPresenterComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	bEndingPlay = true;

	SetControlledPlayer(nullptr);

	if (Widget) Widget->RemoveFromParent();
	Widget = nullptr;

	Super::EndPlay(Reason);
}

void UCCombatHUDPresenterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RefreshActions();
}

// Player Reference Cache

void UCCombatHUDPresenterComponent::CachePlayerReferences(ACPlayer* InPlayer)
{
	Player = InPlayer;
	PlayerCombatTarget = Player.IsValid() ? Player->GetCombatTargetComp() : nullptr;

	PlayerHealth = Player.IsValid() ? Player->GetHealthComp() : nullptr;
	Defense = Player.IsValid() ? Player->FindComponentByClass<UCDefenseComponent>() : nullptr;

	Actions = Player.IsValid() ? Player->FindComponentByClass<UCActionComponent>() : nullptr;
	ActionOrchestrator = Player.IsValid() ? Player->FindComponentByClass<UCActionOrchestratorComponent>() : nullptr;
	Execution = Player.IsValid() ? Player->FindComponentByClass<UCExecutionCollaborationComponent>() : nullptr;

	PlayerCombatSignals = Player.IsValid() ? Player->FindComponentByClass<UCCombatSignalTargetComponent>() : nullptr;
}

// Player Event Binding

void UCCombatHUDPresenterComponent::BindPlayerEvents()
{
	if (Player.IsValid())
		Player->OnEndPlay.AddUniqueDynamic(this, &ThisClass::HandlePlayerEndPlay);

	if (PlayerCombatTarget.IsValid())
		PlayerCombatTarget->OnCombatTargetChanged.AddUObject(this, &ThisClass::HandleTargetChanged);

	if (PlayerHealth.IsValid())
		PlayerHealth->OnHealthValuesChanged.AddUObject(this, &ThisClass::RefreshView);

	if (PlayerCombatSignals.IsValid())
		PlayerCombatSignals->OnCombatSignalTargetAccepted.AddUObject(this, &ThisClass::HandlePlayerCombatResult);
}

void UCCombatHUDPresenterComponent::UnbindPlayerEvents()
{
	if (Player.IsValid())
		Player->OnEndPlay.RemoveDynamic(this, &ThisClass::HandlePlayerEndPlay);

	if (PlayerCombatTarget.IsValid())
		PlayerCombatTarget->OnCombatTargetChanged.RemoveAll(this);

	if (PlayerHealth.IsValid())
		PlayerHealth->OnHealthValuesChanged.RemoveAll(this);

	if (PlayerCombatSignals.IsValid())
		PlayerCombatSignals->OnCombatSignalTargetAccepted.RemoveAll(this);
}

// Target Binding

void UCCombatHUDPresenterComponent::RefreshTarget()
{
	AActor* next = PlayerCombatTarget.IsValid() ? PlayerCombatTarget->GetCombatTargetActor() : nullptr;

	if (Target.Get() != next)
	{
		UnbindTarget();

		if (IsValid(next) && !next->IsActorBeingDestroyed())
		{
			Target = next;
			TargetHealth = next->FindComponentByClass<UCHealthComponent>();
			TargetBalance = next->FindComponentByClass<UCBalanceComponent>();

			next->OnEndPlay.AddUniqueDynamic(this, &ThisClass::HandleTargetEndPlay);

			if (TargetHealth.IsValid())
			{
				TargetHealth->OnHealthValuesChanged.AddUObject(this, &ThisClass::RefreshView);
				TargetHealth->OnDeadStateChanged.AddUObject(this, &ThisClass::HandleDeadStateChanged);
			}

			if (TargetBalance.IsValid())
				TargetBalance->OnBalanceValuesChanged.AddUObject(this, &ThisClass::RefreshView);
		}
	}

	RefreshView();
}

void UCCombatHUDPresenterComponent::UnbindTarget()
{
	if (Target.IsValid()) Target->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleTargetEndPlay);

	if (TargetHealth.IsValid())
	{
		TargetHealth->OnHealthValuesChanged.RemoveAll(this);
		TargetHealth->OnDeadStateChanged.RemoveAll(this);
	}

	if (TargetBalance.IsValid()) TargetBalance->OnBalanceValuesChanged.RemoveAll(this);

	Target.Reset();
	TargetHealth.Reset();
	TargetBalance.Reset();
}

// Presentation Runtime

void UCCombatHUDPresenterComponent::ResetParryPresentationRuntime()
{
	ParryHighlightUntil = 0.0;
	LastParryResultSerial = 0;
}

// View Synchronization

void UCCombatHUDPresenterComponent::RefreshView()
{
	const FHUDActionViewData previousActions = ViewData.Actions;

	ViewData = FCombatHUDViewData();

	ViewData.Actions = previousActions;
	ViewData.bHasPlayer = Player.IsValid() && !Player->IsActorBeingDestroyed();

	if (ViewData.bHasPlayer)
	{
		ViewData.PlayerHealth = ReadHealth(PlayerHealth.Get());
		ViewData.TargetRevision = PlayerCombatTarget.IsValid() ? PlayerCombatTarget->GetCombatTargetRevision() : 0;

		ViewData.bHasTarget = Target.IsValid() && !Target->IsActorBeingDestroyed()
			&& TargetHealth.IsValid() && TargetHealth->IsAlive()
			&& PlayerCombatTarget.IsValid() && PlayerCombatTarget->GetCombatTargetActor() == Target.Get();

		if (ViewData.bHasTarget)
		{
			ViewData.TargetHealth = ReadHealth(TargetHealth.Get());

			const ACEnemy* enemy = Cast<ACEnemy>(Target.Get());
			ViewData.TargetName = enemy && !enemy->GetCombatHUDDisplayName().IsEmpty() ? enemy->GetCombatHUDDisplayName() : NSLOCTEXT("CombatHUD", "Target", "TARGET");

			if (TargetBalance.IsValid())
			{
				ViewData.BalanceMaximum = FMath::Max(0, TargetBalance->GetBalanceThreshold());
				ViewData.BalanceRemaining = FMath::Clamp(ViewData.BalanceMaximum - TargetBalance->GetCurrentBalanceCount(), 0, ViewData.BalanceMaximum);
			}
		}
	}

	if (Widget) Widget->ApplyViewData(ViewData);

	RefreshActions();
}

void UCCombatHUDPresenterComponent::RefreshActions()
{
	FHUDActionViewData next;
	if (Player.IsValid() && !Player->IsActorBeingDestroyed() && PlayerHealth.IsValid() && PlayerHealth->IsAlive())
	{
		EActionRequestRejectReason reason;
		if (ActionOrchestrator.IsValid())
		{
			next.Guard = ActionOrchestrator->QueryCombatActionAvailability(ECombatActionIntent::Guard, reason) ? EHUDActionState::Ready : EHUDActionState::Unavailable;
			next.Dodge = ActionOrchestrator->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason) ? EHUDActionState::Ready : EHUDActionState::Unavailable;
		}

		const bool bExternalInputAllowed = !Execution.IsValid() || Execution->GetExternalCombatInputPolicy() == EExternalCombatInputPolicy::Normal;

		if (bExternalInputAllowed && Defense.IsValid() && Defense->IsGuardingPose())
			next.Guard = EHUDActionState::Active;

		if (bExternalInputAllowed && Actions.IsValid() && Actions->IsActiveActionType(EActionType::Dodge))
			next.Dodge = EHUDActionState::Active;

		if (Execution.IsValid())
		{
			const FExecutionCollaborationRuntimeSnapshot session = Execution->GetExecutionCollaborationRuntimeSnapshot();

			EExecutionAvailabilityBlock block;
			if (session.bHasActiveSession && session.bIsSourceRole)
				next.Execution = EHUDActionState::Active;
			else if (LockAssist.IsValid() && LockAssist->IsTargetLockActive() && Execution->QueryCombatExecutionAvailability(block))
				next.Execution = EHUDActionState::Ready;
		}

		next.bParrySuccess = GetWorld() && GetWorld()->GetTimeSeconds() < ParryHighlightUntil;
	}
	else
	{
		ParryHighlightUntil = 0.0;
	}

	if (next == ViewData.Actions) return;
	ViewData.Actions = next;

	if (Widget) Widget->ApplyActionViewData(next);
}

// Event Handlers

void UCCombatHUDPresenterComponent::HandlePlayerCombatResult(const FCombatSignalTargetPacket& Packet)
{
	if (!Player.IsValid()
		|| !PlayerHealth.IsValid()
		|| !PlayerHealth->IsAlive()
		|| Packet.Context.TargetActor != Player.Get()
		|| !Packet.Result.bAccepted
		|| Packet.Result.DefenseOutcome != EDamageDefenseOutcome::Parry
		|| Packet.ResultSerial == 0 || Packet.ResultSerial <= LastParryResultSerial) return;

	LastParryResultSerial = Packet.ResultSerial;
	ParryHighlightUntil = GetWorld() ? GetWorld()->GetTimeSeconds() + 0.25 : 0.0;
}

void UCCombatHUDPresenterComponent::HandleTargetChanged(const FCombatTargetChange&)
{
	RefreshTarget();
}

void UCCombatHUDPresenterComponent::HandleDeadStateChanged(EDeadState, EDeadState)
{
	RefreshView();
}

void UCCombatHUDPresenterComponent::HandlePlayerEndPlay(AActor*, EEndPlayReason::Type)
{
	SetControlledPlayer(nullptr);
}

void UCCombatHUDPresenterComponent::HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type)
{
	if (Target.Get() != Actor) return;
	
	UnbindTarget();
	RefreshView();
}
