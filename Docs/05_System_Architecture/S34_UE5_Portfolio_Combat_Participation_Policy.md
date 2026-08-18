# S34. Combat Participation Policy

> Status: 승인된 정책 / 최종 계약 재구현 대기
>
> Scope: Enemy Combat Participation의 Evidence, Assignment, Commitment, Engage Admission, Revision 및 Consumer 경계를 정의한다.
>
> Relation: [S33 Common Combat Target Architecture](S33_UE5_Portfolio_System_Architecture.md)는 Player/Enemy 공통 Combat Target 구조와 Goal 진행 상태를 다룬다. 이 문서는 Enemy Participation의 세부 정책에 대한 정규 기준이다.

---

## 1. 결론

Enemy의 Combat Target은 Perception 또는 HitReactive가 직접 설정하지 않는다. 여러 Source가 제공한 Evidence를 Combat Participation System이 중재하고, 그 assignment 결과만 Character Adapter를 통해 Combat Target Kernel에 반영한다.

```text
Source Evidence
→ Combat Participation System
→ commitment-first slot assignment
→ Character Participation Adapter
→ Combat Target Kernel
→ Facing / Blackboard / Action
```

다음은 채택하지 않는다.

- 매 rebuild에서 거리 순으로 Engage / Alert / Observe를 전역 재정렬하는 방식
- 기존 4단계 allocator에 Observe만 기계적으로 추가하는 방식
- Perception 또는 HitReactive가 Combat Target을 직접 Set/Clear하는 방식
- 새 후보·새 hit가 유효한 기존 Engage를 탈취하는 방식

대신, 유효한 기존 Engage의 전투 연속성을 먼저 보호하고, 빈 슬롯에서만 `Observe → Alert → Engage`가 연쇄 승격하는 commitment-first allocator를 사용한다.

---

## 2. 용어와 책임

| 용어 | 의미 |
| --- | --- |
| Evidence | Source가 Participant가 특정 Target에 참여할 근거가 있다고 보고한 사실 |
| Participation Assignment | Subsystem이 부여한 `None / Observe / Alert / Engage` 결과 |
| Commitment | 현재 assignment가 짧은 거리 역전·순간 LOS 상실로 탈취되지 않게 하는 유지 계약 |
| Engage Admission | Engage assignment가 어떤 예산을 점유했는지 나타내는 승인 근거 |
| AssignmentRevision | Target, Role, Admission을 포함한 Participation 상태 세대 |
| CombatTargetRevision | `UCCombatTargetComponent`의 확정 Target 세대 |
| Applied Participation Snapshot | Adapter가 실제 Kernel에 적용한 AssignmentRevision, Target, CombatTargetRevision 묶음 |

### 2.1 구성 요소별 책임

```text
AIController Perception
= Perception evidence 생산

Combat Signal / HitReactive
= HitReactive evidence 생산

UCEnemyCombatParticipationComponent
= Evidence ingress와 assignment egress를 담당하는 Character Adapter
= 후보 비교·선택 권한 없음

UCWorldSubsystem_CombatParticipation
= Evidence registry, Target 선택, Commitment, Slot, Promotion, Revoke 소유

UCCombatTargetComponent
= 확정된 Combat Target의 유일한 Runtime SoT

Facing / Blackboard / Action
= 확정 Target 및 Participation assignment의 Consumer
```

Adapter는 evidence를 전달하고 assignment를 Kernel에 적용할 뿐이다. 후보를 하나 고르거나, Blackboard를 써서 Target을 확정하거나, Source별 Target을 하나만 보관하면 안 된다.

---

## 3. Evidence 정책

### 3.1 Evidence registry

Subsystem은 다음 key마다 claim을 보관한다.

```text
Participant + Source + TargetActor
= 하나의 Evidence claim
```

claim에는 최소한 다음 정보가 필요하다.

```text
TargetActor
Source
Evidence generation
Updated time / expiry / grace
Priority, distance, LOS 등 Source별 판정 재료
```

