# S35. Enemy Balance / Collapse Lifecycle 설계

> 상태: R07 C++ lifecycle 및 Damage Reaction Outcome 계약 구현 완료. CollapseHit asset 연결 및 PIE 검증 대기.
> 범위: Enemy Balance 누적, Collapse In / Loop / Out, Collapse Loop 피격 표현, Count 잠금, Collapse Loop TTL 및 안전한 복구.
> 제외: Player Beta/Burst, Player Balance policy 활성화, Source/Target Execution 협업, Execution consume, UI 완성.

---

## 1. 목적

Enemy의 Balance는 Parry CombatResult에 의해 누적되고, Max 도달 시 Enemy를 일시적으로
무력화하는 Collapse lifecycle을 연다. 이 문서는 수치, Reaction, locomotion, AI, Action,
Movement, Combat Participation의 책임을 섞지 않고 다음 흐름을 고정한다.

```text
Parry CombatResultPacket
→ Balance Count commit
→ threshold 최초 도달
→ CollapseIn Reaction
→ Collapse Loop
→ CollapseHit Reaction (실제 피해 수신 시)
→ Collapse Loop TTL
→ CollapseOut Reaction
→ Balance reset
```

과거 `feat/balance-collapse-lifecycle` 브랜치와 S30은 설계 근거로만 참조한다. 해당
브랜치는 P59/P60보다 이전의 구현 구조이므로 코드 merge 또는 cherry-pick 대상이 아니다.

---

## 2. 핵심 결정

1. `UCBalanceComponent`가 Enemy Balance Count와 Balance lifecycle의 SoT다.
2. Parry 후속 효과는 원래 Damage SourceActor인 receiver에게 전달한다.
3. Player/Enemy CombatResult receiver의 `UCCombatSignalTargetComponent`가 packet 유효성을
   검증하고, Actor는 routing adapter만 맡는다.
4. threshold를 **이번 commit에서 최초로** 넘었을 때만 CollapseIn Reaction을 요청한다.
5. Collapse lifecycle 시작의 권위는 request acceptance가 아니라 실제 `CollapseIn Started` event다.
6. `EBalanceLifecycleState`가 Collapse의 유일한 runtime SoT다. 별도 `bIsCollapseActive` field는 두지 않고,
   소비자별 의미는 lifecycle state에서 파생 query로 제공한다.
7. Collapse Loop TTL은 CollapseIn Started가 아니라 정상 `Completed` 시점부터 시작한다.
8. TTL 만료는 Collapse 상태 해제가 아니라 CollapseOut 요청의 원인이다.
9. `CollapseOut Started`는 Weak Loop pose를 해제하고 normal locomotion 결과를 준비하는 전이 권위다.
   `CollapseOut`의 Reset Notify는 Count reset, 잠금 해제, Facing 복구 및 `Accumulating` 복귀의 정상
   전이 권위다.
10. Collapse는 Combat Participation의 evidence, assignment, CombatTarget을 해제하지 않는다.
11. 기존 CombatResult 직접 `Stagger` request 경로는 제거한다. Player는 현재 동일 ingress만
    사용하고 Balance policy는 TODO로 보류한다. Balance lifecycle은 `CollapseIn / CollapseOut`을
    별도 ReactionType으로 사용한다.
12. 실제 Execution 협업이 구현되기 전에는 `bCanStartExecution` 같은 runtime capability를
    저장하거나 노출하지 않는다.
13. Collapse lifecycle은 `EAIIntentState::Incapacitated`로 투영한다. 우선순위는
    `Dead → active Reaction → Incapacitated → Combat Action → Participation-derived intent`다.
14. Collapse는 CombatTarget을 유지하되 dynamic target-facing과 Gameplay Focus를 즉시 억제한다.
    CollapseOut Reset 뒤에만 현재 유효 CombatTarget을 기준으로 Focus/Facing을 복구한다.
15. `EDamageDefenseOutcome`은 Guard / Parry 같은 방어 판정 사실만 보존한다. 최종 피격 표현은
    `EDamageReactionOutcome`으로 별도 확정해 Target Packet에 기록한다.
