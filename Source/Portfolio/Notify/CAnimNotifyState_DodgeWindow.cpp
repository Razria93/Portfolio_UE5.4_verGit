#include "Notify/CAnimNotifyState_DodgeWindow.h"
#include "Animation/AnimMontage.h"

FString UCAnimNotifyState_DodgeWindow::GetNotifyName_Implementation() const
{
	return Kind == EDodgeWindowKind::Perfect ? TEXT("Dodge Perfect Window") : TEXT("Dodge Invulnerability Window");
}

bool UCAnimNotifyState_DodgeWindow::Contains(const UAnimMontage* InMontage, EDodgeWindowKind InKind, float InPosition)
{
	if (!IsValid(InMontage) || !FMath::IsFinite(InPosition)) return false;
	for (const FAnimNotifyEvent& event : InMontage->Notifies)
	{
		const auto* marker = Cast<UCAnimNotifyState_DodgeWindow>(event.NotifyStateClass);
		if (!marker || marker->Kind != InKind) continue;
		const float start = event.GetTime();
		const float end = start + event.GetDuration();
		// Authored boundaries, not engine trigger offsets. End is exclusive.
		if (FMath::IsFinite(start) && FMath::IsFinite(end) && start >= 0.f && end > start
			&& end <= InMontage->GetPlayLength() + KINDA_SMALL_NUMBER
			&& InPosition >= start && InPosition < end) return true;
	}
	return false;
}
