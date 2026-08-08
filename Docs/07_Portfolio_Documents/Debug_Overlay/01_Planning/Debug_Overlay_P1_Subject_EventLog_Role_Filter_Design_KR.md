# Debug Overlay P1 Subject EventLog Role Filter Design

## 1. 목적

이 문서는 P1 debug overlay의 Player/Enemy EventLog가 같은 combat event를 양쪽 panel에 중복 표시하는 문제를 줄이기 위한 role-aware subject filter 정책을 고정한다.

현재 subject-specific EventLog는 `OwnerName`, `SourceName`, `TargetName` 중 하나라도 표시 대상 actor 이름과 일치하면 해당 panel에 event를 표시한다. 이 방식은 누락을 줄이는 데는 유리하지만, parry 판정, damage commit, CombatResult처럼 Player와 Enemy가 동시에 관여하는 event가 양쪽에 같이 표시되어 evidence 관점이 흐려진다.

이번 단계는 구현 문서가 아니라 정책 문서다. 코드 구현, Store schema 변경, EventLog compact 재작업, 최종 촬영은 하지 않는다.

## 2. 현재 문제

현재 `FDebugOverlaySnapshotStore::GetRecentEventsForSubjectCopy(...)`의 subject match는 다음과 같은 `Actor-Involved` 기준이다.

```text
Entry.OwnerName == SubjectName
Entry.SourceName == SubjectName
Entry.TargetName == SubjectName
```

이 정책은 다음 장점이 있다.

- Player/Enemy가 관여한 event를 쉽게 찾을 수 있다.
- 현재 `FDebugOverlayEventEntry` schema만으로 구현 가능하다.
- record path를 바꾸지 않고 query/display 단계에서만 분리할 수 있다.

하지만 다음 문제가 있다.

- 하나의 combat event가 Player와 Enemy panel 양쪽에 표시될 수 있다.
- `Parry`, `DamageCommitted`, `CombatResult`가 공격자 evidence인지 방어자 evidence인지 모호해진다.
- Player/Enemy panel이 "해당 actor 관점의 log"가 아니라 "해당 actor가 이름으로 등장한 log"처럼 보인다.

## 3. 목표

P1 role filter의 목표는 다음과 같다.

- Player/Enemy EventLog를 actor 관점으로 더 명확히 분리한다.
- event category와 event name별로 owner/source/target/receiver 의미를 다르게 적용한다.
- 중복 표시가 필요한 event와 제거해야 할 event를 구분한다.
- Common EventLog는 복구하지 않는다.
- Store ring buffer는 계속 world 단위 원본 event를 모두 보관한다.
- filter는 record 단계가 아니라 query/display 단계에서 적용한다.

## 4. 정책 후보

| 정책 | 의미 | 장점 | 단점 | P1 판단 |
| --- | --- | --- | --- | --- |
| `Actor-Involved` | owner/source/target 중 하나라도 actor와 일치하면 표시 | 누락이 적음 | 중복이 많고 관점이 흐림 | 현재 방식, 개선 대상 |
| `Actor-Owned` | owner actor가 일치하는 event만 표시 | 주체가 명확함 | receiver 중심 combat result가 누락될 수 있음 | Execution/AI에 적합 |
| `Actor-Received` | 피해/방어 결과를 받은 actor 중심으로 표시 | hit/parry/damage 해석이 명확함 | 공격자 쪽 evidence가 약해질 수 있음 | CombatResult에 적합 |
| `Role-Aware` | event별로 owner/source/target 기준을 다르게 적용 | evidence 관점이 가장 명확함 | 구현 조건이 복잡해짐 | P1 권장안 |

P1 최종 권장안은 `Role-Aware`다.

## 5. Role-Aware 기본 원칙

### 5.1 Execution

Execution event는 `OwnerName` 중심으로 표시한다.

```text
Execution/DecisionResolved
```

표시 기준:

- `OwnerName == PlayerName`이면 Player panel
- `OwnerName == SelectedEnemyName`이면 Enemy panel
- `SourceName`, `TargetName`은 Execution event에서 사용하지 않는다.

이유:

- Action/Reaction decision은 실행 주체가 명확하다.
- 현재 schema에서 Execution은 owner만 안정적으로 채워진다.

### 5.2 AI

