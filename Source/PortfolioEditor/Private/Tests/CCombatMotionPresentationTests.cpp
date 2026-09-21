#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Core/Debug/FDebugOverlayViewDataBuilder.h"
#include "Core/Debug/FDebugOverlayTextFormatter.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FCombatKnockbackDebug.h"
#include "Core/Debug/FActionFacingDebug.h"
#include "Character/Player/CPlayer.h"
#include "Character/Enemy/CEnemy.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace CombatMotionPresentationTest
{
	struct FCVarScope
	{
		IConsoleVariable* Variable;
		FString Previous;
		EConsoleVariableFlags Priority;
		FCVarScope(const TCHAR* Name, int32 Value) : Variable(IConsoleManager::Get().FindConsoleVariable(Name))
		{
			check(Variable);
			Previous = Variable->GetString();
			Priority = static_cast<EConsoleVariableFlags>(Variable->GetFlags() & ECVF_SetByMask);
			Variable->Set(Value, ECVF_SetByCode);
		}
		~FCVarScope()
		{
			Variable->Set(*Previous, ECVF_SetByCode);
			Variable->ClearFlags(ECVF_SetByMask);
			Variable->SetFlags(Priority);
		}
	};

	struct FWorldScope
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		~FWorldScope() { FDebugOverlaySnapshotStore::Reset(World); World->DestroyWorld(false); }
	};

	FDebugOverlayViewData Build(UWorld* World, const APawn* Player, const ACEnemy* Enemy, const FDebugOverlayPanelVisibility& Visibility)
	{
		return FDebugOverlayViewDataBuilder::Build(World, Player, Enemy, {}, {}, {}, {}, {}, {}, {}, {}, Visibility);
	}

	int32 CountText(const FDebugOverlayTextPanels& Panels, const TCHAR* Text)
	{
		int32 count = 0;
		for (const FDebugOverlayTextLine& line : Panels.MainPanel.Lines)
			if (line.Text.Contains(Text)) ++count;
		return count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatMotionPresentationTest, "Portfolio.DebugOverlay.CombatMotion.Presentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatMotionPresentationTest::RunTest(const FString& Parameters)
{
	using namespace CombatMotionPresentationTest;
	FCVarScope knockback(TEXT("Portfolio.DebugOverlay.Knockback.Enabled"), 1);
	FCVarScope facing(TEXT("Portfolio.DebugOverlay.AttackFacing.Enabled"), 1);
	FCVarScope capture(TEXT("Portfolio.DebugOverlay.CaptureEnabled"), 1);
	FWorldScope fixture;
	FDebugOverlayPanelVisibility visibility;
	const auto emptyView = Build(fixture.World, nullptr, nullptr, visibility);
	const auto emptyText = FDebugOverlayTextFormatter::Format(emptyView);
	TestEqual(TEXT("Both actor panels include Knockback"), CountText(emptyText, TEXT("[Knockback]")), 2);
	TestEqual(TEXT("Both actor panels include Attack Facing"), CountText(emptyText, TEXT("[Attack Facing]")), 2);
	TestEqual(TEXT("Missing actors are explicit in each diagnostic"), CountText(emptyText, TEXT("NoActor")), 4);

	ACPlayer* player = fixture.World->SpawnActor<ACPlayer>();
	ACEnemy* enemy = fixture.World->SpawnActor<ACEnemy>();
	if (!TestNotNull(TEXT("Player fixture"), player) || !TestNotNull(TEXT("Enemy fixture"), enemy)) return false;
	FCombatKnockbackDebugRecord record;
	record.Stage = TEXT("Movement"); record.Result = TEXT("Started"); record.Generation = 7;
	record.Context.Spec.Speed = 300.f; record.Context.Spec.Duration = 0.2f;
	record.Context.Direction = FVector::ForwardVector;
	FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(player, record);
	FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(enemy, record);
	FActionFacingDebugRecord facingRecord;
	facingRecord.Result = TEXT("Applied"); facingRecord.Generation = 9;
	facingRecord.ActionKey.ActionType = EActionType::ComboAttack;
	facingRecord.ActionKey.ActionIndex = 2;
	FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(player, facingRecord);
	FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(enemy, facingRecord);
	FDebugOverlaySnapshotStore::AddEvent(player, TEXT("AttackFacing"), TEXT("Applied"), player->GetName(), player->GetName(), enemy->GetName(), TEXT("Presentation fixture"), player, player, enemy);
	const auto visibleText = FDebugOverlayTextFormatter::Format(Build(fixture.World, player, enemy, visibility));
	TestEqual(TEXT("Both actors render actual knockback records"), CountText(visibleText, TEXT("Movement / Started")), 2);
	TestEqual(TEXT("Both actors render actual facing records"), CountText(visibleText, TEXT("Last: Applied")), 2);
	const int32 eventsBefore = FDebugOverlaySnapshotStore::GetRecentEventsCopy(fixture.World, 32).Num();

	visibility.bShowPlayerKnockback = false;
	visibility.bShowEnemyAttackFacing = false;
	const auto sectionText = FDebugOverlayTextFormatter::Format(Build(fixture.World, player, enemy, visibility));
	TestEqual(TEXT("Player Knockback section can hide independently"), CountText(sectionText, TEXT("[Knockback]")), 1);
	TestEqual(TEXT("Enemy Attack Facing section can hide independently"), CountText(sectionText, TEXT("[Attack Facing]")), 1);
	visibility.bShowPlayer = false;
	visibility.bShowEnemy = false;
	const auto hidden = Build(fixture.World, player, enemy, visibility);
	TestEqual(TEXT("Parent toggles hide actor panels"), hidden.ActorPanels.Num(), 0);
	TestEqual(TEXT("Presentation does not mutate event history"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(fixture.World, 32).Num(), eventsBefore);
	TestTrue(TEXT("Presentation does not disable capture"), FDebugOverlaySnapshotStore::IsCollecting());
	FCombatKnockbackDebugHistory history;
	FActionFacingDebugRecord lastFacing;
	TestTrue(TEXT("Hidden sections retain knockback diagnostics"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(player, history));
	TestEqual(TEXT("Hidden sections preserve generation"), history.Last.Generation, uint64(7));
	TestTrue(TEXT("Hidden sections retain facing diagnostics"), FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(enemy, lastFacing));
	TestEqual(TEXT("Hidden sections preserve facing generation"), lastFacing.Generation, uint64(9));

	visibility = {};
	knockback.Variable->Set(0, ECVF_SetByCode);
	facing.Variable->Set(0, ECVF_SetByCode);
	const auto disabled = FDebugOverlayTextFormatter::Format(Build(fixture.World, player, enemy, visibility));
	TestEqual(TEXT("Disabled domain omits Knockback section"), CountText(disabled, TEXT("[Knockback]")), 0);
	TestEqual(TEXT("Disabled domain omits Attack Facing section"), CountText(disabled, TEXT("[Attack Facing]")), 0);
	TestEqual(TEXT("Domain presentation gates do not erase events"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(fixture.World, 32).Num(), eventsBefore);
	return true;
}
#endif
