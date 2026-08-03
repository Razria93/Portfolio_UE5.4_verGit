# Debug Overlay Focus Resolver / Terminology Design

## 1. 목적

이 문서는 Debug Overlay의 Enemy 표시 대상 선택 구조를 리팩터링할 때 참고하기 위한 설계 메모이다.

현재 runtime HUD에는 `EnemySource`, `EnemyTarget`, `EnemySelect` 표기가 존재한다. 이 표기는 동작 자체에는 문제가 없지만, 다음 문제가 있다.

- `Source` / `Target` 용어가 Combat pipeline의 `SourceActor` / `TargetActor` 의미와 충돌한다.
- `EnemyTarget: Selected: BP_Enemy_C_2`처럼 label 안에 다시 `Selected:`가 들어가 읽기 어렵다.
- 성공 케이스에서 현재 선택 대상과 마지막 선택 명령 결과가 같은 Actor 이름을 반복한다.
- HUD가 장기적으로 ViewData / Renderer / UMG override 구조로 분리될 때 의미 단위가 불명확하다.

따라서 Debug Overlay가 현재 보고 있는 대상을 `Target`이 아니라 `Focus`로 표현하고, "현재 focus 상태"와 "마지막 focus 명령 결과"를 분리한다.

## 2. 현재 구조 요약

현재 실제로 채택된 선택 흐름은 다음과 같다.

```text
Editor Tooling / Console Command
        ↓
ACPlayerController
        ↓
UCDebugOverlayTargetComponent
        ↓
ACDebugOverlayHUD::ResolveTargetComponentEnemy()
        ↓
HUD Enemy Panel 표시
```

현재 사용 중인 선택 정책은 두 가지이다.

- Nearest 선택
  - `DebugOverlaySelectNearestTarget`
  - `ACPlayerController::TrySelectDebugOverlayNearestEnemy()`
  - `ACPlayerController::FindClosestDebugOverlayEnemy()`
  - 성공 시 `UCDebugOverlayTargetComponent::SetDebugOverlayTarget(..., EDebugOverlayTargetSource::Nearest)`

- Editor Outliner 선택
  - Editor plugin이 선택 Actor 이름으로 `DebugOverlaySelectActorTarget ActorName` console command 실행
  - `ACPlayerController::TrySelectDebugOverlayActorTarget()`
  - `ACPlayerController::FindDebugOverlayActorByName()`
  - `ACEnemy` 검증 후 `SetDebugOverlayTarget(..., EDebugOverlayTargetSource::EditorSelection)`

HUD는 target을 직접 찾지 않고 다음 값을 읽는다.

```cpp
ACEnemy* targetEnemy = Cast<ACEnemy>(targetComp->GetDebugOverlayTargetActor());
if (!IsValid(targetEnemy)) return nullptr;
```

즉 현재 실제 책임은 다음과 같다.

```text
ACPlayerController
- target 찾기 정책 실행
- nearest enemy 검색
- actor name 검색
- ACEnemy 검증
- 성공/실패 summary 생성

UCDebugOverlayTargetComponent
- 선택된 actor 저장
- 선택 source 저장
- 마지막 선택 summary 저장

ACDebugOverlayHUD
- 저장된 actor를 읽어서 표시
```

## 3. 현재 HUD 표기 문제

현재 성공 케이스 표기는 다음과 같은 형태이다.

```text
EnemySource: TargetComponent.Nearest
EnemyTarget: Selected: BP_Enemy_C_2
EnemySelect: NearestSelected | Target: BP_Enemy_C_2 | Distance: 847 | Radius: 3000
```

문제점:

- `EnemyTarget: Selected: ...`는 `:`가 두 번 들어가 읽기 불편하다.
- `Selected`는 의미 중복이다. `EnemyTarget` label 자체가 이미 현재 선택 대상을 의미한다.
- `EnemyTarget`의 Actor 이름과 `EnemySelect`의 `Target: ...` 값이 성공 케이스에서 반복된다.
- `Source` / `Target` 용어가 Combat summary의 `Source` / `Target`과 섞인다.

실패 케이스에서는 현재 상태와 마지막 명령 결과를 분리해서 볼 필요가 있다.

```text
EnemySource: None
EnemySelect: EditorSelectFailed | NotEnemy | Target: BP_Chest_C_1
```

이 경우 `EnemySource: None`은 현재 focus 대상이 없다는 뜻이고, `EnemySelect`는 마지막 선택 명령이 왜 실패했는지 보여주는 evidence이다.

따라서 성공 케이스의 중복을 없애기 위해 단순히 `EnemySelect`를 제거하면 실패 reason을 잃을 수 있다. 반대로 `EnemyTarget`만 남기면 마지막 명령 실패 이유를 보여줄 수 없다.

## 4. 권장 용어: Target 대신 Focus

Debug Overlay의 표시 대상은 gameplay combat target이 아니라 HUD가 현재 집중해서 보여주는 Actor이다. 따라서 Debug Overlay 내부 용어는 `Target`보다 `Focus`가 적합하다.

권장 표시:

```text
EnemyFocusMode: NearestEnemy
EnemyFocusActor: BP_Enemy_C_2
EnemyFocusCommand: Selected | Actor: BP_Enemy_C_2 | Distance: 847 | Radius: 3000
```

Editor Outliner 성공:

```text
EnemyFocusMode: EditorSelection
EnemyFocusActor: BP_Enemy_C_5
EnemyFocusCommand: Selected | Actor: BP_Enemy_C_5
```

Editor Outliner 실패:

```text
EnemyFocusMode: None
EnemyFocusActor: None
EnemyFocusCommand: Failed | Actor: BP_Chest_C_1 | Reason: NotEnemy
```

Nearest 실패:

```text
EnemyFocusMode: None
EnemyFocusActor: None
EnemyFocusCommand: Failed | Reason: NoEnemy | Radius: 3000
```

주의:

- 이 표기 변경은 HUD 표시 문자열 변경이다.
- 기존 low-risk cleanup 범위에는 포함하지 않는다.
- ViewData 도입 또는 별도 terminology cleanup PR에서 처리한다.

## 5. ViewData 후보

현재 focus 상태와 마지막 focus 명령 결과는 분리해서 보관한다.

```cpp
struct FDebugOverlayFocusViewData
{
    FString FocusMode;          // NearestEnemy / EditorSelection / None
    FString FocusActorName;     // BP_Enemy_C_2 / None

    FString LastFocusCommand;   // Selected / Failed / Cleared
    FString LastFocusActorName; // BP_Enemy_C_2 / BP_Chest_C_1 / None
    FString LastFocusReason;    // Distance: 847 | Radius: 3000 / NotEnemy / NoEnemy / OutOfRange
};
```

의미:

- `FocusMode`
  - 현재 HUD가 보고 있는 대상이 어떤 방식으로 선택됐는지 나타낸다.
- `FocusActorName`
  - 현재 HUD가 실제로 보고 있는 Actor 이름이다.
- `LastFocusCommand`
  - 마지막 focus 선택 명령의 결과 상태이다.
- `LastFocusActorName`
  - 마지막 명령이 대상으로 삼았거나 선택한 Actor 이름이다.
- `LastFocusReason`
  - 성공 근거 또는 실패 이유이다.

성공 케이스에서는 `FocusActorName`과 `LastFocusActorName`이 같을 수 있다. 이 중복은 데이터 문제가 아니라 "현재 상태"와 "마지막 명령 결과"라는 서로 다른 의미를 유지하기 위한 것이다. UMG에서는 필요에 따라 마지막 명령 결과를 접거나 축약해서 표시할 수 있다.

## 6. 저장 컴포넌트 이름 제안

현재 이름:

```cpp
UCDebugOverlayTargetComponent
```

권장 장기 이름:

```cpp
UCDebugOverlayFocusComponent
```

이유:

- Debug Overlay의 표시 대상은 combat target과 다르다.
- `Focus`는 "HUD가 현재 관찰하는 Actor"라는 의미가 명확하다.
- 나중에 Enemy뿐 아니라 Interaction actor, AI pawn, gameplay target 등으로 확장하기 쉽다.

단, class/file rename은 변경 범위가 크므로 별도 PR로 분리한다.

## 7. 저장 컴포넌트 책임

Focus component는 target을 직접 찾지 않는다. 찾기 정책은 외부 resolver 또는 controller helper가 담당하고, focus component는 결과만 저장한다.

권장 책임:

```text
UCDebugOverlayFocusComponent
- 현재 FocusActor 저장
- FocusMode 저장
- 마지막 focus command result 저장
- Clear
- Query
```

비권장 책임:

```text
UCDebugOverlayFocusComponent
- World scan
- Store 조회
- RecentCombat pair 해석
- Actor iteration
- Gameplay targeting 정책 구현
```

이 책임을 component에 넣으면 component가 상태 저장소가 아니라 selection service가 되어 다시 비대해진다.

## 8. Focus Component API 제안

기존 API:

```cpp
bool HasDebugOverlayTarget() const;
AActor* GetDebugOverlayTargetActor() const;
FString GetDebugOverlayTargetSummary() const;
FString GetDebugOverlayTargetSource() const;
FString GetDebugOverlaySelectionSummary() const;

void SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayTargetSource InSource);
void ClearDebugOverlayTarget();
void SetDebugOverlaySelectionSummary(const FString& InSummary);
void ClearDebugOverlaySelectionSummary();
```

권장 장기 API:

```cpp
bool HasDebugOverlayFocusActor() const;
AActor* GetDebugOverlayFocusActor() const;
EDebugOverlayFocusMode GetDebugOverlayFocusMode() const;
const FDebugOverlayFocusCommandResult& GetLastDebugOverlayFocusCommandResult() const;

void SetDebugOverlayFocusActor(AActor* InFocusActor, EDebugOverlayFocusMode InMode);
void ClearDebugOverlayFocusActor();

void SetLastDebugOverlayFocusCommandResult(const FDebugOverlayFocusCommandResult& InResult);
void ClearLastDebugOverlayFocusCommandResult();

void ApplyDebugOverlayFocusResolveResult(const FDebugOverlayFocusResolveResult& InResult);
```

`ApplyDebugOverlayFocusResolveResult()`는 controller가 resolver 결과를 component에 적용하는 단일 entry로 사용할 수 있다.

## 9. Focus Mode enum 제안

현재 enum:

```cpp
enum class EDebugOverlayTargetSource : uint8
{
    None,
    Nearest,
    EditorSelection,
};
```

권장 enum:

```cpp
enum class EDebugOverlayFocusMode : uint8
{
    None,
    NearestEnemy,
    EditorSelection,
    RecentCombat,
    WorldScanFallback,
    GameplayTarget,
};
```

비고:

- `NearestEnemy`는 `Nearest`보다 의미가 명확하다.
- `GameplayTarget`은 나중에 실제 gameplay targeting component가 생겼을 때 adapter mode로 사용한다.
- `RecentCombat`과 `WorldScanFallback`은 현재 HUD 파일에 함수가 남아 있지만 실제 `ResolveDisplayEnemy()` 흐름에서는 호출되지 않는다. 채택 여부는 별도 결정이 필요하다.

## 10. Focus Command Result 제안

```cpp
struct FDebugOverlayFocusCommandResult
{
    FString CommandName; // SelectNearestEnemy / SelectEditorActor / Clear
    FString Status;      // Selected / Failed / Cleared
    FString ActorName;   // BP_Enemy_C_2 / BP_Chest_C_1 / None
    FString Reason;      // Distance: 847 | Radius: 3000 / NotEnemy / NoEnemy / OutOfRange
};
```

이 구조는 현재 `EnemySelect: ...` 문자열을 구조화한 것이다.

주의:

- Store schema와 혼동하지 않는다.
- EventLog record schema로 승격하지 않는다.
- HUD/UMG 표시용 focus command result로 제한한다.

## 11. Focus Resolver 책임

찾기 정책은 `FDebugOverlayFocusResolver` 또는 controller helper가 담당한다.

권장 장기 타입:

```cpp
class FDebugOverlayFocusResolver
{
public:
    static FDebugOverlayFocusResolveResult ResolveNearestEnemy(
        const UWorld* InWorld,
        const APawn* InViewerPawn,
        float InRadius);

    static FDebugOverlayFocusResolveResult ResolveActorByName(
        const UWorld* InWorld,
        const FString& InActorName);

    static FDebugOverlayFocusResolveResult ResolveRecentCombatEnemy(
        const UWorld* InWorld,
        const APawn* InViewerPawn,
        float InStaleSeconds);

    static FDebugOverlayFocusResolveResult ResolveSingleEnemyFallback(
        const UWorld* InWorld);

    static FDebugOverlayFocusResolveResult ResolveGameplayTarget(
        const APawn* InViewerPawn);
};
```