AI event는 AI owner pawn 중심으로 표시한다.

```text
AI/CombatTask
```

표시 기준:

- `OwnerName == SelectedEnemyName` 또는 `SourceName == SelectedEnemyName`이면 Enemy panel
- `TargetName == PlayerName`만으로 Player panel에 표시하는 것은 P1 기본값에서 제외한다.

이유:

- AI task는 Enemy의 의사결정 evidence다.
- Player가 target으로 등장하더라도 Player가 실행한 event는 아니다.

### 5.3 Combat HitWindow / Collision

HitWindow와 collision state event는 weapon/action owner 중심으로 표시한다.

```text
Combat/CollisionEnabled
Combat/CollisionDisabled
Combat/CollisionDisabledIgnored
```

표시 기준:

- `OwnerName == SubjectName` 또는 `SourceName == SubjectName`이면 해당 actor panel
- `TargetName`이 비어 있으면 target match를 사용하지 않는다.

이유:

- HitWindow는 공격/무기 owner가 만든 window evidence다.
- target이 아직 확정되지 않은 event를 receiver panel에 표시하면 오해가 생긴다.

### 5.4 Combat Target Packet

Combat target packet은 source와 target이 모두 의미가 있다.

```text
Combat/TargetAccepted
Combat/TargetRejected
```

표시 기준:

- `SourceName == SubjectName`이면 공격/dispatch actor의 evidence로 표시 가능
- `TargetName == SubjectName` 또는 `OwnerName == SubjectName`이면 방어/피격 actor의 evidence로 표시 가능
- P1에서는 중복 표시를 허용한다.

이유:

- target packet은 공격자와 대상 actor가 모두 event 의미에 직접 관여한다.
- `Accepted`, `Rejected`, `Outcome`, `Final`, `Commit`은 양쪽 관점에서 모두 검증 가치가 있다.

단, HUD 문구에서 이것을 성공/실패 단독 evidence로 과장하지 않는다.

### 5.5 CombatResult

CombatResult는 receiver 중심을 우선한다.

```text
CombatResult/Received
CombatResult/Delivered
CombatResult/PacketReceived
CombatResult/PacketDelivered
```

표시 기준:

- `OwnerName == SubjectName`이면 해당 actor panel에 표시한다.
- `TargetName == SubjectName`이면 해당 actor panel에 표시할 수 있다.
- `SourceName == SubjectName`만 일치하는 경우는 기본 표시에서 제외한다.

이유:

- 현재 `RecordCombatResult(...)`는 `OwnerName`에 receiver actor 이름을 기록한다.
- `Outcome=Parry`, `DamageCommitted`, `Commit`, `Receiver=...`는 결과를 받은 actor 관점의 evidence다.
- 공격자 source match까지 허용하면 parry/damage 결과가 양쪽에 반복 표시된다.

P1 구현 시 `SourceName`만 일치하는 CombatResult가 필요하다고 판단되면 별도 `AttackerEventLog` 또는 Recent Combat 분리에서 다룬다.

### 5.6 CombatResult 대표 event 우선순위

CombatResult 계열은 같은 전투 결과가 `Delivering`, `Delivered`, `PacketReceived`처럼 여러 hook에서 기록될 수 있다. Store 원본 event는 보존하되, Player/Enemy subject panel은 해석용 compact evidence이므로 모든 중간 event를 같은 중요도로 반복 표시하지 않는다.

P1 권장 우선순위:

| Event | Subject panel 판단 | 이유 |
| --- | --- | --- |
| `CombatResult/PacketReceived` | receiver panel primary | 실제 receiver가 결과를 받은 event라 가장 명확함 |
| `CombatResult/Delivered` | receiver panel secondary | 전달 완료 evidence지만 `PacketReceived`와 의미가 겹침 |
| `CombatResult/Delivering` | subject panel 기본 제외 후보 | 중간 전달 상태라 최종 evidence로는 노이즈가 큼 |

P1 role filter 구현에서는 우선 source-only match를 제외해 Player/Enemy 양쪽 중복을 줄인다. 이후에도 같은 panel 안에서 CombatResult line이 과도하게 반복되면 `PacketReceived` 우선 표시 또는 `Delivering` 제외를 별도 low-risk cleanup으로 진행한다.