16. `CollapseHit`은 실제 피해가 Collapse Loop 중에 확정됐을 때 선택되는 Damage Reaction이다. Count,
    Balance lifecycle serial, Loop TTL을 변경하거나 연장하지 않는다.

---

## 3. 용어

| 용어 | 의미 |
| --- | --- |
| Balance Count | Enemy가 축적한 Balance 수치 |
| threshold crossed | 이번 Count commit에서 이전 값은 Max 미만이고 새 값이 Max 이상이 된 경우 |
| Collapse lifecycle | CollapseIn Started부터 CollapseOut Reset 또는 Abort까지의 보호된 수명 |
| Collapse Loop | CollapseIn이 끝난 뒤 Idle locomotion 위에서 보이는 무력화 표현 |
| Collapse Loop TTL | CollapseIn Completed 뒤부터 CollapseOut 요청까지의 Loop 유지 시간 |
| `IsCollapsePoseActive()` | Collapse pose를 보여야 하는지. `CollapseInActive`, `CollapseLoopActive`, `CollapseOutPending`에서 true |
| `IsCollapseLoopActive()` | 실제 Collapse Loop 구간인지. `CollapseLoopActive`에서만 true |
| `IsBalanceLifecycleBlocking()` | 일반 Action / Movement를 차단해야 하는지. `Accumulating` 이외 state에서 true |
| `ShouldSuppressCombatTargetFacing()` | Gameplay Focus / dynamic target-facing을 억제해야 하는지. In Started 뒤부터 Reset 전까지 true |
| `EDamageDefenseOutcome` | Target의 방어 판정 사실. `None`, `Guard`, `Parry` |
| `EDamageReactionOutcome` | Target이 확정한 피격 Reaction 결과. `None`, `Hit`, `BlockHit`, `Parry`, `CollapseHit`, `Dead` |
| CollapseHit | `CommittedDamage > 0`이고 `IsCollapseLoopActive()`일 때 선택되는 Loop 전용 Damage Reaction |
| Execution capability | 향후 Source/Target 협업 실행을 허용하는 별도 상태. R07에는 구현하지 않음 |
| Abort | Reject, Interrupted, Notify 누락, Death 같은 실패·강제 종료에서 Count와 runtime을 안전하게 초기화하는 경로 |
| Shutdown | EndPlay에서 gameplay delegate를 새로 발행하지 않고 timer와 runtime을 정리하는 silent teardown 경로 |

이 query들은 저장 상태가 아니라 `EBalanceLifecycleState`에서 파생한다. 따라서 pose 전환,
행동 차단, Facing 억제의 서로 다른 수명을 별도 bool로 중복 소유하지 않는다.

---

## 4. 책임 경계

| 책임 | 소유자 | 하지 않는 일 |
| --- | --- | --- |
| Count, Max, 잠금, lifecycle ID, Collapse Loop TTL | `UCBalanceComponent` | packet 수신·검증, montage 선택, Combat Participation 해제 |
| Damage / Defense / Health commit, `EDamageReactionOutcome` 확정, Parry CombatResult 검증, Balance commit, CollapseIn/Out Reaction 요청, Balance 관련 Reaction event/Notify 전달 | `UCCombatSignalTargetComponent` | Count lifecycle 직접 수정, montage 실행 상태 소유 |
| CombatResult packet 수신과 TargetComponent forwarding | `ACPlayer`, `ACEnemy` | Count 누적, Reaction 직접 요청 |
| Damage ReactionOutcome mapping, CollapseIn / CollapseOut candidate, key resolve, intervention | `UCReactionOrchestratorComponent` | Damage·Defense·Balance Count 결정 |
| 실제 Reaction Started / terminal event | `UCReactionComponent` | Balance 의미 해석 |
| locomotion 표현 | AnimBP | gameplay state 변경 |
| Collapse 중 intent 억제 | AI Context / BT adapter | Evidence나 Assignment 해제 |
| 최종 Action / Movement 요청 차단, lifecycle 최초 차단 전이에서 active AI move 중지 | ActionOrchestrator / MovementComponent | Balance lifecycle 상태 변경 |
| Death lifecycle | `ACEnemy` Death coordinator | Balance에 Dead phase 추가 |