결과 타입:

```cpp
struct FDebugOverlayFocusResolveResult
{
    bool bResolved = false;
    TWeakObjectPtr<AActor> FocusActor;
    EDebugOverlayFocusMode FocusMode = EDebugOverlayFocusMode::None;
    FDebugOverlayFocusCommandResult CommandResult;
};
```

Resolver는 다음을 수행할 수 있다.

- World actor iteration
- nearest enemy 계산
- actor name / actor label 검색
- `FDebugOverlaySnapshotStore` 조회
- recent combat pair 해석
- gameplay targeting component 조회
- 실패 reason 생성

Resolver는 focus state를 직접 저장하지 않는다. 저장은 focus component가 담당한다.

## 12. Controller 책임

`ACPlayerController`는 console command entry point 역할을 유지한다.

권장 흐름:

```cpp
void ACPlayerController::DebugOverlaySelectNearestTarget()
{
    const FDebugOverlayFocusResolveResult result =
        FDebugOverlayFocusResolver::ResolveNearestEnemy(
            GetWorld(),
            GetPawn(),
            DebugOverlayNearestFocusRadius);

    DebugOverlayFocusComponent->ApplyDebugOverlayFocusResolveResult(result);
}
```

Editor Outliner 선택도 같은 흐름으로 정리한다.

```cpp
void ACPlayerController::DebugOverlaySelectActorTarget(const FString& ActorName)
{
    const FDebugOverlayFocusResolveResult result =
        FDebugOverlayFocusResolver::ResolveActorByName(
            GetWorld(),
            ActorName);

    DebugOverlayFocusComponent->ApplyDebugOverlayFocusResolveResult(result);
}
```

Controller는 명령 진입점과 component 적용을 담당하고, 세부 검색 정책은 resolver로 이동할 수 있다.

## 13. RecentCombat / WorldScanFallback 처리

현재 `CDebugOverlayHUD.cpp`에는 다음 함수가 남아 있다.

```cpp
ResolveRecentCombatEnemy()
ResolveWorldScanFallbackEnemy()
RefreshCachedEnemyIfNeeded()
```

이 함수들은 모두 "어떤 Enemy를 focus할지 찾는 정책"에 해당한다.

그러나 현재 `ResolveDisplayEnemy()`의 실제 흐름에서는 `ResolveTargetComponentEnemy()`만 호출한다. 따라서 RecentCombat / WorldScanFallback을 다시 연결하면 HUD 표시 정책 변경이 된다.

권장 판단:

- 자동 fallback으로 HUD에 다시 연결하지 않는다.
- 채택하려면 명시 command로 분리한다.
  - `DebugOverlaySelectRecentCombatTarget`
  - `DebugOverlaySelectSingleEnemyFallback`
- 구현 위치는 HUD가 아니라 `FDebugOverlayFocusResolver` 또는 controller helper이다.
- `RecentCombat`은 Store를 조회할 수 있지만, Store schema/API를 변경하지 않는다.

## 14. Gameplay TargetingComponent 연동 방향

나중에 실제 gameplay targeting system이 생기면 Debug Overlay는 그 targeting 정책을 다시 구현하지 않는다.

권장 구조:

```text
Gameplay TargetingComponent
- 실제 게임 타게팅 정책 담당
- lock-on / aim assist / priority / visibility / input cycling 등 처리
- CurrentTargetActor 보유

DebugOverlay FocusResolver
- Gameplay TargetingComponent에서 CurrentTargetActor 조회
- Debug Overlay focus로 사용할 수 있는지 검증
- FocusResolveResult로 변환

DebugOverlay FocusComponent
- 결과 저장

HUD / UMG
- FocusViewData 표시
```

예상 resolver:

```cpp
static FDebugOverlayFocusResolveResult ResolveGameplayTarget(
    const APawn* InViewerPawn);
```

실패 reason 후보:

- `NoViewerPawn`
- `NoTargetingComponent`
- `NoGameplayTarget`
- `NotEnemy`

성공 mode:

```text
EnemyFocusMode: GameplayTarget
EnemyFocusActor: BP_Enemy_C_2
EnemyFocusCommand: Selected | Actor: BP_Enemy_C_2 | Reason: GameplayTarget
```

## 15. PR 분리 제안

### PR 1: Focus ViewData 용어 도입

목표:

- `FDebugOverlayFocusViewData` 후보를 도입한다.
- 현재 상태와 마지막 명령 결과를 분리한다.
- 기존 HUD 표시 문자열은 가능하면 유지한다.

주의:

- Store schema/API 변경 금지.
- runtime target selection 정책 변경 금지.

### PR 2: HUD 표시 용어 정리

목표:

- `EnemySource` / `EnemyTarget` / `EnemySelect`를 `EnemyFocusMode` / `EnemyFocusActor` / `EnemyFocusCommand`로 전환한다.
- `EnemyTarget: Selected: ...` 표기를 제거한다.
- `Target` 대신 `Actor` 또는 `FocusActor` 용어를 사용한다.

주의:

- 이 PR은 표시 문자열 변경을 포함한다.
- evidence 문서 또는 스크린샷 기준 갱신이 필요할 수 있다.

### PR 3: Focus Component rename

목표:

- `UCDebugOverlayTargetComponent`를 `UCDebugOverlayFocusComponent`로 rename한다.
- API 이름을 `Target`에서 `Focus`로 정리한다.

주의:

- class/file rename 범위가 크므로 단독 PR로 진행한다.
- `Build.cs`, `.uproject`, config 변경 필요 여부를 사전에 확인한다.

### PR 4: Focus Resolver 도입

목표:

- `ACPlayerController`의 nearest/name lookup helper를 resolver로 이동한다.
- Controller는 command entry와 component 적용만 담당한다.

주의:

- Nearest radius / actor name matching 의미 변경 금지.
- Editor Outliner command bridge 의미 변경 금지.

### PR 5: 선택 정책 확장 여부 결정

목표:

- RecentCombat / WorldScanFallback을 실제 command로 채택할지 결정한다.
- Gameplay TargetingComponent가 생긴 뒤 `GameplayTarget` resolver를 추가한다.

주의:

- 자동 fallback 연결은 HUD 표시 정책 변경이므로 별도 승인 필요.

## 16. 변경 금지 / 보류 항목

- 이번 설계는 문서 메모이며 코드 변경을 의미하지 않는다.
- Store public API/schema 변경 금지.
- EventLog filter/noise/collision 의미 변경 금지.
- HUD 표시 정책 변경은 별도 PR로 분리한다.
- `Pannel_01/02/03` 문자열 변경은 별도 판단 전까지 금지한다.
- RecentCombat / WorldScanFallback 자동 연결 금지.
- Gameplay TargetingComponent를 Debug Overlay PR에서 새로 구현하지 않는다.
- TargetComponent가 Store를 직접 조회하도록 만들지 않는다.
- HUD가 gameplay targeting component를 직접 조회하도록 만들지 않는다.

## 17. 최종 권장 구조

최종적으로 다음 책임 경계를 목표로 한다.

```text
FDebugOverlaySnapshotStore
- Debug evidence 저장/조회

FDebugOverlayFocusResolver
- focus actor 검색/해석
- Store 조회 가능
- gameplay targeting component 조회 가능
- 결과를 구조화해서 반환

UCDebugOverlayFocusComponent
- 현재 focus 상태 저장
- 마지막 focus command result 저장

ACPlayerController
- console command entry
- resolver 호출
- focus component 적용

FDebugOverlayViewDataBuilder
- focus component와 snapshot store를 읽어 구조화 ViewData 구성

FDebugOverlayTextFormatter
- 구조화 ViewData를 Canvas fallback용 text panel/line으로 변환

ACDebugOverlayHUD / Canvas Renderer
- C++ fallback 표시

UMG / Blueprint
- ViewData 기반 디자인 override
```

요약하면, 찾기는 resolver가 담당하고 저장은 focus component가 담당한다. Debug Overlay의 표시 대상은 combat target이 아니므로 `Target` 대신 `Focus` 용어를 사용한다.

## 18. DrawHUD 책임 축소 정책

장기적으로 `ACDebugOverlayHUD::DrawHUD()`는 Debug Overlay 전체 파이프라인을 직접 수행하지 않고 orchestration만 담당한다.

목표 형태:

```cpp
void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
    Super::DrawHUD();

    if (!FDebugOverlaySnapshotStore::IsEnabled()) return;

    FDebugOverlayHUDContext context = MakeHUDContext();
    FDebugOverlayViewData viewData =
        FDebugOverlayViewDataBuilder::Build(context);

    FDebugOverlayTextPanels textPanels =
        FDebugOverlayTextFormatter::Format(viewData);

    FDebugOverlayCanvasRenderer::Draw(*this, *Canvas, textPanels);
#endif
}
```

여기서 `DrawHUD()`는 다음 정도만 수행한다.

```text
1. Shipping 제외 / enabled gate 확인
2. 현재 HUD context 수집
3. 표시용 ViewData 생성 요청
4. ViewData를 Canvas fallback용 text panel로 변환
5. Canvas fallback renderer 호출
```

주의:

- `DrawHUD()`가 새 evidence를 캡처하는 구조로 만들지 않는다.
- runtime evidence는 기존처럼 runtime system과 `FDebugOverlaySnapshotStore`에 의해 기록된다.
- `DrawHUD()` 시점에는 이미 기록된 Store 값과 focus 상태를 읽어 현재 frame에 표시할 ViewData로 변환한다.

## 19. DrawHUD에서 분산할 책임

현재 `CDebugOverlayHUD.cpp`의 `DrawHUD()` 주변에는 다음 책임이 섞여 있다.

```text
Gate
Context 수집
Focus 대상 resolve
SnapshotStore 조회
Player / Enemy / Interaction / EventLog line 구성
3-panel layout 계산
visible line clipping
Canvas DrawRect / DrawText
```

리팩터링 후 책임 분산 목표는 다음과 같다.

### 19.1 Runtime evidence 수집

담당:

```text
FDebugOverlaySnapshotStore
```

역할:

```text
Recent Execution 저장/조회
Recent Combat 저장/조회
Recent AI Event 저장/조회
EventLog ring buffer 저장/조회
filter / noise / collision 표시 정책 유지
Snapshot copy 제공
```

금지:

- HUD layout을 알지 않는다.
- UMG widget을 알지 않는다.
- FocusComponent 상태를 직접 소유하지 않는다.
- ViewData formatting 책임을 갖지 않는다.

### 19.2 Focus 대상 찾기

담당:

```text
FDebugOverlayFocusResolver
```

역할:

```text
NearestEnemy
EditorSelection
RecentCombat
WorldScanFallback
GameplayTarget
```

Resolver는 focus actor를 찾고 `FDebugOverlayFocusResolveResult`를 반환한다. 필요하면 Store를 읽거나 gameplay targeting component를 조회할 수 있다.

금지:

- focus 상태를 직접 저장하지 않는다.
- HUD draw helper에 의존하지 않는다.
- Canvas layout을 알지 않는다.

### 19.3 Focus 상태 저장

담당:

```text
UCDebugOverlayFocusComponent
```

역할:

```text
현재 FocusActor 저장
FocusMode 저장
마지막 FocusCommandResult 저장
Clear / Query 제공
```

금지:

- World scan 하지 않는다.
- Store를 직접 조회하지 않는다.
- gameplay targeting 정책을 구현하지 않는다.

### 19.4 명령 진입점

담당:

```text
ACPlayerController
```

역할:

```text
console command / editor command 수신
FocusResolver 호출
FocusComponent에 결과 적용
```

예상 흐름:

```text
DebugOverlaySelectNearestTarget
    -> FDebugOverlayFocusResolver::ResolveNearestEnemy(...)
    -> FocusComponent->ApplyDebugOverlayFocusResolveResult(...)

DebugOverlaySelectActorTarget
    -> FDebugOverlayFocusResolver::ResolveActorByName(...)
    -> FocusComponent->ApplyDebugOverlayFocusResolveResult(...)
```

### 19.5 HUD context 수집

담당:

```text
ACDebugOverlayHUD::MakeHUDContext()
```

후보 구조:

