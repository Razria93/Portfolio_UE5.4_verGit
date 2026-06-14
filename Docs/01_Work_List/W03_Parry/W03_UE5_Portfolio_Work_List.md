# UE5 Portfolio - Work List

## 제목

**W03: Guard / Parry Action v1 구현**

## 날짜

**2026.06.14**

## 상태

- [ ] **진행중**

---

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
- `ETakeDamageRejectReason`의 `Blocked`, `Parried`는 아직 주석 후보로만 남아 있다.

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
- [ ] Guard Pressed에서 `Block_In` 실행으로 이어지게 구성한다.
  - 세부 구현 요소:
    - [x] `EActionType::Guard`를 추가한다.
    - [x] `CActionOrchestratorComponent::ResolveCombatActionCandidate()`에서 `ECombatActionIntent::Guard`를 `EActionType::Guard`로 해석한다.
    - [x] Guard 시작 요청은 action index `1`을 기준으로 처리한다.
    - [ ] 임시로 `IntentEvent`에 따라 Guard `ActionIndex`를 나눠 `Block_In / Block_Out` ActionData를 선택한다.
      - [ ] `Started -> Guard index 1 -> Block_In`
      - [ ] `Completed -> Guard index 2 -> Block_Out`
      - [ ] Guard 전용 phase / variant key가 필요하면 후속 구조 보완 후보로 기록한다.
- [ ] Guard Held 상태에서 `Block_Hold` 또는 ABP guard hold / locomotion 상태로 유지되게 구성한다.
  - 세부 구현 요소:
    - [ ] v1에서는 Guard Hold를 ABP 상태로 넘기는 방향을 기본으로 둔다.
    - [ ] `Block_In` 종료 이후 Guard Hold 상태가 유지될 수 있도록 action runtime 상태와 animation parameter 연결 후보를 확인한다.
    - [ ] `Block_Hold` montage는 ABP 전환 전 검증용 또는 임시 fallback으로 사용할 수 있는지 확인한다.
- [ ] Guard Released에서 `Block_Out` 실행으로 이어지게 구성한다.
  - 세부 구현 요소:
    - [ ] Guard 종료 요청이 active Guard Action에 전달되는 경로를 정한다.
    - [ ] `Block_Out`은 key release 기반 action 종료 흐름으로 실행한다.
    - [ ] Guard 종료가 완료되면 action runtime 상태와 guard animation 상태가 함께 정리되게 한다.
- [ ] 정상 release뿐 아니라 interrupt / dead / dodge / action stop 상황에서도 방어 runtime 상태가 정리되게 한다.
  - 세부 구현 요소:
    - [ ] `UCAction::ClearRuntime()` 또는 Guard 전용 cleanup 지점에서 guard/parry runtime 값을 정리한다.
    - [ ] action stop / intervention / reaction takeover에서 Guard 상태가 남지 않는지 확인한다.

### 4.2 Guard runtime 상태와 Parry Window 구성

- [x] Guard Action executor는 `CAction_Block` 또는 `CAction_Guard` 중 하나로 확정한다.
  - 세부 구현 요소:
    - [x] Guard가 상위 action이고 Parry는 Guard 내부 window 결과이므로, action executor 이름은 `CAction_Guard`를 우선 후보로 둔다.
    - [x] 기존 action executor 패턴(`CAction_ComboAttack`, `CAction_Dodge`)을 따라 Guard 전용 executor skeleton을 만든다.
    - [ ] `CAction_Guard` decision은 임시로 기존 action relationship을 사용하고, Hold / Release 구현 시 전용 정책으로 재검토한다.
- [ ] Guard Action 안에서 `bIsParryable`, `bIsGuarding` 성격의 runtime 상태를 관리한다.
  - 세부 구현 요소:
    - [ ] runtime 상태는 우선 Guard Action 내부에 둔다.
    - [ ] `IsParryWindowOpen()` / `IsGuarding()`처럼 `TakeDamage` 쪽에서 조회할 수 있는 최소 accessor 후보를 정한다.
    - [ ] 이후 Combat Resolution 분리 시 해당 상태 조회 경계를 그대로 옮길 수 있게 만든다.
- [ ] Block_In 시작 직후 Parry Window를 열 수 있게 한다.
  - 세부 구현 요소:
    - [ ] `Block_In` 시작 시점 또는 notify state begin에서 Parry Window를 연다.
    - [ ] Parry Window는 Guard Hold 전체가 아니라 Guard In 초반 구간에만 유효하게 둔다.
- [ ] `AN_SwitchToGuard` 또는 동등한 notify 시점에서 Parry Window를 닫고 Guard Hold 상태로 전환한다.
  - 세부 구현 요소:
    - [ ] 단발 notify로 Parry 가능 상태를 닫고 Guard 유지 상태를 연다.
    - [ ] notify 이름은 실제 구현 시 `SwitchToGuard` 성격이 드러나게 정한다.
- [ ] `UCAnimNotifyState_ExecutionInterventionWindow`와 `WindowKey = Parry`를 우선 활용할 수 있는지 확인한다.
  - 세부 구현 요소:
    - [ ] 기존 window notify가 action executor에 window key를 전달할 수 있는지 확인한다.
    - [ ] intervention 허용 window와 defensive 판정 window를 같은 저장소로 쓸 때 의미 충돌이 없는지 확인한다.
- [ ] 기존 intervention 허용 window와 Parry 판정 window를 같은 모델로 사용해도 되는지 검증한다.
- [ ] 전용 Parry notify 또는 Guard state notify가 필요한 경우는 후속 보완 후보로 기록한다.

