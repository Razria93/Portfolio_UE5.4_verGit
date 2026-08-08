# Debug Overlay FocusComponent Migration Result

## 1. 목적

이 문서는 `UCDebugOverlayFocusComponent`를 `UCDebugOverlayFocusComponent`로 이주한 결과와, 남겨 둔 `Target` 명명의 호환성 범위를 정리한다.

이번 작업은 구현성 확장 작업이 아니다. Canvas ellipsis, Blueprint/UMG, Runtime LOD actual, BT active node tracking은 포함하지 않는다.

## 2. 이주 결과 요약

| 항목 | 결과 |
| --- | --- |
| class rename | `UCDebugOverlayFocusComponent` -> `UCDebugOverlayFocusComponent` |
| file rename | `CDebugOverlayFocusComponent.h/.cpp` -> `CDebugOverlayFocusComponent.h/.cpp` |
| generated header | `CDebugOverlayFocusComponent.generated.h` |
| Controller member | `DebugOverlayFocusComponent` |
| HUD component lookup | `FindComponentByClass<UCDebugOverlayFocusComponent>()` |
| FocusResolver result source | `EDebugOverlayFocusSource` |
| 기존 enum 이름 | `using EDebugOverlayFocusSource = EDebugOverlayFocusSource` alias로 유지 |
| 기존 Target public API | `UCDebugOverlayFocusComponent` 내부 compatibility wrapper로 유지 |
| console command 이름 | 기존 `Target` command 이름 유지 |
| subobject name | `TEXT("DebugOverlayFocus")` 유지 |
| Store schema/API | 변경 없음 |

`TEXT("DebugOverlayFocus")`는 native subobject/serialized reference 안정성을 위해 유지한다. class/file/type 이름은 Focus로 이주했지만, subobject instance name까지 바꾸는 것은 별도 asset migration 성격이 있으므로 이번 작업에서 제외한다.

## 3. 현재 책임 분리

| 영역 | 현재 책임 |
| --- | --- |
| `UCDebugOverlayFocusComponent` | debug-only focus actor/source/command result 저장 |
| `FDebugOverlayFocusResolver` | nearest/editor-selection focus 후보 탐색과 resolve result 생성 |
| `ACPlayerController` | console command entry, resolver 호출, focus result 적용 |
| `ACDebugOverlayHUD` | FocusComponent에서 표시 대상 enemy와 마지막 command result 조회 |
| `FDebugOverlayViewDataBuilder` | Store/actor/focus context를 ViewData로 구성 |
| `FDebugOverlayTextFormatter` | ViewData를 표시 문자열 panel로 변환 |
| `FDebugOverlaySnapshotStore` | snapshot/event/recent summary 저장 및 copy API 제공 |
| Editor plugin | PIE controller에 기존 console command 전송 |

FocusComponent는 Store, World scan, resolver 책임을 갖지 않는다. Store도 focus selection owner가 아니다.

## 4. 유지한 compatibility 표면

### 4.1 Console command

다음 command 이름은 Editor Tooling과 사용자 workflow가 의존하는 외부 인터페이스이므로 유지한다.

```cpp
DebugOverlaySelectNearestFocus
DebugOverlayClearFocus
DebugOverlaySelectOutlinerFocus
```

내부 구현은 Focus helper와 FocusComponent API로 위임한다.

### 4.2 Target compatibility wrapper

다음 API는 `UCDebugOverlayFocusComponent` 안에 wrapper로 유지한다.

```cpp
bool HasDebugOverlayFocus() const;
bool HasDebugOverlaySelectionSummary() const;
AActor* GetDebugOverlayFocusActor() const;
FString GetDebugOverlayFocusSummary() const;
FString GetDebugOverlayFocusSource() const;
FString GetDebugOverlaySelectionSummary() const;

void SetDebugOverlayFocus(AActor* InTargetActor, EDebugOverlayFocusSource InSource);
void ClearDebugOverlayFocus();
void SetDebugOverlaySelectionSummary(const FString& InSummary);
void ClearDebugOverlaySelectionSummary();
```

현재 runtime 호출부는 Focus API를 사용한다. Target wrapper는 외부 C++ 호환성 보존용이며, 이번 작업에서 제거하지 않는다.

### 4.3 Enum alias

`EDebugOverlayFocusSource`를 주 타입으로 사용한다. 기존 `EDebugOverlayFocusSource` 이름은 alias로 남긴다.

```cpp
using EDebugOverlayFocusSource = EDebugOverlayFocusSource;
```

기존 외부 코드가 enum 이름에 의존할 가능성을 고려한 호환 계층이다.

## 5. 잔여 Target 명명 분류

| 남은 명명 | 분류 | 처리 |
| --- | --- | --- |
| `DebugOverlaySelectNearestFocus` | console command compatibility | 유지 |
| `DebugOverlaySelectOutlinerFocus` | console command compatibility | 유지 |
| `DebugOverlayClearFocus` | console command compatibility | 유지 |
| `ExecuteDebugOverlayFocusCommand` | Editor command sender helper | command 문자열 호환 때문에 유지 |
| `LastFocusCommandStatus` | Editor UI 상태명 | Focus terminology cleanup 완료 |
| `TEXT("DebugOverlayFocus")` | native subobject name | asset/reference 안정성을 위해 유지 |
| `Get/Set/ClearDebugOverlayFocus*` | public compatibility wrapper | 유지 |
| `DebugOverlaySelectionSummary` | compatibility wrapper 명명 | 유지 |
| `CCombatSignalTargetComponent` | gameplay combat signal system | Debug Overlay rename 대상 아님 |