예를 들어 다음 세 claim은 모두 병존한다.

```text
Perception  → A
Perception  → B
HitReactive → A
```

새 Source가 B를 보고했다고 해서 A의 claim 또는 A assignment의 유지 시간이 갱신되면 안 된다.

### 3.2 철회와 종료

한 Source의 철회는 해당 Source·Target claim만 제거한다. 다른 Source의 claim과 현재 assignment는 Subsystem이 재평가한다.

다음은 즉시 hard release다.

- Participant 또는 Target EndPlay / 사망
- 명시적 participation revoke 또는 participant unregister
- 팀 관계 또는 Target identity의 무효화

현재 구현에서 Perception은 명시 철회까지 유지하고, HitReactive만 TTL로 만료한다. HitReactive TTL 만료는 Extra commitment가 유효하면 즉시 hard release가 아니며, commitment 종료 뒤 allocator가 유지·하향·종료를 재평가한다. Perception evidence grace는 별도 후속 정책이다.

### 3.3 시간 정책

다음 시간 정책은 서로 다른 문제를 다룬다.

| 정책 | 역할 |
| --- | --- |
| Evidence grace | 순간 LOS 상실이나 갱신 지연을 흡수한다. |
| Commitment hold | 현재 assignment의 즉시 탈취를 막는다. |
| Assignment lock | 현재 assignment의 유지 권위를 보호한다. Combat Action은 현재 이 lock을 요청하는 사용 사례다. |
| Demotion / reacquire cooldown | 경계에서 Alert·Observe 왕복을 줄인다. |

`AlertStep`은 Alert 상태의 이동 보정이며, assignment 경쟁을 매 tick 흔드는 점수가 아니다.

---

## 4. Commitment-first assignment

### 4.1 유지 조건

기존 Engage assignment는 다음 조건을 만족하면 신규 후보보다 먼저 보존한다.

- Participant와 Target이 유효하고 생존한다.
- hard release 사유가 없다.
- 해당 assignment를 뒷받침하는 Evidence 또는 Commitment hold가 유효하다.
- 점유한 admission 및 Total cap이 유효하다.

짧은 거리 역전, AlertStep 경계 왕복, 순간 LOS 상실, 공격 모션 중 이동 불가는 현재 Engage를 탈취하는 근거가 아니다.

### 4.2 allocator 순서

```text
0. Hard release 처리

1. 유효한 기존 Engage commitment 유지

2. 빈 Engage 슬롯 충원
   Alert 후보 → Observe 후보 → fresh 후보
   cohort 내부에서만 priority / 거리 / 접근성 / LOS 비교

3. Engage로 승격되지 않은 유효 Alert 유지

4. 빈 Alert 슬롯 충원
   Observe 후보 → fresh 후보

5. 상위로 승격되지 않은 유효 Observe 유지

6. 빈 Observe 슬롯 충원
   fresh 후보

7. 이전 / 현재 assignment diff를 한 번 통지
```

따라서 정상 승격은 다음처럼 진행된다.

```text
Engage A 해제
→ Alert B가 Engage
→ Observe C가 Alert
→ Fresh D가 Observe
```

Alert 후보가 없으면 Observe는 Engage를 직접 충원할 수 있다. 거리·priority는 빈 슬롯의 후보 경쟁에 유효하지만, 이미 유효한 Engage의 소유권을 빼앗는 기준이 아니다.

---

## 5. Engage Admission과 ExtraSlot

### 5.1 Admission 종류

```text
GeneralBase
= 유효한 Participation Evidence가 있는 후보가 사용할 수 있는 일반 Engage 예산

HitReactiveExtra
= 유효한 HitReactive Evidence가 있는 후보만 사용할 수 있는 추가 Engage 예산

Total Engage
= GeneralBase와 HitReactiveExtra를 합친 절대 상한
```