### 5.7 Parry / Damage Commit 해석 기준

Parry와 damage commit은 하나의 combat interaction 안에서 여러 event에 나뉘어 기록된다.

| Evidence | 대표 표시 위치 | 해석 |
| --- | --- | --- |
| `Combat/TargetAccepted: Outcome=Parry` | target/receiver panel primary | 방어 판정이 parry로 해석됨 |
| `CombatResult/PacketReceived` | receiver panel primary | parry/damage 결과가 actor에게 수신됨 |
| `Stagger: count/threshold` | actor status line | parry stack 현재값 |
| `Execution/DecisionResolved: Reaction(...)` | reaction owner panel | parry/hit/stagger reaction 실행 결정 |

따라서 parry evidence는 EventLog 한 줄에 모두 몰아넣지 않는다. `Outcome=Parry`, `CombatResult`, `Stagger`, `Reaction`을 각각 다른 역할의 evidence로 해석한다.

## 6. Event별 표시 기준 표

| Event | 표시 대상 | Match 기준 | 중복 허용 | 비고 |
| --- | --- | --- | --- | --- |
| `Execution/DecisionResolved` | 실행 주체 | `OwnerName` | 아니오 | Action/Reaction owner 기준 |
| `Combat/CollisionEnabled` | hit window owner | `OwnerName` 또는 `SourceName` | 아니오 | focus 없음 |
| `Combat/CollisionDisabled` | hit window owner | `OwnerName` 또는 `SourceName` | 아니오 | focus 없음 |
| `Combat/CollisionDisabledIgnored` | hit window owner | `OwnerName` 또는 `SourceName` | 아니오 | focus 없음 |
| `Combat/TargetAccepted` | source와 target | `SourceName` 또는 `TargetName` 또는 `OwnerName` | 예 | 양쪽 actor가 직접 관여 |
| `Combat/TargetRejected` | source와 target | `SourceName` 또는 `TargetName` 또는 `OwnerName` | 예 | reject 이유 확인용 |
| `CombatResult/Delivering` | receiver/target | `OwnerName` 또는 `TargetName` | 아니오 | subject panel 기본 제외 후보 |
| `CombatResult/Received` | receiver/target | `OwnerName` 또는 `TargetName` | 아니오 | source-only match 제외 |
| `CombatResult/Delivered` | receiver/target | `OwnerName` 또는 `TargetName` | 제한 | `PacketReceived`와 중복 가능 |
| `CombatResult/PacketReceived` | receiver/target | `OwnerName` 또는 `TargetName` | 아니오 | receiver 대표 event |
| `CombatResult/PacketDelivered` | receiver/target | `OwnerName` 또는 `TargetName` | 제한 | `PacketReceived`와 중복 가능 |
| `AI/CombatActionTaskSucceeded` | AI owner | `OwnerName` 또는 `SourceName` | 아니오 | target-only match는 기본 제외 |
| `AI/CombatActionTaskRejected` | AI owner | `OwnerName` 또는 `SourceName` | 아니오 | 실패 원인은 AI/Enemy evidence |
| `AI/CombatTask` | AI owner | `OwnerName` 또는 `SourceName` | 아니오 | generic fallback |

## 7. 현재 schema로 가능한 범위

현재 `FDebugOverlayEventEntry`는 다음 field를 가진다.

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

이 schema만으로 가능한 작업:

- category별 match 분기
- event name별 match 분기
- `OwnerName`, `SourceName`, `TargetName` 기반 role-aware match
- source-only CombatResult 제외
- AI target-only match 제외
- hit window target match 제외

이 schema만으로 애매한 작업:

- `OwnerName`이 receiver인지 executor인지 event별로 코드 지식 없이 판별
- `TargetName`과 `Receiver`의 차이를 항상 명확히 구분
- 공격자 관점 damage event와 방어자 관점 damage event를 동시에 정확히 분리
- 여러 actor가 같은 name을 갖는 상황 구분

P1에서는 schema 변경 없이 구현 가능한 범위만 적용한다.

## 8. Schema 변경이 필요한 후보

아래 항목은 P1 role filter 구현 중 필요성이 생기면 사용자 결정 대상으로 분리한다.

