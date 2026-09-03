# S36. Execution Collaboration Architecture

> 상태: R08 Execution Collaboration의 현재 구현 계약. C++는 pair session, target revision,
> Standard Balance lifecycle, Lethal Health/Death handoff까지 이 문서의 계약으로 cutover했다.
> Execution Action/Reaction data, Montage Notify, AnimBP pose, PIE 검증은 별도 Editor 작업으로 남아 있다.
>
> 범위: Collapse Loop 기회 예약, Source/Target 동기 연출, Standard / Lethal outcome, Commit,
> 취소, death presentation handoff.
>
> 제외: Player Balance 정책, Execution UI, 다중 대상 처형, 네트워크 동기화.

---

## 1. 목적

Execution은 기존 Action 또는 Reaction 하나를 추가하는 기능이 아니다. Source와 Target이 하나의
처형 수명과 결과를 공유하는 두 Actor 협업 거래다.

- Source는 Execution Action을 실행한다.
- Target은 Execution Reaction과 결과 적용을 실행한다.
- `UCExecutionCollaborationComponent`는 두 Actor 사이의 pair 관계만 소유한다.
- Action / Reaction Orchestrator는 각 Actor 내부의 arbitration만 담당하며 pair transaction은 소유하지 않는다.

첫 구현 범위는 montage 시작 전에 결정되는 두 정책을 지원한다.

| 정책 | 결과 |
| --- | --- |
| `Standard` | Target은 생존하며 Balance 소유 Down pose를 거쳐 회복한다. |
| `Lethal` | Target은 Source impact frame에서 사망하고 execution 전용 death presentation으로 이어진다. |

이 계약은 `DeathPending`을 도입하지 않는다. Health는 `Alive` 또는 `Dead`이며, HP가 0인 Target이
논리적으로 Alive인 상태로 남아서는 안 된다.

---

## 2. 용어

| 용어 | 의미 |
| --- | --- |
| Source | Execution을 시작하고 Action을 소유하는 Actor |
| Target | 처형을 당하며 Reaction과 결과 적용을 소유하는 Actor |
| Primary In pair | 시작 시 동기화되어 재생되는 Source Action과 Target Reaction |
| Source Commit | Source montage Notify가 표시하는 impact frame. 선택된 outcome을 되돌릴 수 없게 만드는 시점 |
| Execution Down | Standard 전용. Primary pair 이후의 state-driven prone presentation |
| Death presentation mode | death entry와 persistent dead pose를 선택하는 Actor 소유 값. Health state가 아님 |

`FExecutionSessionId`는 하나의 pair transaction을 식별한다. `FCombatTargetSnapshot`은 Source가
요청한 특정 Target 관계를 식별한다. `BalanceLifecycleSerial`은 Target의 특정 Collapse opportunity를
식별한다.

---

## 3. 책임 분리

| 소유자 | 책임 |
| --- | --- |
| `UCExecutionCollaborationComponent` | Pair session, Target-side outcome 결정, partner 검증, Commit 이전 취소, Source Commit routing, primary terminal 관찰 |
| `UCCombatTargetComponent` | Source Target identity와 revision 검증 |
| `UCBalanceComponent` | Collapse opportunity, reservation, Loop TTL pause/resume, Standard Execution Down/Recovery lifecycle |
| Action / Reaction Orchestrator | Actor 내부 arbitration만 담당. pair transaction은 소유하지 않음 |
| `UCCombatSignalTargetComponent` | Target-side execution outcome ingress와 Health commit routing |
| Actor-owned Death lifecycle (`ACEnemy`) | Death presentation mode, death-entry 완료, presentation, dissolve, destroy |
| `UCHealthComponent` | HP 및 확정적인 `Alive -> Dead` 전이만 담당 |
| AnimBP | lifecycle query/mode를 읽어 persistent pose를 선택. gameplay state는 소유하지 않음 |

---

## 4. 공통 진입과 Opportunity Reservation

