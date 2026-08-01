# Debug Overlay P1 EventLog Subject Role Label Design

## 1. 목적

이 문서는 P1 debug overlay의 Player/Enemy EventLog에서 `Combat/TargetAccepted`, `Combat/TargetRejected`가 양쪽 panel에 표시될 때 방향성을 구분하기 위한 subject role label 정책을 고정한다.

직전 role-aware subject filter 구현 이후에도 target packet 계열 event는 source와 target 양쪽 actor가 모두 직접 관여하므로 Player/Enemy 양쪽에 표시될 수 있다. 이 중복은 의도된 동작이지만, 같은 line이 양쪽에 그대로 보이면 어느 쪽이 공격을 보냈고 어느 쪽이 받은 것인지 읽기 어렵다.

이번 작업은 중복 제거가 아니라 표시 의미 보강이다.

## 2. 문제 배경

현재 `Combat/TargetAccepted`, `Combat/TargetRejected`는 다음 이유로 양쪽 표시가 허용되어 있다.

- source actor는 공격/packet dispatch 관점에서 직접 관여한다.
- target actor는 방어/피격/판정 관점에서 직접 관여한다.
- `Outcome`, `Final`, `Commit`, `Accepted` 값은 양쪽 관점 모두에서 evidence 가치가 있다.

하지만 방향 label이 없으면 다음 문제가 생긴다.

- Player panel과 Enemy panel에 같은 event가 반복 표시되는 것처럼 보인다.
- `Outcome=Parry`가 누구에게 들어온 판정인지 즉시 구분하기 어렵다.
- damage/parry evidence를 설명할 때 source/target 관계를 별도로 말해야 한다.

## 3. 최종 정책

P1에서는 `TargetAccepted` / `TargetRejected`에 한해 subject 관점 label을 붙인다.

사용 label:

| Label | 의미 |
| --- | --- |
| `Incoming` | subject가 target/receiver 쪽이다. |
| `Outgoing` | subject가 source/dispatch 쪽이다. |
| `Self` | subject가 source와 target 양쪽에 모두 해당한다. |

정책:

- EventLog 중복 자체는 제거하지 않는다.
- label은 evidence 해석 보조 정보다.
- label은 성공/실패 의미를 갖지 않는다.
- label은 subject-specific EventLog copy에만 적용한다.
- 원본 Store ring buffer event는 수정하지 않는다.

## 4. Label 판정 기준

판정 순서는 다음으로 고정한다.

```text
1. SourceName == SubjectName && TargetName == SubjectName -> Self
2. SourceName == SubjectName -> Outgoing
3. TargetName == SubjectName || OwnerName == SubjectName -> Incoming
4. 그 외 -> label 없음
```

`OwnerName`은 현재 `RecordCombatTargetPacket(...)`에서 target actor 이름으로 기록된다. 따라서 target packet 계열에서 `OwnerName == SubjectName`은 `Incoming`으로 해석한다.

## 5. 표시 예시

Enemy가 공격을 보내고 Player가 판정을 받은 경우:

```text
[Player]
[Event Log: Combat]
Combat/TargetAccepted(Incoming): Outcome=Parry | Final=0.000 | Commit=0.000 | Accepted=true

[Enemy]
[Event Log: Combat]
Combat/TargetAccepted(Outgoing): Outcome=Parry | Final=0.000 | Commit=0.000 | Accepted=true
```

Player가 Enemy를 공격한 경우:

```text
[Player]
[Event Log: Combat]
Combat/TargetAccepted(Outgoing): Outcome=None | Final=15.000 | Commit=15.000 | Accepted=true

[Enemy]
[Event Log: Combat]
Combat/TargetAccepted(Incoming): Outcome=None | Final=15.000 | Commit=15.000 | Accepted=true
```

같은 actor가 source와 target 양쪽에 들어오는 예외 상황:

```text
Combat/TargetAccepted(Self): Outcome=None | Final=0.000 | Commit=0.000 | Accepted=true
```

## 6. 적용 범위

이번 단계에서 label을 붙이는 event:

- `Combat/TargetAccepted`
- `Combat/TargetRejected`

event name matching은 현재 Store에 저장되는 `EventName` 문자열 기준으로 처리한다.

이번 단계에서 label을 붙이지 않는 event:

- `Execution/*`
- `AI/*`
- `Combat/Collision*`
- `CombatResult/*`
- unknown category/event

## 7. 구현 방향

Store schema와 public API를 변경하지 않는다.

구현 위치:

```text
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp
```

권장 helper:

```text
GetSubjectEventRoleLabel(Entry, SubjectName)
MakeSubjectDisplayEventEntry(Entry, SubjectName)
```

동작:

1. `GetRecentEventsForSubjectCopyFromStore(...)`가 ring buffer를 최신순으로 순회한다.
2. category filter와 role-aware subject match를 통과한다.
3. result에 추가하기 직전에 event entry copy를 만든다.
4. copy의 `EventName`에만 `(Incoming)`, `(Outgoing)`, `(Self)` suffix를 붙인다.
5. 원본 ring buffer entry는 수정하지 않는다.

HUD는 기존 `FormatEventLogEntryLine(...)`을 그대로 사용한다.

## 8. 비목표

이번 작업에서 하지 않을 항목:

- `FDebugOverlayEventEntry` schema 변경
- Store public API 변경
- CVar 추가
- Common EventLog 복구
- role-aware match 정책 변경
- CombatResult role label 확장
- EventLog compact 재작업
- Recent block 분리
- HUD layout 변경
- 최종 촬영/패키징
- `.umap`, `.uasset`, config, `Build.cs` 변경

## 9. 검증 기준

PIE 검증에서는 다음을 확인한다.

| 시나리오 | 기대 |
| --- | --- |
| Enemy가 Player를 공격 | Player EventLog는 `Incoming`, Enemy EventLog는 `Outgoing` |
| Player가 Enemy를 공격 | Player EventLog는 `Outgoing`, Enemy EventLog는 `Incoming` |
| `TargetRejected` 발생 | `TargetAccepted`와 같은 label 기준 적용 |
| `CombatResult` 표시 | label이 붙지 않음 |
| `Collision` 표시 | label이 붙지 않음 |
| `EventLogFilter=Combat` | label이 붙은 target packet과 기존 CombatResult filter 포함 유지 |
| `EventLogLimit=0` | label 처리와 무관하게 `NoEvents(Filter=... Limit=0)` 유지 |

## 10. 완료 기준

- `TargetAccepted` / `TargetRejected`의 subject 관점이 `Incoming / Outgoing / Self`로 구분된다.
- Store 원본 ring buffer event는 변경되지 않는다.
- Store API/schema/CVar/HUD layout은 변경되지 않는다.
- role-aware match 정책은 유지된다.
- 빌드와 `git diff --check`가 통과한다.

## 11. 다음 작업

다음 작업은 `P1 EventLog Subject Role Label PIE 체크리스트 갱신`이다.

구현 이후에는 Player/Enemy EventLog에서 target packet 방향성이 기대대로 보이는지 PIE에서 확인한다.
