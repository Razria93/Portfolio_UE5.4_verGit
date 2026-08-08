# Debug Overlay P1 Player/Enemy EventLog Separation Implementation Plan

## 1. 목적

이 문서는 P1 debug overlay에서 Player/Enemy별 EventLog 분리를 구현하기 위한 계획 문서다.

직전 Store subject 분리 설계에서 `OwnerName / SourceName / TargetName` 기반 subject 분리 가능성을 확인했고, 이번 단계에서 Player/Enemy EventLog 분리는 P1에서 구현하는 것으로 결정한다.

이번 문서는 구현 전에 API 범위, HUD 배치, empty state, 검증 기준을 고정한다. 코드 구현은 다음 단계에서 진행한다.

## 2. 최종 결정

| 항목 | 결정 |
| --- | --- |
| Player/Enemy EventLog 분리 | P1에서 구현 |
| Common EventLog | 유지 |
| Store schema 대규모 변경 | 하지 않음 |
| subject 기준 | 기존 `OwnerName / SourceName / TargetName` 사용 |
| category filter | 기존 `Portfolio.DebugOverlay.EventLogFilter`와 결합 |
| Recent summary 분리 | 이번 구현에서는 제외, EventLog 분리 후 재판단 |
| Player/Enemy별 EventLogLimit 분리 | 하지 않음, 기존 `Portfolio.DebugOverlay.EventLogLimit` 공유 |

P1 구현은 "world 공통 EventLog를 없애는 작업"이 아니다. 기존 `[Event Log: Filter]`는 유지하고, Player/Enemy panel 안에 subject-specific EventLog 요약을 추가한다.

## 3. 현재 전제

현재 EventLog entry는 다음 필드를 가진다.

```text
Category
EventName
OwnerName
SourceName
TargetName
Summary
FrameNumber
WorldTimeSeconds
```

현재 category filter는 display/query 단계에서 적용된다.

```text
ring buffer 최신순 순회
-> category filter match
-> EventLogLimit 개수까지 수집
-> HUD 표시
```

Player/Enemy EventLog 분리도 record 단계가 아니라 query/display 단계에서 적용한다.

## 4. 구현 대상 파일

예상 구현 대상:

- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp`
- `Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp`

가능하면 변경하지 않을 파일:

- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h`
- 기존 debug hook 파일들
- `Build.cs`
- config / asset / map 파일

## 5. Store API 계획

P1에서는 최소 API 확장을 권장한다.

```text
static TArray<FDebugOverlayEventEntry> GetRecentEventsForSubjectCopy(
    const UObject* InWorldContextObject,
    int32 InMaxEvents,
    const FString& InFilter,
    const FString& InSubjectName);
```

동작:

1. world ring buffer를 최신순으로 순회한다.
2. `InFilter` 기준 category match를 먼저 적용한다.
3. `InSubjectName`이 `OwnerName / SourceName / TargetName` 중 하나와 일치하는지 확인한다.
4. match된 event만 `InMaxEvents` 개수까지 반환한다.

`InSubjectName`이 비어 있으면 empty array를 반환한다.

Shipping에서는 empty array를 반환한다.

## 6. Subject Match 정책

P1 subject match는 문자열 기반으로 한다.

```text
Entry.OwnerName == SubjectName
Entry.SourceName == SubjectName
Entry.TargetName == SubjectName
```

대소문자 변환이나 fuzzy match는 하지 않는다.

이유:

- `FDebugOverlayEventEntry`는 현재 actor pointer를 저장하지 않는다.
- P1 overlay는 debug evidence display이며 gameplay target system이 아니다.
- 이름 기반 match는 현재 코드 변경 범위를 작게 유지한다.

주의:

- actor rename / PIE suffix 차이는 실제 entry에 기록된 이름 기준으로 판단한다.
- 이름이 비어 있는 event는 subject-specific log에 들어가지 않는다.
- 하나의 combat event가 Player와 Enemy 양쪽에 모두 표시될 수 있다.

## 7. HUD 표시 계획

HUD는 기존 panel 항목 순서를 유지하고, 각 panel 아래에 EventLog 요약을 추가한다.

권장 위치:

```text
[Player]
State:
Action:
Reaction:
Stagger:
Guard:
Movement:
HP:
Runtime LOD:
AI:
PlayerEventLog:

[Enemy]
EnemyFocusMode:
EnemyFocusActor:
EnemyFocusCommand:

State:
Action:
Reaction:
Stagger:
Guard:
Movement:
HP:
Runtime LOD:
AI:
EnemyEventLog:

[Recent Execution]
[Recent Combat]
[Recent AI]
[Event Log: Filter]
```

Common `[Event Log: Filter]`는 그대로 유지한다.

## 8. 표시 형식

Player/Enemy EventLog는 한 줄 요약부터 시작한다.

성공 예:

```text
PlayerEventLog: Execution/DecisionResolved: Action(ComboAttack[0]) | Decision=Accept | Apply=Start | RejectReason=None
EnemyEventLog: CombatResult/Delivered: Outcome=Parry | DamageCommitted=false | Commit=0.000 | Receiver=BP_CEnemy_C_1
```

