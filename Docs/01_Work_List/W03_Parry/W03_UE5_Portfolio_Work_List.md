# UE5 Portfolio - Work List

## 제목

**W03: Parry Action v1 구현**

## 날짜

**2026.06.14**

## 상태

- [ ] **진행중**

---

## 브랜치

- `feature/parry-action`

---

## 1. Branch 목표

이번 Branch는 Player의 Parry Action을 기존 combat 실행 구조에 안전하게 연결하는 것을 목표로 한다.

핵심은 Parry를 완성된 방어 시스템으로 한 번에 구현하는 것이 아니라, 먼저 damage 처리 전단에 Combat Resolution 호출 경계를 만들고 incoming damage를 기존 TakeDamage 처리로 계속 보낼지 중단할지 판단할 수 있는 최소 구조를 검증하는 것이다.

```yaml
핵심 목표
- 현재 Action / TakeDamage / Reaction / Feedback 호출 흐름 확인
- Combat Resolution 최소 호출 경계 구성
- incoming damage packet을 Combat Resolution에서 먼저 확인할 수 있는지 검증
- Continue / Block-Parry 결과에 따라 기존 TakeDamage fallback 유지 여부 확인
- Parry Action 입력 / 실행과 Parry Window 연결
- Parry 성공 시 damage commit 이전 중단, reaction / feedback 연결
- 안정화 후 TakeDamage 판단 요소를 Combat Resolution으로 점진 이관할 후보 기록
```

---

## 2. 완료 기준

- [ ] 현재 Action / TakeDamage / Reaction / Feedback 호출 흐름이 정리되어 있다.
- [ ] `UCCombatResolutionComponent`가 damage 처리 전단의 decision hook 역할로 연결되어 있다.
- [ ] Combat Resolution 결과가 Continue일 때 기존 `UCTakeDamageComponent` 흐름이 유지된다.
- [ ] Combat Resolution 결과가 Block-Parry일 때 damage commit 이전에 기존 damage 처리를 중단할 수 있다.
- [ ] Player 입력에서 Parry Action 요청과 `CAction_Parry` 실행으로 이어진다.
- [ ] `WindowKey = Parry` 기준으로 Parry Window open / close 상태를 확인할 수 있다.
- [ ] Parry 성공 시 damage 무효화, Parry reaction, 기본 feedback 흐름이 검증되어 있다.
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
- Parry 성공 reaction / feedback은 Combat Resolution 결과 또는 별도 요청 경계에서 명시적으로 연결해야 한다.

---

## 4. 작업 범위

### 4.1 Combat Resolution 호출 경계 구성

- [ ] `UCCombatResolutionComponent`를 damage 처리 전단의 최소 decision hook으로 추가한다.
- [ ] 소유 Actor에서 Combat Resolution component를 찾거나 등록할 수 있게 구성한다.
- [ ] `TakeDamage()` 또는 `UCTakeDamageComponent` 초입에서 Combat Resolution을 호출할 위치를 확정한다.
- [ ] Combat Resolution이 incoming damage 정보를 보고 Continue / Block-Parry 성격의 결과를 반환할 수 있게 한다.
- [ ] Continue 결과에서는 기존 `UCTakeDamageComponent` 처리 흐름이 그대로 유지되는지 확인한다.
- [ ] Block-Parry 결과에서는 damage commit 이전에 기존 damage 처리를 중단할 수 있는지 확인한다.

### 4.2 Parry Action 입력 / 실행 연결

- [ ] `ECombatActionIntent::Parry` 추가 위치를 확정한다.
- [ ] `EActionType::Parry` 추가 위치를 확정한다.
- [ ] Player 입력에서 Parry Action request를 생성해 Action Orchestrator로 전달한다.
- [ ] `CAction_Parry`를 추가하고 기존 Action 실행 lifecycle을 따른다.
- [ ] `UCActionComponent.ActionDatas`에 Parry ActionData를 추가한다.
- [ ] Parry ActionData가 `Montage_Parry` 또는 임시 Parry montage 후보와 연결되는지 확인한다.

### 4.3 Parry Window 연결

