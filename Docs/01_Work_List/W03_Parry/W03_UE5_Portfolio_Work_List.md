# UE5 Portfolio - Work List

## 제목

**W03: Guard / Parry Action v1 구현**

## 날짜

**2026.06.14**

## 상태

- [ ] **진행중**

## 브랜치

- `feature/parry-action`

---

## 1. Branch 목표

이번 Branch는 Player의 Guard / Parry Action을 기존 combat 실행 구조에 안전하게 연결하는 것을 목표로 한다.

핵심은 Parry를 완성된 방어 시스템으로 한 번에 구현하는 것이 아니라, 먼저 Guard Action 안에서 Parry Window와 Guard Hold 상태 전환을 안정화하고, `UCTakeDamageComponent` 안에서 임시 defensive resolution 규칙으로 Parry / Guard 분기를 검증하는 것이다.

이 분기 규칙이 안정되면 `TakeDamage` 안의 판단 책임을 이후 `UCCombatResolutionComponent`로 분리할 수 있도록 작은 함수 단위로 정리한다.

```yaml
핵심 목표
- 현재 Action / TakeDamage / Reaction / Feedback 호출 흐름 확인
- Guard Action 입력 / 실행과 Block_In / Block_Hold / Block_Out 흐름 연결
- Guard Action 초반 Parry Window와 이후 Guard Hold 상태 전환 구현
- Guard Hold 상태에서 incoming damage를 받으면 Block_Hit 흐름으로 분기
- Parry Window 중 incoming damage를 받으면 Block_Parry 흐름으로 분기
- Parry 성공 시 damage commit 이전 중단, parry reaction / feedback 연결
- 안정화 후 TakeDamage 안의 defensive resolution 규칙을 Combat Resolution으로 분리할 후보 기록
```

---

## 2. 완료 기준

- [ ] 현재 Action / TakeDamage / Reaction / Feedback 호출 흐름이 정리되어 있다.
- [ ] Player 입력에서 Guard Action 요청과 `CAction_Block` 또는 `CAction_Guard` 실행으로 이어진다.
- [ ] Pressed / Held / Released 입력에 따라 Block_In / Block_Hold / Block_Out 흐름이 연결되어 있다.
- [ ] Guard Action 초반 Parry Window와 이후 Guard Hold 상태 전환이 확인되어 있다.
- [ ] Guard Hold 상태에서 피격 시 Block_Hit 흐름으로 분기한다.
- [ ] Parry Window 중 피격 시 Block_Parry 흐름으로 분기하고 damage commit 이전에 기존 damage 처리를 중단할 수 있다.
- [ ] Parry 성공 시 damage 무효화, parry reaction, 기본 feedback 흐름이 검증되어 있다.
- [ ] Attacker 처리, Perfect Parry, resource 계열, 최종 polish는 후속 범위로 분리되어 있다.

---

## 3. 선행 확인 결과

### 현재 Action 실행 흐름

```text
Player 입력 또는 AI 판단
-> Action Request 생성
-> Action Orchestrator 진입
-> ActionData / ActionExecutor resolve
-> Action Component가 Start / Reserve / Intervene 적용
-> Action montage / notify / feedback / runtime cleanup 처리
```

- 현재 `ECombatActionIntent`에는 `ComboAttack`, `Guard`, `Dodge`가 있고 `Parry`는 없다.
- 현재 `EActionType`에는 `Equip`, `Unequip`, `ComboAttack`, `Dodge`가 있고 `Parry`는 없다.
- `UCAnimNotifyState_ExecutionInterventionWindow`는 이미 `WindowKey` 기반으로 Action / Reaction 양쪽의 intervention window를 열고 닫을 수 있다.
- staged asset 기준으로 Block / Parry 흐름은 `Block_In -> Block_Hold -> Block_Out`을 기본으로 두고, Hold 중 피격은 `Block_Hit`, Parry Window 중 피격은 `Block_Parry`로 분기하는 구성이 자연스럽다.

### 현재 TakeDamage 흐름

```text
target TakeDamage()
-> UCTakeDamageComponent::RequestTakeDamage()
-> ValidateRequest / BuildPayload / BuildContext
-> ValidateContext / CanTakeDamage
-> ComputeTakeDamage
-> CommitTakeDamage
-> ReactionOrchestrator에 damage reaction 요청
-> DamageFeedback 실행
```

- 현재 `UCTakeDamageComponent`는 damage commit 이후에 Reaction / DamageFeedback을 dispatch한다.
- Parry가 damage 자체를 무효화해야 한다면 Reaction 단계보다 앞에서 처리해야 한다.
- Parry는 reject reason이 아니라 `DefenseOutcome::Parry`와 `bShouldCommitDamage=false`로 표현하고, `Blocked` 계열은 Block_Hit / Guard 전용 reaction 정리 시 다시 검토한다.

### 현재 Reaction / Feedback 흐름

```text
TakeDamagePacket
-> Reaction Orchestrator
-> Hit / Dead reaction candidate 결정
-> active action / reaction과 intervention 판단
-> Reaction Component가 reaction 실행
-> Reaction notify를 통해 ReactionFeedback 실행
```