HitReactive-only 후보도 GeneralBase를 사용할 수 있다. Extra는 HitReactive를 Extra에만 가두는 제한이 아니라, GeneralBase가 부족할 때 직접 피격한 Enemy에게 제공하는 추가 보장 예산이다.

새 Engage 충원은 기존 유효 Engage를 먼저 보존한 뒤 GeneralBase, HitReactiveExtra, Total cap을 함께 판정한다. HitReactive evidence는 Extra admission과 빠른 재평가 자격을 제공하지만, 전역 후보 priority 또는 기존 Engage 탈취 권한이 아니다.

### 5.2 Extra commitment

```text
Hit evidence TTL
= 최근 유효 피격이 여전히 HitReactive Evidence인가

Extra minimum commitment
= 최초 Extra Engage 승인 뒤 보장하는 최소 유지 시간
```

새 hit는 Evidence TTL을 갱신한다. hit마다 Extra commitment를 누적 연장하지 않는다.

minimum commitment 종료 후에도 유효 Evidence가 있으면 정상 retention 후보로 남을 수 있다. 유효 Evidence가 없으면 다음 rebuild에서 GeneralBase 전환, Alert/Observe 하향 또는 None 종료를 판정한다.

현재 구현의 시간 정책은 다음과 같다.

| 항목 | 기본값 | 구현 계약 |
| --- | --- | --- |
| Perception evidence | 명시 철회까지 | Perception source는 AIController가 `WithdrawEvidence`할 때 제거한다. 감지 갱신 빈도만으로 TTL 만료시키지 않는다. |
| HitReactive evidence TTL | 2.0초 | accepted hit가 동일 `Participant × HitReactive × Target` claim의 시간을 갱신한다. TTL이 지나면 registry에서 제거한다. |
| HitReactive Extra minimum commitment | 1.0초 | 최초 `HitReactiveExtra` 승인 시 한 번 시작한다. 이후 hit는 evidence TTL만 갱신하며 hold를 연장하지 않는다. |

두 값은 각각 `Portfolio.AI.CombatParticipation.HitReactiveEvidenceTTL`, `Portfolio.AI.CombatParticipation.HitReactiveExtraMinimumCommitment` CVar로 조정한다. TTL이 끝나도 Extra commitment가 살아 있으면 해당 Extra assignment만 유지한다. commitment 종료 후 Perception evidence가 남아 GeneralBase가 가능하면 같은 Target의 `Engage`를 GeneralBase로 옮기며, 불가능하면 Alert/Observe/None을 allocator가 다시 결정한다.

```text
HitReactiveExtra → GeneralBase
= 같은 Target의 같은 Engage를 유지하는 admission metadata 변경
= Combat Target 재등록, Focus 해제, Facing 재설정 없음
```

점유 중인 유효 ExtraSlot은 새 hit가 빼앗지 않는다. 새 HitReactive evidence는 빈 ExtraSlot의 후보가 될 뿐이며, 기존 assignment를 교체하지 않는다.

---

## 6. HitReactive Evidence

### 6.1 목적과 유효성

HitReactive는 Perception 밖에서 Enemy가 유효한 적대 공격을 받은 경우에도 공격자를 전투 후보로 만들기 위한 Evidence Source다.

다음 결과는 유효 Evidence 후보다.

- 정상 피해
- Guard 성공
- Parry 성공

다음은 수용하지 않는다.

- 거절되거나 중복된 hit
- 자기 자신 또는 아군의 공격
- 무효·사망한 공격자
- 결과 적용 후 사망한 Participant
- 실제 전투 주체로 정규화할 수 없는 Source

### 6.2 Identity 정규화

Projectile, Weapon, Effect Actor는 최종 Combat Target이 될 수 없다. 공격자 후보는 다음 순서로 정규화한다.

```text
명시 Combatant / Instigator Pawn
→ 직접 전투 주체인 SourceActor
→ SourceActor의 Instigator Pawn 또는 Controller Pawn
→ 정책상 허용된 Owner Pawn
→ 식별 불가 시 reject
```

