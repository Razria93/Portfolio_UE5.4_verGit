# D20 Work Brief - Parry

## 1. 작업 개요

### 작업 목표

- 스텔라 블레이드 스타일의 `Parry` 액션을 구현한다.

### 작업 유형

- 신규 기능 구현
- 구조 경계 생성 요소 포함

### Work Brief 준비 상태

- 진행 가능
- Feature Work Planning 진입 가능
- 계획차단 / 검토필요 항목 없음
- 계획차단 / 선택필요 항목 없음

### 구현 착수 상태

- 즉시 구현 착수 가능 상태는 아님
- Feature Work Planning 결과에서 구현 단위 / 선행 조건 / 검증 기준을 정리한 뒤 판단
- Parry Animation 입력 Asset과 후속 Architecture baseline은 구현 착수 전에 확인해야 함

### Planning Prompt

- 기본 경로: Feature Work Planning Prompt
- 전환 후보: Refactor Work Planning Prompt
	- 책임 경계 충돌 또는 DamagePipeline 구조 재편 범위가 커질 경우 사용

---

## 2. 정리된 기능 흐름

1. Player가 Parry Action 입력
2. `CAction_Parry` 실행
3. `Montage_Parry` 재생
4. 기존 `UCAnimNotifyState_ExecutionInterventionWindow` / `WindowKey` 모델로 Parry Window Open / Close
5. WeaponActor Overlap과 DamagePipeline을 통해 Player 측 DamagePacket 유입
6. `TakeDamageComponent` 이전 또는 진입부에서 `UCCombatResolutionComponent`가 DamagePacket 해석
7. Player 상태 / Parry Window 상태 확인
8. Parry 성공 판정
9. Damage 완전 무효화
10. Parry Reaction interrupt
11. 기본 DamageFeedback / ReactionFeedback 실행

### 후속 기능 흐름

1. Attacker에게 Parry 성공 패킷 또는 시그널 전달
2. Attacker 측 Combat Resolution 해석
3. Attacker 상태 / 자원 / 체간 게이지 확인
4. Attacker ExecutionState Reaction 전환
5. Attacker Reaction 처리

위 후속 흐름은 D20에서 구현하지 않는다.

---

## 3. 작업 범위

### D20 작업 범위

- `UCCombatResolutionComponent` 신규 구성
- `CAction_Parry` 구현
- Parry Window Open / Close 제어
- Parry 성공 판정
- Parry 성공 시 Damage 완전 무효화
- Parry Reaction interrupt
- 기본 DamageFeedback / ReactionFeedback 연결
- Parry Animation 입력 Asset을 확인해 `Montage_Parry` 신규 작성
- 기존 `UCAnimNotifyState_ExecutionInterventionWindow` / `WindowKey` 모델 활용 또는 확장

### D20 후속 범위

- Attacker Signal 송신 / 수신 방식 결정 및 구현
- Attacker 측 Combat Resolution 해석 및 처리 흐름 구현
- Attacker 체간 게이지 / ExecutionState / Reaction 처리
- Subsystem / Interface 전달 방식 결정
- Perfect Parry / Normal Parry 구분
- Resource / Stamina / Posture / Guard Gauge 설계
- Parry VFX / SFX polish
- Network Replication 검토

---

## 4. 확정된 결정

- 신규 컴포넌트명은 `UCCombatResolutionComponent`로 정한다.
- D20에서는 Resource / Stamina / Posture / Guard Gauge 변화를 처리하지 않는다.
- Parry 성공 시 Player Damage를 완전 무효화한다.
- Parry Window는 기존 `UCAnimNotifyState_ExecutionInterventionWindow` / `WindowKey` 모델을 활용한다.
- 전용 `Montage_Parry` Asset은 현재 없음.
- `Content/03_Animation/GuardAndParry/`는 현재 PR 커밋 범위에 포함되지 않은 입력 Asset 후보로 보며, 실제 구현 Branch에서 존재 여부를 확인한다.
- `Montage_Parry`는 `FActionData.Montage`를 통해 `CAction_Parry`에 연결한다.
- Parry Window는 `Montage_Parry` 안의 `UCAnimNotifyState_ExecutionInterventionWindow`가 `WindowKey`를 열고 닫는 방식으로 연결한다.
- D20의 기본 `WindowKey`는 `Parry`로 둔다.
- D20에서는 `Actor::TakeDamage()` 진입부에서 `UCCombatResolutionComponent`를 먼저 호출한다.
- 과도기 fallback으로 기존 `TakeDamageComponent->RequestTakeDamage()` 경로를 유지한다.
- `UCCombatResolutionComponent`는 combat outcome을 판정하는 계층으로 둔다.
- `TakeDamageComponent`는 resolved damage payload를 소비해 Health commit을 수행하는 계층으로 축소한다.
- D20에서는 `TakeDamageComponent`의 기존 reaction / feedback dispatch는 fallback 경로에만 남긴다.
- System Design Records는 즉시 신규 작성하지 않는다.
- S26 / S27 / S28은 현재 PR 커밋 범위에 포함되지 않은 후속 Architecture baseline 후보로 둔다.
- D20 문서는 S26 / S27 / S28의 존재를 전제하지 않는다.
- S28의 Policy / Gate 전체 리팩터링은 D20 범위에 포함하지 않는다.
- 실제 구현 Branch에서 S26 / S27 / S28이 추가되거나 확인되면 그 기준과의 충돌 여부를 다시 검토한다.