- [ ] `UCAnimNotifyState_ExecutionInterventionWindow`와 `WindowKey = Parry`를 우선 활용한다.
- [ ] Parry Action 실행 중 window open / close 상태가 Action executor에 기록되는지 확인한다.
- [ ] Combat Resolution에서 Parry Window 상태를 조회할 수 있는 최소 API를 결정한다.
- [ ] 기존 intervention 허용 window와 Parry 판정 window를 같은 모델로 사용해도 되는지 검증한다.
- [ ] 전용 Parry notify가 필요한 경우는 후속 보완 후보로 기록한다.

### 4.4 Parry 성공 처리 연결

- [ ] Parry Window 중 incoming damage가 들어오면 Block-Parry 결과를 반환하도록 연결한다.
- [ ] Parry 성공 시 `UCTakeDamageComponent`의 commit 단계로 들어가지 않는지 확인한다.
- [ ] Parry 실패 또는 Parry Window 밖 damage는 기존 TakeDamage fallback으로 이어지게 유지한다.
- [ ] Parry 성공 시 Player 측 Parry reaction을 실행할 수 있는 요청 경계를 정한다.
- [ ] Parry 성공 시 기본 feedback을 실행할 수 있는 요청 경계를 정한다.

### 4.5 판단 요소 점진 이관 후보 기록

- [ ] `UCTakeDamageComponent::CanTakeDamage()`의 기존 정책 중 Combat Resolution으로 옮길 후보를 기록한다.
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
- S28 Policy / Gate 전체 리팩터링
- System Architecture 본문 재작성

---

## 6. 검증 기준

### Build

- [ ] 신규 `UCCombatResolutionComponent` compile 확인
- [ ] 신규 `CAction_Parry` compile 확인
- [ ] Parry enum / ActionData key / WindowKey 변경 compile 확인
- [ ] TakeDamage 진입부 변경 compile 확인

### Code Flow

- [ ] Player input -> Parry Action Request -> Action Orchestrator 흐름 확인
- [ ] Parry Action Request -> `CAction_Parry` 실행 흐름 확인
- [ ] `CAction_Parry` -> montage -> `WindowKey = Parry` open / close 흐름 확인
- [ ] incoming damage -> Combat Resolution -> Continue 결과 확인
- [ ] incoming damage -> Combat Resolution -> Block-Parry 결과 확인
- [ ] Block-Parry 결과에서 기존 damage commit이 실행되지 않는지 확인
- [ ] Continue 결과에서 기존 TakeDamage / Reaction / DamageFeedback 흐름이 유지되는지 확인

### PIE

- [ ] Parry 입력 시 Parry montage가 재생되는지 확인
- [ ] Parry Window 중 피격 시 damage가 무효화되는지 확인
- [ ] Parry Window 밖 피격 시 기존 피격 처리가 유지되는지 확인
- [ ] Parry 성공 시 reaction / feedback이 실행되는지 확인

### Editor / Asset

- [ ] Parry montage 후보 asset 존재 여부 확인
- [ ] `FActionData.Montage`에 Parry montage가 연결되어 있는지 확인
- [ ] `UCAnimNotifyState_ExecutionInterventionWindow`가 montage에 적용되어 있는지 확인
- [ ] `WindowKey = Parry` 설정이 적용되어 있는지 확인

---

## 7. 문서화 기준

- [ ] 구현 완료 후 `P20_UE5_Portfolio_Pull_Request.md`를 작성한다.
- [ ] 구현 중 구조 충돌이 발생하면 System Design Record 또는 note 보완 필요 여부를 판단한다.
- [ ] 검증 과정에서 Editor / Asset 확인이 불완전하면 PR 문서의 미검증 항목에 남긴다.
- [ ] `UCCombatResolutionComponent`가 Guard / Counter 확장 기준으로 의미가 생기면 System Architecture 후속 보완 후보로 기록한다.

---

## 8. 정리

W03은 Parry Action v1을 기존 combat 실행 구조에 안전하게 연결하는 작업이다.

이번 Branch는 Parry를 한 번에 완성된 방어 시스템으로 확장하기보다, Combat Resolution 호출 경계를 먼저 만들고 incoming damage를 기존 TakeDamage 흐름으로 계속 보낼지 중단할지 검증하는 데 집중한다.

이 경계가 안정되면 Parry 판정, damage 무효화, reaction / feedback 연결을 단계적으로 붙이고, 이후 Guard / Counter / Perfect Parry / resource 계열 확장은 후속 Branch에서 이어간다.

---