정규화 뒤에는 생존·팀·자기 자신·허용 타입을 검증한다. 소환체는 Owner/Instigator를 전투 주체로 볼지, 독립 주체로 허용할지를 별도 정책으로 정한다.

### 6.3 반복 hit와 여러 공격자

HitReactive claim은 다음 단위로 병존한다.

```text
(Participant, Source=HitReactive, CombatantTarget)
```

동일 Participant가 동일 Target에게 연속 피격되면 claim을 추가하지 않고 갱신한다.

- 동일 hit serial은 한 번만 수용한다.
- serial이 없으면 동일 프레임·동일 공격자·동일 해결 묶음을 중복 제거한다.
- DOT와 연타는 Evidence TTL 갱신 중심으로 처리한다.
- 동일 assignment라면 assignment event를 다시 발행하지 않는다.
- hit 횟수는 초기 정책에서 threat 점수로 누적하지 않는다.

여러 공격자의 claim도 덮어쓰지 않는다.

```text
(E, HitReactive, A)
(E, HitReactive, B)
```

Target 선택은 Subsystem이 모든 claim과 현재 Commitment를 함께 보고 결정한다.

---

## 7. Kernel, Revision, Projection

### 7.1 변경 책임

Hit는 Kernel을 직접 변경하지 않는다.

```text
HitReceived
→ HitReactive Evidence 등록

Participation assignment 승인
→ Combat Target Kernel Set / Clear
→ ParticipationAssigned / ParticipationRevoked
```

따라서 `HitReceived`는 Combat Signal/Evidence의 의미이고, AI Kernel 변경 사유로 사용하지 않는다. 최종 `ECombatTargetChangeReason`에서는 제거 대상이다.

### 7.2 두 Revision

```text
AssignmentRevision
= Role, Admission, Target을 포함한 Participation 상태 세대

CombatTargetRevision
= 실제 확정 Target Actor 세대
```

```text
Observe A → Alert A → Engage A
→ AssignmentRevision 증가 가능
→ CombatTargetRevision 불변

Engage A → Observe B
→ AssignmentRevision 증가
→ CombatTargetRevision 증가
```

### 7.3 Applied snapshot과 늦은 revoke

Adapter는 assignment event를 받은 뒤 Kernel Set/Clear를 적용하고, 실제 적용 성공 결과를 다음 묶음으로 보관한다.

```text
AppliedParticipationSnapshot
- AssignmentRevision
- TargetActor
- AppliedCombatTargetRevision
```

과거 A assignment의 revoke는 A를 적용했을 때의 Target과 CombatTargetRevision으로만 조건부 Clear한다. 현재 Kernel Snapshot에서 B의 Actor·Revision을 새 expected 값으로 만들면 안 된다. 따라서 늦은 A revoke는 B Target을 지울 수 없다.

### 7.4 Blackboard와 Consumer

Blackboard는 projection이다. assignment Target과 Kernel Snapshot Target이 일치할 때만 유효한 Participation/Target 조합을 기록한다.

```text
Participation Engage
= Focus / Facing / 행동 후보 자격

Action execution
= 실제 공격 시작을 허가하는 별도 예산·쿨다운·lock
```

Extra Engage가 늘어도 모든 Enemy가 같은 시점에 공격하도록 만들면 안 된다.

---

## 8. 구현 상태와 재구현 순서

현재 Goal 7 코드는 Participation Lifecycle 전환의 초기 구현이다. 다음 최종 계약을 모두 만족하지 않으므로 완료로 취급하지 않는다.