현재 C++ API 연결은 다음과 같다.

```text
ACPlayer / ACEnemy::ReceiveCombatResultPacket
→ UCCombatSignalTargetComponent::RequestCombatResultTarget
→ ValidateCombatResultTargetRequest
→ Player는 Balance policy 미보유로 no-op / Enemy는 UCBalanceComponent로 진행
```

```text
ACEnemy::ReceiveCombatResultPacket
→ UCCombatSignalTargetComponent::RequestCombatResultTarget
→ ProcessCombatResultTarget / HandleParryCombatResult
→ UCBalanceComponent::AdvanceBalanceFromParry
→ FBalanceLifecyclePacket
→ UCCombatSignalTargetComponent::DispatchBalanceLifecycleReaction
  → FBalanceLifecycleReactionRequest 조립
→ UCReactionOrchestratorComponent::RequestBalanceLifecycleReaction
→ OnBalanceLifecycleReactionRequestResolved(BalanceLifecyclePacket, Result)
→ UCBalanceComponent::HandleBalanceLifecycleReactionRequestResolved
  → accepted면 Started lifecycle event 대기
  → rejected / ignored면 같은 BalanceLifecycleSerial과 Pending state일 때만 Abort

UCReactionComponent lifecycle event
→ UCCombatSignalTargetComponent::HandleReactionExecutionLifecycleEvent
→ UCBalanceComponent의 Started / terminal transition

UCReactionComponent ResetBalance notify command
→ UCCombatSignalTargetComponent::HandleReactionExecutionNotifyCommand
→ UCBalanceComponent::TryCommitCollapseReset

UCBalanceComponent Collapse Loop TTL expiry
→ UCBalanceComponent internally transitions to CollapseOutPending
→ OnBalanceLifecycleReactionRequested(FBalanceLifecyclePacket)
→ UCCombatSignalTargetComponent::HandleBalanceLifecycleReactionRequested
→ UCCombatSignalTargetComponent::DispatchBalanceLifecycleReaction
```

---

## 5. Authoritative Runtime State

`UCBalanceComponent`가 다음 상태를 소유한다.

```text
CurrentBalanceCount
BalanceThreshold
BalanceLifecycleSerial
BalanceLifecycleState
CollapseLoopTimer
```

### 5.1 Lifecycle State

| 상태 | 의미 | Count 변경 |
| --- | --- | --- |
| `Accumulating` | 정상 누적 상태 | 허용 |
| `CollapseInPending` | Max commit은 완료됐고 CollapseIn Started를 기다림 | 잠금 |
| `CollapseInActive` | CollapseIn montage가 실행 중인 상태 | 잠금 |
| `CollapseLoopActive` | CollapseIn이 정상 완료됐고 Loop TTL이 진행 중인 상태 | 잠금 |
| `CollapseOutPending` | TTL 만료 뒤 CollapseOut request를 보냈고 실제 Started를 기다리는 상태 | 잠금 |
| `CollapseRecovering` | CollapseOut이 Started됐고 Reset Notify를 기다리는 상태 | 잠금 |

`WeakLoop`, `Executionable`을 별도 lifecycle state로 저장하지 않는다. `CollapseRecovering`은
Weak Loop의 표현 상태가 아니라 Count reset 및 lock 해제 전의 CollapseOut 실행 수명을 표현한다.

- Collapse Loop는 `ExecutionState == Idle && IsCollapsePoseActive()`의 표현 결과다.
- CollapseIn / CollapseOut은 active Reaction Context로 확인한다.
- Execution capability는 R08에서 실제 producer와 consumer가 생길 때 별도 상태로 도입한다.

### 5.2 Derived Consumer Queries

`EBalanceLifecycleState` 하나에서 소비자별 query를 파생한다.

