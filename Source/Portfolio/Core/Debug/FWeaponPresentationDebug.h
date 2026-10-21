#pragma once
#include "CoreMinimal.h"

class UCWeaponComponent;
class UNiagaraComponent;
struct FDebugOverlayTextPanel;

enum class EWeaponDebugChannel : uint8 { Socket, Pivot, Override };

// Read-only diagnostics. Handles are channel-local; they are not action generations.
class PORTFOLIO_API FWeaponPresentationDebug
{
public:
 static bool IsEnabled();
 static bool IsFeedbackEnabled();
 static void Begin(const UCWeaponComponent* Component, EWeaponDebugChannel Channel);
 static void Update(const UCWeaponComponent* Component, EWeaponDebugChannel Channel, float Alpha);
 static void NotifyTime(const UCWeaponComponent* Component, EWeaponDebugChannel Channel, uint32 Handle, float Elapsed, float Duration, const TCHAR* Phase);
 static void End(const UCWeaponComponent* Component, EWeaponDebugChannel Channel, const TCHAR* Result);
 static bool Reject(const UCWeaponComponent* Component, EWeaponDebugChannel Channel, const TCHAR* Reason);
 static void ScopeRestored(const UCWeaponComponent* Component, uint32 Scope, FName Socket, const FQuat& Rotation, bool CanRestore, bool HasSocket);
 static void Feedback(const AActor* Owner, const TCHAR* Route, const TCHAR* Channel, const FString& Detail);
 static void SpawnResult(const AActor* Owner, const TCHAR* Route, const UObject* Asset, const UNiagaraComponent* Spawned);
 static void AppendAndDraw(const AActor* Owner, bool Enemy, FDebugOverlayTextPanel& Panel);
 static void ClearWorld(UWorld* World, bool SessionEnded, bool CleanupResources);
};
