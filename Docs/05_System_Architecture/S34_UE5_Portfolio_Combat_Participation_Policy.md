# S34. Combat Participation 정책

> 상태: Evidence 중심 참여 구조 구현 및 정적·PIE 최종 검증 완료.

## 1. 핵심 모델

```text
Perception Evidence / HitReactive Evidence
→ live Evidence aggregate
→ Candidate
→ None / Observe / Alert / Engage Assignment
→ CombatTarget / Facing / Blackboard / Intent
```

`Assignment != None`만이 전투 참여 상태의 권위다. `None`은 CombatTarget을 clear하는
비전투 상태다. Session, Session phase, revision, commitment는 사용하지 않는다.

Perception과 HitReactive는 같은 Evidence → candidate → allocator 경로를 사용한다.
차이는 Evidence 수명과 HitReactiveExtra admission 자격뿐이다.

`Combat Participation Pair`는 하나의 `Participant × Target` 조합이다. Participant는 Evidence를
등록하는 Enemy AI Controller이고, Target은 그 Controller가 인지하거나 피격으로 확인한 Actor다.
같은 Player를 여러 Enemy가 인지해도 각 Pair의 Evidence, Assignment, Last Known Target Context는
서로 공유하지 않는다.

## 2. Active Evidence

Active Evidence는 Candidate, Assignment, Extra 자격의 유일한 근거다. 비활성 또는 만료된
Evidence는 active Evidence Registry에서 즉시 제거해야 하며, stale Candidate나 Extra 자격으로
남아서는 안 된다.

- LOS success는 Perception Evidence를 등록 또는 갱신한다.
- LOS false는 `TargetMemoryTimeout` 동안 Perception Evidence를 유지하고, memory 만료 시에만
  source를 철회한다.
- 수용된 hostile damage / Guard / Parry 결과는 HitReactive Evidence를 등록 또는 갱신한다.
- 새 유효 hit는 기존 ResultSerial과 대기 중인 만료 작업을 대체한다.
- 다른 live source가 남아 있는 동안 source 하나가 끝나면 해당 source만 제거한다. 이 시점에는
  참여 종료나 Investigate를 시작하지 않는다.

## 3. HitReactive Evidence 수명

`Portfolio.AI.CombatParticipation.HitReactivePostReactionTTL`의 기본값은 20초다. 이것은
Extra slot 전용 hold가 아니라 HitReactive Evidence 자체의 유효 기간이다.

```text
Accepted Hit
→ HitReactive Evidence 등록 (post-reaction TTL 미시작)
→ correlated Reaction terminal event
→ HitReactive post-reaction TTL 시작
→ TTL 만료 또는 anchor 반경 이탈
→ HitReactive Evidence 철회
```

- ResultSerial은 Reaction candidate, execution context, lifecycle event까지 전달된다. 현재
  Evidence와 serial이 다른 terminal callback은 무시한다.
- Reaction이 reject, ignore, unavailable이면 accepted hit 시점부터 post-reaction TTL을 시작한다.
- Completed와 Interrupted는 terminal Reaction outcome이다.
- hard release는 Reaction과 TTL 상태를 무시하고 참여 상태를 즉시 제거한다.

### 3.1 HitReactive Evidence anchor

HitReactive는 Perception 종료만으로 제거되면 안 된다. 후방 또는 시야 밖 피격도 참여를
시작할 수 있어야 하기 때문이다. 반대로 긴 TTL만으로 무한 추격해서도 안 된다.

```text
HitReactiveEvidenceAnchorLocation
→ 유효 hit 수용 시점의 Enemy 위치

HitReactiveEvidenceAnchorRadius
→ post-reaction HitReactive Evidence가 살아 있는 동안,
  고정 anchor에서 Target이 이동할 수 있는 최대 반경
```

Reaction 보호가 끝난 뒤 HitReactive Evidence는 다음 두 조건을 모두 만족할 때만 활성이다.

```text
post-reaction TTL이 남아 있음
AND Distance(Target 현재 위치, HitReactiveEvidenceAnchorLocation)
    <= HitReactiveEvidenceAnchorRadius
```

반경은 움직이는 Enemy가 아닌 고정 anchor에서 측정한다. Enemy와 Target의 현재 거리를
기준으로 하면 Enemy가 따라가는 동안 kiting이 무한히 가능해진다. 새 유효 hit는 Evidence
수명과 anchor 위치를 모두 갱신한다.

## 4. Assignment와 release

- 모든 Candidate는 GeneralBase Engage를 먼저 시도한다.
- GeneralBase가 가득 찼을 때만 **live** HitReactive Evidence가 있는 Candidate가
  HitReactiveExtra Engage slot을 사용할 수 있다.