| Lifecycle state | `IsCollapsePoseActive()` | `IsCollapseLoopActive()` | `IsBalanceLifecycleBlocking()` | `ShouldSuppressCombatTargetFacing()` |
| --- | --- | --- | --- | --- |
| `Accumulating` | false | false | false | false |
| `CollapseInPending` | false | false | true | false |
| `CollapseInActive` | true | false | true | true |
| `CollapseLoopActive` | true | true | true | true |
| `CollapseOutPending` | true | false | true | true |
| `CollapseRecovering` | false | false | true | true |

`CollapseOut Started`에서 pose query만 false가 된다. 이는 Collapse_End montage가 재생되는 동안
AnimBP가 normal locomotion 결과를 준비하게 하기 위함이다. Count lock, 일반 행동 차단, Facing
suppression은 Reset Notify까지 유지한다.

`UCBalanceComponent`의 구현 API는 `AdvanceBalanceFromParry`, `HandleCollapseReactionExecutionStarted`,
`HandleCollapseReactionExecutionTerminal`, `TryCommitCollapseReset`,
`AbortBalanceLifecycle`로 구성한다. Parry packet은 TargetActor와 CombatSignalResultSerial로
중복 수신을 방지한다.

### 5.3 Count Commit

```text
Accumulating 상태에서 Parry packet 수신
→ Count 증가 및 Max clamp
→ threshold crossed 계산
→ crossed면 BalanceLifecycleSerial 증가
→ CollapseInPending과 Count 잠금을 먼저 commit
→ FBalanceAdvanceResult 발행
→ CollapseIn request를 정확히 한 번 전송
```

`Count == Max` 비교만으로 Collapse를 시작하지 않는다. 큰 증가량, 중복 packet, Pending 이후의
재진입에서도 request가 한 번만 발생하도록 `PreviousCount < Max && CurrentCount >= Max`를 사용한다.

---

## 6. Parry CombatResult 처리

```text
A가 B에게 DamagePacket 송신
→ B가 Parry 판정
→ CombatResultPacket의 effect receiver = 원래 SourceActor A
→ A의 ICombatResultReceiver
→ A의 UCCombatSignalTargetComponent
→ Parry Balance 처리
```

Enemy와 Player는 모두 CombatResult receiver일 수 있다. 그러나 R07의 Balance Collapse 정책은
Enemy에만 적용한다.

```text
Player가 Enemy를 Parry
→ Enemy Balance 증가
→ Max 도달 시 Enemy Collapse

Enemy가 Player를 Parry
→ Player의 기존 Stagger 정책 유지
→ Player Beta/Burst 정책은 별도 R07 범위에서 결정
```

Player와 Enemy가 receiver라는 사실만으로 같은 resource lifecycle을 강제 공유하지 않는다.

---

## 7. Collapse In / Loop / Out 전이

### 7.1 CollapseIn

```text
CollapseInPending
→ CollapseIn request
→ ReactionComponent Started(full context)
→ BalanceLifecycleSerial / Global key 검증
→ CollapseInActive
```

`Accepted` 반환만으로 `CollapseInActive`로 전이하면 안 된다. request가 수락됐더라도 실제 실행
시작이 누락될 수 있기 때문이다. CollapseIn Started가 pose, Facing suppression, lifecycle 시작의
실제 권위다.

CollapseIn Started 이후:

```text
CollapseIn Completed
→ CollapseLoopActive
→ Collapse Loop TTL 시작

CollapseIn Interrupted / Ignored
→ Abort

CollapseIn Completed인데 Loop TTL이 시작되지 못함
→ Abort
```

### 7.2 Collapse Loop

CollapseIn이 완료되면 Reaction state는 Idle로 복귀한다. AnimBP는 다음 조건으로 Collapse Loop를
표현한다.

```text
ExecutionState == Idle
AND IsCollapsePoseActive() == true
→ Collapse Loop overlay
```

TTL은 CollapseIn의 정상 Completed 시점부터 계산한다. CollapseIn Started 또는 threshold 도달부터
계산하면 In montage 시간만큼 Loop 유지 시간이 줄어든다.

### 7.3 CollapseOut

```text
Collapse Loop TTL 만료
→ CollapseOutPending
→ CollapseOut request
→ CollapseOut Started
→ CollapseRecovering
→ IsCollapsePoseActive() = false
→ Reset Notify
→ Count = 0
→ Accumulating
```

