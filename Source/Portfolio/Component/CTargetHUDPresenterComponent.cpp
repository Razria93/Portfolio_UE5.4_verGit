#include "Component/CTargetHUDPresenterComponent.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"
#include "UI/CTargetHUDWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"

UCTargetHUDPresenterComponent::UCTargetHUDPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// ===== Component Reference =====

void UCTargetHUDPresenterComponent::InitializeReferences(APlayerController* InOwnerPlayerController)
{
	OwnerPlayerController_Injected = InOwnerPlayerController;

	ValidateRequiredReferences();
	SynchronizeTargetState();
}

void UCTargetHUDPresenterComponent::SetCombatTargetComponent(UCCombatTargetComponent* InCombatTargetComponent)
{
	if (CombatTargetComponent_Injected == InCombatTargetComponent)
	{
		SynchronizeTargetState();
		return;
	}

	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.RemoveAll(this);
	}

	CombatTargetComponent_Injected = InCombatTargetComponent;

	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.AddUObject(this, &UCTargetHUDPresenterComponent::HandleCombatTargetChanged);
	}

	SynchronizeTargetState();
}

// ===== Lifecycle =====

void UCTargetHUDPresenterComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateTargetHUDWidget();
	SynchronizeTargetState();
}

void UCTargetHUDPresenterComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	SetCombatTargetComponent(nullptr);

	SetComponentTickEnabled(false);
	DestroyTargetHUDWidget();
	OwnerPlayerController_Injected = nullptr;

	Super::EndPlay(InEndPlayReason);
}

void UCTargetHUDPresenterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTargetMarker();
}

// ===== Validation =====

bool UCTargetHUDPresenterComponent::ValidateRequiredReferences() const
{
	const FRequiredReference requiredReferences[] =
	{
		{ OwnerPlayerController_Injected, TEXT("APlayerController Owner") },
	};

	bool bValid = true;
	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerPlayerController_Injected, this);
	}

	return bValid;
}

// ===== Widget Lifecycle =====

void UCTargetHUDPresenterComponent::CreateTargetHUDWidget()
{
	if (IsValid(TargetHUDWidget)) return;
	if (!IsValid(OwnerPlayerController_Injected) || !OwnerPlayerController_Injected->IsLocalController()) return;
	if (!TargetHUDWidgetClass) return;

	TargetHUDWidget = CreateWidget<UCTargetHUDWidget>(OwnerPlayerController_Injected, TargetHUDWidgetClass);
	if (!IsValid(TargetHUDWidget)) return;

	TargetHUDWidget->AddToViewport(TargetMarkerTuning.WidgetZOrder);
	HideTargetMarker();
}

void UCTargetHUDPresenterComponent::DestroyTargetHUDWidget()
{
	if (!IsValid(TargetHUDWidget)) return;

	HideTargetMarker();
	TargetHUDWidget->RemoveFromParent();
	TargetHUDWidget = nullptr;
}

// ===== Target State =====

void UCTargetHUDPresenterComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	SynchronizeTargetState();
}

void UCTargetHUDPresenterComponent::SynchronizeTargetState()
{
	const FCombatTargetSnapshot snapshot = IsValid(CombatTargetComponent_Injected)
		? CombatTargetComponent_Injected->GetCombatTargetSnapshot()
		: FCombatTargetSnapshot();
	const bool bHasTarget = IsValid(snapshot.TargetActor);

	SetComponentTickEnabled(bHasTarget && IsValid(TargetHUDWidget));

	if (bHasTarget)
	{
		UpdateTargetMarker();
		return;
	}

	HideTargetMarker();
}

// ===== Marker Presentation =====

void UCTargetHUDPresenterComponent::UpdateTargetMarker()
{
	if (!IsValid(TargetHUDWidget)) return;
	if (!IsValid(CombatTargetComponent_Injected))
	{
		HideTargetMarker();
		return;
	}

	FTargetMarkerViewData viewData;
	const FCombatTargetSnapshot snapshot = CombatTargetComponent_Injected->GetCombatTargetSnapshot();
	if (!TryBuildTargetMarkerViewData(Cast<ACEnemy>(snapshot.TargetActor), viewData))
	{
		HideTargetMarker();
		return;
	}

	TargetHUDWidget->UpdateTargetMarker(viewData);
}

void UCTargetHUDPresenterComponent::HideTargetMarker()
{
	if (!IsValid(TargetHUDWidget)) return;

	TargetHUDWidget->UpdateTargetMarker(FTargetMarkerViewData());
}

bool UCTargetHUDPresenterComponent::TryBuildTargetMarkerViewData(const ACEnemy* InTarget, FTargetMarkerViewData& OutViewData) const
{
	OutViewData = FTargetMarkerViewData();
	if (!IsValid(OwnerPlayerController_Injected) || !IsValid(OwnerPlayerController_Injected->GetPawn())) return false;
	if (!IsValid(InTarget)) return false;

	const FVector targetWorldLocation = InTarget->GetTargetMarkerWorldLocation();
	FVector viewLocation = FVector::ZeroVector;
	FRotator viewRotation = FRotator::ZeroRotator;
	OwnerPlayerController_Injected->GetPlayerViewPoint(viewLocation, viewRotation);

	const FVector directionToTarget = (targetWorldLocation - viewLocation).GetSafeNormal();
	if (FVector::DotProduct(viewRotation.Vector(), directionToTarget) <= 0.f) return false;

	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		OwnerPlayerController_Injected,
		targetWorldLocation,
		OutViewData.WidgetPosition,
		true))
	{
		return false;
	}

	const float viewportScale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(OwnerPlayerController_Injected), KINDA_SMALL_NUMBER);
	const FVector2D viewportWidgetSize = UWidgetLayoutLibrary::GetViewportSize(OwnerPlayerController_Injected) / viewportScale;
	const bool bWithinViewport = OutViewData.WidgetPosition.X >= 0.f
		&& OutViewData.WidgetPosition.Y >= 0.f
		&& OutViewData.WidgetPosition.X < viewportWidgetSize.X
		&& OutViewData.WidgetPosition.Y < viewportWidgetSize.Y;

	if (!bWithinViewport) return false;

	OutViewData.bVisible = true;
	return true;
}
