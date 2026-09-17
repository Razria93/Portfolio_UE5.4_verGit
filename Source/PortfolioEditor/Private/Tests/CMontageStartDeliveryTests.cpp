#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/CMontageNotifyProbe.h"
#include "Misc/AutomationTest.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMontageStartDeliveryTest,
	"Portfolio.Animation.Runtime.StartNotifyDelivery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMontageStartDeliveryTest::RunTest(const FString& Parameters)
{
	USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/01_Character/Regacy/Mesh/SK_Mannequin.SK_Mannequin"));
	UAnimMontage* Source = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/04_Montage/Damaged/Default/M_HitReact.M_HitReact"));
	if (!TestNotNull(TEXT("Fixture mesh"), Mesh) || !TestNotNull(TEXT("Fixture montage"), Source)) return false;
	const UWorld::InitializationValues Values = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
	ACharacter* Character = World->SpawnActor<ACharacter>();
	USkeletalMeshComponent* Component = Character->GetMesh();
	Component->SetSkeletalMesh(Mesh);
	Component->SetAnimInstanceClass(UCMontageProbeAnimInstance::StaticClass());
	Component->InitAnim(true);
	UAnimInstance* Anim = Component->GetAnimInstance();
	if (!TestNotNull(TEXT("Real AnimInstance"), Anim)) { World->DestroyWorld(false); return false; }
	for (int32 Mode = 0; Mode < 2; ++Mode)
	{
		UAnimMontage* Montage = DuplicateObject<UAnimMontage>(Source, GetTransientPackage());
		Montage->Notifies.Reset();
		UCMontageNotifyProbe* Probe = NewObject<UCMontageNotifyProbe>(Montage);
		FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
		Event.Notify = Probe;
		Event.Link(Montage, 0.f);
		Event.TriggerTimeOffset = KINDA_SMALL_NUMBER;
		Event.TriggerWeightThreshold = 0.f;
		Event.MontageTickType = Mode == 0 ? EMontageNotifyTickType::Queued : EMontageNotifyTickType::BranchingPoint;
		Montage->RefreshCacheData();
		TestTrue(TEXT("Playback succeeds"), Character->PlayAnimMontage(Montage) > 0.f);
		TestEqual(TEXT("No synchronous new notify during PlayAnimMontage"), Probe->Deliveries, 0);
		Anim->Montage_JumpToSection(Montage->CompositeSections[0].SectionName, Montage);
		TestEqual(TEXT("No synchronous new notify during section jump"), Probe->Deliveries, 0);
		CastChecked<UCMontageProbeAnimInstance>(Anim)->AdvanceProbe(1.f / 30.f);
		AddInfo(FString::Printf(TEXT("Mode=%s DeliveriesAfterUpdate=%d"), Mode == 0 ? TEXT("Queued") : TEXT("BranchingPoint"), Probe->Deliveries));
		TestTrue(TEXT("Probe actually dispatches on animation update"), Probe->Deliveries > 0);
		Anim->Montage_Stop(0.f);
	}
	World->DestroyWorld(false);
	return true;
}
#endif
