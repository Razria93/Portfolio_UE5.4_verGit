# S38. Combat Signal Architecture

> 상태: **현행 코드 기준 확정 / Target Packet 제출 캡처 대기**  
> 작성일: 2026-09-06  
> 근거 감사: `Docs/07_Portfolio_Documents/Portfolio_Production/26_Combat_Interaction_Code_Document_Concordance_Audit (KR).md`

---

## 1. 목적과 범위

Combat Signal은 Source가 전투 사실을 구성·전달하고, Target이 자기 상태와 규칙으로 결과를 확정하는 **단방향 전투 신호 구조**다.

포함 범위:

- Source-side hit/cue 입력 검증과 전달
- Target-side validation, defense, damage, reaction outcome, dispatch
- Target Packet과 debug/audit 관찰 경계

제외 범위:

- Action/Reaction Intervention 전환 정책
- 두 Actor의 Pair reservation·commit 거래
- 모든 signal type의 범용 Handler 객체화

---

## 2. 문제와 설계 요구

과거에는 공격자가 피격자의 Reaction Montage를 직접 실행하거나, 피격자가 사용할 Reaction 정보를 직접 지정하는 방식이 사용됐다. 이 방식은 Source가 Target의 상태·실행·표현 규칙까지 알아야 하므로 Character, Attack, Defense Outcome이 늘어날수록 결합과 분기가 증가한다.

설계 요구는 다음과 같다.

1. Source는 자신이 관찰한 공격·충돌 사실만 구성한다.
2. Target은 Packet과 자신의 상태를 함께 해석해 Defense·Damage·Reaction Outcome을 결정한다.
3. Health, Reaction, Feedback은 Target이 확정한 결과를 각 도메인에 전달해 소비한다.
4. 입력 원인이 다른 Collision Damage와 Timing Cue를 하나의 거대한 공용 상태 변경 Gateway로 일반화하지 않는다.

---

## 3. 책임 경계

| 주체 | 책임 | 하지 않는 일 |
| --- | --- | --- |
| `UCCombatSignalSourceComponent` | HitContext 검증, HitWindow·duplicate 검사, Source Payload/Context, DamageSpec, Target delivery, Cue delivery | Target Guard/Parry/Reaction Outcome 결정 |
| `UCCombatSignalTargetComponent` | Target validation, Defense/Damage/Reaction Outcome, Health commit, Reaction·Feedback·Result dispatch | Source hit window와 공격 선택 소유 |
| Health Component | Target이 결정한 적용 피해의 resource commit | Defense/Reaction 종류 판정 |
| Reaction Orchestrator | Target Outcome을 받은 Reaction 실행 가능 여부·전환 판단 | Damage 감쇠·Defense Outcome 판정 |

`UCCombatSignalTargetComponent`는 현재 함수·구조체 경계로 Evaluate/Apply/Dispatch를 나눈다. 이는 모든 Packet 처리기를 별도 객체로 분리한 구조가 아니다.

---

## 4. Source 전달

### 4.1. Collision Damage

```text
FHitContext
  → ValidateRequest
  → Source Payload / Context
  → ValidateContext / CanSendCombatSignal
  → DamageSpec resolve / request damage 계산
  → TargetActor::TakeDamage(FDefaultDamageEvent)
  → Target Damage entry
```

Source Payload/Context는 Source Actor, DamageCauser, Target Actor, Hit Impact, HitWindowKey, DamageSpecKey, Instigator를 보존한다. Source는 Target이 일반 Hit, Guard, Parry, Dead 중 무엇을 선택할지 결정하지 않는다.

### 4.2. Timing Cue

```text
RequestCombatSignalCue
  → FCombatSignal(TimingCue)
  → Target::RequestCombatSignalTarget
  → ProcessCombatSignalTarget
  → HandleTimingCueSignal
```

현재 `HandleTimingCueSignal()`은 Blink·Repulse cue tag를 유효 입력으로 기록하고 수용한다. 이 경로 안에서 실제 Blink/Repulse gameplay 결과를 적용하는 코드는 확인되지 않았다. 따라서 현행 문서와 포트폴리오에는 cue 전달·수용만 기록한다.

---

## 5. Target-side Outcome Resolution

### 5.1. 기본 흐름

```text
Engine Damage Input
  → Target Payload / Context
  → ValidateContext
  → Target pre-state snapshot
  → CanReceiveCombatSignal
  → ComputeTargetDamage
  → CommitCombatSignalTarget
  → ResolveDamageReactionOutcome
  → FCombatSignalTargetPacket
  → Reaction / Feedback / Result Dispatch / Debug
```

Target Outcome은 다음처럼 확정된다.

```text
Incoming Damage Packet
  + Target dead state
  + External input policy
  + Guard / Parry state
  + Balance lifecycle
      → DefenseOutcome / Damage / ReactionOutcome
```

### 5.2. 현행 Outcome 규칙