TTL 만료에서는 Weak Loop pose를 유지한다. 반면 CollapseOut Started에서는 Weak Loop pose를
해제해 AnimBP가 normal locomotion 결과를 준비하게 한다. Count lock과 Facing suppression은
`CollapseRecovering` 동안에도 유지한다.

Reset Notify는 Enemy가 다시 전투 가능한 상태로 회복되는 정확한 animation frame에 둔다.

```text
CollapseOut Rejected / Interrupted / Ignored
→ Abort

CollapseOut Completed인데 Reset Notify 누락
→ Abort(ResetNotifyMissing)
```

Abort는 Count를 0으로 초기화하고 lock과 timer를 해제한 뒤 `Accumulating`으로 복귀한다.

---

## 8. Reaction Key / Context 계약

Balance lifecycle Reaction은 특정 DamageSpec에 종속되지 않는다.

```text
CollapseIn  = Global + CollapseIn  + INDEX_NONE
CollapseOut = Global + CollapseOut + INDEX_NONE
CollapseHit = DamageSpec + CollapseHit + INDEX_NONE
```

따라서 `FReactionDataKey`는 기존 DamageSpec mode의 직렬화 호환을 유지하면서 Global match mode와
Reaction index를 지원해야 한다. Global과 DamageSpec 간 암묵적 fallback은 허용하지 않는다.

`FReactionExecutionContext`에는 `BalanceLifecycleSerial`을 보존한다. terminal event와 Notify는 다음을
모두 검증한다.

```text
BalanceLifecycleSerial
ReactionType (CollapseIn 또는 CollapseOut)
Reaction key match mode (Global)
현재 BalanceLifecycleState
```

별도 `BalanceStage` enum은 만들지 않는다. stage 의미는 ReactionType과 lifecycle state의 조합으로
판별할 수 있다.

### 8.1 Damage Reaction Outcome 계약

`FCombatSignalTargetResult`은 방어 판정과 최종 피격 표현을 분리해 보존한다.

```text
EDamageDefenseOutcome
→ 방어 사실: None / Guard / Parry

EDamageReactionOutcome
→ 표현 결과: None / Hit / BlockHit / Parry / CollapseHit / Dead
```

`UCCombatSignalTargetComponent`는 Damage와 Health commit이 끝난 뒤 다음 순서로
`ReactionOutcome`을 확정한다.

```text
Dead transition
→ Dead

DefenseOutcome = Parry
→ Parry

DefenseOutcome = Guard
→ BlockHit

CommittedDamage > 0
AND IsCollapseLoopActive() == true
→ CollapseHit

CommittedDamage > 0
→ Hit
```

`UCReactionOrchestratorComponent`는 Balance, Defense, HP를 다시 해석하지 않는다.
Packet의 `ReactionOutcome`을 `EReactionType`으로 mapping하고 ReactionData / Executor를
resolve·orchestrate한다. 따라서 Debug와 Packet audit은 Defense와 Reaction을 각각 확인할 수 있다.

`CollapseHit`은 `CollapseLoopActive`에서만 선택한다. CollapseIn montage 실행 중인
`CollapseInActive`, CollapseOut 요청·복구 구간에서는 선택하지 않는다. 이는 In/Out lifecycle을
별도 피격 표현이 중단시키지 않게 하기 위한 계약이다.

### 8.2 CollapseHit 실행 계약

```text
CollapseLoopActive
→ 실제 Damage 확정
→ DamageReactionOutcome = CollapseHit
→ UCReaction_CollapseHit montage 재생
→ terminal event
→ 기존 Collapse Loop pose로 복귀
```

CollapseHit은 Balance Count, BalanceLifecycleSerial, Collapse Loop TTL을 변경하거나 연장하지 않는다.
CollapseHit executor는 intervention 예외를 직접 소유하지 않는다. 실행 중인 CollapseHit은
ReactionData의 `AllowInterventionRules`에서 incoming `CollapseOut`을 `Always`로 허용해 정상
lifecycle recovery를 통과시킨다. Dead는 기존 Orchestrator 강제 intervention 규칙을 따른다.
첫 구현에서 `WantInterventionRules`는 비워 두므로 CollapseHit은 다른 실행을 강제로 중단하지 않으며,
반복 CollapseHit도 재시작하지 않는다. 향후 연출 요구가 생기면 Want / Allow rule 조합만으로 별도
정책을 연다.