| 영역 | 현재 상태 | 목표 |
| --- | --- | --- |
| Evidence 소유 | Subsystem의 Source×Target registry와 Combat Signal HitReactive producer 구현 완료 | Evidence grace/명시적 source 철회 정책 확장 |
| 선택 책임 | Subsystem이 Source×Target Evidence를 직접 집계 | time policy를 포함한 commitment-aware 단독 선택 |
| allocator | commitment-first ladder, HitReactive TTL, Extra minimum commitment, Action lock 구현 완료 | Perception grace, demotion/reacquire cooldown 추가 |
| admission | GeneralBase / HitReactiveExtra / Total cap 판정, producer 연결, Extra commitment 유지 구현 완료 | balance 검증 |
| revision | Previous/Current assignment change, Adapter applied snapshot, Action Lock identity 검증 구현 완료 | action lifecycle의 세부 lock 정책 확장 |
| projection | applied snapshot과 Kernel Snapshot이 일치할 때만 Blackboard projection | UAsset consumer 실제 전환 |
| HitReactive producer | accepted Combat Signal 결과를 정규화해 `HitReactive` claim으로 보고하고 TTL을 갱신 | DOT 갱신 rate limit 및 명시 source 철회 정책 추가 |

### Phase A — 안전·호환성 정리 (완료)

- 기존 UAsset 직렬화 의미를 유지하도록 `ECombatRole` 값을 명시적으로 고정한다: `None=0`, `Engage=1`, `Alert=2`, `Observe=3`.
- `HitReceived`를 AI Kernel 변경 사유로 사용하지 않도록 정리한다.
- Goal 7 상태와 UAsset 안내를 실제 상태에 맞게 정정한다.

### Phase B — Subsystem-owned Evidence Registry (완료)

- `ReportEvidence(Source, Target, Context)`, `WithdrawEvidence(Source, Target)`, `UnregisterParticipant()`를 Subsystem 계약으로 둔다.
- Source 상실은 해당 claim만 철회한다.
- UnPossess/Enemy 제거는 participant 전체를 즉시 unregister해 ghost slot을 남기지 않는다.

### Phase C — Commitment / Admission allocator (완료)

- Source×Target Evidence를 `(Participant, Target)` candidate로 집계한다. 같은 Target의 Perception과 HitReactive evidence는 하나의 candidate가 되며, 서로 다른 Target evidence는 병존한다.
- `Engage 보존 → Alert/Observe Engage 승격 → fresh Engage → Alert 보존 → Observe Alert 승격 → fresh Alert → Observe 보존 → fresh Observe` 순서의 ladder를 구현한다.
- `GeneralBase`, `HitReactiveExtra`, `Total Engage` cap을 각각 점유·판정한다. HitReactive-only candidate도 GeneralBase를 사용할 수 있고, Extra admission은 HitReactive evidence가 있을 때만 가능하다.
- 현재 Evidence 존재 여부만 retention 최소 조건으로 사용한다. Evidence grace, Commitment hold, Action lock, demotion/reacquire cooldown은 의도적으로 다음 time-policy 단계에 보류한다.

### Phase D — Adapter 적용 계약과 coherent Blackboard projection (완료)

- assignment event는 `PreviousAssignment`, `CurrentAssignment`와 Current의 `AssignmentRevision`을 하나의 payload로 통지한다.
- Adapter는 Kernel Set/Clear 뒤 실제 `TargetActor`, `CombatTargetRevision`, `AssignmentRevision`, Role, Admission을 applied snapshot으로 보관한다.
- revoke와 controller 교체 clear는 현재 Kernel을 새로 읽어 기대값으로 쓰지 않고, Adapter가 보관한 applied `{TargetActor, CombatTargetRevision}`만으로 조건부 Clear한다.
- Target EndPlay 등으로 Kernel이 먼저 바뀐 경우 conditional Clear 실패는 새 Target을 건드리지 않는다. Kernel이 이미 None이면 해당 None Snapshot만 acknowledge하고, 다른 Target이면 applied snapshot을 폐기한다.
- Blackboard Service는 Subsystem assignment와 Kernel을 각각 읽어 조합하지 않는다. Adapter applied snapshot과 현재 Kernel Snapshot의 Target·Revision이 모두 일치할 때만 CombatTarget과 CombatParticipation projection을 함께 기록한다.
- 일치하지 않는 과도 상태는 유효 Combat Target Snapshot이 아니므로 Target/Participation projection을 clear한다. Blackboard의 `None / Revision 0`은 이 경우 unavailable sentinel이며, 정상 clear Snapshot은 실제 증가한 Kernel Revision으로 기록한다.

