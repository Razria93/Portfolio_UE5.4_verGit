#include "Misc/AutomationTest.h"
#include "Component/CWeaponTrailComponent.h"
#include "DataAsset/CWeaponTrailDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Weapon/CWeaponActor.h"
#include "Type/CActionFeedbackTypes.h"
#include "UObject/UnrealType.h"
#include "NiagaraSystem.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeaponTrailSocketAttachmentTest, "Portfolio.Weapon.Trail.SocketAttachment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWeaponTrailSocketAttachmentTest::RunTest(const FString& Parameters)
{
	auto* asset = LoadObject<UWeaponTrailDataAsset>(nullptr, TEXT("/Game/09_DataAsset/DA_WeaponTrail_SciFiSword"));
	auto* sword = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/06_Weapon/Asset/SciFi_Sword/scifi_sword"));
	if (!TestNotNull(TEXT("Trail data"), asset) || !TestNotNull(TEXT("Sword"), sword)) return false;
	const auto* definition = asset->FindTrailDefinition(TEXT("CounterRush_Trail"));
	if (!TestNotNull(TEXT("Counter entry"), definition)) return false;
	TestFalse(TEXT("Default is legacy"), FWeaponTrailDefinition().bAttachToSocket);
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* actor = world->SpawnActor<AActor>();
	auto* mesh = NewObject<USkeletalMeshComponent>(actor);
	actor->SetRootComponent(mesh);
	mesh->RegisterComponent();
	mesh->SetSkeletalMesh(sword);
	auto* trail = NewObject<UWeaponTrailComponent>(actor);
	trail->SetAutoActivate(false);
	trail->RegisterComponent();
	trail->ConfigureAttachment(mesh, *definition);
	trail->SetComponentTickEnabled(false);
	TestTrue(TEXT("Real socket resolves"), trail->IsTrailAttachmentValid());
	for (const float angle : {0.f, 90.f, 173.f})
	{
		mesh->SetWorldTransform(FTransform(FRotator(angle, angle * .3f, 30.f), FVector(angle, 20, 30), FVector(2.f)));
		// No trail update call: scene attachment must propagate with Niagara tick disabled.
		const FTransform expected = FTransform(definition->RelativeRotation, definition->RelativeLocation)
			* mesh->GetSocketTransform(definition->bAttachToSocket ? definition->AttachmentSocket : NAME_None);
		TestTrue(TEXT("Follows socket without Niagara Tick"), trail->GetComponentTransform().Equals(expected, .001));
	}
	FWeaponTrailDefinition offset = *definition;
	offset.RelativeLocation = FVector(1, 2, 3);
	offset.RelativeRotation = FRotator(10, 20, 30);
	trail->ConfigureAttachment(mesh, offset);
	mesh->AddWorldOffset(FVector(50, 30, 10));
	TestTrue(TEXT("Designer offsets follow attachment"), trail->GetComponentTransform().Equals(
		FTransform(offset.RelativeRotation, offset.RelativeLocation) * mesh->GetSocketTransform(offset.bAttachToSocket ? offset.AttachmentSocket : NAME_None), .001));
	FWeaponTrailDefinition missing = *definition;
	missing.bAttachToSocket = true;
	missing.AttachmentSocket = TEXT("MissingSocket");
	trail->ConfigureAttachment(mesh, missing);
	TestFalse(TEXT("Missing socket fails closed"), trail->IsTrailAttachmentValid());
	missing.AttachmentSocket = NAME_None;
	trail->ConfigureAttachment(mesh, missing);
	TestFalse(TEXT("Empty explicit socket fails closed"), trail->IsTrailAttachmentValid());
	trail->ConfigureAttachment(mesh, FWeaponTrailDefinition());
	mesh->AddWorldOffset(FVector(10, 0, 0));
	TestTrue(TEXT("Legacy attachment valid"), trail->IsTrailAttachmentValid());
	TestTrue(TEXT("Legacy follows mesh origin"), trail->GetComponentTransform().Equals(mesh->GetComponentTransform(), .001));
	world->DestroyWorld(false);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeaponTrailFeedbackLifecycleTest, "Portfolio.Weapon.Trail.FeedbackLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWeaponTrailFeedbackLifecycleTest::RunTest(const FString& Parameters)
{
	auto* weaponClass = LoadClass<ACWeaponActor>(nullptr, TEXT("/Game/06_Weapon/BP_CWeaponActor_SciFi_Sword.BP_CWeaponActor_SciFi_Sword_C"));
	if (!TestNotNull(TEXT("Actual weapon BP"), weaponClass)) return false;
	const auto* dataProperty = FindFProperty<FObjectPropertyBase>(weaponClass, TEXT("WeaponTrailDataAsset"));
	if (!TestNotNull(TEXT("Weapon trail data property"), dataProperty)) return false;
	const auto* data = Cast<UWeaponTrailDataAsset>(dataProperty->GetObjectPropertyValue_InContainer(weaponClass->GetDefaultObject()));
	if (!TestNotNull(TEXT("Configured weapon trail data"), data)) return false;
	int32 expectedCount = 0;
	TSet<FName> keys;
	for (const auto& definition : data->TrailDefinitions)
	{
		if (definition.TriggerKey.IsNone() || keys.Contains(definition.TriggerKey))
		{
			AddError(TEXT("Trail definitions require unique non-empty keys."));
			return false;
		}
		keys.Add(definition.TriggerKey);
		for (const auto& system : definition.NiagaraSystems)
		{
			if (!TestNotNull(TEXT("Configured Niagara system"), system.Get())) return false;
			++expectedCount;
		}
	}
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(world);
	auto* weapon = world->SpawnActor<ACWeaponActor>(weaponClass);
	weapon->DispatchBeginPlay();
	TArray<UWeaponTrailComponent*> trails;
	weapon->GetComponents(trails);
	TestEqual(TEXT("One component per configured system"), trails.Num(), expectedCount);
	for (auto* trail : trails)
	{
		TestFalse(TEXT("Starts hidden"), trail->IsVisible());
	}
	for (const auto& definition : data->TrailDefinitions)
	{
		if (definition.NiagaraSystems.IsEmpty()) continue;
		FActionFeedbackRequest request;
		request.TriggerKey = definition.TriggerKey;
		request.ActionFeedbackMatchKey = definition.ActionFeedbackMatchKey;
		request.ActionFeedbackTiming = EActionFeedbackTiming::TriggerWindowBegin;
		TestTrue(TEXT("Notify begin accepted"), weapon->HandleTrailFeedback(request));
		TArray<UWeaponTrailComponent*> active;
		for (auto* trail : trails) if (trail->IsVisible()) active.Add(trail);
		TestEqual(TEXT("Only requested definition's systems activate"), active.Num(), definition.NiagaraSystems.Num());
		for (auto* trail : active)
		{
			TestTrue(TEXT("Selected system belongs to requested definition"), definition.NiagaraSystems.Contains(trail->GetAsset()));
			trail->SetComponentTickEnabled(false);
		}
		weapon->HandleTrailFeedback(request);
		TArray<UWeaponTrailComponent*> repeated;
		weapon->GetComponents(repeated);
		TestEqual(TEXT("Repeated begin reuses components"), repeated.Num(), trails.Num());
		weapon->SetActorLocationAndRotation(FVector(10, 40, 100), FRotator(40, 70, 10));
		for (auto* trail : active)
		{
			const FTransform expected = FTransform(definition.RelativeRotation, definition.RelativeLocation)
				* trail->GetAttachParent()->GetSocketTransform(definition.bAttachToSocket ? definition.AttachmentSocket : NAME_None);
			TestTrue(TEXT("Authored relative transform follows without Niagara tick"), trail->GetComponentTransform().Equals(expected, .001));
		}
		request.ActionFeedbackTiming = EActionFeedbackTiming::TriggerWindowEnd;
		TestTrue(TEXT("Notify end accepted"), weapon->HandleTrailFeedback(request));
		for (auto* trail : trails) TestFalse(TEXT("End leaves all trails hidden"), trail->IsVisible());
		request.ActionFeedbackTiming = EActionFeedbackTiming::TriggerWindowBegin;
		weapon->HandleTrailFeedback(request);
		weapon->DeactivateAllTrails();
		for (auto* trail : trails) TestFalse(TEXT("Cancellation cleanup hides all trails"), trail->IsVisible());
	}
	TestTrue(TEXT("Actor destruction accepted"), weapon->Destroy());
	for (auto* trail : trails) TestFalse(TEXT("Actor teardown unregisters trails"), trail->IsRegistered());
	GEngine->DestroyWorldContext(world);
	world->DestroyWorld(false);
	return true;
}
#endif
