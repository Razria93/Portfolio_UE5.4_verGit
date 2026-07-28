# Debug Overlay P0 SnapshotTypes / SnapshotStore Implementation Plan

## 1. 문서 목적

이 문서는 P0 debug overlay 구현 1단계인 `SnapshotTypes / SnapshotStore`의 구현 계획을 고정한다.

이번 단계의 목적은 HUD를 그리기 전에 overlay가 읽을 데이터 contract를 확정하는 것이다. 이 단계에서는 gameplay state를 새로 소유하거나 복제하지 않는다. SnapshotStore는 제출 영상과 기술문서용 evidence를 위한 개발 전용 최근값 cache다.

코드 구현은 이 문서 이후 단계에서 진행한다.

## 2. 구현 대상 파일

P0 1단계 구현 대상은 다음 3개 파일로 고정한다.

- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp`

HUD renderer 파일인 `CDebugOverlayHUD.h/.cpp`는 다음 단계에서 구현한다.

## 3. 설계 원칙

- Store는 gameplay state의 소유자가 아니다.
- getter로 바로 읽을 수 있는 현재값은 HUD draw 시점에 조회한다.
- event/packet/result처럼 순간적으로 지나가는 값만 Store에 최근값으로 기록한다.
- Store는 `World`별 static map으로 관리한다.
- actor raw pointer를 장기 보관하지 않는다.
- Snapshot 조회 API는 copy를 반환한다.
- Shipping에서는 저장/조회/표시 gate가 no-op 또는 false로 동작한다.
- 기존 `Portfolio.Debug.*Audit` CVar와 overlay 수집 CVar를 결합하지 않는다.

## 4. SnapshotTypes 설계

### 4.1 상태 표현

미수집 상태와 실제 enum 값 `None`을 혼동하지 않기 위해 capture 상태를 별도 enum으로 둔다.

```cpp
enum class EDebugOverlayCaptureState : uint8
{
    NotCaptured,
    Captured,
    Unavailable,
    Stale,
};
```

P0 stale은 시간 기반으로 판정하지 않는다. `Stale`은 World 불일치, 명시 reset 이후 남은 외부 참조, 또는 구현상 stale 판정이 필요한 경우에만 사용한다. 시간 기반 stale threshold는 P1로 넘긴다.

### 4.2 공통 event entry

```cpp
struct FDebugOverlayEventEntry
{
    uint64 FrameNumber = 0;
    float WorldTimeSeconds = 0.0f;
    FString Category;
    FString EventName;
    FString OwnerName;
    FString SourceName;
    FString TargetName;
    FString Summary;
};
```

정책:

- record 시점에 표시용 summary 문자열을 완성한다.
- HUD draw 시점에 packet을 다시 해석하지 않는다.
- actor는 raw pointer가 아니라 `GetNameSafe()` 기반 문자열로 축약한다.
- 필요 시 path name은 P1에서 검토한다.

### 4.3 execution recent summary

```cpp
struct FDebugOverlayExecutionSummary
{
    EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
    uint64 FrameNumber = 0;
    float WorldTimeSeconds = 0.0f;
    FString OwnerName;
    FString Domain;
    FString Decision;
    FString ApplyMode;
    FString RejectReason;
    FString Summary;
};
```

정책:

- Action/Reaction의 현재 active 상태는 HUD draw 시점 getter로 조회한다.
- Store에는 최근 Action/Reaction decision event만 기록한다.
- `ApplyMode`, `RejectReason`은 P0 HUD 필수 표시 항목은 아니지만 event summary 구성에 사용할 수 있다.

### 4.4 combat recent summary

```cpp
struct FDebugOverlayCombatSummary
{
    EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
    uint64 FrameNumber = 0;
    float WorldTimeSeconds = 0.0f;
    FString SourceName;
    FString TargetName;
    FString DamageCauserName;
    int32 HitWindowId = INDEX_NONE;
    FString HitWindowState;
    FString DefenseOutcome;
    bool bHasDamageCommit = false;
    bool bDamageCommitted = false;
    float FinalTakenDamage = 0.0f;
    float CommittedDamage = 0.0f;
    FString Summary;
};
```

정책:

- `DefenseOutcome=None`은 실제 combat 결과일 수 있으므로 미수집 의미로 쓰지 않는다.
- `CaptureState`와 `bHasDamageCommit`으로 값 존재 여부를 분리한다.
- `FinalTakenDamage`, `DamageCommit`은 최근 combat result 기준으로 표시한다.

### 4.5 AI recent summary

```cpp
struct FDebugOverlayAISummary
{
    EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
    uint64 FrameNumber = 0;
    float WorldTimeSeconds = 0.0f;
    FString ControllerName;
    FString PawnName;
    FString TargetName;
    FString Intent;
    FString RequestResult;
    FString RejectReason;
    FString RuntimeLODTier;
    FString Summary;
};
```

정책:

- RuntimeLODTier 현재값은 HUD draw 시점 getter 조회가 우선이다.
- AI combat task success/reject는 EventLog와 최근 AI summary에 기록할 수 있다.
- Runtime LOD interval selection 상세값은 P1 또는 보조 hook으로 유지한다.

### 4.6 전체 snapshot

```cpp
struct FDebugOverlaySnapshot
{
    FDebugOverlayExecutionSummary LastExecution;
    FDebugOverlayCombatSummary LastCombat;
    FDebugOverlayAISummary LastAI;
    TArray<FDebugOverlayEventEntry> RecentEvents;
};
```

Snapshot은 Store 내부 상태의 copy다. HUD는 copy를 읽고 그리는 역할만 가진다.

## 5. SnapshotStore API 설계

`FDebugOverlaySnapshotStore`는 `Core/Debug`의 기존 debug helper와 같은 static helper 형태로 둔다.

### 5.1 Gate API

```cpp
static bool IsEnabled();
static bool IsCollecting();
static int32 GetEventLogDisplayLimit();
```

정책:

- `IsEnabled()`는 `Portfolio.DebugOverlay.Enabled`를 읽는다.
- `IsCollecting()`은 `Portfolio.DebugOverlay.Collect`를 읽는다.
- Shipping에서는 둘 다 false를 반환한다.
- `GetEventLogDisplayLimit()`은 Shipping에서 0을 반환한다.

### 5.2 Record API

```cpp
static void RecordExecutionDecision(const UObject* WorldContextObject, const AActor* OwnerActor, const FString& Domain, const FString& Decision, const FString& ApplyMode, const FString& RejectReason, const TCHAR* EventName);

