# S34. Combat Participation Policy

> Status: Evidence-centric participation implemented

## Core model

```text
Perception Evidence / HitReactive Evidence
→ live Evidence aggregate
→ Candidate
→ None / Observe / Alert / Engage Assignment
→ CombatTarget / Facing / Blackboard / Intent
```

`Assignment != None`만 combat participation 상태다. Evidence가 candidate의 유일한 근거이며 Session, Session phase, revision, commitment는 사용하지 않는다. `None`은 CombatTarget을 clear하는 비전투 상태다.

## Evidence

- LOS success는 Perception Evidence를 등록·갱신한다. LOS false는 memory를 유지하며 즉시 철회하지 않는다. memory timeout에서 철회한다.
- accepted damage, Guard, Parry는 정규화된 hostile combatant의 HitReactive Evidence를 등록·갱신한다.
- Perception과 HitReactive는 같은 candidate/allocator 경로를 사용한다.

## HitReactive Post-Reaction TTL

`Portfolio.AI.CombatParticipation.HitReactivePostReactionTTL`의 기본값은 60초다. Extra 전용 hold가 아니라 **HitReactive Evidence 자체의 유효 기간**이다.

```text
Accepted Hit → Evidence 등록
→ correlated Reaction terminal event
→ ExpireTime = Now + TTL
→ expiry → Evidence 철회
```

- ResultSerial은 Reaction candidate/context/lifecycle까지 전달된다.
- 새 hit은 같은 Evidence의 ResultSerial을 갱신하며, 이전 callback은 serial 불일치로 무시한다.
- Reaction이 시작되지 않거나 reject/ignore되면 accepted hit 시점부터 TTL을 시작한다.
- Completed/Interrupted는 terminal event다. hard release는 TTL과 무관하게 즉시 제거한다.

## Assignment and release

- 모든 live candidate는 GeneralBase Engage slot을 먼저 경쟁한다.
- GeneralBase가 찬 경우에만 live HitReactive Evidence가 있는 candidate가 HitReactiveExtra Engage slot을 사용한다.
- Extra는 별도 lifecycle이 아닌 admission metadata다.
- HitReactive Evidence가 만료되고 Perception Evidence가 남으면 allocator가 GeneralBase/Alert/Observe를 재판정한다.
- 모든 live Evidence가 사라지면 candidate도 사라지고 Assignment는 `None`이다.
- 정확히 일치하는 Engage Action lock만 evidence 소멸 뒤 action 종료/timeout까지 Assignment를 보호한다.

## Investigate and ReturnHome

Investigate는 Assignment, CombatTarget, Facing, combat slot을 보유하지 않는 비전투 BT context다. 현재 Assignment Target의 Perception memory가 만료되고 다른 live Evidence가 없을 때만 1회 요청한다. Hit-only TTL expiry는 자동 Investigate를 만들지 않는다. 새 Evidence가 Assignment를 만들면 Investigate는 취소된다.

ReturnHome edge는 participation suppress를 활성화하고 participant의 모든 Evidence를 철회한다. suppress 중 새 Evidence는 candidate가 되지 않으며, Home 복귀 후 suppress 해제 시 현재 LOS Perception context를 다시 동기화한다.

## Hard release

participant/target death, EndPlay, UnPossess/unregister, hostility 또는 target identity invalid는 Evidence, Assignment, lock을 즉시 정리한다.