---

## 9. AI / Action / Movement / Participation 정책

### 9.1 AI Intent

Collapse는 전투 관계의 종료가 아니라 AI의 일시적인 무력화다. 따라서 `EAIIntentState`에는
`Incapacitated`를 추가하고 `UCBTService_UpdateAIIntentState`는 다음 순서로 의도를 결정한다.

```text
Dead
→ active Reaction
→ Balance lifecycle blocking
→ active Combat Action
→ Combat Participation-derived intent
```

```text
CollapseInPending / CollapseInActive / CollapseLoopActive / CollapseOutPending / CollapseRecovering
→ EAIIntentState::Incapacitated

CollapseIn / CollapseOut Reaction 실행 중
→ 기존 active Reaction 우선 규칙에 따라 Reaction intent
```

`Incapacitated`는 모든 상태이상을 뜻하는 `StatusEffect`가 아니다. Slow, Poison처럼 AI의 모든
행동을 막지 않는 향후 상태이상과 구분되는, Action·Movement·일반 전투 의도를 막는 행동 결과다.

현재 `EAIIntentState::HitReact`는 `bIsActiveReaction`인 모든 비-Dead Reaction을 표현한다. R07은
직렬화된 Blackboard/UAsset 호환을 위해 이 값을 유지한다. enum 선언 순서가 아닌 위 결정 순서가
우선순위를 보장하며, `Incapacitated`는 기존 enum 값의 숫자를 바꾸지 않도록 `Max` 직전에 추가한다.

Intent는 BT의 행동 선택과 관찰 상태다. 실제 차단 권위는 `UCBalanceComponent`의 lifecycle query와
Action/Movement 최종 gate에 남는다. `Incapacitated`도 Dead와 active Reaction처럼 Runtime LOD의
CombatCritical intent로 취급해 Collapse timer, recovery, context 갱신이 저하되지 않게 한다.

### 9.2 억제 범위

```text
IsBalanceLifecycleBlocking() == true
→ 일반 AI combat intent 억제
→ 신규 Attack / Movement Action 거절
→ Move / Jump / non-zero axis input 거절
→ 이미 진행 중인 AI path-following은 Pending 진입에서 StopMovement
```

다음은 cleanup을 위해 허용한다.

```text
StopMovement
StopJump
입력을 0으로 되돌리는 release
Reaction의 정규 intervention과 terminal cleanup
```

Dead, HitReactive, CollapseOut은 Action gate의 대상이 아니다. Reaction intervention 정책에서 충돌
관계를 결정한다.

### 9.3 Combat Participation과 분리

```text
Active Evidence / Assignment / CombatTarget
→ 유지

AI Engage 행동 / Chase / Attack
→ Collapse 동안 억제
```

Collapse는 ReturnHome, soft release, hard release, evidence withdrawal의 사유가 아니다. CollapseOut
후 Enemy는 살아 있는 Evidence와 기존 Assignment에 따라 자연스럽게 전투를 이어 간다.

### 9.4 Facing / Gameplay Focus 억제

Collapse 중 Enemy는 현재 CombatTarget을 계속 알고 있지만, 무릎 꿇은 Loop pose가 Target 위치를
따라 회전해서는 안 된다.

```text
CombatTarget / Assignment
→ 유지

dynamic target-facing / Gameplay Focus
→ Collapse 동안 억제

Actor yaw
→ Collapse 진입 시점의 방향을 유지
→ montage root motion이 회전해야 하면 animation이 권위를 가짐
```

`UCBalanceComponent`의 lifecycle state 변경 event는 `UCEnemyCombatTargetFacingComponent`에 즉시
전달된다. Facing component는 `ShouldSuppressCombatTargetFacing()` query를 적용한다. AI Intent
Blackboard tick을 Facing 억제의 근거로 사용하지 않는다. 이미 설정된 Gameplay Focus가 tick 사이에도
Controller rotation을 갱신할 수 있기 때문이다.