```text
Execution 요청
→ Source CombatTarget Snapshot / Revision 검증
→ Target이 CollapseLoopActive 및 opportunity 가능 여부 검증
→ Target이 자신의 Health 및 `LethalCondition`으로 Standard / Lethal outcome을 한 번 결정
   - Source는 outcome을 요청하거나 변경하지 않음
→ Target이 opportunity 예약
   - Collapse Loop TTL 정지
   - 남은 Loop seconds는 Commit 이전 취소를 위해 보관
→ Target primary Reaction 시작
→ Source primary Action 시작
```

reservation만 유지되는 동안 Balance lifecycle state는 `CollapseLoopActive`다. Source/Target primary pair가
모두 시작되면 Target Balance는 `ExecutionPrimaryActive`로 전이한다. 이 상태는 Primary 연출이 실제로
진행 중이지만 아직 Commit 이전이므로 rollback 가능한 구간이다.

Commit Notify가 통과하면 Target Balance는 `ExecutionPrimaryCommitted`로 전이하고 reservation을 소비한다.
따라서 Commit 이전 취소에서는 정확히 남아 있던 Loop TTL을 재개할 수 있지만, Commit 이후에는 Loop로
복귀하지 않는다.

`IsExecutionOpportunityAvailable()`가 유일한 capability query다.

```text
CollapseLoopActive
AND active execution reservation 없음
```

별도의 `bExecutable` source of truth는 두지 않는다.

### 4.1. Target Outcome 결정

outcome은 Source 입력의 옵션이 아니라 Target이 자신의 상태로 결정하는 계약이다. Target이 reservation을
수락할 때 선택한 `OutcomePolicy`는 `FExecutionCollaborationContext`에 보관되며, primary pair가 시작된
뒤에는 바꾸지 않는다.

| `LethalCondition` | 결과 |
| --- | --- |
| `Disabled` | 항상 `Standard` |
| `HealthRatio` | Target의 `CurrentHP / MaxHP`가 `LethalHealthRatio` 이하이면 `Lethal`, 아니면 `Standard` |

`HealthRatio` 조건은 Alive이며 kill 가능한 Target에만 적용한다. Max HP가 유효하지 않거나 조건이
성립하지 않으면 안전하게 `Standard`로 fallback한다.

---

### 4.2. Execution 시작 기하 조건과 Target 단발 facing 정렬

Execution 입력은 Player Target Lock 정책을 통과한 경우에만 들어온다. Target Lock은 현재 Combat Target을
선택하는 Player-side 입력 조건이며, 협업 컴포넌트 자체는 Target Lock에 의존하지 않는다. Target이 Collapse
기회를 보유한다는 사실만으로는 시작할 수 없으며, Source가 실제로 Target을 향하고 있는지와 두 Actor의
시작 거리를 함께 검증한다.

`UCExecutionCollaborationComponent`의 Source-side `StartGeometrySettings`가 실제 Actor transform을 기준으로
다음 공통 시작 조건을 검증한다.

| 설정 | 책임 |
| --- | --- |
| `MaxStartDistance` | 입력 시점에 허용하는 현재 평면 거리 상한 |
| `MaxSourceFacingAngleDegrees` | Source Actor Forward와 Source→Target 방향 사이의 최대 평면 각도 |

현재 Player가 Source인 정책은 Player의 위치·yaw를 고정하고 Enemy Target의 yaw만 한 번 정렬한다.

```text
Player Execution Input
→ Target Lock 활성 여부 검증
→ Source Combat Target Snapshot / Revision 검증
→ Source 실제 Actor Forward 기준 Target 평면 각도 검증
→ 현재 Source–Target 평면 거리 검증
→ Target Collapse opportunity reservation
→ Target의 yaw만 Source를 향하도록 한 번 정렬
→ Target Primary Reaction / Source Primary Action 시작
→ ExecutionPrimaryActive
```

시작 정렬은 Target의 위치를 이동하지 않는다. pitch/roll, Player transform, 매 Tick 추적·보정도 사용하지
않는다. 이후 위치와 root motion은 각 Primary montage가 소유한다. 따라서 sweep, 위치 보정 거리, 부분 이동
복원 같은 별도 이동 정책은 없다.