- Extra는 별도 lifecycle이 아닌 admission metadata다.
- Perception이 남아 있는 동안 HitReactive Evidence가 끝나면 Extra 자격은 즉시 사라지며,
  allocator는 GeneralBase, Alert, Observe를 다시 판정한다.
- Participant × Target의 모든 live Evidence가 끝나면 Candidate도 끝나고 allocator는 해당
  Assignment를 제거한다.
- 정확히 일치하는 Engage Action lock만 Evidence 종료 뒤에도 action 종료 또는 lock 만료까지
  현재 Assignment를 임시 보존할 수 있다.

`live HitReactive Evidence`에 따른 **새 Extra admission 자격**은 Evidence가 끝나는 즉시 사라진다.
다만 정확히 일치하는 Action lock이 이미 존재하면 진행 중인 action 보호를 위해 기존
`HitReactiveExtra` Assignment와 slot 점유를 action 종료 또는 lock 만료까지 임시 보존할 수 있다.
lock 해제 뒤에는 현재 live Evidence만으로 다시 allocator를 실행하므로, 기존 Extra admission은
독립적으로 유지되지 않는다.

### 4.1 Runtime tuning과 반영 지연

| 항목 | 소유자 | 기본값 | 계약 |
| --- | --- | ---: | --- |
| `HitReactivePostReactionTTL` | `Portfolio.AI.CombatParticipation.HitReactivePostReactionTTL` | 20초 | reaction terminal 뒤 HitReactive Evidence 수명 |
| `HitReactiveEvidenceAnchorRadius` | `Portfolio.AI.CombatParticipation.HitReactiveEvidenceAnchorRadius` | 1000 | 고정 accepted-hit anchor에서 Target의 2D 유효 반경 |
| `AssignmentLockTimeout` | `Portfolio.AI.CombatParticipation.AssignmentLockTimeout` | 3초 | 정확히 일치하는 진행 중 Engage action 보호의 상한 |
| GeneralBase / HitReactiveExtra / Total Engage cap | `Portfolio.AI.RuntimeLOD.EngageAssignment*Cap` | 2 / 3 / 5 | Target당 Engage admission cap |
| Alert / Observe cap | `Portfolio.AI.RuntimeLOD.EngageAssignment*Cap` | 6 / 12 | Target당 비-Engage Assignment cap |
| Perception memory | `FPerceptionSetup::TargetMemoryTimeout` | 3초 | LOS 상실 뒤 Perception Evidence 수명. Source 설정값이다. |
| allocator rebuild interval | `FEngageAssignmentTuning::RebuildInterval` | 0.1초 | 일반 Evidence report/withdraw가 Assignment에 반영되는 최대 주기 |
| assignment warmup | `Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime` | 0초 | 첫 allocator rebuild 지연. 기본 비활성화 |

일반 Perception/HitReactive Evidence report와 source withdraw는 다음 periodic rebuild에서 Assignment를
재평가한다. TTL 시작, Action lock 해제·만료, soft/hard release는 즉시 rebuild를 요청한다.

## 5. Last Known Target Context

Investigate용 기억은 Active Evidence가 아니다. Candidate, Assignment, CombatTarget, Facing,
Extra 자격을 만들지 않는다.

`FCombatParticipationLastKnownTargetContext`는 Participant × Target을 key로 하며, 하나 이상의
Active Evidence가 남아 있는 동안 가장 최근의 실제 관측만 보관한다.

```text
LastKnownLocation
LastObservedVelocity
LastObservedTimeSeconds
LastObservedSource
```

- Perception은 LOS가 있을 때 Target의 관측 위치와 속도로 context를 갱신한다.
- accepted Hit는 hit 수용 시점의 정규화된 공격자 Combatant Target 위치와 속도로 context를
  갱신한다.
- 가장 최신 관측 시각이 우선한다.
- HitReactive가 살아 있다고 해서 Target의 현재 위치를 지속적으로 읽어 context를 덮어쓰지
  않는다.

`HitReactiveEvidenceAnchorLocation`은 Enemy 쪽의 Evidence 유효성 기준이다. Investigate에
사용하는 Target 쪽 `LastKnownLocation`과는 다른 값이다.

## 6. 마지막 Evidence Investigate handoff

Participant × Target의 마지막 Active Evidence가 정상 soft release로 끝날 때만 Combat
Participation은 native event를 한 번 발행한다.

```cpp
OnCombatParticipationEvidenceExhausted
```