---

## 5. 검토필요 항목

### 계획차단 / 검토필요

- 현재 없음

### 비차단 / 검토필요

- `ECombatActionIntent::Parry` / `EActionType::Parry` 추가 범위 확인
- `UCActionComponent.ActionDatas`에 Parry ActionData 추가
- `Actor::TakeDamage()`에서 Combat Resolution 처리 성공 / fallback 조건을 명확히 분리
- 기존 DamagePipeline / TakeDamage / TakeDamageComponent 호출 흐름 확인
- 기존 WeaponActor Overlap 흐름 확인
- 기존 Action / Reaction interrupt 구조 확인
- 기존 Feedback 실행 구조 확인
- 기존 `UCAnimNotifyState_ExecutionInterventionWindow` / WindowKey 적용 방식 확인
- Parry Animation 입력 Asset 또는 `Content/03_Animation/GuardAndParry/` 후보 폴더 존재 여부 확인
- S28 Policy / Gate 전체 리팩터링은 후속 범위로 유지

### 구현 착수 전 확인 항목

- Parry Animation 입력 Asset 또는 `Content/03_Animation/GuardAndParry/` 후보 폴더
- S26 / S27 / S28 Architecture baseline 존재 여부 또는 신규 작성 여부

---

## 6. 선택필요 항목

### 계획차단 / 선택필요

- 현재 없음

### 비차단 / 선택필요

- 현재 없음

---

## 7. 위험 항목

### 구조 위험

- DamagePipeline 진입점 변경으로 기존 피격 흐름에 영향 가능
- `UCCombatResolutionComponent`가 Parry 전용 구현으로 고정되면 Guard / Counter 등 후속 확장 시 구조 변경이 필요할 수 있음
- fallback 유지 조건이 불명확하면 Combat Resolution 처리 결과와 기존 TakeDamage 처리 결과가 중복 적용될 수 있음
- D20에서 `TakeDamageComponent`의 기존 reaction / feedback dispatch를 완전히 제거하면 기존 Hit / Dead 피격 흐름 회귀 위험이 큼
- S28의 Policy / Gate 리팩터링까지 D20에 포함하면 작업 범위가 과도하게 커질 수 있음

### 구현 위험

- `UCCombatResolutionComponent` 신규 구성 비용이 있음
- `CAction_Parry`와 기존 WindowKey 모델의 Window Open / Close 동기화가 필요함
- `ECombatActionIntent` / `EActionType`에 Parry가 없어 입력 요청과 ActionDataKey 연결을 추가해야 함
- Parry Action에서 Parry Reaction으로 interrupt하는 흐름이 기존 Montage lifecycle과 충돌할 수 있음
- Damage 무효화와 Feedback 실행 순서가 어긋날 수 있음

### 검증 위험

- Build 검증 필요
- Code Flow 검증 필요
- PIE에서 Parry 입력 / Window / DamagePacket / Reaction interrupt 확인 필요
- Montage / AnimNotify / Asset 연결 확인 필요
- Codex가 직접 확인하지 못한 Editor / Asset 항목은 미검증으로 기록 필요

## 8. Prompt 라우팅 결과

```yaml
Work Brief 준비 상태
-> 진행 가능
-> Feature Work Planning 진입 가능
-> 계획차단 / 검토필요 항목 없음
-> 계획차단 / 선택필요 항목 없음

구현 착수 상태
-> 즉시 구현 착수 가능 상태는 아님
-> Feature Work Planning 결과에서 구현 단위 / 선행 조건 / 검증 기준을 정리한 뒤 판단

Planning Prompt
-> 기본 경로: Feature Work Planning Prompt
-> 전환 후보: Refactor Work Planning Prompt
-> 책임 경계 충돌 또는 DamagePipeline 구조 재편 범위가 커질 경우 사용

Work Checklist Writing
-> 작성 후보
-> Feature Work Planning 결과에서 최종 작성 여부 판단
-> 작성하는 경우 실행 전에 D20 Work Checklist 작성

실행 계층 후보
-> 구현 계층: Codex 구현 수행, 구현 Prompt 없음
-> 문서화 계층: Verification Log / PR Document / 후속 Architecture baseline 보완 필요 여부 확인
-> 검증 계층: Build / Code Flow / PIE / Asset Blueprint Validation
-> Commit / PR 계층: Git Commit PR Preflight Prompt
```

---

## 9. 다음 단계

1. Feature Work Planning 수행
2. Feature Work Planning에서 비차단 / 검토필요 항목을 선행 확인 항목으로 변환
3. 구현 단위 / 검증 기준 / 문서화 필요 여부 정리
4. Feature Work Planning 결과를 기준으로 D20 Work Checklist 작성 여부 확정