### Phase E — Action authority와 UAsset consumer 전환 범위 (이번 범위 완료)

#### 완료: Action 직전 authority

- `Start Combat Action`은 Blackboard의 `CombatTargetActor / CombatTargetRevision / CombatParticipationState / CombatParticipationRevision`이 현재 Enemy의 applied snapshot과 일치하는 `Engage` authority인지 확인한다.
- `ACEnemy::HandleAICombatAction()`은 Action Orchestrator 직전에 같은 authority를 다시 확인한다. 따라서 BT tick과 실제 Action request 사이에 Target 또는 Participation 세대가 바뀌면 request를 거절한다.
- Combo chain도 최초 Action의 Target snapshot뿐 아니라 Participation revision을 보관하고, 다음 chain request 직전에 다시 검증한다.

#### 결정: UAsset consumer 전환 범위

이번 단계에서는 UAsset을 수정하지 않는다. C++의 `CombatParticipationState`와 `CombatParticipationRevision` projection은 이미 작성되며, 기존 `CombatRole`은 기존 BT asset 호환을 위한 mirror로 병행 유지한다.

다음 수동 asset audit에서 확인할 범위는 아래와 같다.

1. Enemy Blackboard와 AI Performance Blackboard에 `CombatTargetActor`(Object), `CombatTargetRevision`(Int), `CombatParticipationState`(Enum), `CombatParticipationRevision`(Int)가 존재하는지 확인한다.
2. 기본/프로파일 BT의 `Update AIContext`가 `Update Engage Context` 및 `Start Combat Action`보다 먼저 실행되는지 확인한다.
3. Attack subtree의 `Start Combat Action`은 fixed native key로 authority를 읽으므로 별도 selector 교체는 필요 없다.
4. 기존 `CombatRole`을 직접 읽는 Decorator/Service/Runtime LOD consumer를 asset audit으로 목록화한 뒤에만 `CombatParticipationState` 단독 소비로 전환한다.

### Goal 10 — Participation 기반 Intent / BT 정책 정렬 (완료)

- `Update AI Intent State`는 더 이상 Perception awareness 또는 legacy `CombatRole`을 Enemy combat intent의 권위로 사용하지 않는다. coherent `CombatTargetActor`와 `CombatParticipationState`가 함께 있을 때 Participation Role이 `Engage / Alert / Observe` Intent를 결정한다.
- 따라서 HitReactive-only assignment도 Perception 후보가 없다는 이유만으로 Idle이 되지 않는다. `Observe`는 Observe Intent를 유지하고, `Alert / Engage`는 Participation Target까지의 range를 기준으로 Alert 또는 Chase/Engage를 결정한다.
- `Update AIContext`의 Alert-range metric은 coherent Participation Target을 우선 사용하며, Participation이 없을 때만 Perception Target을 fallback으로 사용한다.
- Runtime LOD resolver는 `CombatParticipationState`를 권위 role로 읽는다. legacy `CombatRole` projection은 기존 UAsset 호환을 위한 mirror로 남긴다.
- Dead와 HitReact는 Participation보다 우선하는 로컬 절대 상태다. 진행 중 Combat Action은 현재 Participation이 `Engage`일 때만 Engage Intent를 유지한다. assignment revoke 뒤 Action을 강제로 끊는 Action lock 정책은 별도 후속 범위다.

UAsset은 수정하지 않는다. 수동 asset audit에서는 `Update AIContext`가 Intent/Engage Context Service보다 먼저 실행되고, `CombatParticipationState`와 `CombatParticipationRevision` key가 Blackboard에 존재하는지를 확인해야 한다. `bRequireLOS`가 설정된 기존 Target Decorator는 여전히 Perception LOS를 뜻하며, HitReactive-only branch에 기계적으로 재사용하면 안 된다.