event payload는 Participant, Target, Last Known Target Context, 최종 source, release 직전 해당
Target이 적용된 CombatTarget이었는지를 포함한다. passive context는 payload를 queue한 직후
제거한다.

```text
Perception 종료, HitReactive 생존
→ event 없음

HitReactive 종료, Perception 생존
→ event 없음; Extra 자격은 즉시 종료

마지막 Active Evidence 종료
→ EvidenceExhausted pending event 1회 예약
→ 정확히 일치하는 Action lock이 없다면 allocator가 Assignment 제거
→ Assignment = None 확인 후 EvidenceExhausted event 1회
→ Investigate handoff
```

Subsystem은 Evidence exhaustion 판정을 소유한다. Participation Component는 event를 Owner
Controller에 전달한다. Controller는 다음 조건이 모두 충족될 때만 Blackboard를 써서
Investigate를 요청한다.

- 종료된 Target이 release 직전 적용된 CombatTarget이었다.
- 다른 Target이 새 Assignment를 받지 않았다.
- ReturnHome 또는 participation suppression 상태가 아니다.
- 이미 Investigate 요청 또는 진행 상태가 아니다.

정확히 일치하는 Action lock이 남아 있으면 pending event는 발행하지 않고 유지한다. lock 해제
또는 만료 뒤 allocator가 `Assignment = None`을 반영한 후에만 event를 발행한다. 따라서
`bShouldInvestigate`는 Combat Action이 실제 종료된 뒤에만 기록된다.

새 Perception 또는 Hit Evidence는 대기 중이거나 진행 중인 Investigate context를 취소한 뒤
combat Intent를 재개한다. 이 규칙은 재인지 뒤 stale Investigate 요청이 남는 것을 막는다.

## 7. Investigate 위치 정책

Investigate는 Assignment, CombatTarget, Facing, combat slot을 보유하지 않는 비전투 행동이다.

Evidence 만료 시점의 Target 현재 위치를 읽어서는 안 된다. 시작점은 Last Known Target
Context에서 정한다.

```text
최종 source가 Perception
→ LastKnownLocation 조사

최종 source가 HitReactive
→ LastKnownLocation에서 제한된 velocity prediction 지점 조사
```

HitReactive terminal case의 계산은 다음과 같다.

```text
LastObservedVelocity가 유의미하면:
    direction = Normalize(LastObservedVelocity)
    distance = Min(speed × PredictionLeadSeconds, MaxPredictionDistance)
    investigate location = LastKnownLocation + direction × distance
그 외:
    investigate location = LastKnownLocation
```

결과는 NavMesh에 projection하며, 실패하면 `LastKnownLocation`으로 fallback한다. 이는 보지 못한
Target의 현재 위치나 방향을 wallhack처럼 사용하지 않고 마지막 관측 이동 방향만 제한적으로
조사하게 한다.

기존 BT 호환 때문에 handoff 시 Controller는 `Perception::LastKnownLocation`에 위 조사 지점을
기록하고, `Perception::LastSeenTime`에는 **실제 관측시각이 아니라 handoff 시각**을 기록한다.
BT의 Investigate timeout이 후자의 값을 기준으로 동작하기 때문이다. 실제 관측시각은 native
`LastObservedTimeSeconds` payload에만 보존하며, 두 시간값을 같은 의미로 취급하지 않는다.

## 8. ReturnHome과 hard release

ReturnHome의 false → true edge는 participation suppression을 활성화하고 Participant의 모든
Evidence를 철회한다. suppress 중 새 Evidence는 Candidate를 만들 수 없다. suppress 해제 시에는
현재 LOS Perception Evidence를 다시 report한다. ReturnHome 철회는 Investigate handoff event를
발행하지 않으며 passive Last Known Target Context도 clear한다.

soft Evidence exhaustion과 달리 hard release는 lock을 무시하고 Evidence, Last Known Target Context,
Assignment, Action lock을 함께 제거하며 Investigate를 시작하지 않는다. **감지 시점**은 source마다 다르다.

- lifecycle producer가 있는 immediate trigger: Participant 또는 Target 사망, EndPlay, UnPossess / unregister
- periodic validity trigger: hostility 또는 Target identity 무효

첫 번째는 lifecycle callback에서 즉시 hard release한다. 두 번째는 현재 Registry의 주기적 validity 검사에서
발견한 뒤 같은 hard release 정리를 수행한다. 따라서 현재 동적 hostility/identity invalidation은
`RebuildInterval` 범위의 검출 지연이 있으며, frame-immediate 동작이 필요해질 때만 별도 invalidation
producer를 추가한다.

