#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Component/CHealthComponent.h"
#include "Component/CBalanceComponent.h"
#include "Type/CCombatHUDTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Engine/World.h"
#include "UI/CCombatHUDWidget.h"
#include "UI/CCombatHUDGaugeLayout.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
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
	TestEqual(TEXT("Current / maximum text"), value.GetValueText().ToString(), FString(TEXT("50 / 100")));
	value.Current = 0.f;
	TestEqual(TEXT("Real zero shown numerically"), value.GetValueText().ToString(), FString(TEXT("0 / 100")));
	TestTrue(TEXT("Zero is still available"), value.Availability == EHUDResourceAvailability::Available);
	value.Availability = EHUDResourceAvailability::Unimplemented;
	TestEqual(TEXT("TODO stays a dash"), value.GetValueText().ToString(), FString(TEXT("\u2014")));
	TestTrue(TEXT("TODO distinct from zero"), value.Availability != EHUDResourceAvailability::Available);
	value.Availability = EHUDResourceAvailability::Available;
	value.Maximum = 0.f;
	TestEqual(TEXT("Zero denominator safe"), value.GetFraction(), 0.f);
	TestEqual(TEXT("Invalid max has no fake ratio"), value.GetValueText().ToString(), FString(TEXT("\u2014")));
	value.Maximum = 1000.f;
	value.Current = 850.f;
	TestEqual(TEXT("Grouped maximum"), value.GetValueText().ToString(), FString(TEXT("850 / ")) + FText::AsNumber(1000).ToString());
	value.Current = 0.1f;
	TestTrue(TEXT("Positive fractional HP not displayed as zero"), value.GetValueText().ToString().StartsWith(TEXT("1 / ")));
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
	int32 partialCells = 0;
	widget->WidgetTree->ForEachWidget([&](UWidget* child) {
		const UImage* cell = Cast<UImage>(child);
		const UCanvasPanelSlot* slot = cell ? Cast<UCanvasPanelSlot>(cell->Slot) : nullptr;
		if (slot && FMath::IsNearlyEqual(slot->GetSize().X, 2.5f) && FMath::IsNearlyEqual(slot->GetSize().Y, 5.f)
			&& cell->GetVisibility() == ESlateVisibility::HitTestInvisible) ++partialCells;
	});
	TestEqual(TEXT("Half-filled boundary column clips all three rows to half width"), partialCells, 3);
	UCanvasPanel* healthGrid = Cast<UCanvasPanel>(widget->WidgetTree->FindWidget(TEXT("PlayerHealthGrid")));
	if (TestNotNull(TEXT("Player fixed-unit grid"), healthGrid))
	{
		TestEqual(TEXT("100 HP capacity: one column, foreground and background"), healthGrid->GetChildrenCount(), 6);
		data.PlayerHealth.Maximum = 5000;
		data.PlayerHealth.Current = 3650;
		widget->ApplyViewData(data);
		TestEqual(TEXT("Capacity growth rebuilds to fifty columns"), healthGrid->GetChildrenCount(), 300);
		data.PlayerHealth.Current = 1000;
		widget->ApplyViewData(data);
		TestEqual(TEXT("Damage does not change capacity"), healthGrid->GetChildrenCount(), 300);
		data.PlayerHealth.Maximum = 200;
		widget->ApplyViewData(data);
		TestEqual(TEXT("Capacity shrink removes old columns"), healthGrid->GetChildrenCount(), 12);
	}
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
	data.PlayerHealth.Current = 3650;
	data.PlayerHealth.Maximum = 5000;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDGaugeLayoutTest,
	"Portfolio.UI.CombatHUD.GaugeLayout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDGaugeLayoutTest::RunTest(const FString& Parameters)
{
	using namespace CombatHUDGauge;
	TestEqual(TEXT("BU 1600: eight 2-column units"), ResourceColumns(1600, EnergyPerCell * 2, 64), 16);
	TestEqual(TEXT("BE 1400: seven units"), ResourceColumns(1400, EnergyPerCell * 2, 64), 14);
	TestEqual(TEXT("HP 5000: fifty columns"), ResourceColumns(5000, PlayerHealthPerColumn, 77), 50);
	TestEqual(TEXT("SH 800: two 10-column units"), ResourceColumns(800, ShieldPerColumn, 64), 20);
	TestEqual(TEXT("HP non-multiple capacity adds boundary column"), ResourceColumns(5050, PlayerHealthPerColumn, 77), 51);
	TestEqual(TEXT("Resource allocation bounded"), ResourceColumns(1.e20f, PlayerHealthPerColumn, 77), 77);
	TestEqual(TEXT("BU 125: third cell half alpha"), UnitFill(125, 1600, EnergyPerCell, 2), 0.5f);
	TestEqual(TEXT("BU 125: fourth cell empty"), UnitFill(125, 1600, EnergyPerCell, 3), 0.f);
	TestEqual(TEXT("HP 5050: half final column"), UnitFill(5050, 5050, PlayerHealthPerColumn, 50), 0.5f);
	TestEqual(TEXT("SH 420: half eleventh column"), UnitFill(420, 800, ShieldPerColumn, 10), 0.5f);
	TestEqual(TEXT("Boss SH has five complete groups"), BossGroups, 5);
	TestEqual(TEXT("HP three-row height with wider gaps"), Height(3), 20.f);
	TestEqual(TEXT("Two-row resource height"), Height(2), 12.5f);
	TestEqual(TEXT("Cell spacing within a unit"), ColumnX(1, 2) - CellSize, 2.5f);
	for (float scale : { 0.5f, 0.75f, 0.967f, 1.f, 1.25f, 1.5f })
	{
		const FPixelMetrics pixels(scale);
		TestEqual(TEXT("Boss HP and SH have identical physical width"), pixels.ColumnX(BossHealthColumns - 1, 0),
			pixels.ColumnX(BossGroups * BossUnitColumns - 1, BossUnitColumns, true));
		TestTrue(TEXT("Pixel cell and gap remain visible"), pixels.Cell > 0 && pixels.Gap > 0);
		for (int32 unit : { 0, 2, 11 })
		{
			const int32 count = ColumnsForWidth(580.f, unit);
			TestTrue(TEXT("Pixel grid stays within width budget"), pixels.ColumnX(count - 1, unit) + pixels.Cell <= 580.f * scale);
			for (int32 column = 1; column < count; ++column)
				TestEqual(TEXT("Uniform physical pixel gaps"), pixels.ColumnX(column, unit) - pixels.ColumnX(column - 1, unit) - pixels.Cell,
					unit > 0 && column % unit == 0 ? pixels.GroupGap : pixels.Gap);
		}
	}
	for (int32 unit : { 0, 2, 11 })
	{
		const int32 columns = ColumnsForWidth(320.f, unit);
		TestTrue(TEXT("Grid fits width without stretching cells"), ColumnX(columns - 1, unit) + CellSize <= 320.f);
		if (unit) TestEqual(TEXT("Whole units only"), columns % unit, 0);
	}
	TestEqual(TEXT("BU/BE group gap"), ColumnX(2, 2) - ColumnX(1, 2) - CellSize, UnitGap);
	TestEqual(TEXT("SH group gap"), ColumnX(11, 11) - ColumnX(10, 11) - CellSize, UnitGap);
	TestEqual(TEXT("Empty"), ColumnFill(0.f, 0, 10), 0.f);
	TestEqual(TEXT("Full last column"), ColumnFill(1.f, 9, 10), 1.f);
	TestEqual(TEXT("Partial column area"), ColumnFill(0.25f, 2, 10), 0.5f);
	TestEqual(TEXT("Same fraction does not light following column"), ColumnFill(0.25f, 3, 10), 0.f);
	TestEqual(TEXT("Exact column boundary"), ColumnFill(0.3f, 3, 10), 0.f);
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