```cpp
struct FDebugOverlayHUDContext
{
    UWorld* World = nullptr;
    UCanvas* Canvas = nullptr;
    APawn* ViewerPawn = nullptr;
    APlayerController* OwningPlayerController = nullptr;
    UCDebugOverlayFocusComponent* FocusComponent = nullptr;
};
```

역할:

```text
현재 draw에 필요한 pointer/context만 수집
```

금지:

- Store 데이터를 포맷하지 않는다.
- ViewData를 만들지 않는다.
- 좌표나 panel geometry를 계산하지 않는다.

### 19.6 표시 데이터 생성

담당:

```text
FDebugOverlayViewDataBuilder
```

역할:

```text
Store / FocusComponent / Actor Component / Blackboard 값을 읽어 구조화 ViewData 생성
Player panel 구성
Enemy panel 구성
FocusViewData 구성
Current AI 구성
Recent AI Event 구성
Recent Execution / Recent Combat 구성
EventLog 구성
Interaction panel 구성
```

ViewDataBuilder는 "무슨 값인가"를 결정한다. "어떤 문장으로 표시할지"와 "어디에 그릴지"는 별도 책임으로 분리한다.

권장 책임 경계:

```text
ViewDataBuilder
- 의미 있는 필드 구성
- capture state / stale 여부 / actor name / numeric value / enum display name 구성
- Canvas line을 직접 만들지 않는 것을 목표로 함

TextFormatter
- 구조화 ViewData를 기존 Canvas fallback 표시 문자열로 변환

CanvasRenderer
- text panel을 받아 배치/클리핑/DrawRect/DrawText 수행
```

초기 이행 단계에서는 기존 line 배열을 임시 ViewData로 감싸는 low-risk 단계가 가능하다.

```cpp
struct FDebugOverlayViewData
{
    TArray<FString> MainPanelLines;
    TArray<FString> EventLogLines;
    TArray<FString> InteractionLines;
};
```

그러나 장기 목표는 Blueprint/UMG override를 위해 구조화 ViewData를 제공하는 것이다.

```cpp
struct FDebugOverlayViewData
{
    FDebugOverlayFocusViewData Focus;
    FDebugOverlayActorPanelViewData Player;
    FDebugOverlayActorPanelViewData Enemy;
    FDebugOverlayEventLogViewData EventLog;
    FDebugOverlayInteractionViewData Interaction;
};
```

금지:

- Canvas 좌표를 계산하지 않는다.
- DrawRect / DrawText를 호출하지 않는다.
- Canvas fallback 문장 구성에 영구적으로 종속되지 않는다.
- Store schema/API를 변경하지 않는다.

### 19.7 Text Formatter

담당:

```text
FDebugOverlayTextFormatter
```

역할:

```text
구조화 ViewData를 Canvas fallback용 text panel로 변환
기존 HUD 표시 label / 순서 / 문구 호환 유지
State: ...
Action: ...
EnemyFocusMode: ...
NoEvents(Filter: ...)
NotCaptured / Stale / NotMatched 등 표시 문구 생성
```

후보 구조:

```cpp
struct FDebugOverlayTextPanels
{
    FDebugOverlayTextPanel MainPanel;
    FDebugOverlayTextPanel EventLogPanel;
    FDebugOverlayTextPanel InteractionPanel;
};
```

TextFormatter는 "문자로 어떻게 표현할까"를 담당한다.

금지:

- Store를 직접 읽지 않는다.
- Actor Component / Blackboard를 직접 읽지 않는다.
- Canvas 좌표를 계산하지 않는다.
- DrawRect / DrawText를 호출하지 않는다.

UMG/Blueprint는 가능한 한 `FDebugOverlayTextPanels`가 아니라 구조화 `FDebugOverlayViewData`를 사용한다. text panel은 C++ Canvas fallback 호환층으로 본다.

구조화 우선 정책:

```cpp
enum class EDebugOverlayTextLineRole : uint8
{
    Normal,
    PanelTitle,
    PanelHeader,
    EventLogHeader,
    Warning,
};

enum class EDebugOverlayTextPanelRole : uint8
{
    Main,
    EventLog,
    Interaction,
};

struct FDebugOverlayTextLine
{
    FString Text;
    FString FullText;
    EDebugOverlayTextLineRole Role = EDebugOverlayTextLineRole::Normal;
};

struct FDebugOverlayTextPanel
{
    EDebugOverlayTextPanelRole Role = EDebugOverlayTextPanelRole::Main;
    TArray<FDebugOverlayTextLine> Lines;
};
```

TextFormatter는 line text와 함께 role을 부여한다.

```text
[Debug Overlay Pannel_01]
-> PanelTitle

[Player] / [Enemy] / [Interaction]
-> PanelHeader

[Event Log: All]
-> EventLogHeader

그 외 일반 값
-> Normal
```

이 구조를 사용하면 Renderer가 문자열 비교로 header를 추측하지 않아도 된다. 기존 Canvas fallback의 `Pannel_01/02/03` 문자열은 유지하되, 해당 line의 의미는 `Role`로 표현한다.

### 19.8 Canvas fallback 렌더링

담당:

```text
FDebugOverlayCanvasRenderer
```

역할:

```text
TextPanels를 받아 C++ Canvas HUD로 표시
3-panel layout 계산
background/header/text draw
line height 계산
visible line clipping
EventLog / Interaction 우측 panel 배치
```

예상 API:

```cpp
FDebugOverlayCanvasRenderer::Draw(
    ACDebugOverlayHUD& Hud,
    UCanvas& Canvas,
    const FDebugOverlayTextPanels& TextPanels);
```

금지:

- Store를 직접 읽지 않는다.
- Actor Component / Blackboard를 직접 읽지 않는다.
- FocusComponent를 직접 읽지 않는다.
- 표시할 데이터의 의미를 새로 판단하지 않는다.
- ViewData의 domain field를 직접 문자열로 조합하지 않는다.

## 19.8.1 Canvas text ellipsis 정책

Canvas fallback에서 text가 panel box를 넘어가면 Renderer가 실제 표시 가능 폭을 기준으로 ellipsis 처리한다.

정책:

```text
TextFormatter
- 표시할 원문 text를 만든다.
- line role을 부여한다.
- 원문 FullText를 보존한다.

CanvasRenderer
- panel width / padding / font scale을 기준으로 실제 text 가용 폭을 계산한다.
- DrawText 전에 text가 가용 폭을 넘는지 측정한다.
- 넘치면 "..."으로 줄여서 그린다.

UMG / Blueprint
- FullText를 tooltip / 상세 표시 / 확장 패널 등에 사용할 수 있다.
```

권장 helper:

```cpp
static FString MakeEllipsizedText(
    UCanvas& InCanvas,
    const FString& InText,
    float InMaxWidth,
    UFont* InFont,
    float InScale);
```

동작:

```text
1. 원문이 max width 안에 들어가면 그대로 반환
2. 초과하면 들어가는 최대 prefix 길이를 찾는다
3. prefix + "..." 반환
4. "..."도 폭에 맞지 않는 극단 케이스에서는 "..." 표시를 우선한다
```

문자 수 기준 truncate는 사용하지 않는 것을 권장한다. 한글/영문/숫자/공백의 폭이 다르고, font scale과 panel width에 따라 실제 렌더링 폭이 달라지기 때문이다. Canvas fallback에서는 `Canvas->StrLen` 또는 동등한 text measurement를 사용해 실제 폭 기준으로 줄인다.

금지:

- TextFormatter가 고정 문자 수로 line을 자르지 않는다.
- ViewDataBuilder가 표시 폭을 고려하지 않는다.
- CanvasRenderer가 Store나 ViewData domain 의미를 보고 생략 여부를 판단하지 않는다.
- Ellipsis가 capture state / event filter / focus mode 의미를 바꾸면 안 된다.

### 19.9 Blueprint / UMG override

담당:

```text
UMG Widget / Blueprint adapter
```

역할:

```text
ViewData를 받아 디자인/배치/색상을 Blueprint에서 override
C++ CanvasRenderer는 fallback 기본값으로 유지
```

Blueprint에는 raw Store snapshot이나 raw weak actor pair를 직접 넘기지 않는다. 다음처럼 표시 안전한 값만 넘긴다.

```text
FocusMode
FocusActorName
LastFocusCommand
HealthText
RecentCombatSummary
EventLogRows
AgeSeconds
bStale
```

## 20. DrawHUD 리팩터링 적용 순서

한 번에 완전 구조화하지 않고 다음 순서로 진행한다.

```text
1. FDebugOverlayHUDContext 도입
2. FDebugOverlayViewData 도입
3. FDebugOverlayFocusViewData / ActorStatusViewData 등 구조화 ViewData 도입
4. FDebugOverlayViewDataBuilder로 domain data 구성 책임 이동
5. FDebugOverlayTextFormatter로 Canvas fallback 문장 구성 책임 이동
6. FDebugOverlayCanvasRenderer로 layout/draw 책임 이동
7. EventLog / Interaction / AI ViewData를 점진적으로 구조화
8. UMG / Blueprint override 검토
```

1차 구현에서 모든 패널을 완전 구조화하지 않아도 된다. 다만 최종 책임 경계는 다음 원칙을 따른다.

```text
ViewDataBuilder
- 데이터를 구조화한다.

TextFormatter
- 구조화 데이터를 fallback text로 바꾼다.

CanvasRenderer
- text를 배치하고 그린다.
```

즉 데이터 구성, 텍스트 변환, 렌더링을 서로 다른 책임으로 둔다.

## 21. Actor Status ViewData 분리 메모

현재 Player / Enemy 공통 상태 line은 `CDebugOverlayHUD.cpp`의 `AppendActorStatusLines()`에서 구성된다.

현재 표시 순서:

```text
State: {StateText}
Action: {ActionText}
Reaction: {ReactionText}
HP: {HealthText}
Stagger: {StaggerText}
Guard: {GuardText}
Movement: {MovementText}
Runtime LOD: {RuntimeLODText}
```

이 순서는 임의 나열이 아니라 다음 묶음을 고려한 배치이다.

```text
상태
- State

액션 / 리액션
- Action
- Reaction

리소스 / 방어 상태
- HP
- Stagger
- Guard

로코모션 상태
- Movement

LOD
- Runtime LOD
```

따라서 ActorStatus ViewData로 분리하더라도 기본 Canvas fallback 표시 순서는 유지한다.

후보 구조:

```cpp
struct FDebugOverlayActorStatusViewData
{
    FString StateText;
    FString ActionText;
    FString ReactionText;
    FString HealthText;
    FString StaggerText;
    FString GuardText;
    FString MovementText;
    FString RuntimeLODText;
};
```

Canvas fallback은 기존 line label과 순서를 유지한다.

```text
State: {StateText}
Action: {ActionText}
Reaction: {ReactionText}
HP: {HealthText}
Stagger: {StaggerText}
Guard: {GuardText}
Movement: {MovementText}
Runtime LOD: {RuntimeLODText}
```

UMG / Blueprint override에서는 같은 ViewData를 받아 섹션 단위로 재배치할 수 있다. 다만 C++ fallback의 기본 표시 정책은 별도 PR 전까지 유지한다.

## 22. Actor Status 값 출처

현재 각 line의 값 출처는 다음과 같다.

```text
State
- UCStateComponent::GetCurrentExecutionState()

Action
- UCActionComponent::IsActive()
- UCActionComponent::GetActiveActionType()
- UCActionComponent::GetActiveActionIndex()
- Guard action이면 ResolveGuardActionPhase()로 Guard In / Guard Out / Guard 표시

Reaction
- UCReactionComponent::IsActive()
- UCReactionComponent::GetActiveReactionType()

HP
- UCHealthComponent::GetCurrentHP()
- UCHealthComponent::GetMaxHP()
- UCHealthComponent::GetDeadState()

Stagger
- 현재 ACPlayer / ACEnemy 직접 cast 후 GetParryResultCount(), GetParryStaggerThreshold() 사용
- 장기적으로는 별도 component 책임으로 이동하는 것이 적절함
- 현재 작업에서는 기존 방식 유지

Guard
- UCDefenseComponent::WantsGuarding()
- UCDefenseComponent::IsGuardingPose()
- UCDefenseComponent::CanGuard()
- UCDefenseComponent::CanParry()
- UCDefenseComponent::CanStartGuard()

Movement
- UCMovementComponent::GetCurrentMovementGait()
- UCMovementComponent::GetCurrentSpeed()
- UCMovementComponent::GetCurrentDirection()
- UCMovementComponent::CanMove()
- UCMovementComponent::IsFalling()

Runtime LOD
- 현재 실제 runtime LOD 값 없음
- 기존처럼 N/A 유지
```