Target Lock은 Controller rotation을 Target 방향으로 보조하지만, Character actor yaw가 항상 즉시 일치한다는
보장은 없다. 그래서 최종 시작 허가는 Target Lock 상태만으로 결정하지 않고 실제 Source Actor Forward의
기하 검증까지 통과해야 한다.

정확한 손·무기 접촉을 위해 위치까지 맞춰야 하는 pair가 추가되면, raw actor teleport 대신 별도 Aligning 단계와
Motion Warping/root-motion 기반 보정을 도입한다. 이는 현재 R08의 범위가 아니다.

---

## 5. Standard Execution 계약

Standard는 Death lifecycle이 아니라 Balance lifecycle이다.

```text
CollapseLoopActive
→ opportunity 예약 / Loop TTL 정지
→ Source Standard Execution Action
  + Target Standard Execution In Reaction
→ Balance: ExecutionPrimaryActive
→ Source Commit Notify
  - pair와 현재 reservation 재검증
  - Action Data의 StandardExecutionDamage를 CurrentHP - 1까지 적용
  - Balance: ExecutionPrimaryCommitted
  - opportunity 소비. 이후 Loop rollback 불가
  - 즉시 CollapseOut 요청은 하지 않음
→ 양쪽 primary terminal 관찰 완료
→ Balance: ExecutionDownActive
→ AnimBP: Standard Execution Loop (`bIsExecutionDownPose`)
→ ExecutionDownDuration 만료
→ Target ExecutionRecovery Out Reaction
→ Balance Reset Notify
→ Accumulating / 일반 locomotion
```

`ExecutionDownActive`는 `UCBalanceComponent`가 소유하는 권위 있는 lifecycle state다.
`bIsExecutionDownPose`는 이 state에서 파생되는 AnimBP presentation field일 뿐 독립 gameplay bool이 아니다.

`ExecutionDownDuration`은 `CollapseLoopDuration`과 분리한다.

- `CollapseLoopDuration`: Execution opportunity를 claim할 수 있는 시간
- `ExecutionDownDuration`: 성공한 Standard Execution 뒤 prone/recovery를 유지하는 시간

협업 session은 동기 primary pair가 끝나면 완료한다. 이후 Down/Recovery 수명은 Balance가 소유한다.

---

## 6. Lethal Execution 계약

Lethal은 기존 Health와 Death lifecycle을 사용하되, execution 전용 death presentation mode를 넘긴다.
두 번째 generic `Dead In`은 사용하지 않는다.

```text
CollapseLoopActive
→ opportunity 예약 / Loop TTL 정지
→ Source Lethal Execution Action
  + Target Lethal Execution In Reaction
→ Balance: ExecutionPrimaryActive
→ Source Commit Notify (impact frame)
  - pair / Target Snapshot / reservation 재검증
  - Balance: ExecutionPrimaryCommitted / opportunity 소비
  - ExecutionLethal death entry expectation 준비
  - Target-side Health::TakeDamage(CurrentHP)
  - Health는 즉시 Dead 전이
→ 일반 hard cleanup 시작
→ 현재 Target Lethal Execution In은 끝까지 재생 허용
→ 완료된 Reaction montage 아래에서 AnimBP가 Execution Lethal Dead Loop 노출
→ 기존 death presentation / dissolve / destroy
```

Source Commit은 실질적인 lethal gameplay 적용 시점이다. Standard/Lethal 정책은 pair 시작 전에 결정되므로
두 번째 outcome 선택 지점이 아니다.

Commit 이후 예상되는 Target의 `Alive -> Dead` 전이는 성공 경로이며 session 취소 사유가 아니다.
Commit 이전의 external Death는 취소 및 hard release다.

### 6.1. Death Presentation Mode

death coordinator가 mode를 소유한다. death entry route와 persistent AnimBP pose 선택을 모두
조정하기 때문이다.

```cpp
enum class EDeathPresentationMode : uint8
{
    Default,
    ExecutionLethal,
};
```