| 후보 field | 목적 | P1 판단 |
| --- | --- | --- |
| `Role` | Owner/Source/Target의 semantic role 명시 | P1 기본 구현에서는 보류 |
| `ReceiverName` | CombatResult receiver 명시 | 필요 시 DecisionNeeded |
| `InstigatorName` | 공격자와 source 구분 | P2 후보 |
| `SubjectHint` | HUD subject match용 대표 actor 지정 | P2 후보 |
| `bShowForSource`, `bShowForTarget` | event별 표시 대상 flag | P2 후보 |

actor raw pointer를 EventLog entry에 저장하는 것은 계속 금지한다.

## 9. 구현 영향

다음 구현 단계에서는 `FDebugOverlaySnapshotStore::GetRecentEventsForSubjectCopy(...)` 내부 match helper를 교체한다.

현재:

```text
DoesEventMatchSubject(Entry, SubjectName)
```

P1 role-aware 후보:

```text
DoesEventMatchSubjectByRole(Entry, SubjectName)
DoesExecutionEventMatchSubject(Entry, SubjectName)
DoesCombatEventMatchSubject(Entry, SubjectName)
DoesCombatResultEventMatchSubject(Entry, SubjectName)
DoesAIEventMatchSubject(Entry, SubjectName)
```

filter 적용 순서는 유지한다.

```text
1. world ring buffer 최신순 순회
2. category filter match
3. role-aware subject match
4. EventLogLimit 개수까지 수집
5. HUD 표시
```

HUD layout, EventLog block format, CVar, Store record path는 변경하지 않는다.

원본 timeline 확인을 위해 Store ring buffer에는 모든 event를 계속 저장한다. 다만 HUD 하단 Common EventLog block은 이미 제거된 정책을 유지한다. 즉, P1 subject panel은 해석용 compact evidence이고, 원본 event 보존은 Store 내부 책임으로 둔다.

## 10. 비목표

이번 설계와 다음 구현에서 하지 않을 항목:

- Common EventLog 복구
- EventLog compact 재작업
- Player/Enemy별 EventLogLimit CVar 추가
- Recent Execution / Recent Combat / Recent AI 분리 구현
- Runtime LOD actual 표시
- AI detail 보강
- 최종 촬영/패키징
- actor pointer를 EventLog entry에 저장
- gameplay flow 변경
- `.umap`, `.uasset`, config, `Build.cs` 변경

## 11. 검증 기준

PIE 검증에서는 다음을 확인한다.

| 시나리오 | 기대 |
| --- | --- |
| Player action/reaction | Player EventLog에 표시, Enemy에는 표시되지 않음 |
| Enemy action/AI task | Enemy EventLog에 표시, Player에는 target-only로 표시되지 않음 |
| Enemy hit window | Enemy EventLog에 표시, Player에는 target 없는 collision event로 표시되지 않음 |
| TargetAccepted/Rejected | source/target 양쪽 표시 가능 |
| Parry CombatResult | receiver/target actor panel 우선 표시, source-only 중복 감소 |
| DamageCommitted CombatResult | receiver/target actor panel 우선 표시 |
| EventLogFilter=Combat | Combat/CombatResult 중 role-aware match된 event만 표시 |
| EventLogLimit=0 | 각 panel block에 `NoEvents(Filter=... Limit=0)` 표시 |

검증 중 특정 event가 현재 schema로 충분히 분리되지 않으면 구현을 확장하지 말고 DecisionNeeded로 기록한다.

## 12. 완료 기준

- Player/Enemy EventLog 중복 표시를 줄이기 위한 role-aware 정책이 확정된다.
- 중복 허용 event와 중복 제거 대상 event가 표로 구분된다.
- 현재 schema로 구현 가능한 범위와 추가 결정이 필요한 범위가 분리된다.
- 다음 작업으로 `P1 Subject EventLog Role Filter 구현`을 진행할 수 있다.

## 13. 다음 작업

다음 작업은 `P1 Subject EventLog Role Filter 구현`이다.

구현 범위:

- `FDebugOverlaySnapshotStore.cpp`의 subject match helper를 role-aware 기준으로 변경
- Store API/schema/CVar 변경 없음
- HUD layout 변경 없음
- Common EventLog 복구 없음
- PIE 체크리스트 갱신은 구현 이후 별도 진행
