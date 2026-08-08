# Debug Overlay P1 Store Subject Separation Design

## 1. 목적

이 문서는 P1 debug overlay에서 Player/Enemy별 EventLog 또는 Recent summary를 분리하기 전에, Store event subject 기준을 확정하기 위한 설계 문서다.

현재 EventLog는 world 단위 공통 ring buffer에서 category filter만 적용해 표시한다. 이 구조는 `All / Execution / Combat / AI` 검증에는 충분하지만, Player와 Enemy 각각에 관련된 event를 분리해서 설명하기에는 subject 기준이 부족하다.

이번 문서는 구현 문서가 아니다. 구현 전에 `Owner / Source / Target / selected Enemy` 중 어떤 기준을 신뢰하고, 어떤 항목은 P1에서 보류할지 고정한다.

## 2. 현재 구조

`FDebugOverlayEventEntry`는 이미 subject 분리에 사용할 최소 필드를 가진다.

| 필드 | 현재 의미 |
| --- | --- |
| `Category` | `Execution`, `Combat`, `CombatResult`, `AI` 같은 event 분류 |
| `EventName` | hook에서 전달한 event 이름 |
| `OwnerName` | event를 소유하거나 수신한 actor/controller/pawn 이름 |
| `SourceName` | combat 또는 AI event의 source actor 이름 |
| `TargetName` | combat 또는 AI event의 target actor 이름 |
| `Summary` | HUD 표시용 compact summary |
| `FrameNumber` | 기록 시점 frame |
| `WorldTimeSeconds` | 기록 시점 world time |

Store는 `World`별 `FDebugOverlayWorldStore`에 event ring buffer를 유지한다. 현재 EventLog category filter는 record 단계가 아니라 조회/display 단계에서 적용된다.

```text
Debug hook
-> FDebugOverlaySnapshotStore record API
-> World별 ring buffer에 모든 event 저장
-> HUD draw 시점에 category filter 적용
```

이 정책은 subject 분리 후에도 유지한다. record 단계에서 event를 버리지 않는다.

## 3. 현재 Record 경로별 Subject 현황

| Category | Record API | Owner | Source | Target | 신뢰도 |
| --- | --- | --- | --- | --- | --- |
| `Execution` | `RecordExecutionDecision` | `InOwnerActor` | 비어 있음 | 비어 있음 | Owner 기준만 신뢰 가능 |
| `Combat` hit window | `RecordWeaponCollisionWindow` | `InOwnerActor` | `InOwnerActor` | 비어 있음 | Source 기준 가능, focus 없음 |
| `Combat` target packet | `RecordCombatTargetPacket` | `TargetActor` | `SourceActor` | `TargetActor` | Source/Target 기준 신뢰 가능 |
| `CombatResult` | `RecordCombatResult` | `InReceiverActor` | `SourceActor` | `TargetActor` | Source/Target/Receiver 기준 신뢰 가능 |
| `AI` | `RecordAICombatTask` | `InOwnerPawn` | `InOwnerPawn` | `InTargetActor` | Pawn/Target 기준 신뢰 가능 |

결론:

- `Combat` / `CombatResult` / `AI`는 subject 분리 근거가 비교적 명확하다.
- `Execution`은 현재 owner만 채워져 있어 Player/Enemy 분리 시 owner 기반으로만 분류하는 것이 안전하다.
- hit window 계열 `Combat` event는 target이 없으므로 source 중심으로만 분류한다.

## 4. Subject 후보 비교

| 기준 | 장점 | 한계 | P1 판단 |
| --- | --- | --- | --- |
| `OwnerName` | 모든 event에 가장 가깝게 존재함 | combat에서 receiver/owner 의미가 event별로 다름 | 보조 기준 |
| `SourceName` | 공격자/행위자 기준 설명에 강함 | Execution event에는 없음 | Combat/AI 중심 기준 |
| `TargetName` | 피격자/대상 기준 설명에 강함 | hit window처럼 target 없는 event가 있음 | Enemy panel 분리에 중요 |
| selected Enemy | 현재 HUD Enemy panel과 직접 연결 가능 | 선택 이전/이후 event 해석이 달라질 수 있음 | Enemy EventLog 기준 |
| Player perspective | 사용자가 보는 evidence 구조와 맞음 | 양방향 combat event 중복 가능 | HUD 표시 가공 기준 |
| Enemy perspective | 선택 Enemy 설명에 강함 | selected Enemy가 없으면 표시 불가 | HUD 표시 가공 기준 |