| Mode | Death entry | Persistent pose |
| --- | --- | --- |
| `Default` | 기존 generic `Dead` Reaction | 기존 Default Dead Loop |
| `ExecutionLethal` | 현재 실행 중인 Lethal Execution In을 death entry로 인정 | Execution 전용 Dead Loop |

`ExecutionLethal`에서는 death coordinator가 추가 generic `Dead` Reaction을 요청하거나 기다리지 않는다.
Lethal Execution In이 정상 terminal로 끝나면 그것이 death-entry 완료 신호가 되고 기존
presentation/dissolve 경로가 계속된다. Health가 이미 Dead인 뒤 예상 Lethal In이 interrupted되면
death coordinator는 presentation으로 직접 fallback하며 Balance를 복원하거나 Target을 되살리지 않는다.

`bIsDeadPose`는 계속 `Health == Dead`만 의미한다. AnimBP는 `DeathPresentationMode`와 결합해
Default 또는 Execution 전용 Dead Loop를 고른다. Full-body Lethal In Reaction은 terminal 전까지
시각적으로 우선이므로, Dead Loop는 그 뒤 자연스럽게 보인다.

---

## 7. Action / Reaction Data 계약

정책은 generic pair를 시작한 뒤 Lethal로 교체하지 않는다. Target이 reservation 수락 시점에 자신의
조건으로 정책을 결정하고, 그 결과로 authored Source/Target pair를 고른다.

| 정책 | Source | Target primary | Target 후속 |
| --- | --- | --- | --- |
| `Standard` | `EActionType::Execution` Standard data/index | `EReactionType::ExecutionStandard` | `EReactionType::ExecutionRecovery` |
| `Lethal` | `EActionType::Execution` Lethal data/index | `EReactionType::ExecutionLethal` | Death presentation mode / generic Dead In 없음 |

`ExecutionRecovery`는 초기 asset iteration에서 시각 소재를 재사용하더라도 `CollapseOut`과 구분한다.
`CollapseOut`은 자연 TTL 만료 recovery이고, `ExecutionRecovery`는 성공한 Standard Execution의 결과다.

모든 execution-originated `FReactionCandidate`와 `FReactionExecutionContext`는
`FExecutionSessionId`를 유지한다. terminal과 Notify consumer는 `EReactionType`만이 아니라 session ID도
대조해야 stale terminal이 이후 session에 영향을 주지 않는다.

---

## 8. Commit / 취소 / Terminal 규칙

```text
Source Commit 이전
→ Target 변경, 시작 거절, interruption, external death, EndPlay
→ pair 취소
→ reservation 해제
→ 보관한 Collapse Loop TTL 재개

Standard Commit 이후
→ Balance는 ExecutionPrimaryCommitted 상태로 primary terminal을 기다림
→ opportunity는 소비 상태 유지
→ Collapse Loop로 복귀하지 않음
→ Standard Down/Recovery를 안전하게 완료

Lethal Commit 이후
→ Health Dead는 최종 상태
→ pair rollback 또는 Balance 복원 없음
→ Lethal In terminal이 death-entry presentation을 완료
```

pair session은 동기 primary Action/Reaction terminal까지만 기다린다. Standard는 이후 Balance에,
Lethal은 Death lifecycle에 책임을 넘긴다. 장기 Down 또는 Dead presentation은 pair-session state가 아니다.

---

## 9. C++ 반영 내용

1. **Types 및 correlation**
   - `FExecutionCollaborationContext`의 `TargetActor + SourceTargetRevision`을
     `FCombatTargetSnapshot`으로 교체했다.
   - Execution Reaction Candidate와 execution context에 `FExecutionSessionId`를 전달한다.
   - `ExecutionRecovery`, `ExecutionLethal` ReactionType을 추가하고 policy별 Data를 resolve한다.

2. **Balance / Standard lifecycle**
   - 즉시 `RequestCollapseOutFromExecutionConsume()` 경로를 제거했다.
   - `ExecutionPrimaryActive`, `ExecutionPrimaryCommitted`, `ExecutionDownActive`,
     `ExecutionRecoveryPending`, `ExecutionRecoveryActive` lifecycle state를 추가했다.
   - reservation lifecycle을 Reserve / Activate / Commit / Release로 분리했다.
   - Standard Down duration, ExecutionRecovery request/started/terminal 처리, 일반화된 Balance Reset Notify 계약을 추가했다.