- Reaction은 committed damage 이후의 결과를 기준으로 실행된다.
- Parry 성공으로 damage commit을 막으면 기존 Hit / Dead reaction 흐름은 자동으로 실행되지 않는다.
- Parry 성공 reaction / feedback은 `TakeDamage` 내부 defensive resolution 결과 또는 별도 요청 경계에서 명시적으로 연결해야 한다.

---

## 4. 작업 범위

### 4.1 Guard Action 진입 / 종료 흐름 구성

- [x] `ECombatActionIntent::Guard` 입력 흐름을 Pressed / Released 기준으로 확인한다.
  - 세부 구현 요소:
    - [x] `CPlayerController`에 `Guard` 입력 binding을 추가한다.
    - [x] `E` Pressed는 Guard 시작 요청으로, `E` Released는 Guard 종료 요청으로 연결한다.
    - [x] `CPlayer`의 combat action 요청 경로가 `Started` / `Completed` intent event를 구분할 수 있는지 확인한다.
    - [x] 현재 `HandleCombatAction()`이 `Started`로 고정되어 있으므로, Guard 종료 요청을 처리할 수 있는 확장 지점을 정한다.
- [x] Guard Pressed에서 `Block_In` 실행으로 이어지게 구성한다.
  - 세부 구현 요소:
    - [x] `EActionType::Guard`를 추가한다.
    - [x] `CActionOrchestratorComponent::ResolveCombatActionCandidate()`에서 `ECombatActionIntent::Guard`를 `EActionType::Guard`로 해석한다.
    - [x] `RequestCombatAction()`에서 Guard input press / release side effect를 먼저 반영해 `bWantsGuarding`을 action 실행 lifecycle과 분리한다.
    - [x] Guard 시작 요청은 Guard phase `In` 기준으로 처리한다.
    - [x] 임시로 `IntentEvent`에 따라 Guard phase를 나눠 `Block_In / Block_Out` ActionData를 선택한다.
      - [x] `Started -> Guard phase In -> Block_In`
      - [x] `Completed -> Guard phase Out -> Block_Out`
      - [x] Guard 전용 phase adapter를 추가해 숫자 `ActionIndex` 비교가 실행 판단에 직접 남지 않도록 정리한다.
- [x] Guard Held 상태에서 `Block_Hold` 또는 ABP guard hold / locomotion 상태로 유지되게 구성한다.
  - 세부 구현 요소:
    - [x] v1에서는 Guard Hold를 ABP 상태로 넘기는 방향을 기본으로 둔다.
    - [x] `CAction_Guard`는 Guard In / Guard Out 전환 action만 처리하고, Guard Hold는 `UCDefenseComponent`의 observable overlay 상태로 관리한다.
    - [x] `Block_In` 종료 이후 Guard Hold pose가 유지될 수 있도록 `UCDefenseComponent::IsGuardingPose()`와 AnimInstance `bIsGuardingPose`를 연결한다.
    - [x] Guard 상태 전달은 `CAction_Guard -> UCActionComponent -> UCObservableOverlayComponent -> UCDefenseComponent` 경로의 observable overlay event로 라우팅한다.
    - [ ] `Block_Hold` montage는 ABP 전환 전 검증용 또는 임시 fallback으로 사용할 수 있는지 확인한다.
- [x] Guard Released에서 `Block_Out` 실행으로 이어지게 구성한다.
  - 세부 구현 요소:
    - [x] Guard 종료 요청은 별도 action request로 들어와 `Block_Out` ActionData를 선택한다.
    - [x] Guard release 입력이 들어오면 `Block_Out` 실행 여부와 별개로 `bWantsGuarding=false`를 즉시 반영한다.
    - [x] `Block_Out`은 key release 기반 action 종료 흐름으로 실행한다.
    - [x] `Block_Out` 시작 시 `CanGuard`와 `CanParry`를 false로 내리고, Guard pose 상태도 함께 종료한다.
- [x] `Block_In` 실행 중 release 입력이 들어오면 Guard Out candidate를 deferred로 보관한다.
  - 세부 구현 요소:
    - [x] `Guard Completed` request를 먼저 Guard Out candidate로 해석한다.
    - [x] active Guard phase `In` 상태라면 `AfterGuardInAction` key로 deferred candidate를 저장한다.
    - [x] `Block_In` complete 이후 deferred Guard Out candidate를 공통 `ProcessActionCandidate()` 경로로 재처리한다.
    - [x] deferred candidate 소비 시 active context 정리 이후 재평가되도록 `CAction_Guard::Complete()` 순서를 조정한다.
    - [x] `Reserved`와 별개로 `Deferred` result type을 분리한다.
    - [x] deferred candidate 정리를 위해 전체 제거 / consume key 기준 제거 / consume key + action key 기준 제거 API를 구성한다.
    - [ ] `Block_In` 중 release 시 Hold에 고정되지 않고 `Block_Out`으로 이어지는지 PIE에서 확인한다.