static void RecordWeaponCollisionWindow(const UObject* WorldContextObject, const AActor* OwnerActor, const AActor* WeaponActor, FName CollisionName, int32 HitWindowId, const FString& HitWindowState, const TCHAR* EventName, const TCHAR* Reason);

static void RecordCombatTargetPacket(const UObject* WorldContextObject, const FCombatSignalTargetPacket& Packet, const TCHAR* EventName);

static void RecordCombatResult(const UObject* WorldContextObject, const AActor* ReceiverActor, const FCombatResultPacket& Packet, const TCHAR* EventName);

static void RecordAICombatTask(const UObject* WorldContextObject, const AAIController* AIController, const APawn* OwnerPawn, const AActor* TargetActor, const FString& Intent, const FString& RequestResult, const FString& RejectReason, const TCHAR* EventName);

static void AddEvent(const UObject* WorldContextObject, const FString& Category, const FString& EventName, const FString& OwnerName, const FString& SourceName, const FString& TargetName, const FString& Summary);
```

정책:

- 모든 Record API는 `WorldContextObject`를 받는다.
- `WorldContextObject`에서 `UWorld*`를 얻지 못하면 no-op 처리한다.
- `IsCollecting()`이 false면 새 기록을 하지 않는다.
- `Collect=0` 전환 시 기존 snapshot은 유지하고 새 기록만 중지한다.
- 기존 snapshot 삭제는 `Reset(WorldContextObject)`로만 수행한다.
- P0에서는 Game Thread 호출을 전제로 한다.
- 비 Game Thread 호출 가능성이 보이면 구현 시 `ensure(IsInGameThread())` 후 no-op을 검토한다.

### 5.3 조회 / reset API

```cpp
static bool GetSnapshotCopy(const UObject* WorldContextObject, FDebugOverlaySnapshot& OutSnapshot);
static TArray<FDebugOverlayEventEntry> GetRecentEventsCopy(const UObject* WorldContextObject, int32 MaxEvents);
static void Reset(const UObject* WorldContextObject);
static void ResetAll();
```

정책:

- `GetSnapshotCopy`는 copy를 반환하고 내부 배열 참조를 노출하지 않는다.
- World entry가 없으면 false를 반환하고 기본 snapshot을 넘긴다.
- `GetRecentEventsCopy`는 최신 event가 위에 오도록 반환한다.
- `Reset`은 해당 World의 snapshot만 제거한다.
- `ResetAll`은 PIE 종료, 테스트 cleanup, 명시 초기화에 사용한다.

## 6. World별 Store Lifetime

Store는 전역 단일 snapshot이 아니라 `World`별 static map으로 둔다.

권장 내부 형태:

```cpp
struct FDebugOverlayWorldStore
{
    FDebugOverlaySnapshot Snapshot;
    TArray<FDebugOverlayEventEntry> EventRing;
    int32 NextEventIndex = 0;
    int32 EventCount = 0;
};