## 5. 권장 분리 정책

P1의 권장 정책은 `world 공통 EventLog 유지 + subject-specific query 추가`다.

### 5.1 Common EventLog

현재 `[Event Log: Filter]` block은 유지한다.

역할:

- world 단위 전체 event 흐름 확인
- category filter 검증 유지
- subject 분리 실패 시 fallback diagnostic

### 5.2 Player EventLog

Player EventLog는 Player pawn 이름이 아래 중 하나와 일치하는 event를 포함한다.

```text
OwnerName == PlayerName
SourceName == PlayerName
TargetName == PlayerName
```

의미:

- Player가 action/reaction owner인 Execution event
- Player가 공격 source인 Combat event
- Player가 피격 target/receiver인 CombatResult event
- Player와 직접 관련된 AI/Combat event

### 5.3 Enemy EventLog

Enemy EventLog는 현재 `FocusComponent.NearestFocus`로 선택된 Enemy 이름이 아래 중 하나와 일치하는 event를 포함한다.

```text
OwnerName == SelectedEnemyName
SourceName == SelectedEnemyName
TargetName == SelectedEnemyName
```

selected Enemy가 없으면 Enemy EventLog는 다음처럼 표시한다.

```text
EnemyEventLog: NoTarget
```

선택된 Enemy가 있으나 filter 결과가 없으면 다음처럼 표시한다.

```text
EnemyEventLog: NoEvents(Filter=Combat)
```

### 5.4 Subject 없는 Event

`OwnerName / SourceName / TargetName`이 모두 비어 있거나 신뢰할 수 없는 event는 subject-specific log에 넣지 않는다.

권장 분류:

```text
Common only
```

필요 시 P2에서 `Unattributed` diagnostic 표시를 검토한다.

## 6. Category별 분류 규칙

| Category | Player 분리 기준 | Enemy 분리 기준 | 주의 |
| --- | --- | --- | --- |
| `Execution` | `OwnerName == PlayerName` | `OwnerName == SelectedEnemyName` | Source/Target이 없으므로 owner만 사용 |
| `Combat` hit window | `OwnerName` 또는 `SourceName` match | `OwnerName` 또는 `SourceName` match | focus 없음 |
| `Combat` target packet | owner/source/target match | owner/source/target match | 양쪽 panel에 같은 event가 중복 표시될 수 있음 |
| `CombatResult` | owner/source/target match | owner/source/target match | receiver가 owner로 들어옴 |
| `AI` | target이 Player인 경우 포함 가능 | owner/source가 selected Enemy인 경우 포함 | AI 성공 evidence로 과장 금지 |

중복 표시는 실패가 아니다. 하나의 combat event는 Player와 Enemy가 동시에 관여한 사건이므로 양쪽 subject log에 모두 들어갈 수 있다.

## 7. Filter 적용 순서

subject filter와 category filter를 함께 사용할 경우 순서는 다음으로 고정한다.

```text
1. world ring buffer 최신순 순회
2. category filter match
3. subject match
4. EventLogLimit 개수까지 수집
5. HUD 표시
```

이 순서를 쓰는 이유:

- category filter가 현재 P1 검증 완료 기능이므로 의미를 유지한다.
- subject filter는 그 위에 추가되는 표시 분기다.
- record 단계에서 event를 제거하지 않는다.

## 8. Store API 후보

P1 구현 후보 API:

```text
GetRecentEventsCopy(WorldContext, MaxEvents, CategoryFilter)
GetRecentEventsForSubjectCopy(WorldContext, MaxEvents, CategoryFilter, SubjectName)
```

또는 subject query 조건을 구조체로 분리할 수 있다.

```text
FDebugOverlayEventQuery
- CategoryFilter
- SubjectName
- bMatchOwner
- bMatchSource
- bMatchTarget
```

P1 권장안은 최소 overload 추가다. query 구조체 도입은 조건이 늘어날 때 P2로 넘긴다.