- [ ] 정상 release뿐 아니라 interrupt / dead / dodge / action stop 상황에서도 방어 runtime 상태가 정리되게 한다.
  - 세부 구현 요소:
    - [x] `CAction_Guard::Interrupt()`에서 intervention 문맥 기반으로 Guard runtime 정리와 Guard Out -> Guard In 재진입 예외를 처리한다.
    - [x] action / reaction start 직전에 observable overlay snapshot을 기준으로 Guard overlay를 정리할 수 있게 한다.
    - [x] `ExecutionState`를 확장하지 않고 Guard Hold를 observable overlay policy registry로 관측하는 v1 경계를 구성한다.
    - [x] overlay owner policy는 snapshot 구성 시점에 자기 runtime state를 `FExecutionSnapshot`의 observable overlay snapshot에 기록하도록 구성한다.
    - [x] incoming executor는 observable overlay snapshot을 읽고 실행 가능 여부와 필요한 overlay handling을 함께 결정하도록 구성한다.
    - [x] overlay handling은 단일 값이 아니라 `TArray<EObservableOverlayHandling>`로 누적한다.
    - [x] `WantObservableOverlayRequirement()` / `AllowObservableOverlayRequirement()` 구조를 제거하고 `ResolveObservableOverlayExecutionCondition()` 기준으로 단순화한다.
    - [x] requested overlay handling은 실행 직전 overlay owner policy의 `CanApply / Apply` 단계를 거쳐 적용되도록 구성한다.
    - [x] Guard 정리를 정상 종료 / 간섭 종료 / overlay 정리 / overlay 복구로 나누고, 각각 `HandleGuardLifecycleCompleted`, `HandleGuardLifecycleInterrupted`, `ClearGuardOverlay`, `RestoreGuardOverlay`로 분리한다.
    - [x] Guard Out은 Guard overlay가 남아 있을 때만 의미 있는 incoming action으로 보고, overlay가 없으면 ignore되도록 구성한다.
    - [x] Guard Out의 overlay 정리는 pre-start handling이 아니라 Guard Out action lifecycle에서 처리하도록 둔다.
    - [x] Dodge / Hit / Dead처럼 Guard lifecycle을 끝내야 하는 incoming execution은 실행 전에 `ClearGuardState` handling을 요청하도록 구성한다.
    - [x] Guard Out 시작 시 `CanStartGuard`를 잠그고, `AllowGuardStart` 단발 notify 시점부터 Guard In 재진입을 허용하도록 구성한다.
    - [x] `AllowGuardStart` notify 이후 Guard In이 들어오면 Guard Out을 intervention으로 끊고 Guard In이 상태를 덮어쓰도록 구성한다.
    - [ ] dodge / reaction takeover에서 Guard 상태가 남지 않는지 PIE에서 확인한다.
    - [x] interrupt / forced stop 시 Guard Out deferred candidate를 barrier key 기준으로 정리하도록 호출 지점을 연결한다.

- [x] Action lifecycle에서 `Interrupt`와 `Stop`의 역할을 분리한다.
  - 내부 구현 요소:
    - [x] `Interrupt`는 orchestration intervention directive를 거친 incoming / active 간섭 종료 진입점으로 고정한다.
    - [x] `Stop`은 directive 없이 호출되는 hard stop / fallback 진입점으로 둔다.
    - [x] `Interrupt`가 `Stop`을 감싸지 않도록 하고, 두 진입점은 공통 `HandleActionStop()` 종료 파이프라인만 공유한다.
    - [x] Guard Out -> Guard In 재진입 예외는 `Interrupt`에서만 incoming Guard In 여부를 확인해 처리한다.
    - [x] Reaction lifecycle도 동일하게 `Interrupt` / `Stop` 진입점과 공통 `HandleReactionStop()` 종료 파이프라인으로 정리한다.
    - [x] 현재 호출 경로가 없는 `CAction_Guard::Stop()` override는 제거한다.

### 4.2 Guard runtime 상태와 Parry Window 구성

- [x] Guard Action executor는 `CAction_Block` 또는 `CAction_Guard` 중 하나로 확정한다.
  - 세부 구현 요소:
    - [x] Guard가 상위 action이고 Parry는 Guard 내부 window 결과이므로, action executor 이름은 `CAction_Guard`를 우선 후보로 둔다.
    - [x] 기존 action executor 패턴(`CAction_ComboAttack`, `CAction_Dodge`)을 따라 Guard 전용 executor skeleton을 만든다.
    - [x] `CAction_Guard` decision은 Guard In / Guard Out phase만 처리하도록 제한한다.
- [x] Guard / Parry runtime 상태 조회 경계를 구성한다.
  - 세부 구현 요소:
    - [x] Guard 입력 의도는 v1에서 `UCDefenseComponent::WantsGuarding()`으로 노출한다.
    - [x] Guard Hold pose 상태는 v1에서 `UCDefenseComponent::IsGuardingPose()`로 노출한다.
    - [x] 실제 Guard 판정 상태는 v1에서 `UCDefenseComponent::CanGuard()`로 노출한다.
    - [x] 실제 Parry 판정 상태는 v1에서 `UCDefenseComponent::CanParry()`로 노출한다.
    - [x] `FExecutionSnapshot`에는 `FObservableOverlaySnapshot`을 두고, `UCDefenseComponent`가 Guard overlay snapshot을 채우게 한다.
    - [x] Guard overlay snapshot에는 `bWantsGuarding`, `bIsGuardingPose`, `bCanGuard`, `bCanParry`, `bCanStartGuard`를 포함한다.
    - [x] `UCActionComponent`는 Guard 전용 Defense 라우터가 아니라 observable overlay event 전달 지점으로 축소한다.
    - [x] `UCActionComponent`와 `UCReactionComponent`는 Orchestrator result에 누적된 overlay handling을 실행 시작 전에 적용한다.
    - [ ] 이후 Combat Resolution 분리 시 해당 상태 조회 경계를 그대로 옮길 수 있게 만든다.
