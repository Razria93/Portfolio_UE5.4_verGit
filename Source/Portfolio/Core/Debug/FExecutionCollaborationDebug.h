#pragma once

#include "CoreMinimal.h"
#include "Type/CExecutionCollaborationTypes.h"

class ACharacter;
class UWorld;
class UCExecutionCollaborationComponent;

struct FExecutionCollaborationDebugSnapshot
{
	bool bHasSnapshot = false;
	const ACharacter* OwnerCharacter = nullptr;
	FExecutionCollaborationRuntimeSnapshot Runtime;
	FExecutionStartGeometrySnapshot StartGeometry;
};

struct FExecutionCollaborationDebugOverlayDetails
{
	bool bHasSnapshot = false;
	FString RoleText;
	FString PartnerText;
	FString StateText;
	FString OutcomeText;
	FString SessionText;
	FString ReservationText;
	FString TerminalText;
	FString GeometryText;
};

class PORTFOLIO_API FExecutionCollaborationDebug
{
public:
	// Display Gates
	static bool IsEnabled();
	static bool ShouldDrawStartGeometry();
	static bool ShouldDrawPairLink();
	static bool ShouldDrawWorldText();

	// Audit Gate
	static bool ShouldAuditExecutionCollaboration();

	// Snapshot / Presentation
	static FExecutionCollaborationDebugSnapshot BuildSnapshot(const ACharacter* InOwnerCharacter);
	static FExecutionCollaborationDebugOverlayDetails BuildOverlayDetails(const FExecutionCollaborationDebugSnapshot& InSnapshot);
	static void DrawWorldDebug(UWorld* InWorld, const FExecutionCollaborationDebugSnapshot& InSourceSnapshot);

	// Lifecycle Audit
	static void RecordStartTrace(const UObject* InOwnerObject, const TCHAR* InStage, const FString& InDetail = FString());
	static void RecordLifecycleEvent(const UCExecutionCollaborationComponent* InComponent, const TCHAR* InEvent, const FString& InDetail = FString());
	static void RecordOutcomeDamageApplied(const UCExecutionCollaborationComponent* InComponent, float InAppliedDamage, bool bInLethal);
};