## 23. Actor Status 리팩터링 주의점

다음 표시 정책은 ActorStatus ViewData 분리 중 유지한다.

- component가 없으면 `N/A`를 표시한다.
- action / reaction component가 있으나 active가 아니면 `None`을 표시한다.
- enum은 기존처럼 `CompactEnumText()` 정책을 적용한다.
- Guard action은 `Guard In`, `Guard Out`, `Guard` 예외 표시를 유지한다.
- Runtime LOD는 실제 구현하지 않고 `N/A`를 유지한다.
- Stagger는 현재 Player/Enemy class 직접 접근 방식이지만, 이번 ViewData 분리에서 새 component 설계를 끼워 넣지 않는다.

Stagger 관련 장기 판단:

```text
현재:
- ACPlayer / ACEnemy 직접 cast
- parry result count / stagger threshold 직접 조회

장기:
- Stagger / posture / parry resource 성격의 component로 이동 후보
- Debug Overlay는 해당 component 또는 ViewData field를 읽는 구조로 변경

이번 범위:
- 기존 표시 유지
- 새 stagger component 구현 금지
```

## 24. Enemy AI ViewData 분리 메모

Enemy AI 표시는 현재 두 종류의 정보를 같은 Enemy panel 안에 표시한다.

```text
[Current AI]
- Blackboard에서 읽은 현재 runtime state

[Recent AI Event]
- SnapshotStore에 마지막으로 기록된 AI event evidence
```

두 블록은 이름이 비슷하지만 의미가 다르다.

```text
Current AI = 현재 runtime state
Recent AI Event = 마지막 event evidence
```

따라서 ViewData에서도 두 책임을 분리한다.

```cpp
struct FDebugOverlayEnemyAIViewData
{
    FDebugOverlayCurrentAIViewData Current;
    FDebugOverlayRecentAIEventViewData RecentEvent;
};
```

## 25. Current AI ViewData

현재 `AppendEnemyCurrentAIBlock()`은 `ACEnemy`의 controller를 `ACAIController`로 cast하고, blackboard 값을 직접 읽어 다음 line을 만든다.

현재 Canvas fallback 표시 순서:

```text
[Current AI]
Controller: ...
Pawn: ...
Target: ...
IntentState: ...
ReturnHome: ...
UsePatrol: ...
HasLOS: ...
DistanceToTarget: ...
IsCombatAction: ...
```

권장 구조화 ViewData:

```cpp
struct FDebugOverlayCurrentAIViewData
{
    bool bHasEnemy;
    bool bHasController;
    bool bHasBlackboard;
    bool bHasTargetActor;

    FString ControllerName;
    FString PawnName;
    FString TargetActorName;
    FString IntentStateName;

    bool bReturnHome;
    bool bUsePatrol;
    bool bHasLOS;
    float DistanceToTarget;
    bool bIsCombatAction;
};
```

책임:

```text
ViewDataBuilder
- Enemy / Controller / Blackboard 유효성 확인
- Blackboard 값 읽기
- enum display name 구성
- bHasTargetActor 계산

TextFormatter
- true / false 문자열 변환
- DistanceToTarget float format 적용
- target이 없으면 DistanceToTarget을 N/A로 표시
- 기존 line label / 순서 유지

CanvasRenderer
- AI 값 의미를 모르고 text line만 배치/표시
```

주의:

- `Current AI`의 `Target`은 Blackboard target actor이다.
- Debug Overlay의 `FocusActor`와 다른 개념이다.
- 표기 혼동을 줄이기 위해 구조화 ViewData에서는 `TargetActorName`보다 장기적으로 `AITargetActorName` 이름도 검토할 수 있다.

## 26. Recent AI Event ViewData

현재 `AppendEnemyRecentAIEventBlock()`은 `FDebugOverlaySnapshot::LastAI`를 읽고, 현재 focus enemy와 마지막 AI event pawn이 같은지 확인한다.

현재 표시 상태:

```text
Enemy invalid
-> NoTarget

Snapshot 없음 or LastAI not captured
-> NotCaptured

LastAI.PawnName != selected enemy name
-> NotMatched
-> Selected: ...
-> LastPawn: ...

event age > DebugOverlayRecentAIEventStaleSeconds
-> Stale Time: ...s
-> Last Pawn: ...
-> Note: Not current AI evidence

정상
-> Task: ...
-> Result: ...
-> Age: ...
-> RejectReason: ...
```

stale 기준:

```cpp
DebugOverlayRecentAIEventStaleSeconds = 5.0f
```

권장 구조화 ViewData:

```cpp
struct FDebugOverlayRecentAIEventViewData
{
    EDebugOverlayCaptureState CaptureState;

    bool bHasEnemy;
    bool bMatchedSelectedEnemy;
    bool bStale;

    FString SelectedEnemyName;
    FString LastPawnName;

    FString TaskName;
    FString ResultName;
    float AgeSeconds;
    FString RejectReason;
};
```

책임:

```text
ViewDataBuilder
- Snapshot / LastAI capture state 확인
- 현재 focus enemy 이름과 LastAI.PawnName 비교
- AgeSeconds 계산
- bStale 계산
- TaskName / ResultName / RejectReason 구성

TextFormatter
- NoTarget / NotCaptured / NotMatched / Stale 문구 생성
- Stale Time format 적용
- 정상 상태 line 구성
- 기존 line label / 순서 유지

CanvasRenderer
- Recent AI Event block의 text line만 표시
```

주의:

- `NotMatched`는 오류가 아니라 evidence 경계 표시이다.
- 현재 focus enemy와 마지막 AI event pawn이 다를 수 있음을 보여준다.
- `Stale`은 실패가 아니라 freshness 표시이다.
- 오래된 event를 현재 AI evidence처럼 보이지 않게 하기 위한 표시 정책이다.
- BT active node tracking은 여기서 구현하지 않는다.

## 27. AI 표시 리팩터링 금지/보류 항목

- Current AI와 Recent AI Event를 하나의 의미로 합치지 않는다.
- Blackboard target actor와 Debug Overlay FocusActor를 같은 필드로 취급하지 않는다.
- Recent AI Event의 `NotMatched`, `Stale`, `NotCaptured`, `NoTarget` 문구 의미를 변경하지 않는다.
- stale 기준 5초를 임의 변경하지 않는다.
- BT active node tracking을 이번 구조화 작업에 끼워 넣지 않는다.
- Store schema/API를 변경하지 않는다.
- Canvas fallback line 순서는 별도 표시 정책 PR 전까지 유지한다.

## 28. EventLog / Interaction ViewData 분리 메모

EventLog와 Interaction 패널은 모두 Store evidence를 표시하지만 의미가 다르다.

```text
EventLog
- 여러 event row의 history list
- Category / EventName / Summary 조합
- filter / display limit 적용 결과

Interaction
- Snapshot의 latest summary block
- LastExecution / LastCombat 단일 요약
- CaptureState와 RawSummary가 핵심
```

따라서 ViewData에서도 EventLog와 Interaction을 합치지 않는다.

```cpp
struct FDebugOverlayEventLogViewData;
struct FDebugOverlayInteractionViewData;
```

## 29. EventLog ViewData

현재 EventLog는 `DrawHUD()`에서 Store를 조회한 뒤 `AppendEventLogBlock()`으로 line을 구성한다.

현재 입력:

```cpp
const FString eventLogFilter =
    FDebugOverlaySnapshotStore::GetEventLogFilter();

const int32 eventLogLimit =
    FDebugOverlaySnapshotStore::GetEventLogDisplayLimit();

const TArray<FDebugOverlayEventEntry> recentEvents =
    FDebugOverlaySnapshotStore::GetRecentEventsCopy(
        world,
        eventLogLimit,
        eventLogFilter);
```

현재 Canvas fallback 표시 정책:

```text
[Debug Overlay Pannel_02]

[Event Log: {Filter}]

Snapshot 없음
-> NotCaptured

Limit == 0
-> NoEvents(Filter: {Filter} Limit: 0)

Event 배열 empty
-> NoEvents(Filter: {Filter})

Event 있음
-> {Category}/{EventName}: {Summary}
```

권장 구조화 ViewData:

```cpp
struct FDebugOverlayEventLogRowViewData
{
    FString Category;
    FString EventName;
    FString Summary;
};

struct FDebugOverlayEventLogViewData
{
    bool bHasSnapshot;
    FString FilterName;
    int32 DisplayLimit;
    TArray<FDebugOverlayEventLogRowViewData> Rows;
};
```

책임:

```text
ViewDataBuilder
- EventLog filter / display limit 읽기
- GetRecentEventsCopy() 결과를 row ViewData로 보존
- bHasSnapshot 보존

TextFormatter
- [Event Log: {Filter}] line 생성
- NotCaptured 생성
- NoEvents(Filter: {Filter} Limit: 0) 생성
- NoEvents(Filter: {Filter}) 생성
- {Category}/{EventName}: {Summary} line 생성

CanvasRenderer
- EventLog line의 의미를 모르고 text panel만 표시
```

주의:

- `GetRecentEventsCopy()`가 이미 filter와 limit이 적용된 결과를 반환한다.
- TextFormatter는 event를 다시 필터링하지 않는다.
- `NoEvents(Filter: X Limit: 0)`과 `NoEvents(Filter: X)`는 의미가 다르므로 유지한다.
- `CombatResult` category를 `Combat`으로 합치지 않는다.
- EventLog filter/noise/collision 의미를 변경하지 않는다.

## 30. Interaction / Recent Summary ViewData

현재 Interaction panel은 Snapshot의 latest summary 두 개를 표시한다.

```text
[Debug Overlay Pannel_03]

[Interaction]
[Recent Execution]
...

[Recent Combat]
...
```

현재 구성:

```cpp
AppendSnapshotSummaryBlock(
    Lines,
    TEXT("[Recent Execution]"),
    Snapshot.LastExecution.Summary,
    Snapshot.LastExecution.CaptureState,
    bHasSnapshot,
    false);

AppendSnapshotSummaryBlock(
    Lines,
    TEXT("[Recent Combat]"),
    Snapshot.LastCombat.Summary,
    Snapshot.LastCombat.CaptureState,
    bHasSnapshot,
    true);
```

권장 구조화 ViewData:

```cpp
struct FDebugOverlaySummaryBlockViewData
{
    FString Title;
    EDebugOverlayCaptureState CaptureState;
    FString RawSummary;
};

struct FDebugOverlayInteractionViewData
{
    bool bHasSnapshot;
    FDebugOverlaySummaryBlockViewData RecentExecution;
    FDebugOverlaySummaryBlockViewData RecentCombat;
};
```

책임:

```text
ViewDataBuilder
- bHasSnapshot 보존
- LastExecution CaptureState / RawSummary 보존
- LastCombat CaptureState / RawSummary 보존
- RecentExecution / RecentCombat block title 보존

TextFormatter
- [Interaction] line 생성
- [Recent Execution] / [Recent Combat] title line 생성
- CaptureState에 따라 NotCaptured / Unavailable / Stale 문구 생성
- Captured + RawSummary 있음이면 Canvas fallback용 summary split 수행
- Recent Combat 앞 빈 줄 유지

CanvasRenderer
- Interaction line의 의미를 모르고 text panel만 표시
```

## 31. Summary split 정책

현재 Snapshot summary는 이미 한 줄짜리 표시 요약 문자열로 기록된다.

예:

```text
Pawn: BP_Player | Action: Combo[0] | Result: Accepted
```

Canvas fallback HUD는 이 값을 그대로 한 줄로 표시하지 않고, `" | "` 기준으로 나누어 여러 line으로 표시한다.

```text
Pawn: BP_Player
Action: Combo[0]
Result: Accepted
```

이 split은 데이터 의미 변환이 아니라 표시 방식이다. 따라서 Builder가 아니라 TextFormatter 책임으로 둔다.

정책:

```text
ViewDataBuilder
- RawSummary를 보존한다.
- CaptureState를 보존한다.
- 이 summary가 RecentExecution인지 RecentCombat인지 보존한다.

TextFormatter
- CaptureState가 Captured가 아니면 CaptureStateText를 표시한다.
- Captured이고 RawSummary가 있으면 " | " 기준으로 split한다.
- split 결과를 Canvas fallback line으로 만든다.

CanvasRenderer
- split 여부를 모른다.
- 전달받은 line을 배치/표시만 한다.

UMG / Blueprint
- RawSummary 또는 구조화 field를 사용해 별도 표현을 선택할 수 있다.
```