- [x] Block_In 시작 직후 Parry Window를 열 수 있게 한다.
  - 세부 구현 요소:
    - [x] `Block_In` 시작 시점에 `bCanParry`를 true로 연다.
    - [x] Parry Window는 Guard Hold 전체가 아니라 Guard In 초반 구간에만 유효하게 둘 계획이다.
- [x] `AN_SwitchToGuard` 또는 동등한 notify 시점에서 Parry Window를 닫고 Guard Hold 상태로 전환한다.
  - 세부 구현 요소:
    - [x] 단발 notify로 Parry 가능 상태를 닫고 Guard 판정 상태를 연다.
    - [x] `UCAnimNotify_SwitchToGuard`를 추가하고 notify 이름은 `SwitchToGuard`로 정한다.
    - [x] `SwitchToGuard` notify는 Guard phase `In` 기준으로 `Block_In`에서만 처리되도록 구성한다.
    - [x] release가 `SwitchToGuard` 전에 들어온 경우에는 Parry Window만 닫고 Guard 판정은 열지 않는다.
    - [x] release가 `SwitchToGuard` 이후 들어온 경우에는 release 전까지 Guard 판정을 인정한다.
- [ ] Guard / Parry 판정은 notify state window가 아니라 단발 notify 기반 상태 전환으로 구성한다.
  - 세부 구현 요소:
    - [x] montage 경계 사이의 판정 공백을 피하기 위해 Parry / Guard 상태를 duration window가 아니라 상태값으로 유지한다.
    - [x] `SwitchToGuard` 단발 notify에서 Parry를 닫고 Guard 판정을 켠다.
    - [x] `Block_Out` 시작 시점에 Guard pose를 종료해 Out montage와 ABP Guard pose가 겹치지 않게 한다.
    - [ ] `GuardPoseEnd` 단발 notify는 Out 시작 정리만으로 부족한 경우의 fallback 후보로 검토한다.

### 4.3 Guard Hold 중 피격 처리 연결

- [x] Guard Hold 상태에서 incoming damage가 들어오면 일반 Hit가 아니라 Guard Hit 흐름으로 분기한다.
  - 세부 구현 요소:
    - [x] `UCTakeDamageComponent`에서 damage commit 이전에 `UCDefenseComponent` 상태를 조회한다.
    - [x] Guard Hold 중이면 `DefenseOutcome::Guard`를 남기고 `BlockHit` reaction 후보로 분기한다.
    - [x] Guard-In의 `SwitchToGuard` 이후 구간도 `CanGuard()` 기준으로 `BlockHit` 후보에 포함한다.
- [x] Guard Hit에서는 damage를 완전 무효화하지 않고, v1 기준 damage 감소 정책으로 처리한다.
  - 세부 구현 요소:
    - [x] v1 1차 기준은 damage 50% 감소로 둔다.
    - [ ] Guard Gauge / stamina / posture 감소는 후속 후보로 남긴다.
    - [x] 이번 단계의 핵심 검증은 Guard packet이 가로채지고 `BlockHit` reaction 후보가 생성되는지로 둔다.
- [x] Guard Hit 결과로 `BlockHit` reaction을 실행할 수 있는 요청 경계를 정한다.
  - 세부 구현 요소:
    - [x] `BlockHit`은 Guard Action 내부 실행이 아니라 damage packet 결과로 발생하는 defender reaction으로 둔다.
    - [x] `EReactionType::BlockHit`과 `CReaction_BlockHit`을 추가한다.
    - [x] Guard-In guard 구간에서 `BlockHit`이 들어오면 active Guard-In action을 intervention으로 중단한다.
    - [x] BlockHit 중에는 Guard overlay를 유지한다.
    - [x] BlockHit 중 release 입력은 `AfterGuardBlockReaction` key로 Guard Out candidate를 deferred 저장한다.
    - [x] BlockHit complete 이후 `AfterGuardBlockReaction`과 `AfterGuardInAction` deferred Guard Out candidate를 소비한다.
    - [x] 현재 `Hit / Dead` reaction은 각각의 reaction executor에서 Guard overlay를 clear하는 정책으로 정리한다.
    - [x] reaction base는 공통 clear 정책을 갖지 않고, 세부 reaction executor가 자기 overlay execution condition을 판단하도록 정리한다.
    - [x] `BlockHit / Parry`에 대응하는 ReactionData와 montage asset 연결은 Editor에서 진행한다.