### Goal 11 — Action Lock과 Participation Revoke 경계 (완료)

```text
AI Combat Action started
→ { TargetActor, CombatTargetRevision, AssignmentRevision } Action Lock 등록
→ Subsystem은 동일 Engage assignment를 lock timeout 또는 Action 종료까지 보존
→ Action 종료 / 취소 / timeout
→ lock 해제 뒤 allocator 재평가
```

- Enemy는 Action request 직전에 이미 검증한 Target/CombatTargetRevision/AssignmentRevision으로 lock을 요청한다. Adapter는 applied snapshot과 일치할 때만 Subsystem에 전달하고, Subsystem은 현재 Engage assignment와 target hostility·생존 상태를 다시 검증한다.
- lock은 새 assignment나 새 Target을 만들지 않는다. evidence가 순간적으로 사라져도 lock이 살아 있는 동안 **동일 assignment**만 유지한다. 새로운 후보·거리·priority는 기존 lock을 탈취하지 못한다.
- Assignment lock은 `Portfolio.AI.CombatParticipation.AssignmentLockTimeout` CVar(기본 3초)로 failsafe 만료된다. 새 combo action이 실제 시작되면 같은 identity의 lock timeout을 새 action 기준으로 갱신한다.
- Combat Action이 끝나거나 Interrupted되어 Action Type이 Combat Action이 아니게 되면 Adapter가 lock을 해제하고 즉시 allocator를 재평가한다.
- Target EndPlay와 Target Health의 `Dead` 전이는 evidence·Assignment Lock을 즉시 제거하고 rebuild한다. hostility 상실, participant unregister/UnPossess, Enemy death도 hard release다. Kernel TargetActor 또는 CombatTargetRevision이 lock 기록과 달라져도 Adapter가 lock을 즉시 해제한다. lock은 이 사유를 기다리지 않으며 evidence·assignment와 함께 제거된다. Enemy death는 participation adapter를 unregister하고 conditional Combat Target clear를 수행한다.
- Target EndPlay binding은 evidence가 TTL로 사라진 뒤에도 action lock이 남아 있으면 유지한다. 따라서 lock만 남은 Target도 EndPlay에서 즉시 해제된다.

Assignment Lock은 현재 assignment 유지 장치일 뿐, 공격의 실제 실행 시간·동시 공격 예산을 결정하지 않는다. Combat Action은 현재 이 lock을 요청하는 첫 사용 사례이며, Action execution 예산과 animation/hit-window별 lock 세분화는 별도 정책이다.


### Goal 8 — Combat Signal HitReactive Evidence Producer (완료)

- `UCCombatSignalTargetComponent`는 일반 피해, Guard, Parry를 모두 target-side evaluation이 끝난 accepted packet으로 통지한다. 거절 결과는 통지하지 않는다.
- `UCEnemyHitReactiveComponent`는 Enemy의 Combat Signal Target 결과를 구독한다. 결과 적용 뒤 Enemy가 사망했거나, 자신/아군/무효 Actor이거나, Combatant로 정규화할 수 없으면 evidence를 보고하지 않는다.
- 공격 주체는 명시 `SourceActor`를 먼저 검증하고, 이어 Instigator Pawn, DamageCauser, DamageCauser의 Instigator/Owner 순으로 정규화한다. 현재 AI 후보 계약에 맞는 `ITargetContextProvider`와 hostile team attitude를 모두 만족해야 한다.
- accepted result에는 Enemy별 단조 증가 `ResultSerial`을 부여한다. producer는 이미 처리한 serial을 다시 처리하지 않으므로 동일 결과의 중복 delegate delivery가 claim 갱신으로 증폭되지 않는다. Source Combat Signal의 hit-window 중복 억제와 함께 같은 해결 결과를 한 번만 evidence ingress로 보낸다.
- producer는 `UCEnemyCombatParticipationComponent::ReportEvidence(HitReactive, CombatantTarget, Context)`만 호출한다. Combat Target, Focus, Blackboard, Action을 직접 변경하지 않는다.
- Perception과 HitReactive가 같은 Target을 보고하면 Subsystem은 하나의 `(Participant, Target)` candidate로 집계하며, 서로 다른 공격자의 claim은 각각 병존한다.