| 조건 | Defense / Damage | Reaction Outcome |
| --- | --- | --- |
| Already Dead | Reject | 없음 |
| `RejectAll` | Reject | 없음 |
| `DamageOnly` | damage만 commit | 없음 |
| Parry 가능 | damage commit 없음 | Parry |
| Guard 가능 | Guard multiplier로 감쇠 | BlockHit |
| 일반 damage | commit | Hit / CollapseHit / Dead |

Target은 Damage를 Commit한 뒤 Dead 상태와 Defense Outcome을 사용해 Reaction Outcome을 확정하고, 그 후 `FCombatSignalTargetPacket`을 Reaction·Feedback·Result consumer에 전달한다.

### 5.3. Source 성공 상태의 제한

Source의 `CommitCombatSignalSource()`는 `SendDamageToTarget()` 반환 Damage가 0 이하이면 Source Context를 `CommitFailed`로 기록한다. Parry는 Target이 accepted packet을 dispatch하면서도 damage를 commit하지 않아 반환값이 0일 수 있다.

따라서 Parry의 성공·실패는 Source Context의 committed damage만으로 판단하지 않는다. Target Packet의 Accepted, DefenseOutcome, ReactionOutcome, HP 전후, Result dispatch를 함께 확인한다.

---

## 6. 요청 경계와 Execution Pair의 관계

Combat Signal의 Collision Damage와 Timing Cue는 각자 입력·검증·전달 진입점이 다르다. 공통화하는 대상은 Source/Target 책임과 Target 결과 소비 경계이지, 모든 전투 요청을 하나의 Damage handler로 넣는 것이 아니다.

Execution Pair는 Combat Signal의 특수 Packet이 아니다. Pair는 별도 Collaboration Component가 양측 조건·Reservation·Commit·Release를 소유한다.

Pair Commit은 Target Component의 `RequestExecutionOutcomeTarget()`을 사용하지만, 이는 일반 `HandleDefaultDamageEvent()`와 다른 `FExecutionOutcomePacket` 처리 경로다. 이 접점의 상세 계약은 S36에서 다룬다.

---

## 7. 불변 조건

| ID | 조건 | 상태 |
| --- | --- | --- |
| CS-01 | Source는 Target의 Defense/Reaction Outcome을 직접 결정하지 않는다. | Code Confirmed / Documented |
| CS-02 | Target은 자신의 상태와 Packet으로 Defense·Damage·Reaction Outcome을 확정한다. | Code Confirmed / Documented |
| CS-03 | Target은 Outcome 확정 뒤 Reaction·Feedback·Result dispatch를 요청한다. | Code Confirmed |
| CS-04 | Parry는 damage commit 없이 accepted Target Packet이 될 수 있다. | Code Confirmed |
| CS-05 | Source Context의 0 damage `CommitFailed`는 Target Parry 실패를 뜻하지 않는다. | Code Confirmed |
| CS-06 | Timing Cue의 현재 코드 근거는 tag 수용이며, 실제 gameplay effect가 아니다. | Code Confirmed |
| CS-07 | Target 내부 함수 경계는 존재하지만 범용 Packet Handler 객체화는 보류 상태다. | Code Confirmed / Documented |

---

## 8. 증거 상태와 표현 제한

| 주장 | 코드 | 문서 | 런타임 | 제출 상태 |
| --- | --- | --- | --- | --- |
| Source→Target 사실 전달 경계 | Confirmed | J01/N05/N06 | 없음 | Needs Capture |
| Target Outcome Resolution | Confirmed | J01/N05/N06 | Guard/Parry 후보 | Needs Capture |
| Guard/Parry 결과 packet | Confirmed | B12 보조 | 기존 후보 | Needs Capture |
| Timing Cue 전달·수용 | Confirmed | N05/N06 | 없음 | Needs Capture |

금지 표현:

- Source가 Target Montage 또는 Reaction 종류를 직접 실행·결정한다.
- Source `CommitFailed`가 모든 Target Outcome 실패를 뜻한다.
- Blink/Repulse gameplay 기능이 완성됐다.
- 모든 Packet 종류가 이미 범용 Handler 객체로 분리됐다.
- Execution Pair가 Combat Signal의 하위 분기다.

---

## 9. 포트폴리오 연결

| 페이지 | 역할 | 런타임 캡처 게이트 |
| --- | --- | --- |
| 13p | Source가 hit/cue 사실을 구성하고 Target에 전달하는 단방향 책임 경계 | 동일 사건의 Source Context와 Target Packet trace |
| 14p | Target이 Packet과 Target State로 Normal/Guard/Parry Outcome을 확정하는 경계 | 동일 계열 hit의 Normal / Guard / Parry Target Packet 비교 |

13p와 14p는 하나의 단방향 Combat Signal 구조를 두 페이지로 나눈다. 기존 C14 Data-driven Resolve는 이 문서의 Target Outcome 사례가 아니며, 별도 배치 결정을 거쳐야 한다.