- [x] Guard Hold 중 피격 처리와 기존 Hit / Dead reaction 흐름이 충돌하지 않는지 PIE에서 확인한다.
  - 세부 구현 요소:
    - [x] Guard Hold 중 피격 시 `BlockHit` reaction이 실행되는지 확인한다.
    - [x] Guard-In에서 `SwitchToGuard` 이후 피격 시 `BlockHit` reaction으로 분기되는지 확인한다.
    - [x] Parry Window 중 피격 시 `Parry` reaction이 실행되는지 확인한다.

### 4.4 TakeDamage 내부 defensive resolution 임시 구현

- [ ] `UCTakeDamageComponent` 내부에 Parry / Guard 판정을 위한 작은 private 함수 단위를 만든다.
  - 세부 구현 요소:
    - [ ] `ValidateContext` 이후, `ComputeTakeDamage` / `CommitTakeDamage` 이전을 우선 삽입 후보로 둔다.
    - [ ] `CanTakeDamage()` 기존 정책과 섞기보다, 별도 defensive resolution 함수로 분리해 둔다.
    - [ ] 함수 결과는 Parry / Guard / None을 구분할 수 있는 최소 enum 또는 result 구조로 둔다.
- [ ] Parry Window 중 incoming damage가 들어오면 Parry 성공 분기로 라우팅한다.
  - 세부 구현 요소:
    - [ ] active Guard Action의 Parry Window 상태를 확인한다.
    - [ ] Parry 성공이면 기존 damage commit으로 진행하지 않는다.
- [ ] Guard Hold 중 incoming damage가 들어오면 Guard Hit 분기로 라우팅한다.
  - 세부 구현 요소:
    - [ ] active Guard Action이 있고 Parry Window가 닫힌 상태면 Guard Hit 후보로 처리한다.
    - [ ] Guard Hit damage 감소 정책은 v1에서 최소값으로 둔다.
- [ ] Parry / Guard에 해당하지 않으면 기존 TakeDamage 흐름을 유지한다.
  - 세부 구현 요소:
    - [ ] 기존 `CanTakeDamage()`, `ComputeTakeDamage()`, `CommitTakeDamage()`, `DispatchTakeDamageCommitted()` 순서를 유지한다.
    - [ ] 일반 피격의 Reaction / DamageFeedback 결과가 회귀하지 않는지 확인한다.
- [ ] Parry 성공 시 damage commit 이전에 기존 damage 처리를 중단할 수 있는지 확인한다.
- [ ] 임시 함수들은 이후 `UCCombatResolutionComponent`로 옮기기 쉬운 형태로 둔다.

### 4.5 Parry 성공 처리 연결

- [ ] Parry 성공 시 damage를 0으로 처리하거나 damage commit 자체를 차단하는 방식을 확정한다.
  - 세부 구현 요소:
    - [ ] 권장 기본값은 damage commit 자체를 차단하는 방식으로 둔다.
    - [ ] return damage 값은 `0.f`로 반환하되, 기존 damage reject와 구분 가능한 reason 후보를 남긴다.
- [ ] Parry 성공 시 기존 Hit reaction이 실행되지 않게 한다.
  - 세부 구현 요소:
    - [ ] `DispatchTakeDamageCommitted()`가 호출되지 않도록 하거나, Parry 전용 dispatch 경계를 따로 둔다.
    - [ ] 기존 Hit / Dead reaction이 Parry 성공과 동시에 실행되지 않는지 확인한다.
- [ ] Parry 성공 시 `Block_Parry` montage 또는 parry reaction을 실행할 수 있는 요청 경계를 정한다.
  - 세부 구현 요소:
    - [ ] `Block_Parry`는 Guard Action executor가 직접 처리할지, reaction 요청으로 처리할지 비교한다.
    - [ ] v1에서는 defender 쪽 성공 animation 확인을 우선한다.
- [ ] Parry 성공 feedback을 기존 DamageFeedback / ReactionFeedback과 충돌하지 않게 연결한다.
  - 세부 구현 요소:
    - [ ] Parry success feedback은 기존 damage feedback과 별도 요청으로 둘지 확인한다.
    - [ ] attacker reaction / counter feedback은 후속 범위로 남긴다.
- [ ] Parry 실패 또는 Parry Window 밖 damage는 기존 TakeDamage fallback으로 이어지게 유지한다.

### 4.6 Combat Resolution 분리 후보 기록

- [ ] `UCTakeDamageComponent` 내부에 임시로 둔 Parry / Guard 판정 함수를 Combat Resolution 후보로 기록한다.
  - 세부 구현 요소:
    - [ ] 이번 Branch에서는 `UCCombatResolutionComponent`를 완성하지 않는다.
    - [ ] Parry / Guard 판정 함수의 입력값과 출력값을 후속 component API 후보로 기록한다.
- [ ] `UCTakeDamageComponent::CanTakeDamage()`의 기존 정책 중 Combat Resolution으로 옮길 후보를 기록한다.
  - 세부 구현 요소:
    - [ ] dead / invulnerable / iframe / guard / parry처럼 “damage를 받을 수 있는가”를 결정하는 정책을 분류한다.
    - [ ] health commit이나 damage feedback처럼 결과 적용 책임은 Combat Resolution 후보에서 제외한다.