3. **Collaboration orchestration**
   - 시작 전 Standard/Lethal Source 및 Target primary data를 resolve한다.
   - `Commit Execution` Notify는 공통 Action Notify routing을 거쳐 `UCAction_Execution`이 처리하고, 해당 Action만 collaboration commit을 요청한다.
   - Commit 이후 일치하는 Lethal Death event는 취소가 아니라 성공으로 처리한다.
   - Standard 양쪽 primary terminal 후 Balance Execution Down으로 이관한다.
   - Lethal Commit에서는 CollapseOut을 요청하지 않고 consumed opportunity를 Death 경로에 이관한다.

4. **Target-side lethal outcome 및 death coordinator**
   - `Health::TakeDamage(CurrentHP)` 이전에 `ExecutionLethal` death entry expectation을 준비한다.
   - Enemy death coordinator가 active Lethal Execution In을 death entry로 사용하고 해당 mode에서는 generic Dead fallback을 우회하도록 확장한다.
   - 기존 generic Damage Death 경로는 변경하지 않는다.

5. **AnimBP, Data, Observability**
   - `bIsExecutionDownPose`를 Balance lifecycle state에서 파생한다.
   - death presentation mode로 Default / ExecutionLethal Dead Loop를 선택한다.
   - Standard Down remaining time과 lifecycle state의 focused debug visibility를 추가한다.
   - pair session, reservation, outcome policy, terminal, 시작 기하 조건의 전용 Execution Collaboration debug 표시를 추가한다.

---

## 10. Execution Session Debug 계약

Execution Session debug는 Balance lifecycle debug와 Recent Action / Reaction event summary를 대체하지 않는다.
각 도메인은 다음의 서로 다른 권위를 관찰한다.

| 도메인 | 관찰하는 권위 |
| --- | --- |
| Balance / Collapse | Target Balance count, lifecycle, Loop / Down TTL |
| Execution Session | Source × Target pair session, reservation, outcome, primary terminal, 시작 기하 조건 |
| Recent Action / Reaction | Event Log에 남은 가장 최근 Action / Reaction 판단 |

`Panel_01`의 Player / Enemy detail block은 각 Actor 관점에서 다음을 표시한다.

- 역할(Source / Target), partner actor, pair session serial, collaboration state(Reserved / Active / Committed)
- 선택된 outcome(Standard / Lethal), Target의 Collapse opportunity reservation, Source Action / Target Reaction terminal
- Player Source에서는 현재 Combat Target에 대한 시작 거리·정면 각도·허용 여부

World debug는 gameplay 상태를 바꾸지 않는 read-only 시각화다.

```text
Source 시작 기하 조건
→ 최대 시작 거리 원, Source forward ± 허용각 경계, Source → Target line
→ 유효: Green / 무효: Red

활성 pair session
→ Source ↔ Target link와 중앙 session label
→ Reserved: Yellow / Active: Magenta / Committed: Purple
```

제어 CVar는 다음으로 고정한다.

```text
Portfolio.DebugOverlay.ExecutionSession.Enabled
Portfolio.DebugOverlay.ExecutionSession.DrawStartGeometry
Portfolio.DebugOverlay.ExecutionSession.DrawPairLink
Portfolio.DebugOverlay.ExecutionSession.DrawWorldText
Portfolio.Debug.ExecutionSessionAudit

Portfolio.DebugOverlay.Player.ExecutionSession.Enabled
Portfolio.DebugOverlay.Enemy.ExecutionSession.Enabled
```

`ExecutionSession` Event Log category에는 reservation, pair activation, commit, damage apply, terminal,
cancel, completion event를 기록한다. 일반 Action / Reaction 판단은 별도 `ActionReaction` category에 기록한다. Audit CVar는 같은 lifecycle record를 Output Log에도 남긴다.

---

