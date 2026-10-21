#include "Misc/AutomationTest.h"
#include "Audit/CDodgeAssetsCommandlet.h"
#include "Audit/DodgeAssetValidation.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CWeaponComponent.h"
#include "Type/CActionDataTypes.h"
#include "Type/CReactionDataTypes.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "DataAsset/CWeaponTrailDataAsset.h"
#include "Notify/CAnimNotify_PerfectDodgePresentation.h"
#include "UObject/UnrealType.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace DodgeAssetValidationTest
{
	template<typename T> const T& ReadField(const UObject* Object, const TCHAR* Name)
	{
		return *FindFProperty<FProperty>(Object->GetClass(), Name)->ContainerPtrToValuePtr<T>(Object);
	}
	AActor* StellarDefaults()
	{
		auto* Blueprint = LoadObject<UBlueprint>(nullptr, TEXT("/Game/01_Character/01_Player/BP_CPlayer_Stellar"));
		return Blueprint && Blueprint->GeneratedClass ? Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject()) : nullptr;
	}
	const FReactionData* PerfectRow(AActor* Defaults)
	{
		const auto* Component = Defaults ? Defaults->FindComponentByClass<UCReactionComponent>() : nullptr;
		return Component ? ReadField<TArray<FReactionData>>(Component, TEXT("ReactionDatas")).FindByPredicate(
			[](const FReactionData& Row) { return Row.ReactionDataKey.ReactionType == EReactionType::PerfectDodge; }) : nullptr;
	}
	const FActionData* RushRow(AActor* Defaults)
	{
		const auto* Component = Defaults ? Defaults->FindComponentByClass<UCActionComponent>() : nullptr;
		return Component ? ReadField<TArray<FActionData>>(Component, TEXT("ActionDatas")).FindByPredicate(
			[](const FActionData& Row) { return Row.ActionDataKey.ActionType == EActionType::CounterRush; }) : nullptr;
	}
	UWeaponTrailDataAsset* ConfiguredTrails(AActor* Defaults)
	{
		const auto* Weapons = Defaults ? Defaults->FindComponentByClass<UCWeaponComponent>() : nullptr;
		if (!Weapons) return nullptr;
		const auto* ClassProperty = FindFProperty<FObjectPropertyBase>(Weapons->GetClass(), TEXT("WeaponActorClass"));
		const auto* Class = Cast<UClass>(ClassProperty->GetObjectPropertyValue_InContainer(Weapons));
		const auto* DataProperty = Class ? FindFProperty<FObjectPropertyBase>(Class, TEXT("WeaponTrailDataAsset")) : nullptr;
		return DataProperty ? Cast<UWeaponTrailDataAsset>(DataProperty->GetObjectPropertyValue_InContainer(Class->GetDefaultObject())) : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgeCurrentAssetReferencesTest, "Portfolio.Combat.Dodge.AssetAudit.CurrentReferences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgeCurrentAssetReferencesTest::RunTest(const FString& Parameters)
{
	auto* Defaults = DodgeAssetValidationTest::StellarDefaults();
	if (!TestNotNull(TEXT("Current Stellar defaults"), Defaults)) return false;
	const bool bWasDirty = Defaults->GetOutermost()->IsDirty();
	FString Error;
	TestTrue(TEXT("Current authored materials, montages, sequences and weapon data are valid"), DodgeAssetValidation::ValidatePlayer(Defaults, true, Error));
	TestTrue(*Error, Error.IsEmpty());
	auto* Commandlet = NewObject<UCDodgeAssetsCommandlet>();
	TestEqual(TEXT("Explicit Verify follows current references"), Commandlet->Main(TEXT("-Verify")), 0);
	AddExpectedError(TEXT("Verify cannot be combined with Apply"), EAutomationExpectedErrorFlags::Contains, 1);
	TestEqual(TEXT("Conflicting flags fail before any authoring mode"), Commandlet->Main(TEXT("-BodyGlowPoints -Verify -Apply")), 1);
	AddExpectedError(TEXT("Full bootstrap overwrites presentation defaults"), EAutomationExpectedErrorFlags::Contains, 1);
	TestEqual(TEXT("Full authoring still requires legacy opt-in"), Commandlet->Main(TEXT("-Apply")), 1);
	TestEqual(TEXT("Validation and rejected commands preserve package dirtiness"), Defaults->GetOutermost()->IsDirty(), bWasDirty);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgePerfectAssetRejectionTest, "Portfolio.Combat.Dodge.AssetAudit.PerfectRejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgePerfectAssetRejectionTest::RunTest(const FString& Parameters)
{
	const auto* Authored = DodgeAssetValidationTest::PerfectRow(DodgeAssetValidationTest::StellarDefaults());
	if (!TestNotNull(TEXT("Authored perfect row"), Authored) || !TestNotNull(TEXT("Authored montage"), Authored->Montage)) return false;
	FString Error;
	FReactionData Row = *Authored;
	Row.Montage = DuplicateObject<UAnimMontage>(Authored->Montage, GetTransientPackage());
	TestTrue(TEXT("A differently named referenced montage is accepted"), DodgeAssetValidation::ValidatePerfectReaction(Row, Error));
	Row.Montage = nullptr;
	TestFalse(TEXT("Missing montage is rejected"), DodgeAssetValidation::ValidatePerfectReaction(Row, Error));
	TestTrue(TEXT("Missing reference has a diagnostic"), Error.Contains(TEXT("montage reference")));
	Row = *Authored; Row.ReactionExecutorKey = nullptr;
	TestFalse(TEXT("Missing executor is rejected"), DodgeAssetValidation::ValidatePerfectReaction(Row, Error));
	Row = *Authored; Row.PlayRate = 0.f;
	TestFalse(TEXT("Zero playback rate is rejected"), DodgeAssetValidation::ValidatePerfectReaction(Row, Error));
	Row = *Authored; Row.StartSectionName = TEXT("MissingAuditSection");
	TestFalse(TEXT("Missing start section is rejected"), DodgeAssetValidation::ValidatePerfectReaction(Row, Error));
	Row = *Authored;
	Row.Montage = DuplicateObject<UAnimMontage>(Authored->Montage, GetTransientPackage());
	Row.Montage->Notifies.RemoveAll([](const FAnimNotifyEvent& Event)
	{
		const auto* Notify = Cast<UCAnimNotify_PerfectDodgePresentation>(Event.Notify);
		return Notify && Notify->Cue == EPerfectDodgePresentationCue::Slow;
	});
	TestFalse(TEXT("Missing slow marker is rejected"), DodgeAssetValidation::ValidatePerfectReaction(Row, Error));
	Row.Montage = DuplicateObject<UAnimMontage>(Authored->Montage, GetTransientPackage());
	const auto* Cue = Row.Montage->Notifies.FindByPredicate([](const FAnimNotifyEvent& Event) { return bool(Cast<UCAnimNotify_PerfectDodgePresentation>(Event.Notify)); });
	if (!TestNotNull(TEXT("Authored cue"), Cue)) return false;
	const FAnimNotifyEvent Duplicate = *Cue;
	Row.Montage->Notifies.Add(Duplicate);
	TestFalse(TEXT("Duplicate presentation marker is rejected"), DodgeAssetValidation::ValidatePerfectReaction(Row, Error));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDodgeRushAssetRejectionTest, "Portfolio.Combat.Dodge.AssetAudit.CounterRejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDodgeRushAssetRejectionTest::RunTest(const FString& Parameters)
{
	auto* Defaults = DodgeAssetValidationTest::StellarDefaults();
	const auto* Authored = DodgeAssetValidationTest::RushRow(Defaults);
	auto* Trails = DodgeAssetValidationTest::ConfiguredTrails(Defaults);
	if (!TestNotNull(TEXT("Authored rush row"), Authored) || !TestNotNull(TEXT("Authored montage"), Authored->Montage)
		|| !TestNotNull(TEXT("Configured weapon trail data"), Trails)) return false;
	FString Error;
	FActionData Row = *Authored;
	Row.Montage = DuplicateObject<UAnimMontage>(Authored->Montage, GetTransientPackage());
	TestTrue(TEXT("Referenced rush montage does not need the bootstrap name"), DodgeAssetValidation::ValidateCounterRush(Row, Trails, Error));
	TestFalse(TEXT("Missing configured trail asset is rejected"), DodgeAssetValidation::ValidateCounterRush(Row, nullptr, Error));
	Row.Montage = nullptr;
	TestFalse(TEXT("Missing rush montage is rejected"), DodgeAssetValidation::ValidateCounterRush(Row, Trails, Error));
	Row = *Authored; Row.ActionExecutorKey = nullptr;
	TestFalse(TEXT("Missing rush executor is rejected"), DodgeAssetValidation::ValidateCounterRush(Row, Trails, Error));
	Row = *Authored; Row.Montage = DuplicateObject<UAnimMontage>(Authored->Montage, GetTransientPackage());
	Row.Montage->SlotAnimTracks.Reset();
	TestFalse(TEXT("No referenced animation segment is rejected"), DodgeAssetValidation::ValidateCounterRush(Row, Trails, Error));
	Row.Montage = DuplicateObject<UAnimMontage>(Authored->Montage, GetTransientPackage());
	if (!TestTrue(TEXT("Authored segment exists"), Row.Montage->SlotAnimTracks.Num() > 0 && Row.Montage->SlotAnimTracks[0].AnimTrack.AnimSegments.Num() > 0)) return false;
	auto& Segment = Row.Montage->SlotAnimTracks[0].AnimTrack.AnimSegments[0];
	auto* Source = Cast<UAnimSequence>(Segment.GetAnimReference());
	if (!TestNotNull(TEXT("Authored sequence"), Source)) return false;
	auto* Sequence = DuplicateObject<UAnimSequence>(Source, GetTransientPackage());
	Segment.SetAnimReference(Sequence, false);
	TestTrue(TEXT("Validation follows a differently named sequence"), DodgeAssetValidation::ValidateCounterRush(Row, Trails, Error));
	Sequence->bForceRootLock = false;
	TestFalse(TEXT("Invalid root-lock setting is rejected on the actual referenced sequence"), DodgeAssetValidation::ValidateCounterRush(Row, Trails, Error));
	Row = *Authored;
	auto* EmptyTrails = DuplicateObject<UWeaponTrailDataAsset>(Trails, GetTransientPackage());
	EmptyTrails->TrailDefinitions.Reset();
	TestFalse(TEXT("No matching configured trail definition is rejected"), DodgeAssetValidation::ValidateCounterRush(Row, EmptyTrails, Error));
	auto* MissingSystems = DuplicateObject<UWeaponTrailDataAsset>(Trails, GetTransientPackage());
	for (auto& Definition : MissingSystems->TrailDefinitions) Definition.NiagaraSystems.Reset();
	TestFalse(TEXT("Trail with no Niagara systems is rejected"), DodgeAssetValidation::ValidateCounterRush(Row, MissingSystems, Error));
	return true;
}
#endif