### 4.3 Guard Hold 중 피격 처리 연결

- [ ] Guard Hold 상태에서 incoming damage가 들어오면 일반 Hit가 아니라 Guard Hit 흐름으로 분기한다.
  - 세부 구현 요소:
    - [ ] `UCTakeDamageComponent`에서 commit 이전에 active Guard 상태를 조회할 수 있는 경계를 만든다.
    - [ ] Guard Hold 중이면 일반 Hit reaction 요청으로 바로 가지 않고 Guard Hit 결과로 분기한다.
- [ ] Guard Hit에서는 damage를 완전 무효화하지 않고, 기존 damage 흐름을 유지하거나 guard damage 정책 후보로 남긴다.
  - 세부 구현 요소:
    - [ ] v1 1차 기준은 damage 50% 감소 후보로 둔다.
    - [ ] Guard Gauge 감소는 구현 비용이 작고 현재 구조에 무리가 없을 때만 2차 후보로 검토한다.
    - [ ] 이번 단계의 핵심 검증은 Guard Hit 분기와 `Block_Hit` 실행 여부로 둔다.
- [ ] Guard Hit 결과로 `Block_Hit` montage 또는 guard hit reaction을 실행할 수 있는 요청 경계를 정한다.
  - 세부 구현 요소:
    - [ ] Guard Hit를 기존 Reaction Orchestrator로 보낼지, Guard Action 쪽 실행 요청으로 처리할지 비교한다.
    - [ ] 일반 Hit / Dead reaction과 같은 경로를 쓰면 기존 피격 흐름과 충돌할 수 있으므로 v1에서는 명시적인 요청 경계를 둔다.
- [ ] Guard Hold 중 피격 처리와 기존 Hit / Dead reaction 흐름이 충돌하지 않는지 확인한다.

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
- [ ] 이번 Branch에서 실제 이관할 항목과 후속 Branch로 넘길 항목을 분리한다.

---

## 5. 비범위

- Attacker 측 Parry 성공 signal 송신 / 수신
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

- [ ] 신규 Guard Action executor compile 확인
- [ ] Guard enum / ActionData key / WindowKey 변경 compile 확인
- [ ] TakeDamage defensive resolution 임시 함수 compile 확인

### Code Flow

- [ ] Player input Pressed -> Guard Action Request -> Action Orchestrator 흐름 확인
- [ ] Guard Action Request -> Guard Action executor 실행 흐름 확인
- [ ] Guard Action -> Block_In -> Parry Window open 흐름 확인
- [ ] SwitchToGuard notify -> Parry Window close -> Guard Hold 상태 전환 확인
- [ ] Guard Released -> Block_Out 실행 흐름 확인
- [ ] incoming damage -> TakeDamage defensive resolution -> Parry 성공 분기 확인
- [ ] incoming damage -> TakeDamage defensive resolution -> Guard Hit 분기 확인
- [ ] Parry 성공 분기에서 기존 damage commit이 실행되지 않는지 확인
- [ ] 일반 피격에서 기존 TakeDamage / Reaction / DamageFeedback 흐름이 유지되는지 확인

### PIE

- [ ] Guard 입력 시 Block_In montage가 재생되는지 확인
- [ ] Guard 유지 시 Block_Hold 또는 Guard Hold 상태가 유지되는지 확인
- [ ] Guard release 시 Block_Out montage가 재생되는지 확인
- [ ] Parry Window 중 피격 시 damage가 무효화되는지 확인
- [ ] Parry Window 밖 피격 시 기존 피격 처리가 유지되는지 확인
- [ ] Guard Hold 중 피격 시 Block_Hit 또는 guard hit reaction이 실행되는지 확인
- [ ] Parry 성공 시 Block_Parry 또는 parry reaction / feedback이 실행되는지 확인

### Editor / Asset

- [ ] Block_In / Block_Hold / Block_Out / Block_Hit / Block_Parry asset 존재 여부 확인
- [ ] `FActionData.Montage`에 Guard / Block montage가 연결되어 있는지 확인
- [ ] `UCAnimNotifyState_ExecutionInterventionWindow` 또는 전환 notify가 montage에 적용되어 있는지 확인
- [ ] `WindowKey = Parry` 설정이 적용되어 있는지 확인

---

## 7. 문서화 기준

- [ ] 구현 완료 후 `P20_UE5_Portfolio_Pull_Request.md`를 작성한다.
- [ ] 구현 중 구조 충돌이 발생하면 System Design Record 또는 note 보완 필요 여부를 판단한다.
- [ ] 검증 과정에서 Editor / Asset 확인이 불완전하면 PR 문서의 미검증 항목에 남긴다.
- [ ] TakeDamage 내부 defensive resolution 규칙이 Guard / Counter 확장 기준으로 의미가 생기면 System Architecture 후속 보완 후보로 기록한다.

---

## 8. 정리

W03은 Guard / Parry Action v1을 기존 combat 실행 구조에 안전하게 연결하는 작업이다.

이번 Branch는 Combat Resolution을 처음부터 완성된 component로 분리하기보다, Guard Action 안에서 Parry Window와 Guard Hold 상태 전환을 먼저 안정화하고 `TakeDamage` 내부에서 Parry / Guard 분기를 임시로 검증하는 데 집중한다.

이 규칙이 안정되면 Parry / Guard 판정 책임을 `UCCombatResolutionComponent`로 분리하고, Counter / Perfect Parry / resource 계열 확장은 후속 Branch에서 이어간다.

---