filter 결과 없음:

```text
PlayerEventLog: NoEvents(Filter=Combat)
EnemyEventLog: NoEvents(Filter=Combat)
```

Enemy focus 없음:

```text
EnemyEventLog: NoTarget
```

EventLogLimit 0:

```text
PlayerEventLog: NoEvents(Filter=Combat Limit=0)
EnemyEventLog: NoEvents(Filter=Combat Limit=0)
```

subject name 없음:

```text
PlayerEventLog: N/A
EnemyEventLog: NoTarget
```

## 9. 표시 line 수 정책

P1 최소 구현은 각 subject별 1줄 표시를 권장한다.

이유:

- 현재 overlay는 이미 Player/Enemy 상태, Recent summary, Common EventLog를 표시한다.
- subject-specific log를 3~5줄씩 추가하면 화면 가독성이 급격히 떨어질 수 있다.
- P1에서는 "분리 query가 동작한다"는 evidence가 우선이다.

후속 확장 후보:

- 각 subject별 2~3줄 표시
- `Portfolio.DebugOverlay.SubjectEventLogLimit` CVar 추가
- Common EventLog 숨김/표시 preset 추가

위 항목은 P1 기본 구현에서 제외한다.

## 10. Category Filter 결합

기존 `Portfolio.DebugOverlay.EventLogFilter`를 그대로 사용한다.

예:

```text
Portfolio.DebugOverlay.EventLogFilter Combat
```

기대:

- Common EventLog는 Combat 계열만 표시한다.
- PlayerEventLog는 Player가 관련된 Combat 계열 event 중 최신 1건을 표시한다.
- EnemyEventLog는 selected Enemy가 관련된 Combat 계열 event 중 최신 1건을 표시한다.

잘못된 filter 값은 기존 정책대로 `All` fallback을 사용한다.

## 11. 구현 제외 범위

이번 구현에서 제외한다.

- `FDebugOverlayEventEntry` schema 변경
- actor pointer를 EventLog entry에 저장
- Player/Enemy별 EventLogLimit CVar 추가
- Common EventLog 제거
- Recent Execution / Recent Combat / Recent AI subject 분리
- EventLog 추가 compact
- Player/Enemy별 3~5 line EventLog
- Runtime LOD actual 표시
- AI detail 보강
- `.umap`, `.uasset`, config, `Build.cs` 변경
- 최종 촬영/패키징

## 12. 위험 요소와 대응

| 위험 | 설명 | 대응 |
| --- | --- | --- |
| 이름 기반 match | actor identity를 완전히 보장하지 않는다. | debug evidence display 기준으로만 사용 |
| 화면 과밀 | subject log가 추가되면 overlay가 커진다. | P1은 subject별 1줄만 표시 |
| 중복 표시 | 같은 combat event가 Player/Enemy 양쪽에 표시될 수 있다. | 양쪽 subject가 모두 관련된 event로 설명 |
| selected Enemy 없음 | Enemy log를 만들 기준이 없다. | `EnemyEventLog: NoTarget` 표시 |
| filter 오해 | filter로 숨긴 event를 미발생처럼 해석할 수 있다. | 문서와 HUD에서 filter 기준을 유지 |
| Recent summary 혼동 | Recent block은 아직 world 단위다. | EventLog 분리와 별도라고 명시 |

## 13. 검증 계획

PIE에서 다음을 확인한다.

| 시나리오 | 기대 |
| --- | --- |
| PIE 진입 직후 | PlayerEventLog는 관련 event 또는 NoEvents, EnemyEventLog는 NoTarget |
| `EventLogFilter All` | Player/Enemy subject 기준 최신 event 표시 |
| `EventLogFilter Execution` | Player/Enemy별 Execution 관련 event만 표시 |
| `EventLogFilter Combat` | Player/Enemy별 Combat/CombatResult 관련 event만 표시 |
| `EventLogFilter AI` | 관련 AI event 또는 `NoEvents(Filter=AI)` |
| `EventLogLimit 0` | subject log도 `NoEvents(Filter=... Limit=0)` |
| `DebugOverlaySelectNearestFocus` | EnemyEventLog 기준 subject가 selected Enemy로 전환 |
| `DebugOverlayClearFocus` | EnemyEventLog가 `NoTarget`으로 복귀 |

## 14. 완료 기준

- Store에 subject-aware EventLog query가 추가된다.
- 기존 category filter 동작은 유지된다.
- Player panel에 `PlayerEventLog:`가 표시된다.
- Enemy panel에 `EnemyEventLog:`가 표시된다.
- selected Enemy가 없으면 `EnemyEventLog: NoTarget`이 표시된다.
- Common `[Event Log: Filter]` block은 유지된다.
- 코드/문서 claim이 실제 표시값과 일치한다.

## 15. 다음 작업

다음 작업은 `P1 Player/Enemy EventLog 분리 구현`이다.

구현 후에는 별도 PIE 체크리스트를 갱신하고, 수동 검증 결과를 문서화한다.