이 구조를 택하면 같은 데이터라도 UI마다 다른 표시가 가능하다.

```text
Canvas fallback
- " | " 기준 split 후 여러 줄 표시

UMG compact mode
- RawSummary 한 줄 표시

UMG structured mode
- Label / Value grid로 표시

Tooltip
- RawSummary 전체 표시
```

## 32. EventLog / Interaction 리팩터링 금지/보류 항목

- EventLog와 Interaction summary를 하나의 ViewData로 합치지 않는다.
- EventLog row의 `Category / EventName / Summary` 의미를 변경하지 않는다.
- EventLog filter / display limit 의미를 변경하지 않는다.
- `NoEvents(Filter: ... Limit: 0)` 문구 의미를 변경하지 않는다.
- `NoEvents(Filter: ...)` 문구 의미를 변경하지 않는다.
- `NotCaptured`, `Unavailable`, `Stale` capture state 표시를 임의 변경하지 않는다.
- Recent Execution / Recent Combat 순서를 바꾸지 않는다.
- Recent Combat 앞 빈 줄 정책은 Canvas fallback compatibility로 유지한다.
- Summary `" | "` split은 TextFormatter 책임으로 둔다.
- CanvasRenderer가 Store나 Summary 의미를 직접 판단하지 않게 한다.

## 33. Snapshot Store 경계 정책

`FDebugOverlaySnapshotStore`는 Debug Overlay의 모든 화면 데이터를 소유하는 UI model이 아니라, runtime code에서 명시적으로 기록한 debug evidence 저장소이다.

현재 Store 성격:

```text
runtime system에서 hook/record된 evidence 저장
LastExecution / LastCombat / LastAI 저장
EventLog 저장
RecentCombatPair 저장
public copy API 제공
```

현재 Store가 하지 않는 일:

```text
월드 전체 자동 스캔
현재 focus enemy component state 자동 capture
HUD panel line 생성
UMG ViewData 생성
Canvas layout 계산
```

따라서 ViewData 분리 후에도 Store는 data source로만 사용한다.

```text
FDebugOverlaySnapshotStore
-> raw evidence copy 제공

FDebugOverlayViewDataBuilder
-> Store evidence + FocusActor live state를 읽어 구조화 ViewData 생성

FDebugOverlayTextFormatter
-> ViewData를 fallback text로 변환

FDebugOverlayCanvasRenderer
-> text panel을 배치/표시
```

## 34. Store API 섹션 정렬

Store 관련 API는 기능명 나열보다 데이터 섹션 기준으로 정렬해 문서화한다.

### 34.1 Control / CVar

```text
IsEnabled()
- DrawHUD gate

GetEventLogFilter()
- EventLog 표시 filter 조건

GetEventLogDisplayLimit()
- EventLog 표시 limit 조건
```

비고:

- `GetEventLogFilter()`와 `GetEventLogDisplayLimit()`은 CVar/control API이면서 EventLogViewData의 표시 조건으로도 보존된다.

### 34.2 Snapshot

```text
TryGetSnapshotCopy()
- FDebugOverlaySnapshot 복사
- LastExecution
- LastCombat
- LastAI
- RecentEvents
```

용도:

```text
InteractionViewData
- LastExecution / LastCombat RawSummary와 CaptureState 읽기

RecentAIEventViewData
- LastAI Raw data와 CaptureState 읽기
```

주의:

- `Snapshot.RecentEvents`는 snapshot copy에 포함되지만, EventLog panel 표시에는 `GetRecentEventsCopy()`를 사용하는 쪽이 명확하다.

### 34.3 EventLog

```text
GetRecentEventsCopy()
- EventLog panel용 row 조회
- filter / limit 적용 결과
- Category
- EventName
- SubjectName
- Summary
- WorldTimeSeconds
- FrameNumber

GetRecentEventsForSubjectCopy()
- 특정 subject/category 기준 최근 event 조회
- Actor Recent Execution block 등에 사용 가능
```

ViewData 사용:

```text
FDebugOverlayEventLogViewData
- FilterName = GetEventLogFilter()
- DisplayLimit = GetEventLogDisplayLimit()
- Rows = GetRecentEventsCopy()
```

주의:

- ViewDataBuilder / TextFormatter / Renderer에서 EventLog를 다시 필터링하지 않는다.
- Store 조회 결과를 신뢰한다.

### 34.4 RecentCombatPair

```text
TryGetRecentCombatPair()
- FDebugOverlayRecentCombatPair 복사
- SourceActor
- TargetActor
- SourceName
- TargetName
- WorldTimeSeconds
```

용도:

```text
FDebugOverlayFocusResolver
- RecentCombat focus mode에서 사용
- viewer pawn과 source/target pair를 비교해 focus enemy 후보를 판단
```

주의:

- Store가 focus actor를 선택하지 않는다.
- Store는 pair evidence만 제공한다.
- FocusResolver가 stale / validity / match 여부를 판단한다.

## 35. Store와 Enemy live state 경계

현재 Enemy state는 Store snapshot에 저장되지 않는다. 현재 선택된 focus enemy actor를 기준으로 ViewDataBuilder가 component를 live read한다.

현재 구조:

```text
FocusComponent
-> 현재 FocusActor 제공

ViewDataBuilder
-> FocusActor가 ACEnemy인지 확인
-> UCStateComponent / UCActionComponent / UCReactionComponent / UCHealthComponent / UCDefenseComponent / UCMovementComponent live read
-> FDebugOverlayActorStatusViewData 생성
```

Store snapshot으로 확장하는 대안은 다음과 같다.

```text
Focus enemy state를 Store에 별도 snapshot으로 기록
-> Store schema에 FocusEnemyStatus / PlayerStatus 등 추가
-> 누군가 매 frame 또는 특정 시점에 RecordActorStatus() 호출
-> ViewDataBuilder는 Store snapshot만 읽음
```

예상 추가 schema:

```cpp
struct FDebugOverlayActorStatusSnapshot
{
    FString ActorName;
    FString StateName;
    FString ActionName;
    float CurrentHP;
    float MaxHP;
    FString MovementGaitName;
    float MovementSpeed;
    float WorldTimeSeconds;
    uint64 FrameNumber;
};
```

그러나 이 확장은 현재 범위에서 채택하지 않는다.

이유:

```text
Enemy는 여러 명일 수 있다.
Focus 대상이 바뀔 수 있다.
Focus가 없을 수 있다.
Enemy가 destroy될 수 있다.
RecentCombat 대상과 FocusActor가 다를 수 있다.
EditorSelection 대상과 AI event 대상이 다를 수 있다.
```

Store에 focus enemy state를 기록하려면 다음 정책이 추가로 필요하다.

```text
어떤 enemy를 capture할지
언제 capture할지
focus 변경 시 이전 snapshot을 어떻게 처리할지
destroy/stale을 어떻게 처리할지
player status도 Store에 넣을지
component 값과 Store 값의 frame delay를 허용할지
```

따라서 현재 결정:

```text
Enemy current state는 Store에 snapshot으로 넣지 않는다.
FocusActor component live read를 ViewDataBuilder 책임으로 둔다.
Store는 hook/record된 runtime evidence 저장소로 유지한다.
```

## 36. Store API ViewData 확장 보류

ViewData 분리를 위해 Store에 ViewData 전용 API를 추가하지 않는다.

현재 권장 흐름:

```cpp
FDebugOverlaySnapshot Snapshot;
FDebugOverlaySnapshotStore::TryGetSnapshotCopy(World, Snapshot);

TArray<FDebugOverlayEventEntry> Events =
    FDebugOverlaySnapshotStore::GetRecentEventsCopy(World, Limit, Filter);

FDebugOverlayRecentCombatPair Pair;
FDebugOverlaySnapshotStore::TryGetRecentCombatPair(World, Pair);
```

```text
Store
-> raw evidence copy 제공

ViewDataBuilder
-> EventLogViewData / InteractionViewData / RecentAIEventViewData 생성
```

보류하는 확장 예:

```cpp
FDebugOverlaySnapshotStore::GetEventLogViewData(...)
FDebugOverlaySnapshotStore::GetInteractionViewData(...)
FDebugOverlaySnapshotStore::GetEnemyAIViewData(...)
FDebugOverlaySnapshotStore::GetEventLogPanelLines(...)
FDebugOverlaySnapshotStore::GetInteractionPanelLines(...)
```

이런 API는 Store가 ViewData 또는 TextFormatter 책임을 갖게 만들 수 있다.

보류 이유:

```text
Store는 evidence 저장고다.
ViewData는 표시 계층의 data model이다.
Text line은 Canvas fallback 호환 표현이다.
Store가 ViewData/Text line을 만들면 저장/해석/표시 책임이 섞인다.
현재 public copy API만으로 ViewDataBuilder가 필요한 데이터를 만들 수 있다.
```

따라서 현재 결정:

```text
Store schema/API는 ViewData 분리를 위해 확장하지 않는다.
ViewDataBuilder는 Store public copy API만 사용한다.
Store 내부 map/ring buffer/helper에 직접 접근하지 않는다.
```

## 37. RecentCombatPair focus mode 채택

`FDebugOverlayRecentCombatPair`는 focus resolver의 정식 mode로 채택한다.

단, 자동 fallback이 아니라 명시 command/mode로만 사용한다.

권장 흐름:

```text
DebugOverlaySelectRecentCombatTarget
-> FDebugOverlayFocusResolver::ResolveRecentCombatEnemy(...)
-> FDebugOverlaySnapshotStore::TryGetRecentCombatPair(...)
-> viewer pawn과 SourceActor / TargetActor 비교
-> 상대 actor가 ACEnemy이면 FocusMode = RecentCombat
-> FocusComponent->ApplyDebugOverlayFocusResolveResult(...)
```

주의:

- `ResolveDisplayEnemy()`에 자동 fallback으로 다시 연결하지 않는다.
- Store가 focus를 선택하지 않는다.
- Store는 recent combat pair evidence만 제공한다.
- FocusResolver가 stale / invalid / not matched reason을 생성한다.
- `Source` / `Target` 용어는 RecentCombatPair 내부 의미로 유지하되, HUD 표시 대상은 `FocusActor` 용어를 사용한다.

## 38. SnapshotTypes / ViewDataTypes / TextPanelTypes 파일 경계

Debug Overlay 타입은 저장용, 표시용, fallback text용을 파일 단위로 분리한다.

권장 경계:

```text
FDebugOverlaySnapshotTypes.h
- Store 저장용 raw evidence schema

FDebugOverlayViewDataTypes.h
- 구조화된 display data schema

FDebugOverlayTextPanelTypes.h
- Canvas fallback용 text representation schema
```

이 분리는 다음 책임 흐름을 기준으로 한다.

```text
ViewDataBuilder
- FDebugOverlayViewData 생성

TextFormatter
- FDebugOverlayViewData -> FDebugOverlayTextPanels 변환

CanvasRenderer
- FDebugOverlayTextPanels를 받아 그림
```

## 39. FDebugOverlaySnapshotTypes.h 역할

`FDebugOverlaySnapshotTypes.h`는 Store가 저장하고 복사해주는 raw evidence 타입만 담는다.

유지 대상:

```text
EDebugOverlayCaptureState
FDebugOverlayEventEntry
FDebugOverlaySnapshot
FDebugOverlayRecentCombatPair
LastExecution / LastCombat / LastAI 계열 raw snapshot 타입
```

성격:

```text
runtime evidence schema
Store copy API의 반환/출력 schema
record/hook된 데이터의 저장 schema
```

비대상:

```text
FDebugOverlayViewData
FDebugOverlayFocusViewData
FDebugOverlayActorStatusViewData
FDebugOverlayEnemyAIViewData
FDebugOverlayEventLogViewData
FDebugOverlayInteractionViewData
FDebugOverlayTextPanels
FDebugOverlayTextLine
```

즉 SnapshotTypes에 ViewData나 TextPanel 타입을 섞지 않는다.

## 40. SnapshotTypes에서 ViewData로 복사/변환되는 값

`FDebugOverlaySnapshot`은 Store copy 단위이고, ViewDataBuilder가 이를 읽어 표시용 ViewData로 분해한다.

```text
FDebugOverlaySnapshot.LastExecution
-> FDebugOverlayInteractionViewData.RecentExecution

FDebugOverlaySnapshot.LastCombat
-> FDebugOverlayInteractionViewData.RecentCombat

FDebugOverlaySnapshot.LastAI
-> FDebugOverlayEnemyAIViewData.RecentEvent

FDebugOverlaySnapshot.RecentEvents
-> 직접 EventLog panel 주입력으로 사용하지 않음
-> EventLog panel은 GetRecentEventsCopy() 결과 사용
```