static TMap<TWeakObjectPtr<UWorld>, FDebugOverlayWorldStore> StoresByWorld;
```

정책:

- World별 분리는 PIE, multi PIE, map travel stale 리스크를 낮춘다.
- `TWeakObjectPtr<UWorld>` key 사용 여부는 구현 시 컴파일/해시 지원을 확인한다.
- `TWeakObjectPtr<UWorld>`가 부적절하면 `TObjectKey<UWorld>` 또는 raw `UWorld*` key와 cleanup 정책을 검토한다.
- key 방식이 애매하면 구현 전에 사용자에게 질문한다.

## 7. EventLog Ring Buffer 정책

P0 저장 capacity는 코드 상수 `32`로 고정한다.

```cpp
static constexpr int32 DebugOverlayEventStoreCapacity = 32;
```

`Portfolio.DebugOverlay.EventLogLimit`은 화면 표시 line 수다. 저장 capacity와 혼동하지 않는다.

정책:

- 표시 기본값은 5로 둔다.
- 표시 clamp 범위는 0~5로 둔다.
- 저장 capacity는 32로 둔다.
- overflow 시 가장 오래된 event를 덮어쓴다.
- HUD 반환 순서는 최신 event가 먼저 오도록 한다.
- P0 화면은 3~5 lines 표시를 기본으로 한다.

## 8. CVar 연결

P0 1단계에서 사용할 CVar는 다음으로 고정한다.

- `Portfolio.DebugOverlay.Enabled`
- `Portfolio.DebugOverlay.Collect`
- `Portfolio.DebugOverlay.Preset`
- `Portfolio.DebugOverlay.EventLogLimit`

권장 기본값:

| CVar | 기본값 | 의미 |
| --- | ---: | --- |
| `Portfolio.DebugOverlay.Enabled` | `0` | HUD 표시 비활성 |
| `Portfolio.DebugOverlay.Collect` | `0` | Store 기록 비활성 |
| `Portfolio.DebugOverlay.Preset` | `0` | P0 기본 preset |
| `Portfolio.DebugOverlay.EventLogLimit` | `5` | 화면 표시 event line 수 |

기존 패턴에 맞춰 CVar 선언은 `.cpp`의 anonymous namespace 안에서 `#if !UE_BUILD_SHIPPING`로 감싼다.

## 9. Shipping / Build 정책

Shipping 정책:

- CVar 선언은 `#if !UE_BUILD_SHIPPING` 내부에 둔다.
- public gate API는 Shipping에서 false 또는 0을 반환한다.
- Record API는 Shipping에서 no-op 처리한다.
- 조회 API는 Shipping에서 false 또는 empty copy를 반환한다.
- HUD 표시 경로는 다음 단계에서도 `#if !UE_BUILD_SHIPPING` 보호를 전제로 한다.

Build 정책:

- P0 1단계에서는 `Build.cs` 변경을 하지 않는다.
- `Core`, `CoreUObject`, `Engine` 의존성 내에서 구현한다.
- UMG/Slate dependency를 추가하지 않는다.
- CSV profiler counter를 추가하지 않는다.

## 10. Thread / Access 정책

P0 Store는 Game Thread 전용으로 설계한다.

정책:

- 기존 debug helper들이 `GetValueOnGameThread()`를 사용하므로 overlay store도 Game Thread 호출을 기준으로 둔다.
- lock-free 구조나 `FCriticalSection`은 P0에서 사용하지 않는다.
- 비 Game Thread record 가능성이 확인되면 구현을 멈추고 사용자에게 질문한다.
- HUD는 snapshot copy만 읽는다.

## 11. Hook 연결 계획

1단계 문서는 Store API contract까지만 고정한다. 실제 hook 연결은 다음 구현 단계에서 진행한다.

P0 hook 후보:

- `FExecutionOrchestratorDebug::RecordActionExecutionResultForAudit`
- `FExecutionOrchestratorDebug::RecordReactionExecutionResultForAudit`
- `FCombatSignalDebug::RecordWeaponCollisionWindowForAudit`
- `FCombatSignalDebug::RecordTargetAcceptedForAudit`
- `FCombatSignalDebug::RecordTargetRejectedForAudit`
- `FCombatSignalDebug::RecordCombatResultDispatchForAudit`
- `FCombatResultDebug::RecordCombatResultReceivedForAudit`
- `FAICombatBTDebug::RecordCombatActionTaskSucceededForAudit`
- `FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit`

정책:

- 기존 audit CVar가 꺼져 있어도 `Portfolio.DebugOverlay.Collect=1`이면 overlay store 기록은 가능해야 한다.
- 기존 로그 출력과 overlay 수집 gate를 강하게 결합하지 않는다.
- hook 연결 중 P0 문서 밖 항목이 필요해 보이면 구현을 멈추고 사용자에게 질문한다.

## 12. 값 없음 표시 정책

HUD 표시 문구는 다음 정책을 따른다.

| 상태 | 의미 |
| --- | --- |
| `N/A` | 현재 actor/component에서 조회할 수 없음 |
| `Pending` | 초기화 또는 연결 대기 |
| `NotCaptured` | 최근 event가 아직 기록되지 않음 |
| `None` | 실제 enum/string 값이 None |
| `Unavailable` | 구조상 P0에서 표시하지 않음 |
| `Stale` | World/reset 기준상 신뢰할 수 없는 이전 값 |

P0에서는 `None`을 미수집 의미로 사용하지 않는다.

## 13. 검증 계획

문서 이후 구현 단계에서 다음을 검증한다.

- `FDebugOverlaySnapshotTypes.h` 단독 include 컴파일
- `FDebugOverlaySnapshotStore.h/.cpp` 컴파일
- Shipping guard에서 CVar 미선언 참조가 없는지 확인
- Shipping API가 no-op/false/empty copy인지 확인
- `Build.cs` 변경 없이 빌드되는지 확인
- `Collect=0`에서 새 event가 기록되지 않는지 확인
- `Collect=0` 전환 시 기존 snapshot이 유지되는지 확인
- `Reset(World)`가 해당 World snapshot만 제거하는지 확인
- event log overflow 시 capacity 32를 넘지 않는지 확인
- `EventLogLimit`이 표시 line 수로만 동작하는지 확인
- `GetSnapshotCopy`가 내부 참조가 아닌 copy를 반환하는지 확인

## 14. 구현 중 사용자에게 질문해야 할 항목

다음 상황이 발생하면 임의 결정하지 않고 사용자에게 질문한다.

- World map key로 `TWeakObjectPtr<UWorld>` 사용이 컴파일 또는 해시 정책과 충돌하는 경우
- `TObjectKey<UWorld>` 또는 raw `UWorld*` key로 변경해야 하는 경우
- P0 표시 항목 외 field 추가가 필요해 보이는 경우
- `Build.cs` 의존성 추가가 필요해 보이는 경우
- Game Thread 외 호출 가능성이 확인되는 경우
- EventLog 저장 capacity 32를 바꿔야 할 근거가 생기는 경우
- 기존 debug audit CVar와 overlay collect gate를 결합해야 할 이유가 생기는 경우

## 15. 다음 구현 단계 진입 조건

다음 조건을 만족하면 P0 1단계 구현으로 진입한다.

- 이 문서의 파일명, CVar, Store lifetime 정책이 승인되어 있다.
- Store가 `World`별 static map으로 구현된다는 점이 확정되어 있다.
- `EventLogLimit`은 표시 line 수, 저장 capacity는 32로 확정되어 있다.
- `Collect=0`은 기존 snapshot 유지 + 새 기록 중지로 확정되어 있다.
- P0 stale은 World/Reset 기준으로만 처리한다.
- 코드 구현 중 새 범위를 추가하지 않는다.

## 16. 최종 결정 요약

P0 SnapshotStore는 `Core/Debug` 아래 static helper로 구현한다. 내부 상태는 World별로 분리하고, HUD는 Store의 copy와 gameplay getter current value를 조합해 표시한다.

EventLog는 저장 capacity 32의 ring buffer이며, `Portfolio.DebugOverlay.EventLogLimit`은 화면 표시 line 수만 제어한다. `Collect=0`은 기존 snapshot을 지우지 않고 새 기록만 중지한다. P0 stale은 시간 기반이 아니라 World/Reset 기준으로만 처리한다.
