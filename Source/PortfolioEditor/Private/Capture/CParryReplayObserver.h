#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CParryReplayDiagnostic.h"
#include "CParryReplayObserver.generated.h"

class ACharacter;
class ACWeaponActor;
class UShapeComponent;
class UPrimitiveComponent;
class UCCombatSignalTargetComponent;
struct FCombatSignalTargetPacket;

// Editor-only listener. No writes to action, defense, weapon or HUD state.
UCLASS()
class UCParryReplayObserver : public UObject
{
    GENERATED_BODY()
public:
    bool Bind(ACharacter* Attacker, ACharacter* Defender);
    void Unbind();
    void Sample();
    void RecordInput(double Time, float Delay);
    FString Describe() const;
    bool CanComplete() const;
    PortfolioReplay::FParryReplayDiagnostic Diagnostic;
private:
    bool IsFirstAttack() const;
    double Now() const;
    UFUNCTION() void OnWindowOpened();
    UFUNCTION() void OnWindowClosed();
    UFUNCTION() void OnContact(AActor* Attacker, AActor* Causer, UShapeComponent* Shape,
        AActor* Target, UPrimitiveComponent* Component, int32 BodyIndex, bool bSweep, const FHitResult& Hit);
    void OnAccepted(const FCombatSignalTargetPacket& Packet);
    TWeakObjectPtr<ACharacter> Source, Target;
    TWeakObjectPtr<ACWeaponActor> Weapon;
    TWeakObjectPtr<UCCombatSignalTargetComponent> Signal;
    FDelegateHandle AcceptedHandle;
    bool bBindingIncomplete = false;
    int32 WindowId = INDEX_NONE;
    double InputAt = -1, ContactAt = -1, OpenAt = -1, CloseAt = -1;
    float GuardDelay = -1;
};