### Goal 9 — Evidence Time Policy와 Extra Commitment (완료)

- `HitReactive` evidence는 target-side accepted hit마다 동일 claim을 갱신하고, 마지막 갱신 뒤 TTL이 지나면 Subsystem registry에서 제거한다. claim의 Generation과 assignment revision을 hit 횟수로 누적하지 않는다.
- `HitReactiveExtra` Engage가 최초 승인되면 participant·target 단위의 minimum commitment를 시작한다. 같은 Extra assignment의 후속 hit 또는 TTL refresh는 종료 시각을 연장하지 않는다.
- TTL 만료 후 commitment가 남아 있으면 해당 Extra slot만 유지한다. 대상 변경, participant unregister, Target EndPlay, admission 변경, hard release는 hold를 기다리지 않고 commitment를 제거한다.
- commitment 종료 뒤에는 evidence가 남아 있으면 일반 allocator가 admission을 다시 고른다. GeneralBase가 가능하면 Extra에서 GeneralBase로 이동하고, Engage 예산이 없으면 Alert/Observe/None으로 하향할 수 있다.

이번 단계에서도 Perception evidence grace, demotion/reacquire cooldown, DOT/연타의 producer-level rate limit, 명시 HitReactive source 철회는 구현하지 않는다.

---

## 9. 필수 검증 시나리오

| 상황 | 기대 결과 |
| --- | --- |
| Player가 조금 이동해 B가 더 가까워짐 | A Engage 유지, B는 Alert/Observe |
| A Target EndPlay | A 즉시 해제, Alert/Observe 연쇄 승격 |
| Alert 부재 | Observe가 Engage를 직접 충원 |
| Perception A 상실 + HitReactive A 유지 | A Participation 유지 |
| HitReactive A TTL 만료 + Extra commitment 유효 | A의 Extra Engage 유지 |
| HitReactive A TTL·Extra commitment 모두 만료 + Perception A 유지 | A Engage를 GeneralBase로 재평가하거나 예산에 따라 하향 |
| 모든 A Evidence 상실 | hold 종료 뒤 하향 또는 release |
| Perception A + HitReactive B | 두 claim 보존, Subsystem이 선택 |
| 늦은 A revoke | B Combat Target 유지 |
| UnPossess / Enemy 제거 | participant 즉시 unregister, slot 회수 |
| 동일 accepted result 중복 delivery | `ResultSerial` guard로 evidence ingress 한 번 |
| 동일 Target 연타 / DOT | 같은 Source×Target claim 갱신, TTL/rate limit은 후속 time policy |
| 점유된 ExtraSlot에 새 hit | 기존 Extra Engage 탈취 없음 |
| Extra → GeneralBase | Target / Focus / Facing churn 없음 |
| Action 중 Evidence 일시 상실 | 동일 Engage assignment 유지, 새 후보로 교체 없음 |
| Action 종료 / Interrupted | lock 해제 후 즉시 allocator 재평가 |
| Action 중 Target EndPlay / 사망 / hostility 상실 | lock 무시, 즉시 release |
| 40/80 Enemy | cap별 Focus·Movement·Animation·BT p95 재측정 |

---

## 10. 범위 밖

- 범용 Threat / scoring 시스템
- 소환체의 독립 Combatant 정책 확정
- Perception evidence grace, demotion/reacquire cooldown
- DOT/연타의 producer-level rate limit과 명시 HitReactive source 철회 정책
- Blackboard/BT UAsset의 일괄 수정
- Action execution 예산의 구체 수치

이 항목들은 본 정책을 전제로 한 별도 Goal에서 다룬다.