- [ ] Parry / Guard / Block / iframe / invulnerable 같은 수신자 측 방어 판단의 책임 위치를 검토한다.
- [ ] deferred action candidate의 장기 확장 정책을 후속 후보로 기록한다.
  - 세부 구현 요소:
    - [ ] `SourceExecutionId` 기반 lifecycle 정리 필요 여부를 검토한다.
    - [ ] filter struct 기반 deferred clear API가 필요한 시점을 검토한다.
    - [ ] retry / timeout / expire 정책이 필요한 deferred 유형을 분류한다.
- [ ] observable overlay policy registry의 장기 소유 위치를 후속 후보로 기록한다.
  - 세부 구현 요소:
    - [x] 기존 v1에서는 Orchestrator가 snapshot 구성을 위해 policy를 보관하고, Action / Reaction Component가 handling 적용을 위해 같은 policy를 보관했다.
    - [x] `UCObservableOverlayComponent`를 추가해 policy 등록 / snapshot 구성 / handling 적용 위임을 하나의 component로 모았다.
    - [x] Action / Reaction Orchestrator는 `UCObservableOverlayComponent`를 통해 observable overlay snapshot을 구성한다.
    - [x] Action / Reaction Component는 `UCObservableOverlayComponent`를 통해 requested overlay handling을 적용한다.
    - [x] `UCDefenseComponent`는 Guard overlay state owner이자 `IObservableOverlayPolicy` 구현체로 유지한다.
    - [x] Guard input / lifecycle / notify event는 `ActionComponent -> DefenseComponent` 직통 호출이 아니라 `UCObservableOverlayComponent`의 event routing으로 전달한다.
- [ ] 이번 Branch에서 실제 이관할 항목과 후속 Branch로 넘길 항목을 분리한다.

#### 현재 코드 기준 Combat pipeline 분리 작업 후보

- [x] `ApplyDamageComponent`와 `TakeDamageComponent`의 현재 책임을 `N05` 기준으로 분석한다.
  - 현재 `ApplyDamageComponent`는 source-side damage 적용자가 아니라 hit context를 combat request로 구성하고 target에게 전달하는 requester 성격이 강하다.
  - 현재 `TakeDamageComponent`는 receiver adapter, resolution, damage apply, consequence coordination을 모두 가진 압축형 컴포넌트다.
- [ ] `TakeDamageComponent`를 우선 `Receive / Resolve / Apply / Coordinate` 단계로 함수 경계를 정리한다.
  - `Receive`: `RequestTakeDamage`, `ProcessTakeDamage`, `HandleDefaultDamageEvent`, `BuildPayload`, `BuildContext`
  - `Resolve`: `ValidateContext`, `CanTakeDamage`, `ComputeMitigatedDamage`, `ComputeFinalTakenDamage`, defensive outcome 결정
  - `Apply`: `CommitTakeDamage`, `CommitDamageToHealth`, health / resource 상태 변경
  - `Coordinate`: defender reaction, feedback, attacker result packet, rejected result 처리
- [ ] `CombatResolutionResult` 후보 구조를 정한다.
  - `FTakeDamageResult`를 즉시 폐기하지 않고, `DefenseOutcome`, damage commit 여부, final damage, reaction 후보, feedback 후보, external result 후보의 소유 위치를 정한다.
- [ ] `CombatConsequenceCoordinator` 후보 경계를 정한다.
  - 기존 `DispatchCombatResultToDefender`, `DispatchCombatResultToAttacker`, `DispatchRejectedCombatResult`를 후속 coordinator 후보로 본다.
  - coordinator는 montage / VFX / HP commit을 직접 실행하지 않고 각 domain request를 구성해 전달한다.
- [ ] `ApplyDamageComponent` 리네임 / 축소는 후속 작업으로 둔다.
  - `ApplyDamageComponent`는 장기적으로 `CombatRequester` 또는 `CombatRequestSource` 후보지만, 현재 branch에서는 `TakeDamageComponent` 책임 경계 정리가 우선이다.
  - hit window tracking, duplicate target check, spec resolve, target dispatch가 source-side request pipeline으로 유지될 수 있는지 확인한다.

### 4.7 판정 흐름 안정화 후 필수 리팩터링 후보

- [ ] Guard phase key 구조를 보완한다.
  - 현재 v1은 `EGuardActionPhase` adapter를 통해 숫자 `ActionIndex` 직접 비교를 제거했다.
  - 다만 ActionData 조회 key 자체는 여전히 `ActionIndex`에 의존한다.
  - Parry / Guard 판정 흐름을 닫은 뒤에는 데이터에 Guard phase / variant를 직접 표현할 수 있는 key 구조를 검토한다.
- [ ] Guard overlay cleanup ownership을 serial / lifecycle id 기준으로 보완한다.
  - 현재 v1은 `GuardOut -> GuardIn` 재진입에서 old GuardOut cleanup이 new GuardIn state를 지우지 않도록 케이스 기반 예외를 둔다.
  - 판정 흐름 안정화 후에는 Guard overlay generation / lifecycle id를 도입해, cleanup 요청이 아직 같은 세대의 Guard state를 대상으로 하는지 검증한다.
  - 목표는 케이스별 예외가 아니라 “내가 만든 상태만 정리한다”는 ownership 기준으로 cleanup을 제어하는 것이다.