현재 Target identity는 `ITargetContextProvider` 구현 여부다. Evidence ingress와 Pair validity 검사가
같은 조건을 사용하므로, 해당 identity 조건을 만족하지 않는 Actor는 active Evidence/Assignment/lock을
유지할 수 없다. 동적으로 identity 또는 hostility가 바뀌는 시스템을 도입하면 해당 변경 event가
immediate invalidation producer가 된다.

## 9. 브랜치 마감 검증 매트릭스

이 표는 현재 runtime 계약의 검증 항목이다. `정적 확인`은 코드 경로를 추적해 확인한 항목이고,
`PIE 확인`은 TestRoom에서 실제 Blackboard, CombatTarget, role/slot debug 표시까지 확인해야 마감할 수 있는 항목이다.

| 시나리오 | 기대 결과 | 확인 방식 |
| --- | --- | --- |
| LOS 상실 → memory timeout | Perception Evidence만 만료되고 다른 source가 남으면 Assignment/Investigate는 유지 | 정적 확인 + PIE |
| accepted Hit → reaction terminal → TTL | 최신 ResultSerial만 TTL을 시작하며 stale terminal callback은 무시 | 정적 확인 + PIE |
| TTL 또는 anchor 반경 이탈 | HitReactive Evidence와 새 Extra 자격이 제거되고 allocator가 재평가 | 정적 확인 + PIE |
| Base 2/2 + Extra 3/3 | GeneralBase를 우선 배정하고 live HitReactive Evidence만 Extra를 사용 | 정적 확인 + PIE |
| Extra Evidence 만료 중 exact Action lock | 자격은 종료하되 기존 Extra Assignment는 해당 lock 해제/만료까지 보호 | 정적 확인 + PIE |
| 두 source의 종료 순서 | 마지막 Active Evidence 하나가 끝날 때만 EvidenceExhausted를 예약 | 정적 확인 + PIE |
| 마지막 Evidence 종료 뒤 새 Evidence 또는 다른 Target Assignment | stale Investigate event를 취소하거나 폐기 | 정적 확인 + PIE |
| ReturnHome | suppress 중 ingress와 Investigate handoff를 막고, 해제 뒤 현재 LOS를 다시 report | 정적 확인 + PIE |
| participant/target death, EndPlay, UnPossess/unregister | Evidence, context, Assignment, lock을 즉시 hard release | 정적 확인 + PIE |
| hostility/identity invalid | 현재 `RebuildInterval` 범위의 periodic validity 검사 뒤 hard release. future dynamic system이 frame-immediate를 요구하면 producer 추가 | 정적 확인 + PIE |
| Evidence / Assignment / lock-only Target 참조 | 세 참조가 모두 사라진 뒤에만 Target lifecycle binding 해제 | 정적 확인 + PIE |

### 9.1 현재 브랜치 감사 기록

| Gate | 결과 | 비고 |
| --- | --- | --- |
| `f3b6bac` 대비 구조/책임 감사 | 통과 | request/lease 기반 구조에서 Evidence-centric authority로 전환됐으며, active source에 삭제된 request API 참조가 없다. |
| C++/UHT build | 통과 | `PortfolioEditor Win64 Development` build 성공. |
| Blueprint compile commandlet | 통과 | `CompileAllBlueprints -ProjectOnly -NoSave` 결과 0 errors, 0 warnings, failed-load 0. `ABP_AIPerf_Character` 정리 뒤에도 같은 결과를 재확인했다. |
| TestRoom headless load | 통과 | `/Game/00_UnitTest/TestRoom`을 `-game -NullRHI`로 기동해 `LoadMap`, `Bringing World ... up for play`, 정상 teardown을 확인했다. |
| `git diff --check` | 통과 | 문서 및 Pair validity 보완 뒤 whitespace error 없음. |
| TestRoom PIE matrix | 통과 | §9의 runtime/BT/Blackboard/UAsset 시나리오를 수동 확인했다. A/B Target에서는 기존 PlayerDummy Pair의 Engage/Alert Assignment를 유지하고 Player Pair는 unassigned candidate로 남아 Participant당 하나의 Assignment를 유지했다. 마지막 Evidence 종료 뒤 다른 Target Assignment가 있으면 stale Investigate도 시작하지 않았다. |
| AIPerf Baseline compatibility | 통과 | `BB_AIPerf_Default`의 Combat Participation key를 동기화하고 Focus 의존성과 stale BT 값을 정리했다. Baseline 40Enemy headless load와 PIE에서 ensure·stale 값 없이 상태 전이를 확인했다. |
