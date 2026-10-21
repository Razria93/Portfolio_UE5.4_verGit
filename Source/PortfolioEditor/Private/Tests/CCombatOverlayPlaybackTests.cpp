#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Type/CCombatOverlayPlayback.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatOverlayOwnershipTest, "Portfolio.Combat.Presentation.OverlayOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatOverlayOwnershipTest::RunTest(const FString& Parameters)
{
	UMaterialInterface* material = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/CharacterOverlay/Materials/Material_Instance/MI_Fresnel"));
	if (!TestNotNull(TEXT("Restored Fresnel asset"), material)) return false;
	USkeletalMeshComponent* first = NewObject<USkeletalMeshComponent>();
	USkeletalMeshComponent* second = NewObject<USkeletalMeshComponent>();
	UMaterial* previous = NewObject<UMaterial>();
	UMaterial* foreign = NewObject<UMaterial>();
	first->SetOverlayMaterial(previous);
	FCombatOverlayPlayback playback;
	TestTrue(TEXT("Start modular overlays"), playback.Start(first, {first, second, first}, material, TEXT("Alpha")));
	TestEqual(TEXT("Duplicate mesh is leased once"), playback.Leases.Num(), 2);
	TestTrue(TEXT("Separate mesh instances"), first->GetOverlayMaterial() != second->GetOverlayMaterial());
	playback.SetWeight(.35f);
	float weight = -1.f;
	first->GetOverlayMaterial()->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Alpha")), weight);
	TestEqual(TEXT("Weight reaches material parameter"), weight, .35f);
	first->SetOverlayMaterial(foreign);
	playback.SetWeight(.7f);
	TestEqual(TEXT("Foreign owner is dropped"), playback.Leases.Num(), 1);
	playback.Restore();
	TestTrue(TEXT("Foreign overlay preserved"), first->GetOverlayMaterial() == foreign);
	TestNull(TEXT("Originally empty overlay restored"), second->GetOverlayMaterial());
	TestTrue(TEXT("Restart"), playback.Start(first, {first}, material, TEXT("Alpha")));
	playback.Restore();
	TestTrue(TEXT("Saved overlay restored"), first->GetOverlayMaterial() == foreign);
	TestFalse(TEXT("Reject wrong parameter"), playback.Start(first, {first}, material, TEXT("NotAnExistingParameter")));
	TestTrue(TEXT("Failed start does not alter mesh"), first->GetOverlayMaterial() == foreign);
	return true;
}
#endif