### 4.8 TakeDamage 방어 분기 v1 구현 결과

- [x] `UCTakeDamageComponent`에서 `UCDefenseComponent` 상태를 읽어 damage commit 이전에 Parry / Guard 분기를 판단한다.
- [x] `CanParry()`가 true이면 `DefenseOutcome::Parry`와 `bShouldCommitDamage=false`로 damage commit을 막는다.
  - v1에서는 기존 Hit / Dead reaction과 DamageFeedback이 실행되지 않는지 확인하는 것을 우선한다.
  - `Block_Parry` reaction / feedback 연결은 후속 작업으로 남긴다.
- [x] `CanGuard()`가 true이면 damage mitigation 단계에서 incoming damage를 임시로 50% 감소시킨다.
  - v1에서는 Guard packet이 가로채지는지 확인하는 목적이며, Guard Gauge / stamina / Block_Hit 전용 reaction은 후속 작업으로 남긴다.
- [x] Parry / Guard에 해당하지 않는 경우 기존 `CanTakeDamage() -> ComputeTakeDamage() -> CommitTakeDamage() -> DispatchTakeDamageCommitted()` 흐름을 유지한다.

### 4.9 Defensive Outcome / Reaction 분기 구조

- [x] `EDamageDefenseOutcome`을 추가해 TakeDamage 결과가 `None / Guard / Parry` 중 어떤 방어 판정인지 남기도록 구성한다.
- [x] `FTakeDamageContext`와 `FTakeDamageResult`에 `DefenseOutcome`을 전달한다.
- [x] Parry는 `bAccepted=true`, `bShouldCommitDamage=false`, `DefenseOutcome::Parry`로 남겨 reaction candidate를 만들 수 있게 한다.
- [x] Guard는 `DefenseOutcome::Guard`를 남기고, v1 기준 damage 50% 감소를 유지한다.
- [x] `EReactionType::BlockHit`, `EReactionType::Parry`를 추가하고 `ResolveDamageReactionType()`에서 defensive outcome 기준으로 분기한다.
  - `Parry -> Parry`
  - `Dead transition -> Dead`
  - `Guard -> BlockHit`
  - `CommittedDamage > 0 -> Hit`
- [x] `BlockHit / Parry`에 대응하는 전용 reaction executor를 추가한다.
  - `CReaction_BlockHit`
  - `CReaction_Parry`
- [x] BlockHit 중 release 입력은 `AfterGuardBlockReaction` key로 Guard Out candidate를 지연 실행한다.
  - BlockHit 중에는 Guard overlay를 유지한다.
  - BlockHit complete 이후 deferred Guard Out candidate를 소비한다.
- [x] `BlockHit / Parry`에 대응하는 ReactionData와 montage asset 연결은 Editor에서 진행한다.

### 4.10 Guard Locomotion / Asset 연결 결과

- [x] Guard Hold에서 사용할 `BS_Guard`를 추가하고 ABP Guard pose / locomotion 흐름에 연결한다.
- [x] Guard 상태에서는 Guard 전용 movement override를 적용한다.
  - Guard In / Hold / BlockHit 유지 중에는 Guard 8-way locomotion 기준을 사용한다.
  - Guard Out / Hit / Parry / interrupt로 Guard 상태가 종료되면 기본 movement mode로 복구한다.
- [x] Guard / BlockHit / Parry 관련 ActionData / ReactionData의 montage 참조를 Editor에서 연결한다.
- [x] Guard / Block 관련 animation asset naming을 현재 프로젝트 규칙에 맞춰 정리한다.
- [x] PIE에서 Guard In / Hold / Out, BlockHit, Parry reaction, Guard locomotion 전환을 확인한다.

---

### 4.11 Parry Result Packet / Receiver Boundary

- [x] Parry 성공 결과를 attacker 측에 되돌려줄 `FCombatResultPacket` 경계를 추가한다.
  - `DefenseOutcome`, source / target / instigator / damage causer, impact info, committed damage 정보를 포함한다.
  - v1에서는 `DefenseOutcome::Parry` 결과만 attacker result dispatch 대상으로 둔다.
- [x] `ICombatResultReceiver` interface를 추가해 attacker 측 result 수신 경계를 만든다.
  - `ReceiveCombatResultPacket()`은 아직 attacker reaction을 실행하지 않고 수신 로그만 남긴다.
  - `ACEnemy`와 `ACPlayer`가 v1 receiver를 구현해 양방향 검증이 가능하게 한다.
- [x] `UCTakeDamageComponent`에서 defender 처리와 attacker result dispatch를 분리한다.
  - defender 쪽은 기존 reaction / feedback 흐름을 유지한다.
  - attacker 쪽은 `FCombatResultPacket`을 구성해 result receiver actor에게 전달한다.