`FocusComponent.NearestFocus` / `FocusComponent.OutlinerFocus` 표시 문자열은 class rename 결과에 맞춰 `FocusComponent.Nearest` / `FocusComponent.EditorSelection`로 갱신했다.

## 6. Store schema/API 변경 판단

Store schema/API 변경은 필요하지 않다.

근거:

- `FDebugOverlayFocusResolver`는 Store를 참조하지 않는다.
- `UCDebugOverlayFocusComponent`는 focus actor/source/command result만 저장하고 Store를 참조하지 않는다.
- `ACPlayerController`는 resolver result를 FocusComponent에 적용할 뿐 Store를 변경하지 않는다.
- `ACDebugOverlayHUD`는 focus 대상 actor를 resolve한 뒤 기존 ViewData build context에 전달한다.
- `FDebugOverlayViewDataBuilder`는 기존 `TryGetSnapshotCopy`, `GetRecentEventsCopy`, `GetRecentEventsForSubjectCopy`, `GetEventLogFilter`, `GetEventLogDisplayLimit` copy API만 사용한다.
- `FDebugOverlaySnapshot`, `FDebugOverlayEventEntry`, `FDebugOverlayRecentCombatPair`의 구조 변경이 필요하지 않다.

따라서 이번 이주는 Store public API/schema 변경 없이 완료하는 것이 맞다.

## 7. Core Redirect / asset reference 판단

코드 검색과 `Content`/`Config` binary text 검색 기준으로 기존 `UCDebugOverlayFocusComponent`, `CDebugOverlayFocusComponent`, `DebugOverlayFocusComponent` serialized reference는 발견되지 않았다.

현재 판단:

- Core Redirect는 현재 증거만으로는 추가하지 않는다.
- `.uasset`, `.umap`, config 변경은 하지 않는다.
- 사용자의 clean rebuild / editor loading / BP compile / PIE 검수에서 reference 깨짐이 발견되면 그때 Core Redirect 추가를 검토한다.

## 8. 검증 기준

필수 확인:

```text
rg "UCDebugOverlayFocusComponent|CDebugOverlayFocusComponent|DebugOverlayFocusComponent|ResolveTargetComponentEnemy|EDebugOverlayFocusSource|Core/Debug/CDebugOverlayFocusComponent" Source/Portfolio Plugins/PortfolioDebugOverlayEditor
rg "DebugOverlayFocus|GetDebugOverlayFocus|SetDebugOverlayFocus|ClearDebugOverlayFocus|DebugOverlaySelection|FocusComponent\\." Source/Portfolio Plugins/PortfolioDebugOverlayEditor
rg -a "UCDebugOverlayFocusComponent|CDebugOverlayFocusComponent|DebugOverlayFocusComponent" Content Config
git diff --check
PortfolioEditor Win64 Development build
```

사용자 검수:

- clean/full rebuild
- editor loading
- BP compile
- PIE 진입
- `DebugOverlaySelectNearestFocus`
- `DebugOverlaySelectOutlinerFocus`
- `DebugOverlayClearFocus`
- component/class reference 깨짐 없음

## 9. 이번 작업에서 제외한 항목

- Canvas text overflow / ellipsis
- Blueprint/UMG adapter
- Blueprint/UMG override
- Runtime LOD actual
- BT active node tracking
- gameplay target/lock-on system
- 자동 RecentCombat focus fallback
- Store schema/API 실제 변경
- `Build.cs`, `Portfolio.uproject`, `.umap`, `.uasset` 변경

## 10. 다음 구현성 작업 후보

이번 작업 이후 구현성 작업은 별도 판단 대상이다.

우선순위 후보:

1. Canvas text overflow / ellipsis
2. Runtime LOD actual 데이터 소스 조사 및 구현
3. BT active node tracking 설계
4. Blueprint/UMG adapter/override 설계

## 11. 현재 브랜치 / 다음 브랜치 고정 메모

이 문서는 이주 결과와 호환성 범위를 고정하기 위한 문서이므로, 후속 작업에서 범위가 섞이지 않도록 아래처럼 분리한다.

### 11.1 현재 브랜치에서 마무리할 것

- Focus terminology cleanup을 마감한다.
- Editor UI label/description/status 변수명을 Focus 기준으로 정리한다.
- public command 문자열은 유지한다.
- Target compatibility wrapper, `EDebugOverlayFocusSource` alias, `TEXT("DebugOverlayFocus")` 유지 사유를 문서로 남긴다.
- 구조 검토/검증 문서와 내용이 일치하는지 맞춘다.

### 11.2 다음 브랜치에서 구현할 것

- Focus command result 구조체화
- Focus mode enum 확장
- RecentCombat 명시 command 경로
- Target compatibility wrapper 실제 제거 여부 판단
- `EDebugOverlayFocusSource` alias 실제 제거 여부 판단
- `TEXT("DebugOverlayFocus")` subobject name 변경 및 asset migration/Core Redirect 검토

### 11.3 이번 브랜치에서 하지 않을 것

- Canvas text overflow / ellipsis
- Runtime LOD actual
- BT active node tracking
- Blueprint/UMG adapter
- Blueprint/UMG override
- Store schema/API 실제 변경
- Core Redirect 선제 추가
- gameplay target/lock-on system 구현
- 자동 RecentCombat focus fallback 추가

### 11.4 다음 브랜치 인계 기준

- wrapper/alias/subobject name은 하나의 호환성 제거 패키지로 묶는다.
- missing class/reference가 실제로 확인되기 전에는 Core Redirect를 추가하지 않는다.
- 구조 리뷰 문서는 현재 책임 분리만 추적하고 기능 확장 계획은 별도 문서로 분리한다.
