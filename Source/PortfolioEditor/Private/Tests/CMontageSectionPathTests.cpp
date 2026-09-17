#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Audit/MontageSectionPath.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMontageSectionPathTest,
	"Portfolio.Animation.Audit.SectionPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMontageSectionPathTest::RunTest(const FString& Parameters)
{
	using namespace ExecutionMontageAudit;
	UAnimMontage* Montage = NewObject<UAnimMontage>();
	Montage->AddAnimCompositeSection(TEXT("A"), 0.f);
	Montage->AddAnimCompositeSection(TEXT("B"), 1.f);
	Montage->AddAnimCompositeSection(TEXT("C"), 2.f);
	Montage->CompositeSections[0].NextSectionName = TEXT("B");
	Montage->CompositeSections[1].NextSectionName = TEXT("C");
	Montage->CompositeSections[2].NextSectionName = NAME_None;
	const FSectionPath Default = BuildSectionPath(Montage, NAME_None, 1.f);
	TestEqual(TEXT("Default start reaches all sections"), Default.Sections.Num(), 3);
	const FSectionPath FromB = BuildSectionPath(Montage, TEXT("B"), 1.f);
	TestEqual(TEXT("Explicit start skips A"), FromB.Sections.Num(), 2);
	TestFalse(TEXT("Earlier Complete is unreachable"), FromB.ContainsTime(Montage, 0.5f));
	TestTrue(TEXT("Complete in B is reachable"), FromB.ContainsTime(Montage, 1.5f));
	TestFalse(TEXT("Invalid start is rejected"), BuildSectionPath(Montage, TEXT("Missing"), 1.f).Error.IsEmpty());
	Montage->CompositeSections[1].NextSectionName = NAME_None;
	TestEqual(TEXT("Disconnected C is skipped"), BuildSectionPath(Montage, TEXT("B"), 1.f).Sections.Num(), 1);
	Montage->CompositeSections[1].NextSectionName = TEXT("A");
	const FSectionPath Loop = BuildSectionPath(Montage, TEXT("B"), 1.f);
	TestTrue(TEXT("Backward loop detected"), Loop.bLoops);
	TestEqual(TEXT("Loop traversal terminates"), Loop.Sections.Num(), 2);
	Montage->CompositeSections[1].NextSectionName = TEXT("Missing");
	TestFalse(TEXT("Broken link rejected"), BuildSectionPath(Montage, TEXT("B"), 1.f).Error.IsEmpty());
	TestFalse(TEXT("Reverse playback is not silently passed"), BuildSectionPath(Montage, NAME_None, -1.f).Error.IsEmpty());
	return true;
}
#endif