- [x] PIE에서 Parry 성공 시 `[CombatResultDispatch] Delivering / Delivered`, `[CombatResult] Received / Packet` 로그가 출력되는지 확인한다.
  - result dispatch 흐름과 packet 내부 내용이 분리되어 출력된다.
  - `Receiver=BP_CEnemy`, `Requester=BP_CPlayer`, `Source=BP_CEnemy`, `DamageCauser=BP_CWeaponActor_Sword` 기준으로 결과 전달을 확인했다.
  - attacker reaction / stagger 연결은 후속 작업으로 남긴다.

---

## 5. 비범위

- Attacker 측 Parry 성공 signal 송신 / 수신
- Blink / Repulse cue packet 전달 구조
- Attacker 측 stagger / reaction / counter 처리
- Perfect Parry / Normal Parry 구분
- Resource / Stamina / Posture / Guard Gauge 처리
- Network Replication
- 최종 VFX / SFX polish
- `UCCombatResolutionComponent` 최종 분리 완료
- S28 Policy / Gate 전체 리팩터링
- System Architecture 본문 재작성

---

## 6. 검증 기준

### Build

- [x] 신규 Guard Action executor compile 확인
- [x] Guard enum / ActionData key / WindowKey 변경 compile 확인
- [x] TakeDamage defensive resolution 임시 함수 compile 확인

### Code Flow

- [x] Player input Pressed -> Guard Action Request -> Action Orchestrator 흐름 확인
- [x] Guard Action Request -> Guard Action executor 실행 흐름 확인
- [x] Guard Action -> Block_In -> Parry Window open 흐름 확인
- [x] SwitchToGuard notify -> Parry Window close -> Guard Hold 상태 전환 확인
- [x] Guard Released -> Block_Out 실행 흐름 확인
- [x] incoming damage -> TakeDamage defensive resolution -> Parry 성공 분기 확인
- [x] incoming damage -> TakeDamage defensive resolution -> Guard Hit 분기 확인
- [x] Parry 성공 분기에서 기존 damage commit이 실행되지 않는지 확인
- [x] 일반 피격에서 기존 TakeDamage / Reaction / DamageFeedback 흐름이 유지되는지 확인

### PIE

- [x] Guard 입력 시 Block_In montage가 재생되는지 확인
- [x] Guard 유지 시 Block_Hold 또는 Guard Hold 상태가 유지되는지 확인
- [x] Guard release 시 Block_Out montage가 재생되는지 확인
- [x] Parry Window 중 피격 시 damage가 무효화되는지 확인
- [x] Parry Window 밖 피격 시 기존 피격 처리가 유지되는지 확인
- [x] Guard Hold 중 피격 시 Block_Hit 또는 guard hit reaction이 실행되는지 확인
- [x] Parry 성공 시 Block_Parry 또는 parry reaction / feedback이 실행되는지 확인
- [x] Guard Hold 중 `BS_Guard` 8-way locomotion이 적용되는지 확인
- [x] Guard 종료 / interruption 이후 기본 movement mode로 복구되는지 확인

### Editor / Asset

- [x] Block_In / Block_Hold / Block_Out / Block_Hit / Block_Parry asset 존재 여부 확인
- [x] `FActionData.Montage`에 Guard / Block montage가 연결되어 있는지 확인
- [x] `FReactionData.Montage`에 BlockHit / Parry montage가 연결되어 있는지 확인
- [x] `UCAnimNotifyState_ExecutionInterventionWindow` 또는 전환 notify가 montage에 적용되어 있는지 확인
- [x] `WindowKey = Parry` 설정이 적용되어 있는지 확인
- [x] Guard / Block 관련 animation asset naming을 현재 규칙에 맞춰 정리했는지 확인

---

## 7. 문서화 기준

- [ ] 구현 완료 후 `P20_UE5_Portfolio_Pull_Request.md`를 작성한다.
- [ ] 구현 중 구조 충돌이 발생하면 System Design Record 또는 note 보완 필요 여부를 판단한다.
- [ ] 검증 과정에서 Editor / Asset 확인이 불완전하면 PR 문서의 미검증 항목에 남긴다.
- [ ] TakeDamage 내부 defensive resolution 규칙이 Guard / Counter 확장 기준으로 의미가 생기면 System Architecture 후속 보완 후보로 기록한다.
- [x] Attacker 측 counter signal / Blink / Repulse / cue packet 전달 구조는 `Docs/06_notes/N04_Blink_Repulse_Combat_Packet_Design_Note.md`에 후속 판단 기록으로 분리한다.
- [x] Combat intent / request / resolution / routing 계층 구조는 `Docs/06_notes/N05_Combat_Intent_Request_Resolution_Routing_Design_Note.md`에 후속 판단 기록으로 분리한다.

---

## 8. 정리

W03은 Guard / Parry Action v1을 기존 combat 실행 구조에 안전하게 연결하는 작업이다.

이번 Branch는 Combat Resolution을 처음부터 완성된 component로 분리하기보다, Guard Action 안에서 Parry Window와 Guard Hold 상태 전환을 먼저 안정화하고 `TakeDamage` 내부에서 Parry / Guard 분기를 임시로 검증하는 데 집중한다.

이 규칙이 안정되면 Parry / Guard 판정 책임을 `UCCombatResolutionComponent`로 분리하고, Counter / Perfect Parry / resource 계열 확장은 후속 Branch에서 이어간다.

---