`EDebugOverlayCaptureState`는 ViewData로 보존한다.

```text
ViewDataBuilder
- CaptureState 보존

TextFormatter
- NotCaptured / Unavailable / Stale 등 표시 문구로 변환
```

주의:

- Builder가 CaptureState를 `N/A`나 빈 문자열로 조기 변환하지 않는다.
- CaptureState 의미와 표시 문구를 분리한다.

## 41. EventEntry raw schema와 EventLog ViewData

`FDebugOverlayEventEntry`는 Store raw EventLog row이다.

Raw schema:

```text
Category
EventName
SubjectName
Summary
WorldTimeSeconds
FrameNumber
```

ViewData 1차 후보:

```cpp
struct FDebugOverlayEventLogRowViewData
{
    FString Category;
    FString EventName;
    FString Summary;
};
```

필요하면 이후 다음 값을 display-safe field로 추가한다.

```text
SubjectName
AgeSeconds
FrameNumber
```

주의:

- `FDebugOverlayEventEntry`를 UMG/Blueprint에 raw로 직접 넘기지 않는다.
- 필요한 field만 `FDebugOverlayEventLogRowViewData`로 복사한다.
- `{Category}/{EventName}: {Summary}` 조합은 TextFormatter 책임이다.

## 42. RecentCombatPair raw schema와 FocusResolver

`FDebugOverlayRecentCombatPair`는 recent combat evidence raw schema이다.

Raw schema:

```text
SourceActor
TargetActor
SourceName
TargetName
WorldTimeSeconds
```

`SourceActor` / `TargetActor`는 resolver 내부 판단용이다.

```text
FocusResolver
- viewer pawn이 SourceActor이면 TargetActor를 focus 후보로 봄
- viewer pawn이 TargetActor이면 SourceActor를 focus 후보로 봄
- 상대 actor가 ACEnemy이면 RecentCombat focus로 채택
```

Blueprint/UMG로 직접 넘기지 않는 값:

```text
TWeakObjectPtr<AActor> SourceActor
TWeakObjectPtr<AActor> TargetActor
```

표시용으로 필요하면 다음처럼 display-safe 값만 ViewData로 복사한다.

```cpp
struct FDebugOverlayRecentCombatFocusEvidenceViewData
{
    FString SourceName;
    FString TargetName;
    float AgeSeconds;
    bool bStale;
    bool bMatchedViewerPawn;
};
```

다만 focus 표시의 최종 기본 형태는 다음처럼 FocusViewData에 정리한다.

```text
FocusMode = RecentCombat
FocusActorName = BP_Enemy_C_2
LastFocusCommand = Selected / Failed
LastFocusReason = Source: ... | Target: ... | Age: ...
```

## 43. FDebugOverlayViewDataTypes.h 역할

`FDebugOverlayViewDataTypes.h`는 HUD/UMG/TextFormatter가 사용할 구조화 display data를 담는다.

후보 타입:

```text
FDebugOverlayViewData
FDebugOverlayFocusViewData
FDebugOverlayActorStatusViewData
FDebugOverlayEnemyAIViewData
FDebugOverlayCurrentAIViewData
FDebugOverlayRecentAIEventViewData
FDebugOverlayEventLogViewData
FDebugOverlayEventLogRowViewData
FDebugOverlayInteractionViewData
FDebugOverlaySummaryBlockViewData
```

성격:

```text
Store raw schema가 아님
Renderer layout schema가 아님
UMG/Canvas fallback 양쪽이 사용할 수 있는 display model
```

## 44. FDebugOverlayTextPanelTypes.h 역할

`FDebugOverlayTextPanelTypes.h`는 Canvas fallback Renderer 직전의 text representation을 담는다.

후보 타입:

```text
EDebugOverlayTextLineRole
EDebugOverlayTextPanelRole
FDebugOverlayTextLine
FDebugOverlayTextPanel
FDebugOverlayTextPanels
```

성격:

```text
Store 저장용 schema가 아님
ViewData 원본 schema가 아님
TextFormatter가 만든 Canvas fallback용 text panel
CanvasRenderer 입력 schema
```

예:

```cpp
struct FDebugOverlayTextLine
{
    FString Text;
    FString FullText;
    EDebugOverlayTextLineRole Role = EDebugOverlayTextLineRole::Normal;
};
```

역할:

```text
Text
- 기본 표시 text

FullText
- ellipsis 전 원문
- UMG tooltip/detail 후보

Role
- PanelTitle / PanelHeader / EventLogHeader / Warning / Normal
```

이 타입들은 데이터를 저장하기 위한 구조체가 아니라, 이미 표시 문장으로 변환된 데이터를 Renderer가 안정적으로 그릴 수 있게 담는 구조체이다.

## 45. 타입 파일 분리 금지/보류 항목

- SnapshotTypes에 ViewData 타입을 추가하지 않는다.
- SnapshotTypes에 TextPanel 타입을 추가하지 않는다.
- ViewDataTypes가 Store 내부 map/ring buffer lifecycle을 알게 하지 않는다.
- TextPanelTypes가 Store raw schema에 의존하지 않게 한다.
- CanvasRenderer가 `FDebugOverlaySnapshot`이나 `FDebugOverlayEventEntry`를 직접 받지 않게 한다.
- UMG/Blueprint에 raw `FDebugOverlaySnapshot`이나 weak actor pair를 직접 넘기지 않는다.

## 46. CDebugOverlayTargetComponent 현재 역할

현재 `UCDebugOverlayTargetComponent`는 이름은 TargetComponent이지만 실제 역할은 gameplay combat target system이 아니라 Debug Overlay가 현재 보고 있는 actor 상태를 저장하는 component이다.

현재 저장 값:

```cpp
TWeakObjectPtr<AActor> DebugOverlayTargetActor;
EDebugOverlayTargetSource DebugOverlayTargetSource;
FString DebugOverlaySelectionSummary;
```

의미:

```text
DebugOverlayTargetActor
-> HUD가 현재 보고 있는 actor

DebugOverlayTargetSource
-> 그 actor가 어떤 방식으로 선택됐는지

DebugOverlaySelectionSummary
-> 마지막 선택 명령 결과 summary
```

Focus 용어로 보면 다음과 같다.

```cpp
TWeakObjectPtr<AActor> FocusActor;
EDebugOverlayFocusMode FocusMode;
FDebugOverlayFocusCommandResult LastFocusCommandResult;
```

따라서 장기적으로 `UCDebugOverlayTargetComponent`는 `UCDebugOverlayFocusComponent`로 전환한다.

## 47. FocusComponent 전환 후 목표 구조

장기 목표:

```cpp
class UCDebugOverlayFocusComponent : public UActorComponent
{
private:
    TWeakObjectPtr<AActor> FocusActor;
    EDebugOverlayFocusMode FocusMode = EDebugOverlayFocusMode::None;
    FDebugOverlayFocusCommandResult LastFocusCommandResult;
};
```

역할:

```text
FocusActor 저장
FocusMode 저장
LastFocusCommandResult 저장
Clear / Query 제공
```

비역할:

```text
Nearest enemy 검색
Actor name 검색
World scan
Store 조회
RecentCombat pair 해석
Gameplay targeting 정책 구현
ViewData 생성
Text line 생성
```

즉 찾기는 `FDebugOverlayFocusResolver`, 저장은 `UCDebugOverlayFocusComponent`가 담당한다.

## 48. Target API에서 Focus API로 전환

현재 API:

```cpp
bool HasDebugOverlayTarget() const;
AActor* GetDebugOverlayTargetActor() const;
FString GetDebugOverlayTargetSummary() const;
FString GetDebugOverlayTargetSource() const;
FString GetDebugOverlaySelectionSummary() const;

void SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayTargetSource InSource);
void ClearDebugOverlayTarget();
void SetDebugOverlaySelectionSummary(const FString& InSummary);
void ClearDebugOverlaySelectionSummary();
```

장기 API:

```cpp
bool HasDebugOverlayFocusActor() const;
AActor* GetDebugOverlayFocusActor() const;
FString GetDebugOverlayFocusActorName() const;
EDebugOverlayFocusMode GetDebugOverlayFocusMode() const;
const FDebugOverlayFocusCommandResult& GetLastDebugOverlayFocusCommandResult() const;

void SetDebugOverlayFocusActor(AActor* InFocusActor, EDebugOverlayFocusMode InMode);
void ClearDebugOverlayFocusActor();
void SetLastDebugOverlayFocusCommandResult(const FDebugOverlayFocusCommandResult& InResult);
void ClearLastDebugOverlayFocusCommandResult();
void ApplyDebugOverlayFocusResolveResult(const FDebugOverlayFocusResolveResult& InResult);
```

전환 방향:

```text
GetDebugOverlayTargetActor()
-> GetDebugOverlayFocusActor()

GetDebugOverlayTargetSource()
-> GetDebugOverlayFocusMode()

GetDebugOverlayTargetSummary()
-> GetDebugOverlayFocusActorName()

GetDebugOverlaySelectionSummary()
-> GetLastDebugOverlayFocusCommandResult()

SetDebugOverlayTarget()
-> SetDebugOverlayFocusActor()

ClearDebugOverlayTarget()
-> ClearDebugOverlayFocusActor()
```

## 49. 표시 문자열 변경 예시

현재 표시:

```text
EnemySource: TargetComponent.Nearest
EnemyTarget: Selected: BP_Enemy_C_2
EnemySelect: NearestSelected | Target: BP_Enemy_C_2 | Distance: 847 | Radius: 3000
```

문제:

```text
Target 용어가 combat pipeline과 충돌한다.
EnemyTarget: Selected: ... 는 ':'가 중복된다.
Selected는 의미 중복이다.
성공 케이스에서 actor name이 반복된다.
```

장기 표시:

```text
EnemyFocusMode: NearestEnemy
EnemyFocusActor: BP_Enemy_C_2
EnemyFocusCommand: Selected | Actor: BP_Enemy_C_2 | Distance: 847 | Radius: 3000
```

실패 표시:

```text
EnemyFocusMode: None
EnemyFocusActor: None
EnemyFocusCommand: Failed | Actor: BP_Chest_C_1 | Reason: NotEnemy
```

이 표시는 `TextFormatter`가 `FDebugOverlayFocusViewData`와 `FDebugOverlayFocusCommandResult`를 기반으로 생성한다.

## 50. FocusCommandResult 구조화

현재 `DebugOverlaySelectionSummary`는 문자열 하나로 마지막 선택 명령 결과를 저장한다.

현재 예:

```text
NearestSelected | Target: BP_Enemy_C_2 | Distance: 847 | Radius: 3000
EditorSelectFailed | NotEnemy | Target: BP_Chest_C_1
```

장기 구조:

```cpp
struct FDebugOverlayFocusCommandResult
{
    FString CommandName; // SelectNearestEnemy / SelectEditorActor / SelectRecentCombat / Clear
    FString Status;      // Selected / Failed / Cleared
    FString ActorName;   // BP_Enemy_C_2 / BP_Chest_C_1 / None
    FString Reason;      // Distance: 847 | Radius: 3000 / NotEnemy / NoEnemy
};
```

예:

```cpp
// Nearest success
{
    CommandName = TEXT("SelectNearestEnemy");
    Status = TEXT("Selected");
    ActorName = TEXT("BP_Enemy_C_2");
    Reason = TEXT("Distance: 847 | Radius: 3000");
}

// Editor selection failure
{
    CommandName = TEXT("SelectEditorActor");
    Status = TEXT("Failed");
    ActorName = TEXT("BP_Chest_C_1");
    Reason = TEXT("NotEnemy");
}
```

이 구조를 사용하면 UMG에서 command name, status, actor name, reason을 별도 widget으로 표현할 수 있고, Canvas fallback은 TextFormatter가 기존 호환 line으로 변환할 수 있다.

## 51. Clear focus와 LastFocusCommandResult 생명주기 분리

현재 `ClearDebugOverlayTarget()`은 target actor/source와 selection summary를 같이 지운다.

현재:

```cpp
void ClearDebugOverlayTarget()
{
    DebugOverlayTargetActor.Reset();
    DebugOverlayTargetSource = EDebugOverlayTargetSource::None;
    ClearDebugOverlaySelectionSummary();
}
```