```text
CollapseIn Started
→ SetCombatTargetFacingSuppressed(true)
→ Gameplay Focus clear
→ ControllerDesired target-facing 해제
→ Target 추적 회전 중단

CollapseOut Started
→ Weak Loop pose 해제
→ Facing suppression 유지

CollapseOut Reset Notify
→ SetCombatTargetFacingSuppressed(false)
→ 현재 CombatTarget이 유효하면 Gameplay Focus / ControllerDesired 복구
→ Target이 없으면 Focus clear 및 기본 locomotion facing 유지
```

suppression 중 CombatTarget이 교체·해제되면 Facing component는 내부 target snapshot만 최신화하고
Focus를 다시 설정하지 않는다. suppression 해제 시점에 현재 snapshot을 한 번 읽어 복구한다.

---

## 10. Death / EndPlay / 실패 복구

Death는 Balance의 별도 state가 아니다.

```text
Health Dead
→ ACEnemy::BeginDeathLifecycle()
→ UCBalanceComponent::AbortBalanceLifecycle(OwnerDeath)
→ Combat Participation hard release
→ Action / Weapon / AI cleanup
→ Dead Reaction / Presentation / Destroy
```

EndPlay에서는 gameplay delegate를 새로 발행하지 않는 silent shutdown으로 timer와 runtime state를
정리한다. 늦은 timer, notify, terminal event는 BalanceLifecycleSerial과 현재 state가 다르면 무시한다.

---

## 11. Future Execution Boundary

R07은 timeout 기반 Collapse lifecycle까지만 구현한다.

```text
CollapseIn → Collapse Loop → TTL → CollapseOut → Reset
```

R08에서 실제 Source/Target Execution 협업이 확정된 뒤에만 다음을 추가한다.

```text
bCanStartExecution
Execution consume
Source / Target reservation
CombatTarget revision 재검증
position / facing alignment
complete / cancel / death 정책
```

`IsCollapsePoseActive()`와 미래 `bCanStartExecution`은 같은 의미가 아니며, Execution
availability가 Collapse montage의 어느 Notify 또는 Loop 시점에 열리는지도 R08의 소비자 계약과 함께
결정한다.

---

## 12. Asset / Notify 계약

R07의 수동 Editor 작업은 코드 계약이 확정된 뒤 수행한다.

| 대상 | Editor 작업 | 완료 조건 |
| --- | --- | --- |
| `Collapse_Start_Montage` | `UCAnimNotify_CompleteReaction` 배치 | CollapseIn이 terminal event를 남긴다. |
| `Collapse_End_Montage` | 실제 회복 frame에 `UCAnimNotify_ResetBalanceLifecycle`, 그 뒤 `UCAnimNotify_CompleteReaction` 배치 | Reset 이후에만 Out terminal이 발생한다. |
| `Collapse_Hit01_Montage` | `Collapse_Hit01`을 source로 montage를 만들고 `UCAnimNotify_CompleteReaction` 배치 | 정상 Complete 뒤에는 Loop pose로 복귀하고, CollapseOut interruption이면 recovery로 전환한다. |
| `BP_CEnemy` | Reaction Data에 `Global + CollapseIn + INDEX_NONE`, `Global + CollapseOut + INDEX_NONE`, `DamageSpec + CollapseHit + INDEX_NONE` entry를 추가하고 해당 montage 및 executor를 연결한다. CollapseHit의 `WantInterventionRules`는 비우고, `AllowInterventionRules`에는 incoming `CollapseOut`, `Always`를 추가한다. | CollapseIn/Out은 Global, CollapseHit은 기존 DamageSpec fallback 계약을 사용한다. CollapseOut 허용은 executor 예외가 아닌 data policy다. |
| `BP_AIPerf_Enemy` | parent 상속이 두 Reaction Data와 Component를 그대로 받는지 확인 | AIPerf actor도 같은 Collapse lifecycle을 실행한다. |
| `ABP_Character` | Idle overlay에 `bIsCollapsePose && CurrentExecutionState == Idle` 조건으로 `Collapse_Loop`을 연결하고, Full-body Reaction Slot이 그 뒤에서 CollapseHit montage를 재생하는지 확인 | Dead는 계속 최상위 우선순위이고, CollapseOut Started 직후 locomotion 결과를 준비한다. |
| `ABP_AIPerf_Character` | 위 AnimBP 계약을 동일하게 반영 | AIPerf refresh throttle과 무관하게 lifecycle delegate 값이 즉시 투영된다. |
| `BT_Default` / `BT_AIPerf_Default` | `Incapacitated` Intent의 passive branch를 추가한다. branch는 `IsBalanceLifecycleBlocking()`이 false가 될 때까지 행동 요청을 보내지 않는다. | Collapse 중 기존 HitReact/Combat/Participation branch가 재진입하지 않는다. |
| `BB_Default` / `BB_AIPerf_Default` | enum key가 `Incapacitated` 값을 해석하는지 compile·save로 확인 | 새 Blackboard key는 만들지 않는다. |

