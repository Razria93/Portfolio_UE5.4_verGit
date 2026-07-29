# Debug Overlay P0.5 Compact Display / Subject Review

## 1. 목적

이 문서는 P0.5 debug overlay의 표시 문자열을 캡처 evidence에 적합하게 줄이고, `Action` / `Reaction` 뒤에 실제 실행 subject를 함께 표시할 수 있는지 구현 전 코드 근거를 확정한다.

이번 문서는 분석 및 구현 범위 확정 문서이며, 코드 구현은 포함하지 않는다.

## 2. 현재 문제

현재 overlay는 enum 전체 이름을 그대로 표시한다.

- `ExecutionState::Reaction`
- `EActionType::ComboAttack[1]`
- `EReactionType::Hit`
- `EMovementGait::Run`
- `EDeadState::Alive`

EventLog / Recent summary는 이미 일부 compact format으로 정리되었지만, execution summary는 다음처럼 domain만 있고 실제 실행 subject가 빠져 있다.

- 현재: `Reaction | Decision=Accept | Apply=Intervene | RejectReason=None`
- 목표: `Reaction(Hit) | Decision=Accept | Apply=Intervene | RejectReason=None`
- 목표: `Action(ComboAttack[1]) | Decision=Accept | Apply=Start | RejectReason=None`

## 3. HUD Current Value Compact 검토

### 코드 근거

- `CDebugOverlayHUD.cpp`
  - `FormatExecutionState`
  - `FormatActiveAction`
  - `FormatActiveReaction`
  - `FormatActorMovement`
  - `FormatActorHealth`
- `UCActionComponent`
  - `GetActiveActionType`
  - `GetActiveActionIndex`
- `UCReactionComponent`
  - `GetActiveReactionType`

### 판단

HUD current value compact는 `CDebugOverlayHUD.cpp` 내부 formatter에서 처리 가능하다.

`FDebugOverlaySnapshotStore` 또는 gameplay component API 변경은 필요하지 않다.

### 권장 표시

| 항목 | 현재 예시 | 권장 예시 | 분류 |
| --- | --- | --- | --- |
| State | `ExecutionState::Reaction` | `Reaction` | Ready |
| Action | `EActionType::ComboAttack[1]` | `ComboAttack[1]` | Ready |
| Reaction | `EReactionType::Hit` | `Hit` | Ready |
| Movement | `Gait=EMovementGait::Run` | `Gait=Run` | Ready |
| HP | `DeadState=EDeadState::Alive` | `DeadState=Alive` | Ready |

### Helper 정책

`FDebugOverlaySnapshotStore.cpp`의 `CompactEnumText`는 anonymous namespace 내부 helper이므로 HUD에서 직접 재사용할 수 없다.

P0.5에서는 공용 helper API를 새로 노출하지 않고, HUD 내부에 동일 목적의 private/local helper를 두는 방향을 권장한다.

공용 debug text formatter 분리는 P1 이후 중복이 늘어날 때 검토한다.

## 4. Execution Summary Subject 검토

### 코드 근거

Action execution 결과는 `FActionExecutionResult::ResolvedContext`를 포함한다.

- `FActionExecutionResult`
  - `Decision`
  - `ApplyMode`
  - `ResolvedContext`
  - `RejectReason`
- `ResolvedContext.ActionDataKey`
  - `ActionType`
  - `ActionIndex`

Reaction execution 결과는 `FReactionExecutionResult::ResolvedContext`를 포함한다.

- `FReactionExecutionResult`
  - `Decision`
  - `ApplyMode`
  - `ResolvedContext`
  - `RejectReason`
- `ResolvedContext.ReactionDataKey`
  - `ReactionType`

`FExecutionOrchestratorDebug.cpp`의 audit log는 이미 `FormatActionDataKey(InResult.ResolvedContext.ActionDataKey)` 및 `FormatReactionDataKey(InResult.ResolvedContext.ReactionDataKey)`를 사용한다. 즉, subject 원천 데이터는 존재한다.

### 현재 Store API 한계

`FDebugOverlaySnapshotStore::RecordExecutionDecision`은 현재 다음 정보만 받는다.

- `Domain`
- `Decision`
- `ApplyMode`
- `RejectReason`
- `EventName`

따라서 Store 내부 summary는 현재 domain 중심으로만 조립된다.

```text
Action | Decision=Accept | Apply=Start | RejectReason=None
Reaction | Decision=Accept | Apply=Intervene | RejectReason=None
```

### 판단

subject 원천 데이터는 Ready지만, overlay store summary까지 전달하려면 Store API와 호출부 확장이 필요하다.

분류는 `HookNeeded`로 둔다.

### 권장 구현 방향

`FDebugOverlaySnapshotStore::RecordExecutionDecision`에 subject 표시 문자열을 선택 인자로 추가한다.

권장 인자 예:

```cpp
const FString& InSubject
```

권장 summary 조립:

```text
{Domain}({Subject}) | Decision={Decision} | Apply={ApplyMode} | RejectReason={RejectReason}
```

subject가 비어 있으면 기존 형식으로 fallback한다.

```text
{Domain} | Decision={Decision} | Apply={ApplyMode} | RejectReason={RejectReason}
```

### 권장 표시