하지만 실패 케이스에서는 target은 비워야 하고 마지막 실패 결과는 표시해야 한다.

현재 controller 흐름:

```text
Clear target
Set selection summary = 실패 이유
```

장기 흐름:

```text
ClearDebugOverlayFocusActor()
SetLastDebugOverlayFocusCommandResult(FailedResult)
```

예:

```text
EnemyFocusMode: None
EnemyFocusActor: None
EnemyFocusCommand: Failed | Actor: BP_Chest_C_1 | Reason: NotEnemy
```

따라서 `FocusActor`와 `LastFocusCommandResult`는 별도 생명주기를 갖는다.

## 52. FocusMode enum

현재 enum:

```cpp
enum class EDebugOverlayTargetSource : uint8
{
    None,
    Nearest,
    EditorSelection,
};
```

장기 enum:

```cpp
enum class EDebugOverlayFocusMode : uint8
{
    None,
    NearestEnemy,
    EditorSelection,
    RecentCombat,
    WorldScanFallback,
    GameplayTarget,
};
```

결정:

- `Nearest`보다 `NearestEnemy`를 사용한다.
- `RecentCombat`은 채택한다.
- `WorldScanFallback`은 자동 fallback이 아니라 별도 결정/명시 command 후보로 둔다.
- `GameplayTarget`은 나중에 실제 gameplay targeting component가 생기면 adapter mode로 사용한다.

## 53. 임시 Target API wrapper 정책

`Target` API wrapper는 영구 API가 아니다. class/API rename 전환 중 빌드와 호출부 변경을 안전하게 넘기기 위한 임시 호환층이다.

예:

```cpp
AActor* GetDebugOverlayTargetActor() const
{
    return GetDebugOverlayFocusActor();
}

void SetDebugOverlayTarget(AActor* InActor, EDebugOverlayTargetSource InSource)
{
    SetDebugOverlayFocusActor(InActor, ConvertTargetSourceToFocusMode(InSource));
}
```

목적:

```text
1. 내부 저장 모델을 Focus 기준으로 먼저 바꾼다.
2. 기존 호출부가 잠깐 빌드될 수 있게 한다.
3. 호출부를 Focus API로 전부 교체한다.
4. 모든 호출부가 바뀌면 Target wrapper를 제거한다.
```

권장 전환:

```text
PR 1
- Focus API 추가
- 기존 Target API는 wrapper로 유지
- 새 코드/ViewDataBuilder/TextFormatter는 Focus API 사용

PR 2
- 기존 호출부를 Focus API로 전부 변경
- Target API wrapper 제거

PR 3
- class/file rename이 필요하면 UCDebugOverlayTargetComponent -> UCDebugOverlayFocusComponent
```

규칙:

- wrapper를 계속 사용하지 않는다.
- 신규 코드는 Focus API만 사용한다.
- wrapper 제거 PR을 별도로 계획한다.

## 54. TargetComponent / FocusComponent 금지 항목

- FocusComponent가 Store를 직접 조회하지 않는다.
- FocusComponent가 World scan을 수행하지 않는다.
- FocusComponent가 nearest enemy를 계산하지 않는다.
- FocusComponent가 RecentCombatPair를 해석하지 않는다.
- FocusComponent가 gameplay targeting policy를 구현하지 않는다.
- FocusComponent가 ViewData/TextPanel을 만들지 않는다.
- `Selected:` 같은 표시 문자열을 component API에서 만들지 않는다.
- `TargetComponent.Nearest` 같은 표시 문자열을 component API에서 만들지 않는다.

## 55. CPlayerController command entry / resolver 분리

현재 `ACPlayerController`는 Debug Overlay console command entry이면서 실제 target search 정책도 일부 수행한다.

현재 command entry:

```cpp
void DebugOverlaySelectNearestTarget();
void DebugOverlayClearTarget();
void DebugOverlaySelectActorTarget(const FString& ActorName);
```

현재 search helper:

```cpp
bool TrySelectDebugOverlayNearestEnemy();
bool TrySelectDebugOverlayActorTarget(const FString& InActorName);
ACEnemy* FindClosestDebugOverlayEnemy(float& OutDistance) const;
AActor* FindDebugOverlayActorByName(const FString& InActorName) const;
```

장기 책임 분리:

```text
ACPlayerController
- console command entry
- resolver 호출
- FocusComponent에 결과 적용
- log 출력

FDebugOverlayFocusResolver
- nearest enemy 검색
- actor name / editor label 검색
- RecentCombatPair 해석
- stale / invalid / not matched reason 생성

UCDebugOverlayFocusComponent
- resolver 결과 저장
```

예상 흐름:

```cpp
void ACPlayerController::DebugOverlaySelectNearestTarget()
{
    const FDebugOverlayFocusResolveResult Result =
        FDebugOverlayFocusResolver::ResolveNearestEnemy(
            GetWorld(),
            GetPawn(),
            DebugOverlayNearestFocusRadius);

    DebugOverlayFocusComponent->ApplyDebugOverlayFocusResolveResult(Result);
}
```

## 56. Console command 이름 유지 정책

외부 console command 이름은 당장 변경하지 않는다.

현재 이름:

```text
DebugOverlaySelectNearestTarget
DebugOverlaySelectActorTarget
DebugOverlayClearTarget
```

결정:

```text
초기 구현 단계
- 기존 Target 기반 console command 이름 유지
- Editor plugin command string 유지
- 내부 resolver/component/viewdata 용어는 Focus로 전환

후반 정리 단계
- 필요하면 Focus 기반 command alias 추가 검토
- 기존 Target command 제거 여부는 별도 판단
```

예:

```cpp
void ACPlayerController::DebugOverlaySelectNearestTarget()
{
    const FDebugOverlayFocusResolveResult Result =
        FDebugOverlayFocusResolver::ResolveNearestEnemy(...);

    DebugOverlayFocusComponent->ApplyDebugOverlayFocusResolveResult(Result);
}
```

즉 public command surface는 `Target` 이름을 유지하지만, 내부 구현은 `Focus` 책임 구조를 사용한다.

이름 변경을 별도 PR로 즉시 분리하지 않는다. 전체 작업 후반 정리 항목으로 미룬다.

이유:

```text
기존 Editor Tooling command string 호환 유지
사용 중인 console command 습관 유지
ViewData / Resolver / Renderer 구조화와 command rename을 동시에 섞지 않음
별도 PR을 추가로 만들 여력을 줄임
```

## 57. CPlayerController 리팩터링 결정 항목

결정:

- `FindClosestDebugOverlayEnemy()`는 `FDebugOverlayFocusResolver::ResolveNearestEnemy()`로 이동 후보이다.
- `FindDebugOverlayActorByName()`은 `FDebugOverlayFocusResolver::ResolveActorByName()`으로 이동 후보이다.
- `TrySelectDebugOverlayNearestEnemy()`와 `TrySelectDebugOverlayActorTarget()`은 resolver 호출 + FocusComponent 적용 흐름으로 축소한다.
- `DebugOverlaySelectRecentCombatTarget` command를 추가해 RecentCombat focus mode를 명시적으로 실행한다.
- RecentCombat은 자동 fallback으로 연결하지 않는다.
- Nearest radius는 CVar로 승격하지 않고 Controller 또는 command layer에서 resolver 인자로 전달한다.
- 기존 Target command 이름은 초기 구현에서 유지한다.

보류:

- Focus 기반 command alias 추가
  - 예: `DebugOverlaySelectNearestFocus`
  - 예: `DebugOverlayClearFocus`
- 기존 Target command 제거 여부
- Clear command 실행 시 `LastFocusCommandResult = Cleared`를 남길지 여부

권장:

- Clear command는 장기적으로 `FocusActor` clear와 `LastFocusCommandResult` 처리를 분리한다.
- 다만 기존 표시 동작과 달라질 수 있으므로 implementation phase에서 별도 확인 후 적용한다.

## 58. Editor Tooling 경계

`PortfolioDebugOverlayEditorModule`은 runtime Debug Overlay 구조를 직접 소유하지 않는다. Editor plugin은 command sender로만 유지한다.

현재 역할:

```text
Debug Overlay panel 제공
CVar widget 제공
Select Nearest Target 버튼 제공
Select Outliner Actor 버튼 제공
Clear Target 버튼 제공
PIE world의 PlayerController에 ConsoleCommand 전송
```

유지할 경계:

```text
Editor plugin
- command string 생성
- PIE world / PlayerController resolve
- ConsoleCommand 호출
- editor selection actor name 전달

Runtime
- command 실행
- FocusResolver 호출
- FocusComponent 저장
- ViewDataBuilder / TextFormatter / CanvasRenderer 처리
```

금지:

- Editor plugin이 `FDebugOverlayViewData`를 만들지 않는다.
- Editor plugin이 `FDebugOverlayTextFormatter`를 호출하지 않는다.
- Editor plugin이 `FDebugOverlayCanvasRenderer`를 알지 않는다.
- Editor plugin이 `FDebugOverlayFocusResolver`를 직접 호출하지 않는다.
- Editor plugin이 Store schema/API를 직접 해석하지 않는다.
- Editor plugin이 runtime module에 editor-only dependency를 끌어오게 만들지 않는다.

RecentCombat command가 추가되면 Editor panel에 버튼을 추가할 수 있다.

권장:

```text
Button label
- Select Recent Combat Target

Command
- DebugOverlaySelectRecentCombatTarget

Editor responsibility
- PIE PlayerController에 command 전송만 수행
```

Editor plugin은 계속 runtime command bridge 역할로 제한한다.

## 59. 권장 구현 순서

현재 설계 결정 기준으로 다음 순서를 권장한다.

```text
1. Type split
   - FDebugOverlaySnapshotTypes.h는 raw evidence schema로 유지
   - FDebugOverlayViewDataTypes.h 추가
   - FDebugOverlayTextPanelTypes.h 추가

2. Builder / Formatter / Renderer skeleton
   - DrawHUD orchestration 축소
   - FDebugOverlayHUDContext 도입
   - FDebugOverlayViewDataBuilder 도입
   - FDebugOverlayTextFormatter 도입
   - FDebugOverlayCanvasRenderer 도입

3. ActorStatusViewData 구조화
   - State / Action / Reaction / HP / Stagger / Guard / Movement / Runtime LOD
   - 기존 표시 순서 유지
   - Stagger component 설계는 보류

4. EventLog / Interaction ViewData 구조화
   - EventLog는 history list
   - Interaction은 latest summary block
   - Summary split은 TextFormatter 책임

5. EnemyAIViewData 구조화
   - Current AI와 Recent AI Event 분리
   - Blackboard live state와 Store evidence 분리
   - BT active node tracking 구현 금지

6. FocusViewData / Target display terminology 전환
   - EnemySource / EnemyTarget / EnemySelect를 Focus 용어로 전환
   - Selected: 표기 제거
   - 기존 console command 이름은 유지

7. FocusResolver 도입
   - NearestEnemy
   - ActorByName / EditorSelection
   - RecentCombat explicit command
   - 자동 fallback 연결 금지

8. TargetComponent -> FocusComponent 전환
   - Focus API 추가
   - Target API wrapper는 임시 호환층으로만 사용
   - 호출부 전환 후 wrapper 제거
   - class/file rename은 작업 후반에 수행

9. Editor Tooling command button 정리
   - 기존 command string 유지
   - RecentCombat command 버튼 추가 가능
   - Editor plugin은 command sender로 유지

10. UMG / Blueprint adapter 검토
   - raw Store snapshot 노출 금지
   - structured ViewData 기반 override 검토
```

## 60. Issue #88 코드 품질 계획 대조

GitHub issue #88 `N00: 코드 품질 PR 진행 현황 정리`의 후속 코드 품질 항목과 현재 Debug Overlay 설계를 대조했다.

### P38 Type Header / Helper Boundary

관련 있음.

반영 내용:

```text
SnapshotTypes / ViewDataTypes / TextPanelTypes 파일 분리
Store raw schema와 UI display schema 분리
ViewDataBuilder / TextFormatter / CanvasRenderer helper 책임 분리
FocusResolver와 FocusComponent 책임 분리
```

주의:

- 작은 display type을 Store raw type header에 섞지 않는다.
- helper를 전역 Type 헤더에 몰아넣지 않는다.

### P39 Tuning Constants Cleanup

일부 관련 있음.

현재 결정:

```text
Canvas layout/style 상수는 CanvasRenderer config 후보
Nearest radius는 CVar 승격 없이 command/controller layer에서 resolver 인자로 전달
Recent AI stale 5초 등 기존 정책 값은 임의 변경하지 않음
```

