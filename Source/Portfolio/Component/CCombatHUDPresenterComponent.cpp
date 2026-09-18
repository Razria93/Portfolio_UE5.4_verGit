#include "Component/CCombatHUDPresenterComponent.h"
#include "Character/Player/CPlayer.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CHealthComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CCombatTargetComponent.h"
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

UCCombatHUDPresenterComponent::UCCombatHUDPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	WidgetClass = UCCombatHUDWidget::StaticClass();
}

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

void UCCombatHUDPresenterComponent::SetControlledPlayer(ACPlayer* InPlayer)
{
	if (Player.IsValid()) Player->OnEndPlay.RemoveDynamic(this, &ThisClass::HandlePlayerEndPlay);
	if (PlayerHealth.IsValid()) PlayerHealth->OnHealthValuesChanged.RemoveAll(this);
	if (CombatTarget.IsValid()) CombatTarget->OnCombatTargetChanged.RemoveAll(this);
	UnbindTarget();
	Player = bEndingPlay ? nullptr : InPlayer;
	PlayerHealth = Player.IsValid() ? Player->GetHealthComp() : nullptr;
	CombatTarget = Player.IsValid() ? Player->GetCombatTargetComp() : nullptr;
	if (Player.IsValid()) Player->OnEndPlay.AddUniqueDynamic(this, &ThisClass::HandlePlayerEndPlay);
	if (PlayerHealth.IsValid()) PlayerHealth->OnHealthValuesChanged.AddUObject(this, &ThisClass::RefreshView);
	if (CombatTarget.IsValid()) CombatTarget->OnCombatTargetChanged.AddUObject(this, &ThisClass::HandleTargetChanged);
	RefreshTarget();
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

void UCCombatHUDPresenterComponent::RefreshTarget()
{
	AActor* next = CombatTarget.IsValid() ? CombatTarget->GetCombatTargetActor() : nullptr;
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
			if (TargetBalance.IsValid()) TargetBalance->OnBalanceValuesChanged.AddUObject(this, &ThisClass::RefreshView);
		}
	}
	RefreshView();
}

void UCCombatHUDPresenterComponent::RefreshView()
{
	ViewData = FCombatHUDViewData();
	ViewData.bHasPlayer = Player.IsValid() && !Player->IsActorBeingDestroyed();
	if (ViewData.bHasPlayer)
	{
		ViewData.PlayerHealth = ReadHealth(PlayerHealth.Get());
		ViewData.TargetRevision = CombatTarget.IsValid() ? CombatTarget->GetCombatTargetRevision() : 0;
		ViewData.bHasTarget = Target.IsValid() && !Target->IsActorBeingDestroyed()
			&& TargetHealth.IsValid() && TargetHealth->IsAlive()
			&& CombatTarget.IsValid() && CombatTarget->GetCombatTargetActor() == Target.Get();
		if (ViewData.bHasTarget)
		{
			ViewData.TargetHealth = ReadHealth(TargetHealth.Get());
			const ACEnemy* enemy = Cast<ACEnemy>(Target.Get());
			ViewData.TargetName = enemy && !enemy->GetCombatHUDDisplayName().IsEmpty()
				? enemy->GetCombatHUDDisplayName() : NSLOCTEXT("CombatHUD", "Target", "TARGET");
			if (TargetBalance.IsValid())
			{
				ViewData.BalanceMaximum = FMath::Max(0, TargetBalance->GetBalanceThreshold());
				ViewData.BalanceRemaining = FMath::Clamp(
					ViewData.BalanceMaximum - TargetBalance->GetCurrentBalanceCount(), 0, ViewData.BalanceMaximum);
			}
		}
	}
	if (Widget) Widget->ApplyViewData(ViewData);
}

void UCCombatHUDPresenterComponent::HandleTargetChanged(const FCombatTargetChange&)
{
	RefreshTarget();
}

void UCCombatHUDPresenterComponent::HandleDeadStateChanged(EDeadState, EDeadState)
{
	RefreshView();
}

void UCCombatHUDPresenterComponent::HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type)
{
	if (Target.Get() != Actor) return;
	UnbindTarget();
	RefreshView();
}

void UCCombatHUDPresenterComponent::HandlePlayerEndPlay(AActor*, EEndPlayReason::Type)
{
	SetControlledPlayer(nullptr);
}
