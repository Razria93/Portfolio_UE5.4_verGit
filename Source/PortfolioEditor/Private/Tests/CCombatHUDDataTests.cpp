#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Component/CHealthComponent.h"
#include "Component/CBalanceComponent.h"
#include "Type/CCombatHUDTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Engine/World.h"
#include "UI/CCombatHUDWidget.h"
#include "Slate/WidgetRenderer.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/BufferArchive.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Character/Player/CPlayer.h"
#include "Component/CCombatHUDPresenterComponent.h"
#include "Component/CCombatTargetComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDHealthObservationTest,
	"Portfolio.UI.CombatHUD.HealthObservation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDHealthObservationTest::RunTest(const FString& Parameters)
{
	UCHealthComponent* health = NewObject<UCHealthComponent>();
	int32 changes = 0;
	bool bDeadObservedFirst = false;
	health->OnHealthValuesChanged.AddLambda([&]() {
		++changes;
		if (health->GetCurrentHP() == 0.f)
			TestTrue(TEXT("Death state committed before value observation"), bDeadObservedFirst);
	});
	health->OnDeadStateChanged.AddLambda([&](EDeadState, EDeadState next) {
		bDeadObservedFirst = next == EDeadState::Dead;
	});
	health->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	TestEqual(TEXT("Initial values"), changes, 1);
	health->TakeDamage(25.f);
	TestEqual(TEXT("Damage"), health->GetCurrentHP(), 75.f);
	health->TakeHeal(10.f);
	TestEqual(TEXT("Heal"), health->GetCurrentHP(), 85.f);
	health->TryUpdateMaxHP(200.f, EMaxHPUpdatePolicy::ClampCurrent);
	TestEqual(TEXT("Max-only change is observed"), changes, 4);
	health->TakeDamage(0.f);
	health->TakeHeal(-1.f);
	TestEqual(TEXT("No-op ignored"), changes, 4);
	health->TryKill();
	TestEqual(TEXT("Kill observed"), changes, 5);
	health->TryKill();
	TestEqual(TEXT("Repeated kill ignored"), changes, 5);
	health->InitializeHealth(100.f, 10.f, EMaxHPUpdatePolicy::FillToMax);
	TestEqual(TEXT("Reinitialize"), health->GetCurrentHP(), 100.f);
	health->TakeDamage(100.f);
	TestEqual(TEXT("Lethal damage observed"), changes, 7);
	health->InitializeHealth(0.f, 0.f, EMaxHPUpdatePolicy::ClampCurrent);
	TestEqual(TEXT("Zero max change observed"), changes, 8);
	health->OnHealthValuesChanged.Clear();
	health->OnDeadStateChanged.Clear();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDBalanceObservationTest,
	"Portfolio.UI.CombatHUD.BalanceObservation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDBalanceObservationTest::RunTest(const FString& Parameters)
{
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World"), world)) return false;
	AActor* target = world->SpawnActor<AActor>();
	UCBalanceComponent* balance = NewObject<UCBalanceComponent>(target);
	int32 changes = 0;
	balance->OnBalanceValuesChanged.AddLambda([&]() { ++changes; });
	FCombatResultPacket packet;
	packet.TargetActor = target;
	packet.DefenseOutcome = EDamageDefenseOutcome::Parry;
	packet.CombatSignalResultSerial = 1;
	balance->AdvanceBalanceFromParry(packet);
	TestEqual(TEXT("Intermediate parry notified"), changes, 1);
	TestEqual(TEXT("First count"), balance->GetCurrentBalanceCount(), 1);
	balance->AdvanceBalanceFromParry(packet);
	TestEqual(TEXT("Duplicate ignored"), changes, 1);
	packet.CombatSignalResultSerial = 2;
	balance->AdvanceBalanceFromParry(packet);
	packet.CombatSignalResultSerial = 3;
	balance->AdvanceBalanceFromParry(packet);
	TestEqual(TEXT("Threshold notified"), changes, 3);
	balance->ShutdownBalanceRuntime();
	TestEqual(TEXT("Shutdown/reset notified"), changes, 4);
	TestEqual(TEXT("Reset count"), balance->GetCurrentBalanceCount(), 0);
	balance->OnBalanceValuesChanged.Clear();
	world->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDResourceViewTest,
	"Portfolio.UI.CombatHUD.ResourceView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDResourceViewTest::RunTest(const FString& Parameters)
{
	FHUDResourceViewData value;
	value.Availability = EHUDResourceAvailability::Available;
	value.Maximum = 100.f;
	value.Current = 50.f;
	TestEqual(TEXT("Fraction"), value.GetFraction(), 0.5f);
	value.Current = 0.f;
	TestTrue(TEXT("Zero is still available"), value.Availability == EHUDResourceAvailability::Available);
	value.Availability = EHUDResourceAvailability::Unimplemented;
	TestTrue(TEXT("TODO distinct from zero"), value.Availability != EHUDResourceAvailability::Available);
	value.Availability = EHUDResourceAvailability::Available;
	value.Maximum = 0.f;
	TestEqual(TEXT("Zero denominator safe"), value.GetFraction(), 0.f);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDWidgetConstructionTest,
	"Portfolio.UI.CombatHUD.WidgetConstruction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetConstructionTest::RunTest(const FString& Parameters)
{
	UCCombatHUDWidget* widget = NewObject<UCCombatHUDWidget>();
	TestTrue(TEXT("Native UMG initializes"), widget->Initialize());
	widget->TakeWidget();
	FCombatHUDViewData data;
	data.bHasPlayer = true;
	data.bHasTarget = true;
	data.TargetName = FText::FromString(TEXT("TEST"));
	data.PlayerHealth.Availability = EHUDResourceAvailability::Available;
	data.PlayerHealth.Current = 50.f;
	data.PlayerHealth.Maximum = 100.f;
	data.BalanceMaximum = 3;
	data.BalanceRemaining = 2;
	widget->ApplyViewData(data);
	TestEqual(TEXT("Current player data"), widget->GetViewData().PlayerHealth.GetFraction(), 0.5f);
	widget->ApplyViewData(FCombatHUDViewData());
	TestFalse(TEXT("Clear removes target"), widget->GetViewData().bHasTarget);
	widget->ReleaseSlateResources(true);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDRenderPreviewTest,
	"Portfolio.UI.CombatHUD.RenderPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDRenderPreviewTest::RunTest(const FString& Parameters)
{
	if (GUsingNullRHI)
	{
		AddInfo(TEXT("RenderPreview requires an RHI; skipped in NullRHI mode."));
		return true;
	}
	UCCombatHUDWidget* widget = NewObject<UCCombatHUDWidget>();
	widget->Initialize();
	const TSharedRef<SWidget> slate = widget->TakeWidget();
	FCombatHUDViewData data;
	data.bHasPlayer = data.bHasTarget = true;
	data.TargetName = FText::FromString(TEXT("TARGET / 스텔라"));
	data.PlayerHealth.Availability = data.TargetHealth.Availability = EHUDResourceAvailability::Available;
	data.PlayerHealth.Current = 72;
	data.PlayerHealth.Maximum = 100;
	data.TargetHealth.Current = 60;
	data.TargetHealth.Maximum = 100;
	data.BalanceMaximum = 3;
	data.BalanceRemaining = 2;
	widget->ApplyViewData(data);
	FWidgetRenderer* renderer = new FWidgetRenderer(true);
	const FVector2D sizes[] = { FVector2D(1920,1080), FVector2D(2560,1440), FVector2D(3440,1440) };
	for (const FVector2D& size : sizes)
	{
		UTextureRenderTarget2D* target = renderer->DrawWidget(slate, size);
		FlushRenderingCommands();
		// Warm font fallback/layout caches before exporting the stable frame.
		renderer->DrawWidget(target, slate, size, 0.f);
		FlushRenderingCommands();
		FBufferArchive bytes;
		TestTrue(TEXT("Export actual UMG render"), FImageUtils::ExportRenderTarget2DAsPNG(target, bytes));
		const FString filename = FPaths::ProjectSavedDir() / TEXT("Automation/CombatHUD") /
			FString::Printf(TEXT("hud-%dx%d.png"), int32(size.X), int32(size.Y));
		TestTrue(TEXT("Save preview"), FFileHelper::SaveArrayToFile(bytes, *filename));
	}
	BeginCleanup(renderer);
	widget->ReleaseSlateResources(true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDPresenterLifecycleTest,
	"Portfolio.UI.CombatHUD.PresenterLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDPresenterLifecycleTest::RunTest(const FString& Parameters)
{
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	ACPlayer* player = world->SpawnActor<ACPlayer>();
	if (!TestNotNull(TEXT("Player"), player)) { world->DestroyWorld(false); return false; }
	UCCombatHUDPresenterComponent* presenter = NewObject<UCCombatHUDPresenterComponent>(player);
	player->GetHealthComp()->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	presenter->SetControlledPlayer(player);
	TestTrue(TEXT("Player bound"), presenter->GetViewData().bHasPlayer);
	AActor* first = world->SpawnActor<AActor>();
	AActor* second = world->SpawnActor<AActor>();
	auto makeHealth = [](AActor* actor, float maxHP) {
		UCHealthComponent* health = NewObject<UCHealthComponent>(actor);
		actor->AddInstanceComponent(health);
		health->InitializeHealth(maxHP, maxHP, EMaxHPUpdatePolicy::ClampCurrent);
		return health;
	};
	UCHealthComponent* firstHealth = makeHealth(first, 100.f);
	UCHealthComponent* secondHealth = makeHealth(second, 200.f);
	UCCombatTargetComponent* selection = player->GetCombatTargetComp();
	selection->RequestSetCombatTarget(first, ECombatTargetChangeReason::PlayerSelection);
	TestTrue(TEXT("Selected target shown"), presenter->GetViewData().bHasTarget);
	firstHealth->TakeDamage(25.f);
	TestEqual(TEXT("Target damage refreshed"), presenter->GetViewData().TargetHealth.Current, 75.f);
	selection->RequestSetCombatTarget(second, ECombatTargetChangeReason::PlayerSelection);
	firstHealth->TakeDamage(25.f);
	TestEqual(TEXT("Old target cannot overwrite new snapshot"), presenter->GetViewData().TargetHealth.Current, 200.f);
	TestFalse(TEXT("Old target subscription removed"), firstHealth->OnHealthValuesChanged.IsBoundToObject(presenter));
	secondHealth->TryKill();
	TestFalse(TEXT("Dead target hidden"), presenter->GetViewData().bHasTarget);
	selection->RequestClearCombatTarget(ECombatTargetChangeReason::ManualClear);
	TestFalse(TEXT("Cleared target hidden"), presenter->GetViewData().bHasTarget);
	selection->RequestSetCombatTarget(first, ECombatTargetChangeReason::PlayerSelection);
	// Explicit delegate delivery checks cleanup without starting unrelated combat systems.
	first->OnEndPlay.Broadcast(first, EEndPlayReason::Destroyed);
	TestFalse(TEXT("Target EndPlay clears HUD"), presenter->GetViewData().bHasTarget);
	presenter->SetControlledPlayer(nullptr);
	TestFalse(TEXT("Unpossess clears player"), presenter->GetViewData().bHasPlayer);
	TestFalse(TEXT("Player subscription removed"), player->GetHealthComp()->OnHealthValuesChanged.IsBoundToObject(presenter));
	presenter->SetControlledPlayer(player);
	player->OnEndPlay.Broadcast(player, EEndPlayReason::Destroyed);
	TestFalse(TEXT("Player EndPlay clears HUD"), presenter->GetViewData().bHasPlayer);
	world->DestroyWorld(false);
	return true;
}
#endif
