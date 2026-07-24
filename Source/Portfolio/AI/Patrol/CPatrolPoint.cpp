#include "AI/Patrol/CPatrolPoint.h"

#include "Components/TextRenderComponent.h"

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
	TextRenderComp->SetWorldSize(50.f);
	TextRenderComp->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	TextRenderComp->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	TextRenderComp->SetText(FText::FromString(TEXT("PatrolPoint")));
}
