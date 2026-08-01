# Debug Overlay P1 Subject EventLog Semantics Cleanup Plan

## 1. 목적

이 문서는 P1 debug overlay의 Player/Enemy subject EventLog에서 확인된 두 가지 해석 문제를 정리하고, 후속 구현 기준을 고정한다.

- `Combat/TargetAccepted(Incoming/Outgoing)` label이 Parry/Guard 상황에서 직관과 어긋나 보이는 문제
- `CombatResult/Delivering`, `CombatResult/Delivered`, `CombatResult/PacketReceived`가 함께 표시되어 같은 결과가 여러 번 적용된 것처럼 보이는 문제

이번 단계는 설계 문서 작업이다. 코드 구현, 촬영, 패키징, `.umap`, `.uasset`, config, `Build.cs` 변경은 하지 않는다.

## 2. 문제 배경

현재 Player/Enemy subject EventLog는 `Combat/TargetAccepted`와 `Combat/TargetRejected`에 대해 `Incoming / Outgoing / Self` label을 붙인다.

이 label은 `SourceName`, `TargetName`, `OwnerName` 기준으로 붙으며, 원래 의미는 다음과 같다.

| 현재 label | 기존 의미 |
| --- | --- |
| `Incoming` | subject가 target/receiver 쪽이다. |
| `Outgoing` | subject가 source/dispatch 쪽이다. |
| `Self` | subject가 source와 target 양쪽에 모두 해당한다. |

이 기준은 target packet 방향성에는 맞지만, Parry/Guard evidence를 읽는 방식과 충돌할 수 있다. `Outcome=Parry`, `Outcome=Guard`는 자연스럽게 방어자 관점으로 읽히기 때문이다.

예를 들어 Enemy가 공격하고 Player가 Parry한 상황에서 Player panel은 `Incoming`, Enemy panel은 `Outgoing`으로 표시된다. 이는 "공격 packet이 Player에게 들어왔다"는 의미로는 맞지만, 화면을 보는 사람은 "Player가 Parry를 수행했다"는 관점으로 읽기 때문에 뒤집힌 것처럼 보일 수 있다.

## 3. 용어 정리 문제

`Incoming / Outgoing`은 방향성 용어다.

하지만 P1 debug overlay의 목적은 최종 evidence에서 다음을 빠르게 설명하는 것이다.

- 누가 공격자인가
- 누가 방어자인가
- Parry/Guard/Hit 결과가 어느 actor 관점의 evidence인가

따라서 target packet label은 방향성보다 역할 기반 이름이 더 적합하다.

권장 용어:

| 기존 | 변경 권장 |
| --- | --- |
| `Incoming` | `Defender` |
| `Outgoing` | `Attacker` |
| `Self` | `Self` |

## 4. Target Packet Label 최종 권장안

P1 후속 구현에서는 `Combat/TargetAccepted`, `Combat/TargetRejected` label을 다음 기준으로 바꾼다.

```text
1. SourceName == SubjectName && TargetName == SubjectName -> Self
2. SourceName == SubjectName -> Attacker
3. TargetName == SubjectName || OwnerName == SubjectName -> Defender
4. 그 외 -> label 없음
```

적용 대상은 기존과 동일하다.

- `Combat/TargetAccepted`
- `Combat/TargetRejected`

적용하지 않는 대상도 기존과 동일하다.

- `Execution/*`
- `AI/*`
- `Combat/Collision*`
- `CombatResult/*`
- unknown category/event

## 5. 표시 예시

### 5.1 Player가 Enemy를 공격하는 경우

```text
[Player]
[Event Log: Combat]
Combat/TargetAccepted(Attacker): Outcome=None | Final=15.000 | Commit=15.000 | Accepted=true

[Enemy]
[Event Log: Combat]
Combat/TargetAccepted(Defender): Outcome=None | Final=15.000 | Commit=15.000 | Accepted=true
```

해석:

- Player는 source/dispatch actor이므로 `Attacker`
- Enemy는 target/receiver actor이므로 `Defender`

### 5.2 Enemy가 Player를 공격하고 Player가 Parry/Guard한 경우

```text
[Player]
[Event Log: Combat]
Combat/TargetAccepted(Defender): Outcome=Parry | Final=0.000 | Commit=0.000 | Accepted=true

[Enemy]
[Event Log: Combat]
Combat/TargetAccepted(Attacker): Outcome=Parry | Final=0.000 | Commit=0.000 | Accepted=true
```

해석:

- Player는 방어/피격 판정을 받은 actor이므로 `Defender`
- Enemy는 공격 packet source이므로 `Attacker`

`Outcome=Parry`, `Outcome=Guard`가 붙은 line에서 `Defender`가 보이면 최종 evidence 설명이 더 자연스럽다.

### 5.3 Self 예외

source와 target이 같은 actor로 기록되는 예외 상황은 계속 `Self`를 사용한다.

```text
Combat/TargetAccepted(Self): Outcome=None | Final=0.000 | Commit=0.000 | Accepted=true
```

P1 기본 검증에서 `Self`를 반드시 재현할 필요는 없다.

## 6. CombatResult Event 의미 정리

현재 overlay에는 다음 CombatResult 계열 event가 표시될 수 있다.

| Event | 의미 | P1 subject panel 판단 |
| --- | --- | --- |
| `CombatResult/Delivering` | combat result를 receiver에게 보내는 dispatch 시점 진단 | primary evidence로 부적합할 수 있음 |
| `CombatResult/Delivered` | combat result dispatch 완료 진단 | `PacketReceived`와 중복 해석 가능 |
| `CombatResult/PacketReceived` | receiver가 실제 combat result packet을 받은 hook | subject panel primary evidence로 가장 적합 |

