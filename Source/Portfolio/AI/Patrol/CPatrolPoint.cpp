#include "AI/Patrol/CPatrolPoint.h"

#include "Components/TextRenderComponent.h"

namespace
{
	constexpr float PatrolPointTextWorldSize = 50.f;
	constexpr float PatrolPointTextHeight = 200.f;
	constexpr float PatrolPointTextFacingYaw = 180.f;
}

ACPatrolPoint::ACPatrolPoint()
{
	USceneComponent* rootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	check(rootComp);

	SetRootComponent(rootComp);

	TextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text"));
	check(TextRenderComp);

	TextRenderComp->SetupAttachment(rootComp);

	TextRenderComp->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextRenderComp->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextBottom);
	TextRenderComp->SetWorldSize(PatrolPointTextWorldSize);
	TextRenderComp->SetRelativeLocation(FVector(0.f, 0.f, PatrolPointTextHeight));
	TextRenderComp->SetRelativeRotation(FRotator(0.f, PatrolPointTextFacingYaw, 0.f));
	TextRenderComp->SetText(FText::FromString(TEXT("PatrolPoint")));
}