주의:

- 이번 설계에서 constants를 config/DataAsset/CVar로 승격하지 않는다.
- tuning 정책 변경은 별도 구현 단계에서 검토한다.

### P40 API Const Consistency

관련 있음.

반영 방향:

```text
ViewDataBuilder는 Store public copy API를 read-only로 사용
FocusComponent query API는 const 유지
Renderer는 input TextPanels를 const reference로 받음
Formatter는 input ViewData를 const reference로 받음
```

주의:

- 구조화 구현 시 read-only API const 정합성을 같이 확인한다.

### P41 Debug Log Policy

관련 있음.

반영 방향:

```text
Debug Overlay는 시각적 debug tool / UI debug panel 성격
Store / FocusResolver / Controller command 결과는 표시 evidence와 log 책임을 분리
Editor plugin은 command status 표시만 수행
```

주의:

- hot path log를 늘리지 않는다.
- Visual debug UI 책임과 runtime log 책임을 섞지 않는다.
- Shipping HUD claim을 하지 않는다.

### P42 Naming / Typo / API Cleanup

관련 있음.

반영 내용:

```text
Target 용어를 Focus 용어로 전환
EnemyTarget: Selected: ... 표기 제거 계획
Source / Target 용어는 RecentCombatPair 내부 의미로만 유지
Public console command 이름은 호환을 위해 초기에는 유지
Pannel_01/02/03 문자열은 별도 결정 전까지 유지
```

주의:

- command rename은 작업 후반 정리 항목으로 미룬다.
- API wrapper는 임시 호환층이며 영구 API가 아니다.

### P43 TODO Status Cleanup

관련 있음.

보류/금지로 명시한 항목:

```text
Runtime LOD actual 구현 금지
BT active node tracking 구현 금지
Stagger component 설계는 보류
WorldScanFallback 자동 연결 금지
GameplayTarget mode는 gameplay targeting component 구현 후 adapter로 검토
Store Enemy state snapshot화 보류
Store ViewData API 확장 보류
```

### P44 PR Record Format Sweep

문서 형식 관련.

현재 문서는 KR 기준으로 작성하며, 구현 PR 분리안과 금지/보류 항목을 명시한다.

### P34~P37 AI profiling / LOD 항목

직접 구현 대상 아님.

주의:

```text
Debug Overlay 설계에서 Runtime LOD actual 구현을 하지 않는다.
AI LOD/perception/update 최적화는 issue #88의 별도 성능 작업 흐름으로 남긴다.
Debug Overlay의 Runtime LOD 표시는 기존 N/A 정책을 유지한다.
```

결론:

```text
현재 설계는 issue #88의 보편 코드 품질 기준과 충돌하지 않는다.
특히 P38/P41/P42/P43 기준을 Debug Overlay 구조 분리에 반영했다.
다만 AI LOD, profiling asset, tuning config 승격은 이번 문서/구현 범위가 아니므로 보류한다.
```

## 61. 구현 작업 분할 계획

ViewData / TextFormatter / CanvasRenderer 전체 구조를 한 번에 구현하지 않는다. 신규 파일 수와 helper 이동량이 크고, HUD 표시 문자열/순서 회귀 검증 부담이 크기 때문이다.

구현은 다음 순서로 분할한다.

```text
1. CanvasRenderer skeleton split
   - Canvas layout/render 책임만 먼저 분리
   - 표시 line 구성은 기존 흐름 유지
   - TextPanelTypes 도입
   - role 기반 header 처리 도입
   - 실제 ellipsis truncation은 후속으로 보류 가능

2. ViewDataBuilder / TextFormatter skeleton
   - DrawHUD를 context -> builder -> formatter -> renderer 흐름으로 축소
   - ViewDataBuilder는 구조화 data 생성
   - TextFormatter는 ViewData -> TextPanels 변환

3. ActorStatusViewData 구조화
   - State / Action / Reaction / HP / Stagger / Guard / Movement / Runtime LOD
   - 기존 표시 순서 유지

4. EventLog / InteractionViewData 구조화
   - EventLog history list와 Interaction latest summary block 분리
   - Summary split은 TextFormatter 책임 유지

5. EnemyAIViewData 구조화
   - Current AI와 Recent AI Event 분리
   - Blackboard live state와 Store evidence 분리

6. FocusViewData terminology 전환
   - EnemySource / EnemyTarget / EnemySelect를 Focus 표기로 전환
   - Selected: 표기 제거
   - 기존 console command 이름은 유지

7. FocusResolver 도입
   - NearestEnemy
   - ActorByName / EditorSelection
   - RecentCombat explicit command

8. TargetComponent -> FocusComponent 전환
   - Focus API 추가
   - Target API wrapper는 임시 호환층
   - 호출부 전환 후 wrapper 제거
   - class/file rename은 작업 후반 수행

9. Editor Tooling command button 정리
   - RecentCombat command button 추가 가능
   - Editor plugin은 command sender로 유지

10. Canvas text ellipsis
   - 실제 panel width / font scale 기준 text measurement 기반 truncation
   - 표시 결과가 바뀌므로 별도 검증

11. UMG / Blueprint adapter 검토
   - structured ViewData 기반 override
   - raw Store snapshot / weak actor pair 노출 금지
```

## 62. 1차 구현 계획: CanvasRenderer skeleton split

1차 구현은 `CanvasRenderer` 분리만 수행한다.

목표:

```text
HUD 표시 문자열과 line 구성은 유지한다.
Canvas layout/render 책임만 FDebugOverlayCanvasRenderer로 분리한다.
DrawHUD의 표시 data 생성 책임은 아직 유지한다.
```

대상 파일:

```text
수정
- Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp

신규
- Source/Portfolio/Core/Debug/FDebugOverlayTextPanelTypes.h
- Source/Portfolio/Core/Debug/FDebugOverlayCanvasRenderer.h
- Source/Portfolio/Core/Debug/FDebugOverlayCanvasRenderer.cpp
```

1차에서 하지 않는 것:

```text
FDebugOverlayViewDataTypes.h 추가
FDebugOverlayViewDataBuilder 추가
FDebugOverlayTextFormatter 추가
ViewData 구조화
TextFormatter로 line 구성 이동
Focus 용어 표시 변경
TargetComponent rename
FocusResolver 도입
RecentCombat command 추가
Store schema/API 변경
Controller / Editor plugin 변경
UMG / Blueprint 구현
Runtime LOD actual 구현
BT active node tracking 구현
Stagger component 구현
Console command 이름 변경
```

1차 구현 세부 내용:

```text
1. FDebugOverlayTextPanelTypes.h 추가
   - EDebugOverlayTextLineRole
   - EDebugOverlayTextPanelRole
   - FDebugOverlayTextLine
   - FDebugOverlayTextPanel
   - FDebugOverlayTextPanels

2. CDebugOverlayHUD.cpp에서 기존 TArray<FString> line을 TextPanel로 변환
   - 기존 line text 유지
   - panel title/header role 부여
   - role 부여는 기존 문자열과 동일한 의미로만 수행

3. FDebugOverlayCanvasRenderer 추가
   - 기존 3-panel geometry 산식 이동
   - CalculateOverlayLinesHeight 이동
   - CalculateVisibleOverlayLineCount / MakeVisibleOverlayLines 이동
   - DrawOverlayLines 이동
   - header 판단은 line Role 기반으로 수행

4. CDebugOverlayHUD::DrawHUD()
   - 기존 line 생성은 유지
   - renderer 호출만 FDebugOverlayCanvasRenderer::Draw(...)로 변경
```

보존할 표시 정책:

```text
Pannel_01/02/03 문자열 유지
EnemySource / EnemyTarget / EnemySelect 유지
Selected: 표기 유지
NotCaptured / NoTarget / NotMatched / Stale 유지
NoEvents(Filter: ...) 유지
NoEvents(Filter: ... Limit: 0) 유지
EventLog row "{Category}/{EventName}: {Summary}" 유지
Recent Execution / Recent Combat 순서 유지
Recent Combat 앞 빈 줄 유지
Current AI / Recent AI Event 순서 유지
3-panel layout 조건 유지
Interaction panel width 520 유지
EventLog min width 420 유지
```

검증 기준:

```text
git diff --check 통과
허용 파일 외 변경 없음
PortfolioEditor Win64 Development 빌드 성공
Renderer가 Store / Actor / Blackboard / FocusComponent를 직접 읽지 않음
CDebugOverlayHUD.cpp의 line 생성 결과가 기존 표시 정책과 동일함
Store public API/schema 변경 없음
Target/Controller/Editor plugin 변경 없음
Build.cs / uproject / config / asset 변경 없음
```

권장 커밋 메시지:

```text
refactor(debug): split overlay canvas renderer
```

## 63. 1차 구현 목표모드 / 에이전트 운용

1차 구현에서는 목표모드를 사용한다.

권장 목표:

```text
Debug Overlay HUD 표시 문자열과 line 구성 정책을 유지하면서, Canvas layout/render 책임만 FDebugOverlayCanvasRenderer와 FDebugOverlayTextPanelTypes로 분리한다.
```

에이전트는 read-only 리뷰 역할로 사용한다. 실제 패치 작성과 최종 판단은 메인 에이전트가 수행한다.

권장 에이전트:

```text
Agent A: Canvas layout 회귀 검토
- 기존 3-panel geometry 산식 유지 여부 확인
- EventLog / Interaction panel width, margin, clipping 조건 유지 여부 확인
- header/background draw 정책 유지 여부 확인

Agent B: 표시 문자열/line role 회귀 검토
- Pannel_01/02/03 문자열 유지 여부 확인
- EnemySource / EnemyTarget / EnemySelect 등 기존 line text 변경 여부 확인
- line role이 표시 의미만 보조하고 text를 바꾸지 않는지 확인

Agent C: 변경 범위/금지 항목 검토
- Store / TargetComponent / Controller / Editor plugin 변경 여부 확인
- Build.cs / uproject / config / asset 변경 여부 확인
- Store schema/API 변경 여부 확인
```

에이전트 금지:

```text
에이전트가 직접 파일을 수정하지 않는다.
에이전트가 표시 정책 변경을 제안하더라도 1차 구현에는 반영하지 않는다.
에이전트 결과가 충돌하면 메인 에이전트가 판단하거나 사용자에게 질문한다.
```

## 64. 브랜치 고정 메모

이 문서는 설계 메모이지만, 후속 브랜치로 내용이 흩어지지 않도록 현재 브랜치와 다음 브랜치의 경계를 고정해 둔다.

### 64.1 현재 브랜치에서 마무리할 것

- Debug Overlay 내부 용어를 Target에서 Focus로 정리한다.
- Editor UI label, help text, command status 같은 표시 문자열을 Focus 기준으로 정리한다.
- public console command 문자열은 유지한다.
- Target compatibility wrapper, `EDebugOverlayTargetSource` alias, `TEXT("DebugOverlayTarget")` subobject name의 유지 사유를 문서로 남긴다.
- 구조 리뷰와 검증 문서를 현재 구현 상태에 맞게 최신화한다.

### 64.2 다음 브랜치에서 구현할 것

- Focus command result를 문자열 요약에서 구조체 기반 모델로 전환한다.
- Focus mode enum을 확장한다.
- RecentCombat 명시 command 경로를 추가한다.
- Target compatibility wrapper의 실제 제거 여부를 검증 후 판단한다.
- `EDebugOverlayTargetSource` alias의 실제 제거 여부를 검증 후 판단한다.
- `TEXT("DebugOverlayTarget")` subobject name 변경과 asset migration/Core Redirect 필요성을 별도 검토한다.

### 64.3 이번 브랜치에서 하지 않을 것

- Canvas text overflow / ellipsis
- Runtime LOD actual
- BT active node tracking
- Blueprint/UMG adapter
- Blueprint/UMG override
- Store schema/API 실제 변경
- Core Redirect 선제 추가
- gameplay target/lock-on system 구현
- 자동 RecentCombat focus fallback 추가

### 64.4 다음 브랜치 인계 기준

- current state 와 last command result 분리 설계가 필요할 때는 `FDebugOverlayFocusCommandResult`를 기준으로 구조화한다.
- mode 확장이나 RecentCombat command 추가는 FocusResolver/Controller 경계에서 처리한다.
- 호환성 제거 3종(wrapper, alias, subobject name)은 하나의 migration 패키지로 묶어 검토한다.
- missing class/reference가 실제로 발견되기 전에는 Core Redirect를 추가하지 않는다.