현재 화면에서는 `Delivering`, `Delivered`, `PacketReceived`가 한 panel에 함께 표시되어 damage/parry result가 여러 번 발생한 것처럼 보일 수 있다.

P1 subject EventLog의 목적은 compact evidence이므로, dispatch 과정 전체를 모두 보여주는 것보다 receiver가 실제 결과를 받은 event를 우선하는 편이 명확하다.

## 7. CombatResult 표시 정책 권장안

후속 구현에서는 Player/Enemy subject EventLog에서 `CombatResult/PacketReceived`를 primary evidence로 둔다.

권장 정책:

- `CombatResult/PacketReceived`
  - subject panel 표시 유지
  - receiver-side evidence로 사용
- `CombatResult/Delivering`
  - subject panel 기본 표시에서 제외 권장
  - dispatch diagnostic 후보로 격하
- `CombatResult/Delivered`
  - subject panel 기본 표시에서 제외 권장
  - `PacketReceived`와 중복되는 경우가 많으므로 diagnostic 후보로 격하

subject match 기준:

```text
CombatResult primary match:
OwnerName == SubjectName
```

보류/검토:

- `TargetName == SubjectName`
  - 현재 combat result packet의 `TargetActor`는 실제 receiver라기보다 원래 피격/방어 actor 의미로 남을 수 있다.
  - Parry 흐름에서는 result receiver가 `SourceActor` 쪽으로 선택될 수 있으므로 `TargetName`을 receiver match로 간주하면 다시 중복/오해가 생길 수 있다.
  - 따라서 P1 cleanup 구현에서는 `OwnerName == SubjectName` 중심을 기본으로 두고, `TargetName` match는 기본 제외 후보로 본다.
- `SourceName == SubjectName`
  - source-only match는 계속 제외한다.

## 8. 현재 화면 기준 의도와 다른 점

최근 PIE 화면에서 다음 문제가 확인되었다.

- `Outcome=Parry`, `Outcome=Guard`가 붙은 target packet에 `Incoming/Outgoing`이 붙어 공격자/방어자 관점이 섞여 보인다.
- Player/Enemy 양쪽에서 같은 target packet이 표시되는 것은 허용되지만, label이 방향성 용어라 evidence 해석에 즉시 도움이 되지 않는다.
- `CombatResult/Delivering`, `CombatResult/Delivered`, `CombatResult/PacketReceived`가 반복 표시되어 같은 parry/damage result가 여러 번 처리된 것처럼 보인다.
- `EventLogFilter=All`은 검증 대상이 섞이므로 role label 검증에는 `Portfolio.DebugOverlay.EventLogFilter Combat`이 더 적합하다.
- `EnemySelect` radius가 문서 기본값과 다르게 보이는 경우, target selection tuning 문서와 코드 정합성 검토가 별도로 필요하다.

## 9. 후속 구현 방향

후속 구현은 가능하면 `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp` 중심으로 제한한다.

구현 후보:

1. subject role label 문자열 변경
   - `Incoming` -> `Defender`
   - `Outgoing` -> `Attacker`
   - `Self` 유지
2. CombatResult subject match 조정
   - `PacketReceived` 중심
   - `OwnerName == SubjectName` 우선
   - `SourceName` 단독 match 제외 유지
3. `Delivering` / `Delivered` subject panel 노출 제한
   - Store 원본 ring buffer record는 유지
   - subject display query에서 제외하거나 후순위 diagnostic으로 격하
4. PIE checklist 갱신
   - `Incoming/Outgoing` 기준 문구를 `Attacker/Defender` 기준으로 교체

Store schema, public API, CVar, HUD layout은 변경하지 않는다.

## 10. 비목표

이번 설계와 후속 구현에서 제외할 항목:

- `FDebugOverlayEventEntry` schema 변경
- actor pointer 저장
- CVar 추가
- HUD layout 변경
- Recent block 분리
- Player/Enemy별 EventLogLimit 추가
- Runtime LOD actual 표시
- AI detail 보강
- 최종 촬영/패키징
- `.umap`, `.uasset`, config, `Build.cs` 변경

## 11. 검증 기준

후속 구현 후 PIE에서 다음을 확인한다.

| 시나리오 | 기대 |
| --- | --- |
| Player가 Enemy를 공격 | Player `Attacker`, Enemy `Defender` |
| Enemy가 Player를 공격 | Enemy `Attacker`, Player `Defender` |
| Parry/Guard outcome | 방어 actor panel에 `Defender` label |
| TargetRejected | `TargetAccepted`와 같은 label 기준 |
| CombatResult PacketReceived | receiver subject panel 중심 표시 |
| CombatResult Delivering/Delivered | subject panel 기본 노이즈 감소 |
| EventLogFilter=Combat | role label과 CombatResult 정책 확인 가능 |
| EventLogLimit=0 | 기존 `NoEvents(Filter=Combat Limit=0)` 유지 |

## 12. 완료 기준

- `Incoming/Outgoing` 혼동 원인이 문서에 설명되어 있다.
- `Attacker/Defender/Self` label 기준이 확정되어 있다.
- `CombatResult/Delivering`, `Delivered`, `PacketReceived`의 의미와 표시 우선순위가 정리되어 있다.
- 후속 구현 범위가 Store display/query 중심으로 제한되어 있다.
- 기존 Store schema/API/CVar/HUD layout을 유지하는 방향이 명시되어 있다.

## 13. 다음 작업

다음 작업은 `P1 Subject EventLog Semantics Cleanup 구현`이다.

구현 단계에서는 `FDebugOverlaySnapshotStore.cpp`의 subject role label helper와 CombatResult subject match helper를 최소 범위로 조정한다.
