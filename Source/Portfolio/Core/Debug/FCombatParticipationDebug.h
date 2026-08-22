#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatParticipationTypes.h"

class UWorld;

struct FCombatParticipationDebugOverlayDetails
{
	bool bHasSnapshot = false;
	FString RoleText;
	FString AdmissionText;
	FString EvidenceText;
	FString PerceptionLifetimeText;
	FString HitReactiveLifetimeText;
	FString TargetText;
	FString AssignmentRevisionText;
	FString RetentionText;
};

class PORTFOLIO_API FCombatParticipationDebug
{
public:
	static bool IsEnabled();
	static bool ShouldDrawWorldText();
	static bool ShouldDrawWorldRing();
	static bool ShouldDrawHitReactiveEvidenceAnchor();

	static FCombatParticipationDebugOverlayDetails BuildOverlayDetails(const FCombatParticipationDebugSnapshot& InSnapshot, const AActor* InParticipantActor);
	static TArray<FString> BuildWorldSummaryLines(const FCombatParticipationDebugSnapshot& InSnapshot);
	static void DrawWorldDebug(UWorld* InWorld, const FCombatParticipationDebugSnapshot& InSnapshot);
};