## 11. 필요한 Editor 작업

C++ cutover 완료 뒤 runtime 계약을 바꾸지 않는 범위에서 다음을 구성한다.

- `BP_CPlayer`: `EActionType::Execution` Standard index `0`, Lethal index `1` Data와 각각의 Source montage를 등록한다. Standard Data에는 0보다 큰 `StandardExecutionDamage`를 설정한다. 두 Data 모두 `UCAction_Execution`을 사용한다. Source `ExecutionCollaboration`의 `Start Geometry`에서 authored montage의 시작 허용 범위에 맞춰 `MaxStartDistance`, `MaxSourceFacingAngleDegrees`를 설정한다.
- `BP_CEnemy`: Global `ExecutionStandard`, `ExecutionLethal`, `ExecutionRecovery` Reaction Data를 등록한다. Primary 둘은 `UCReaction_Execution`, Recovery는 `UCReaction_ExecutionRecovery`를 사용한다. `Execution|Outcome`에서 `LethalCondition`과 `LethalHealthRatio`를 설정한다.
- Source Standard/Lethal montage에는 각각 impact frame의 `Commit Execution` 및 terminal의 `Complete Action` Notify를 둔다.
- Target Standard/Lethal In montage에는 `Complete Reaction` Notify를 둔다. Standard Recovery montage에는 `Reset Balance Lifecycle` 뒤 `Complete Reaction` Notify를 둔다.
- `ABP_Character`: `bIsExecutionDownPose`로 Standard Execution Down loop를 선택하고, Dead branch에서는 `DeathPresentationMode`로 Default/ExecutionLethal Dead Loop를 선택한다. Dead branch가 Execution Down보다 우선한다.
- committed primary pair와 Standard Recovery를 일반 Hit/Action이 밀어내지 않도록 intervention rule을 authoring한다. Death/EndPlay hard release는 이 규칙과 별개로 계속 작동한다.

---

## 12. 검증 기준

- Standard와 Lethal이 서로 다른 authored Source/Target primary pair를 resolve한다.
- `LethalCondition = Disabled`면 Standard pair만 선택되고, `HealthRatio`에서는 threshold 전후가 각각 Standard/Lethal pair를 고른다.
- 두 번째 Source는 이미 예약된 Collapse opportunity를 예약하지 못한다.
- Primary pair 시작 뒤 Commit 전에는 `ExecutionPrimaryActive`이며, 취소는 정확히 보관된 Collapse Loop TTL을 재개한다.
- Standard Commit은 `ExecutionPrimaryCommitted`로 전이하고 StandardExecutionDamage를 non-lethal로 적용하며, CollapseOut을 요청하거나 Target primary Reaction을 중단하지 않는다.
- Standard pair 완료 후 Execution Down에 진입하고 ExecutionRecovery가 Balance를 한 번만 reset한다.
- Lethal Commit은 Source impact frame에서 CurrentHP만큼의 Health damage를 정확히 한 번 적용해 Dead로 전이한다.
- Lethal Execution In은 자신의 예상 Dead 전이로 취소되지 않는다.
- ExecutionLethal mode에서는 두 번째 generic Dead In이 시작되지 않는다.
- Default Damage Death는 기존 generic Dead Reaction 및 Default Dead Loop를 계속 사용한다.
- stale Commit, Notify, terminal은 session/revision/lifecycle correlation 검증에서 거절된다.
- EndPlay와 external death는 runtime을 안전하게 정리하며 committed opportunity를 복원하거나 Lethal Target을 되살리지 않는다.
- Target Lock이 비활성인 Player 입력은 Execution 요청을 만들지 않는다. 활성 상태여도 실제 Player Actor Forward가 `MaxSourceFacingAngleDegrees` 밖이거나 `MaxStartDistance` 밖이면 reservation 없이 시작을 거절한다.
- 기하 조건을 통과한 Target은 Primary pair 시작 직전에 위치를 유지한 채 Player를 향하는 yaw만 한 번 정렬된다. 이후 startup이 거절되면 reservation이 해제되고 Collapse Loop TTL이 재개된다.
