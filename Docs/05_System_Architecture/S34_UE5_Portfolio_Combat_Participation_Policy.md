# S34. Combat Participation 정책

> 상태: Evidence 중심 참여 구조 구현 완료. 마지막 Evidence 종료 기반 Investigate handoff 구현·검증 진행 중.

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

`Portfolio.AI.CombatParticipation.HitReactivePostReactionTTL`의 기본값은 60초다. 이것은
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
→ EvidenceExhausted event 1회
→ 정확히 일치하는 Action lock이 없다면 allocator가 Assignment 제거
→ Investigate handoff
```

Subsystem은 Evidence exhaustion 판정을 소유한다. Participation Component는 event를 Owner
Controller에 전달한다. Controller는 다음 조건이 모두 충족될 때만 Blackboard를 써서
Investigate를 요청한다.

- 종료된 Target이 release 직전 적용된 CombatTarget이었다.
- 다른 Target이 새 Assignment를 받지 않았다.
- ReturnHome 또는 participation suppression 상태가 아니다.
- 이미 Investigate 요청 또는 진행 상태가 아니다.

정확히 일치하는 Action lock이 남아 있으면 `bShouldInvestigate`를 한 번 기록하되 Combat
Intent는 lock 해제까지 유지한다. 이후 `Assignment = None`이 되면 Intent service가 대기 중인
요청을 소비한다.

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

다음은 soft Evidence exhaustion이 아닌 hard release다.

- Participant 또는 Target 사망
- EndPlay
- UnPossess / unregister
- hostility 또는 Target identity 무효

Target/Participant 사망, EndPlay, UnPossess/unregister처럼 lifecycle producer가 있는 hard release는
lock 상태와 관계없이 Evidence, Last Known Target Context, Assignment, Action lock을 즉시 제거하며
Investigate를 시작하지 않는다. hostility 또는 Target identity 무효는 현재 Registry의 주기적
validity 검사에서도 같은 방식으로 정리된다. 프레임 동기 즉시성을 요구하게 되면 별도 invalidation
producer를 추가해야 한다.