`Collapse_Loop`은 montage가 아니라 AnimBP 표현이다. 따라서 Loop TTL은 CollapseIn Completed event에서
시작하며 Loop에 별도 Complete Notify를 추가하지 않는다. `ResetBalanceLifecycle` Notify는
`CompleteReaction`보다 반드시 먼저 배치한다.

CollapseIn에는 R07에서 필수 Notify가 없다. Loop TTL은 CollapseIn Completed event에서 시작한다.
Execution availability Notify는 R08 전에는 추가하지 않는다.

---

## 13. 검증 기준

| 시나리오 | 기대 결과 |
| --- | --- |
| Parry 후 Max 미도달 | Count만 증가, CollapseIn request 없음 |
| threshold 최초 도달 | Pending과 lock이 먼저 commit되고 CollapseIn request는 한 번 |
| Pending 중 추가 Parry | Count와 lifecycle 변경 없음 |
| CollapseIn Started | `CollapseInActive`, Collapse pose 준비 및 Facing suppression 시작 |
| CollapseIn Completed | `CollapseLoopActive`, Collapse Loop TTL 시작 |
| Collapse Loop 중 실제 피해 | `Defense: None`, `Reaction: CollapseHit`, 전용 montage 재생 뒤 Loop pose 복귀 |
| CollapseHit 실행 중 TTL 만료 | CollapseOut이 CollapseHit을 중단하고 정상 recovery 진행 |
| CollapseIn Reject / Interrupt / Ignore | Abort, Count 0, unlock |
| Loop TTL 만료 | `CollapseOutPending`, CollapseOut request, Weak Loop pose 유지 |
| CollapseOut Started | `CollapseRecovering`, Weak Loop pose 해제, normal locomotion 결과 준비, lock/Facing suppression 유지 |
| CollapseOut Reset Notify | Count 0, unlock, `Accumulating`, 현재 target 기준 Focus/Facing 재개 |
| Out terminal인데 Reset Notify 없음 | Abort 및 audit |
| Collapse 중 AI/Action/Movement 요청 | 일반 전투 요청 차단 |
| Collapse 중 Evidence / Assignment | 유지 |
| CollapseIn / Loop 중 Target 이동 | CombatTarget은 유지하되 Gameplay Focus와 dynamic target-facing은 없음 |
| CollapseOut Reset Notify | 현재 유효 CombatTarget 기준으로 Focus/Facing 재개 |
| Death / EndPlay | timer와 lifecycle 즉시 정리, stale callback 무시 |
| 기존 Player Stagger | Collapse lifecycle로 변경되지 않음 |

---

## 14. 범위 밖

- Player Beta / Burst 자원 모델 및 소비 정책
- Source / Target Execution 협업과 consume
- 다중 threshold와 outcome 우선순위
- 복수 Collapse presentation variant 선택
- Scripted immunity, cutscene suspend 같은 external lock
- R13의 최종 HUD/Player Resource UI

이 항목은 필요가 확인될 때 별도 설계·작업 단위로 연다.
