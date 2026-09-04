#include "Core/Debug/FExecutionCollaborationDebug.h"

#include "Component/CExecutionCollaborationComponent.h"
#include "Core/Debug/FDebugOverlayEventCategory.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FLog.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarExecutionSessionDebugEnabled(
		TEXT("Portfolio.DebugOverlay.ExecutionSession.Enabled"), 0,
		TEXT("Enable Execution Session debug data and world visualization. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarExecutionSessionDrawStartGeometry(
		TEXT("Portfolio.DebugOverlay.ExecutionSession.DrawStartGeometry"), 1,
		TEXT("Draw the current Source Execution start distance and facing geometry. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarExecutionSessionDrawPairLink(
		TEXT("Portfolio.DebugOverlay.ExecutionSession.DrawPairLink"), 1,
		TEXT("Draw the active Execution Session Source-to-Target link. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarExecutionSessionDrawWorldText(
		TEXT("Portfolio.DebugOverlay.ExecutionSession.DrawWorldText"), 1,
		TEXT("Draw active Execution Session state text at the pair midpoint. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarExecutionSessionAudit(
		TEXT("Portfolio.Debug.ExecutionSessionAudit"), 0,
		TEXT("Write Execution Session start and lifecycle diagnostics to the Output Log. 0: disabled, 1: enabled."), ECVF_Default);
#endif

	FString CompactEnumText(const FString& InQualifiedName)
	{
		int32 separatorIndex = INDEX_NONE;
		return InQualifiedName.FindLastChar(TEXT(':'), separatorIndex)
			&& separatorIndex + 1 < InQualifiedName.Len()
			? InQualifiedName.RightChop(separatorIndex + 1)
			: InQualifiedName;
	}

	FString FormatSessionText(const FExecutionCollaborationRuntimeSnapshot& InRuntime)
	{
		if (!InRuntime.bHasActiveSession) return TEXT("None");
		return FString::Printf(TEXT("%s / #%u"), *GetNameSafe(InRuntime.CollaborationContext.SessionId.SourceActor), InRuntime.CollaborationContext.SessionId.Serial);
	}

	FString BuildAuditSummary(const UCExecutionCollaborationComponent* InComponent, const FString& InDetail)
	{
		if (!IsValid(InComponent)) return InDetail;

		const FExecutionCollaborationRuntimeSnapshot runtime = InComponent->GetExecutionCollaborationRuntimeSnapshot();
		const FString baseSummary = runtime.bHasActiveSession
			? FString::Printf(TEXT("Role=%s | State=%s | Outcome=%s | Session=%s | SourceTerminal=%s | TargetTerminal=%s"),
				runtime.bIsSourceRole ? TEXT("Source") : TEXT("Target"),
				*CompactEnumText(UEnum::GetValueAsString(runtime.CollaborationState)),
				*CompactEnumText(UEnum::GetValueAsString(runtime.CollaborationContext.OutcomePolicy)),
				*FormatSessionText(runtime),
				runtime.bSourceActionTerminal ? TEXT("Done") : TEXT("Waiting"),
				runtime.bTargetReactionTerminal ? TEXT("Done") : TEXT("Waiting"))
			: TEXT("State=None");
		return InDetail.IsEmpty() ? baseSummary : FString::Printf(TEXT("%s | %s"), *baseSummary, *InDetail);
	}

	FColor ResolvePairColor(const EExecutionCollaborationState InState)
	{
		switch (InState)
		{
		case EExecutionCollaborationState::Reserved: return FColor::Yellow;
		case EExecutionCollaborationState::Active: return FColor(255, 80, 255);
		case EExecutionCollaborationState::Committed: return FColor(200, 90, 255);
		default: return FColor::White;
		}
	}

	FVector ResolveCharacterBottomAnchor(const AActor* InActor)
	{
		if (!IsValid(InActor)) return FVector::ZeroVector;

		if (const ACharacter* character = Cast<ACharacter>(InActor))
		{
			if (const UCapsuleComponent* capsule = character->GetCapsuleComponent())
			{
				// Keep ground-oriented geometry readable without z-fighting against the floor.
				return capsule->GetComponentLocation() - FVector(0.f, 0.f, capsule->GetScaledCapsuleHalfHeight()) + FVector(0.f, 0.f, 2.f);
			}
		}

		return InActor->GetActorLocation() + FVector(0.f, 0.f, 2.f);
	}
}

bool FExecutionCollaborationDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarExecutionSessionDebugEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FExecutionCollaborationDebug::ShouldDrawStartGeometry()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarExecutionSessionDrawStartGeometry.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FExecutionCollaborationDebug::ShouldDrawPairLink()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarExecutionSessionDrawPairLink.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FExecutionCollaborationDebug::ShouldDrawWorldText()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarExecutionSessionDrawWorldText.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FExecutionCollaborationDebug::ShouldAuditExecutionCollaboration()
{
#if !UE_BUILD_SHIPPING
	return CVarExecutionSessionAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

FExecutionCollaborationDebugSnapshot FExecutionCollaborationDebug::BuildSnapshot(const ACharacter* InOwnerCharacter)
{
	FExecutionCollaborationDebugSnapshot snapshot;
	if (!IsEnabled() || !IsValid(InOwnerCharacter)) return snapshot;

	const UCExecutionCollaborationComponent* collaborationComp = InOwnerCharacter->FindComponentByClass<UCExecutionCollaborationComponent>();
	if (!IsValid(collaborationComp)) return snapshot;

	snapshot.bHasSnapshot = true;
	snapshot.OwnerCharacter = InOwnerCharacter;
	snapshot.Runtime = collaborationComp->GetExecutionCollaborationRuntimeSnapshot();
	collaborationComp->BuildSourceExecutionStartGeometrySnapshot(snapshot.StartGeometry);
	return snapshot;
}

FExecutionCollaborationDebugOverlayDetails FExecutionCollaborationDebug::BuildOverlayDetails(const FExecutionCollaborationDebugSnapshot& InSnapshot)
{
	FExecutionCollaborationDebugOverlayDetails details;
	if (!IsEnabled() || !InSnapshot.bHasSnapshot) return details;

	details.bHasSnapshot = true;
	const FExecutionCollaborationRuntimeSnapshot& runtime = InSnapshot.Runtime;
	if (!runtime.bHasActiveSession)
	{
		details.RoleText = TEXT("None");
		details.StateText = TEXT("None");
		details.OutcomeText = TEXT("None");
		details.SessionText = TEXT("None");
		details.PartnerText = TEXT("None");
		details.ReservationText = TEXT("None");
		details.TerminalText = TEXT("Source: -- | Target: --");
	}
	else
	{
		const FExecutionCollaborationContext& context = runtime.CollaborationContext;
		details.RoleText = runtime.bIsSourceRole ? TEXT("Source") : TEXT("Target");
		details.StateText = CompactEnumText(UEnum::GetValueAsString(runtime.CollaborationState));
		details.OutcomeText = CompactEnumText(UEnum::GetValueAsString(context.OutcomePolicy));
		details.SessionText = FormatSessionText(runtime);
		details.PartnerText = runtime.bIsSourceRole ? GetNameSafe(context.TargetSnapshot.TargetActor) : GetNameSafe(context.SessionId.SourceActor);
		details.ReservationText = FString::Printf(TEXT("Balance #%u | Saved Loop: %.2f s"), context.OpportunityReservation.BalanceLifecycleSerial, context.OpportunityReservation.SuspendedLoopRemainingSeconds);
		details.TerminalText = FString::Printf(TEXT("Source: %s | Target: %s"), runtime.bSourceActionTerminal ? TEXT("Done") : TEXT("Waiting"), runtime.bTargetReactionTerminal ? TEXT("Done") : TEXT("Waiting"));
	}

	if (InSnapshot.StartGeometry.bHasTarget)
	{
		details.GeometryText = FString::Printf(TEXT("%s | Distance: %.0f / %.0f | Angle: %.1f / %.1f"),
			InSnapshot.StartGeometry.bIsValid ? TEXT("Ready") : TEXT("Blocked"),
			InSnapshot.StartGeometry.CurrentDistance,
			InSnapshot.StartGeometry.MaxDistance,
			InSnapshot.StartGeometry.CurrentFacingAngleDegrees,
			InSnapshot.StartGeometry.MaxFacingAngleDegrees);
	}
	else
	{
		details.GeometryText = TEXT("No Target");
	}

	return details;
}

void FExecutionCollaborationDebug::DrawWorldDebug(UWorld* InWorld, const FExecutionCollaborationDebugSnapshot& InSourceSnapshot)
{
#if !UE_BUILD_SHIPPING
	if (!IsEnabled() || !IsValid(InWorld) || !InSourceSnapshot.bHasSnapshot || !IsValid(InSourceSnapshot.OwnerCharacter)) return;

	const FVector sourceLocation = InSourceSnapshot.OwnerCharacter->GetActorLocation();
	const FExecutionStartGeometrySnapshot& geometry = InSourceSnapshot.StartGeometry;
	if (ShouldDrawStartGeometry() && geometry.bHasTarget && IsValid(geometry.TargetActor))
	{
		const FColor geometryColor = geometry.bIsValid ? FColor::Green : FColor::Red;
		const FVector sourceGeometryAnchor = ResolveCharacterBottomAnchor(InSourceSnapshot.OwnerCharacter);
		const FVector targetGeometryAnchor = ResolveCharacterBottomAnchor(geometry.TargetActor);
		DrawDebugCircle(InWorld, sourceGeometryAnchor, geometry.MaxDistance, 48, geometryColor, false, 0.f, 0, 1.25f, FVector::ForwardVector, FVector::RightVector, false);

		const FVector sourceForward = InSourceSnapshot.OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
		const FVector leftBoundary = sourceForward.RotateAngleAxis(-geometry.MaxFacingAngleDegrees, FVector::UpVector) * geometry.MaxDistance;
		const FVector rightBoundary = sourceForward.RotateAngleAxis(geometry.MaxFacingAngleDegrees, FVector::UpVector) * geometry.MaxDistance;
		DrawDebugLine(InWorld, sourceGeometryAnchor, sourceGeometryAnchor + leftBoundary, geometryColor, false, 0.f, 0, 1.5f);
		DrawDebugLine(InWorld, sourceGeometryAnchor, sourceGeometryAnchor + rightBoundary, geometryColor, false, 0.f, 0, 1.5f);
		DrawDebugLine(InWorld, sourceGeometryAnchor, targetGeometryAnchor, geometryColor, false, 0.f, 0, 2.f);
	}

	const FExecutionCollaborationRuntimeSnapshot& runtime = InSourceSnapshot.Runtime;
	if (!runtime.bHasActiveSession || !runtime.bIsSourceRole || !IsValid(runtime.CollaborationContext.TargetSnapshot.TargetActor)) return;

	const FVector targetLocation = runtime.CollaborationContext.TargetSnapshot.TargetActor->GetActorLocation();
	const FVector midpoint = (sourceLocation + targetLocation) * 0.5f;
	const FColor pairColor = ResolvePairColor(runtime.CollaborationState);
	if (ShouldDrawPairLink())
	{
		DrawDebugLine(InWorld, sourceLocation, targetLocation, pairColor, false, 0.f, 0, 3.f);
	}

	if (ShouldDrawWorldText())
	{
		const FString text = FString::Printf(TEXT("[EXEC #%u | %s | %s]\nSource: %s | Target: %s"),
			runtime.CollaborationContext.SessionId.Serial,
			*CompactEnumText(UEnum::GetValueAsString(runtime.CollaborationContext.OutcomePolicy)),
			*CompactEnumText(UEnum::GetValueAsString(runtime.CollaborationState)),
			runtime.bSourceActionTerminal ? TEXT("Done") : TEXT("Waiting"),
			runtime.bTargetReactionTerminal ? TEXT("Done") : TEXT("Waiting"));
		DrawDebugString(InWorld, midpoint, text, nullptr, pairColor, 0.f, false, 1.f);
	}
#endif
}

void FExecutionCollaborationDebug::RecordLifecycleEvent(const UCExecutionCollaborationComponent* InComponent, const TCHAR* InEvent, const FString& InDetail)
{
	if (!IsValid(InComponent)) return;

	const AActor* ownerActor = InComponent->GetOwner();
	if (!IsValid(ownerActor)) return;

	const FExecutionCollaborationRuntimeSnapshot runtime = InComponent->GetExecutionCollaborationRuntimeSnapshot();
	const AActor* sourceActor = runtime.bHasActiveSession ? runtime.CollaborationContext.SessionId.SourceActor : ownerActor;
	const AActor* targetActor = runtime.bHasActiveSession ? runtime.CollaborationContext.TargetSnapshot.TargetActor : nullptr;
	const FString summary = BuildAuditSummary(InComponent, InDetail);
	FDebugOverlaySnapshotStore::AddEvent(ownerActor, DebugOverlayEventCategory::ExecutionSession, InEvent ? InEvent : TEXT("SessionUnknown"), GetNameSafe(ownerActor), GetNameSafe(sourceActor), GetNameSafe(targetActor), summary, ownerActor, sourceActor, targetActor);

	if (!ShouldAuditExecutionCollaboration()) return;
	FLog::Log(FString::Printf(TEXT("[ExecutionCollaboration|%s] Owner=%s | %s"), InEvent ? InEvent : TEXT("Unknown"), *GetNameSafe(ownerActor), *summary));
}

void FExecutionCollaborationDebug::RecordStartTrace(const UObject* InOwnerObject, const TCHAR* InStage, const FString& InDetail)
{
	if (!ShouldAuditExecutionCollaboration()) return;

	FLog::Log(FString::Printf(TEXT("[ExecutionCollaboration|StartTrace] Owner=%s | Stage=%s | %s"),
		*GetNameSafe(InOwnerObject),
		InStage ? InStage : TEXT("Unknown"),
		InDetail.IsEmpty() ? TEXT("None") : *InDetail));
}

void FExecutionCollaborationDebug::RecordOutcomeDamageApplied(const UCExecutionCollaborationComponent* InComponent, const float InAppliedDamage, const bool bInLethal)
{
	RecordLifecycleEvent(InComponent, bInLethal ? TEXT("LethalDamageApplied") : TEXT("StandardDamageApplied"), FString::Printf(TEXT("AppliedDamage=%.2f"), InAppliedDamage));
}