## 9. HUD 출력 후보

P1에서 바로 구현할 경우 HUD는 다음 후보 중 하나를 선택한다.

### 후보 A: Common EventLog 유지 + subject-specific summary 추가

```text
[Player]
...
PlayerEventLog: ...

[Enemy]
...
EnemyEventLog: ...

[Event Log: Combat]
...
```

장점:

- 기존 common EventLog를 유지해 회귀 위험이 낮다.
- Player/Enemy 분리 결과를 작게 검증할 수 있다.

단점:

- 화면 line 수가 늘어난다.

### 후보 B: Common EventLog를 Player/Enemy EventLog로 대체

```text
[Player Event Log: Combat]
...

[Enemy Event Log: Combat]
...
```

장점:

- 최종 evidence에서 subject 분리가 명확하다.

단점:

- 기존 category filter 검증 화면과 달라진다.
- `NoTarget`, `NoEvents` 상태가 많으면 화면이 복잡해질 수 있다.

P1 권장안은 후보 A다. Common EventLog를 유지하고, subject-specific 출력은 보강 block으로 단계적으로 추가한다.

## 10. 위험 요소

| 위험 | 설명 | 대응 |
| --- | --- | --- |
| 이름 기반 match 한계 | 현재 event entry는 actor pointer가 아니라 이름 문자열을 저장한다. | P1에서는 evidence display 기준으로만 사용 |
| selected Enemy 변경 | 과거 event가 현재 selected Enemy 기준으로 재해석될 수 있다. | 시간/frame 표시 또는 "current selected 기준" 문구 유지 |
| 중복 표시 | Player/Enemy 양쪽에 같은 combat event가 표시될 수 있다. | 양쪽 subject가 모두 관련된 event로 설명 |
| subject 없는 event | Execution 이외 일부 event가 target을 갖지 않을 수 있다. | Common only 처리 |
| 가독성 | panel line 증가로 overlay가 커질 수 있다. | P1에서는 line 수 제한 또는 one-line summary 우선 |
| evidence 과장 | subject match가 약한 event를 성공 evidence로 보일 수 있다. | 실제 채워진 subject만 사용 |

## 11. P1/P2 결정 후보

| 항목 | 권장 |
| --- | --- |
| Player/Enemy EventLog 완전 분리 | P1 보강 또는 P2 |
| Common EventLog 유지 | P1 유지 |
| Recent Execution subject 분리 | P2 후보 |
| Recent Combat subject 분리 | P1 보강 후보 |
| Recent AI subject 분리 | P1 후반 또는 P2 |
| Store schema 확장 | P1에서는 가급적 피함 |
| Query 구조체 도입 | P2 후보 |

## 12. 구현 전 체크리스트

- hook별 `OwnerName / SourceName / TargetName` 채움 상태를 다시 확인한다.
- selected Enemy 이름을 HUD에서 안정적으로 얻을 수 있는지 확인한다.
- subject 없는 event는 Common only로 처리한다.
- category filter와 subject filter 적용 순서를 유지한다.
- Player/Enemy 양쪽 중복 표시를 실패로 보지 않는다고 문서화한다.
- 화면 line 수 증가가 캡처 가독성을 해치지 않는지 확인한다.
- 실제 코드에서 읽지 못하는 subject는 성공 evidence처럼 표시하지 않는다.

## 13. 다음 작업

다음 작업은 `Player/Enemy Recent/EventLog 분리 구현 여부 결정`이다.

결정해야 할 항목:

1. P1에서 subject-specific EventLog를 구현할지, P2로 넘길지
2. Common EventLog를 유지하면서 보강 block만 추가할지
3. Recent summary까지 subject 분리할지
4. Store API를 overload로 확장할지 query 구조체로 확장할지

## 14. 결론

P1 Store subject 분리는 기존 `FDebugOverlayEventEntry`의 `OwnerName / SourceName / TargetName`을 우선 활용한다.

P1 권장 방향은 Store schema를 크게 바꾸지 않고, category filter 이후 subject match를 적용하는 query를 추가하는 것이다. Common EventLog는 유지하고, Player/Enemy subject-specific 표시를 보강 block으로 추가할지 여부를 다음 결정 단계에서 확정한다.