| Domain | Subject 근거 | 권장 표시 | 분류 |
| --- | --- | --- | --- |
| Action | `ResolvedContext.ActionDataKey.ActionType/ActionIndex` | `Action(ComboAttack[1])` | HookNeeded |
| Reaction | `ResolvedContext.ReactionDataKey.ReactionType` | `Reaction(Hit)` | HookNeeded |

## 5. Guard Alias 검토

### 코드 근거

`CActionKeyTypes.h`에 Guard phase index가 명시되어 있다.

| Phase | 실제 ActionIndex |
| --- | --- |
| Guard In | `1` |
| Guard Out | `2` |
| Guard Hold | `3` |
| Guard Hit | `4` |
| Guard Parry | `5` |

`GetGuardActionPhaseIndex`와 `ResolveGuardActionPhase`가 index와 phase를 양방향으로 해석한다.

### 판단

`Guard[1] (Guard In)` 및 `Guard[2] (Guard Out)` 표시는 코드 근거가 충분하므로 Ready다.

단, `Guard[0] (Guard In)` / `Guard[1] (Guard Out)`처럼 overlay 전용 0-based 재번호화를 적용하는 것은 실제 runtime key와 달라 오해 소지가 있다. 이 방식은 사용자 표시 정책 결정이 필요하다.

### 권장 표시

P0.5에서는 실제 ActionIndex를 유지한다.

```text
Guard[1] (Guard In)
Guard[2] (Guard Out)
```

Guard Hold / Hit / Parry는 P0.5에서 표시 대상이 되면 같은 방식으로 확장한다.

```text
Guard[3] (Guard Hold)
Guard[4] (Guard Hit)
Guard[5] (Guard Parry)
```

### 분류

| 항목 | 분류 | 이유 |
| --- | --- | --- |
| 실제 index 유지 alias | Ready | `CActionKeyTypes.h` helper 근거 있음 |
| 0-based compact alias | ReviewNeeded | 실제 runtime key와 다름 |
| data asset display name 조회 | ReviewNeeded | `FActionData`에 display/semantic name 필드 없음 |

## 6. Store Compact Helper 재사용 검토

`FDebugOverlaySnapshotStore.cpp`의 `CompactEnumText`는 Store 내부 summary 생성 전용 helper다.

HUD current value compact를 위해 이 helper를 public API로 노출하면 다음 단점이 있다.

- SnapshotStore가 display formatting utility 역할까지 갖게 된다.
- header/API 변경 범위가 불필요하게 커진다.
- P0.5의 단순 표시 개선보다 구조 변경이 커진다.

따라서 P0.5에서는 HUD 내부 local helper 중복을 허용한다.

공용 helper는 P1에서 다음 조건이 생기면 별도 검토한다.

- Store/HUD 외 3개 이상 debug renderer가 같은 compact 규칙을 사용
- enum compact 규칙이 단순 `::` suffix 제거를 넘어설 때
- 표시 정책을 테스트 가능한 공용 함수로 고정할 필요가 생길 때

## 7. 구현 범위 제안

### Ready

- `CDebugOverlayHUD.cpp`
  - enum prefix compact helper 추가
  - State / Action / Reaction / Movement / HP current value compact 적용
  - Guard action alias 적용

### HookNeeded

- `FDebugOverlaySnapshotStore.h`
  - `RecordExecutionDecision`에 subject 인자 추가
- `FDebugOverlaySnapshotStore.cpp`
  - subject 포함 summary format 적용
  - subject empty fallback 유지
- `FExecutionOrchestratorDebug.cpp`
  - Action subject 생성 후 Store 호출에 전달
  - Reaction subject 생성 후 Store 호출에 전달

### ReviewNeeded

- Guard index를 overlay 전용 0-based로 재표기할지 여부
- Guard alias를 `Guard In` / `Guard Out` 대신 `GuardIn` / `GuardOut`으로 붙일지 여부
- 공용 compact formatter 분리 여부

### Exclude

- audit log 출력 format 변경
- gameplay component API 변경
- SnapshotTypes 구조 변경
- EventLog subject 분리
- Event category filter
- Player/Enemy panel layout 변경
- `Build.cs`, config, `.umap`, `.uasset` 변경

## 8. 구현 전 결정

다음 구현 단계에서는 아래 결정을 기준으로 진행하는 것을 권장한다.

1. HUD current value는 모두 enum prefix를 제거한다.
2. Execution summary는 `Action(Subject)` / `Reaction(Subject)` 형식을 사용한다.
3. Guard alias는 실제 index를 유지해 `Guard[1] (Guard In)`, `Guard[2] (Guard Out)`으로 표시한다.
4. Store compact helper는 public API로 노출하지 않는다.
5. audit log format은 변경하지 않는다.

## 9. 다음 구현 작업 후보

다음 작업은 `P0.5 compact display subject 구현`으로 분리한다.

예상 수정 파일:

- `Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp`
- `Source/Portfolio/Core/Debug/FExecutionOrchestratorDebug.cpp`

예상 검증:

- `PortfolioEditor Win64 Development` 빌드
- `git diff --check`
- PIE에서 다음 표시 확인
  - `State: Reaction`
  - `Action: ComboAttack[1]`
  - `Reaction: Hit`
  - `Movement: Gait=Run`
  - `HP: ... DeadState=Alive`
  - `Execution/DecisionResolved: Reaction(Hit) | Decision=Accept | Apply=Intervene | RejectReason=None`
  - `Action: Guard[1] (Guard In)` 또는 `Action: Guard[2] (Guard Out)`
