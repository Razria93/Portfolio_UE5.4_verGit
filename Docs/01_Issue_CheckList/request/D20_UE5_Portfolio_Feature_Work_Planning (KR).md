# D20 Feature Work Planning - Parry

## 1. Source of Truth 확인

```yaml
기준 입력
-> Docs/01_Issue_CheckList/request/D20_UE5_Portfolio_Work_Brief (KR).md

Planning Prompt
-> Docs/08_AI_Workflow/05_Prompt_Library/01_Prompt_Files/03_Work_Planning/02_Feature_Work_Planning_Prompt (KR).md

관련 System Architecture 후보
-> S26 / S27 / S28 Architecture baseline은 현재 PR 커밋 범위에 포함되지 않음
-> 실제 D20 구현 Branch에서 존재 여부를 확인하거나 신규 작성 여부를 결정

관련 코드 후보
-> Source/Portfolio/Component/CActionComponent.*
-> Source/Portfolio/Component/CTakeDamageComponent.*
-> Source/Portfolio/Component/CDamageFeedbackComponent.*
-> Source/Portfolio/Component/CReactionFeedbackComponent.*
-> Source/Portfolio/Action/CAction.*
-> Source/Portfolio/Notify/CAnimNotifyState_ExecutionInterventionWindow.*
-> Source/Portfolio/Weapon/CWeaponActor.*

관련 Asset 후보
-> Content/03_Animation/GuardAndParry/
-> 현재 PR 커밋 범위에 포함되지 않은 입력 Asset 후보이므로 실제 구현 Branch에서 존재 여부 확인 필요
```

현재 Planning은 D20 Work Brief 기준으로 작성한다.

구현 착수 가능 여부는 이 문서의 구현 단위, 선행 조건, 검증 기준을 확인한 뒤 판단한다.

---

## 2. Work Brief 항목 처리 결과

### 확정된 결정

- `UCCombatResolutionComponent`를 신규 구성한다.
- D20에서는 Resource / Stamina / Posture / Guard Gauge 변화를 처리하지 않는다.
- Parry 성공 시 Player Damage를 완전 무효화한다.
- Parry Window는 기존 `UCAnimNotifyState_ExecutionInterventionWindow` / `WindowKey` 모델을 우선 활용한다.
- `Montage_Parry`는 `FActionData.Montage`를 통해 `CAction_Parry`에 연결한다.
- D20의 기본 `WindowKey`는 `Parry`로 둔다.
- `Actor::TakeDamage()` 진입부에서 `UCCombatResolutionComponent`를 먼저 호출한다.
- 과도기 fallback으로 기존 `TakeDamageComponent->RequestTakeDamage()` 경로를 유지한다.
- S28 Policy / Gate 전체 리팩터링은 D20 범위에 포함하지 않는다.
- S26 / S27 / S28 Architecture baseline은 현재 PR 커밋 범위에 없으므로 D20 Planning의 확정 Source of Truth로 취급하지 않는다.

### 계획차단 항목

- 현재 없음

### 비차단 / 검토필요 항목의 Planning 변환

```yaml
ECombatActionIntent / EActionType Parry 추가 범위
-> 선행 확인 항목
-> Action request와 ActionData key 연결 단위에서 확인

UCActionComponent.ActionDatas Parry ActionData 추가
-> 구현 단위 전 탐색 항목
-> 기존 ActionData 등록 방식 확인 후 추가

Actor::TakeDamage() Combat Resolution / fallback 조건 분리
-> 핵심 구현 단위
-> 중복 damage commit 방지 기준 필요

기존 DamagePipeline / TakeDamage / TakeDamageComponent 호출 흐름
-> 선행 확인 항목
-> fallback 유지 범위 판단에 필요

기존 WeaponActor Overlap 흐름
-> 선행 확인 항목
-> D20에서는 기존 DamagePacket 유입 흐름을 유지

기존 Action / Reaction interrupt 구조
-> 핵심 구현 단위
-> Parry Action에서 Parry Reaction interrupt 가능 여부 확인

기존 Feedback 실행 구조
-> 구현 단위 전 탐색 항목
-> DamageFeedback / ReactionFeedback 호출 위치 확인

기존 ExecutionInterventionWindow / WindowKey 적용 방식
-> 핵심 구현 단위
-> Parry Window Open / Close 기준

Parry Animation 입력 Asset 후보
-> Asset 확인 항목
-> Montage_Parry 구성 가능 여부 확인

S28 Policy / Gate 전체 리팩터링
-> 후속 범위 유지
-> S28 baseline이 실제로 존재하거나 작성된 뒤 기준 충돌 여부 재검토
```

---

## 3. 현재 구조 탐색 대상

### 코드 탐색

- `CActionComponent`
  - Action request / ActionData lookup / Action 실행 방식 확인
  - `Parry` ActionData 추가 위치 확인

- `CAction`
  - Montage 재생 / 종료 / interrupt 대응 구조 확인
  - `CAction_Parry` 파생 가능성 확인

- `CAnimNotifyState_ExecutionInterventionWindow`
  - `WindowKey` 저장 / open / close 전달 방식 확인
  - `Parry` WindowKey 추가만으로 표현 가능한지 확인

- `Actor::TakeDamage()` 구현 위치
  - 현재 `CTakeDamageComponent` 호출 진입부 확인
  - `UCCombatResolutionComponent` 선처리 삽입 위치 확인

- `CTakeDamageComponent`
  - Health commit / reaction / feedback 처리 책임 확인
  - fallback 경로로 남길 최소 책임 확인

- `CDamageFeedbackComponent` / `CReactionFeedbackComponent`
  - Parry 성공 Feedback을 연결할 수 있는 호출 방식 확인

- `CWeaponActor` / Weapon overlap 흐름
  - 기존 DamagePacket 유입이 유지되는지 확인

### Asset / Editor 탐색

- Parry Animation 입력 Asset 후보
  - `Content/03_Animation/GuardAndParry/`
  - 현재 PR 커밋 범위에 포함되지 않은 폴더이므로 실제 구현 Branch에서 존재 여부 확인
- 후보 Animation으로 `Montage_Parry` 구성 가능 여부 확인
- Montage 안에 `UCAnimNotifyState_ExecutionInterventionWindow`와 `WindowKey = Parry` 설정 가능 여부 확인

---

## 4. 기능 구현 목표

D20의 목표는 Player 측 Parry 1차 구현이다.

```yaml
구현 결과
-> Player가 Parry Action을 실행할 수 있음
-> Parry Montage 구간에서 Parry Window가 열리고 닫힘
-> DamagePacket 유입 시 UCCombatResolutionComponent가 Parry 성공 여부를 먼저 판단함
-> Parry 성공 시 Player Damage가 완전 무효화됨
-> Parry 성공 시 Parry Reaction interrupt와 기본 Feedback이 실행됨
-> 기존 TakeDamage fallback 경로가 유지됨

비목표
-> Attacker Signal 송신 / 수신
-> Attacker 측 Reaction 처리
-> Perfect / Normal Parry 구분
-> Resource / Stamina / Posture / Guard Gauge 처리
-> Network
-> 최종 VFX / SFX polish
```

---

## 5. 선행 확인 단위

### 선행 확인 단위 1. 기존 구조 확인 및 연결 지점 확정

선행 조건
-> 없음

목표
-> D20 구현이 들어갈 코드 / Asset 연결 지점을 확정한다.

수정 범위
-> 없음
-> 코드 / 문서 / Asset 탐색

비범위
-> 코드 수정
-> Asset 수정

위험
-> 기존 DamagePipeline 진입부를 잘못 판단하면 이후 구현 단위가 흔들림

사용자 결정 필요 여부
-> 불필요

검증 기준
-> Build: 해당 없음
-> Code Flow: 기존 Action / TakeDamage / Window / Feedback 흐름 확인
-> PIE: 해당 없음
-> Editor: Parry Animation 입력 Asset 후보 확인
-> Asset: `Montage_Parry` 구성 후보 확인

문서화 필요 여부
-> Work Checklist: 선행 확인 항목으로 반영
-> System Design Records: 충돌 확인 시에만 보완
-> Verification Log: 실제 확인 결과 기록 후보

---

## 6. 구현 단위 제안

### 구현 단위 1. Parry Action 타입 / ActionData 연결

선행 조건
-> `ECombatActionIntent` / `EActionType` 사용 범위 확인
-> `UCActionComponent.ActionDatas` 등록 방식 확인

목표
-> Player가 Parry Action을 요청하고 `CAction_Parry`를 실행할 수 있게 한다.

수정 범위
-> Action enum / intent enum 후보
-> `UCActionComponent`
-> 신규 `CAction_Parry`
-> ActionData / Blueprint / Editor 설정

비범위
-> Attacker 측 Action / Reaction
-> Perfect / Normal Parry 분기

위험
-> `EActionType`에 Parry를 추가하는 방식이 기존 intent / action 분리 기준과 충돌할 수 있음
-> ActionData 누락 시 PIE에서 Action 실행이 실패할 수 있음

사용자 결정 필요 여부
-> 불필요
-> 단, enum 확장 방식이 기존 Action 설계 또는 후속 Architecture baseline과 충돌하면 재검토

검증 기준
-> Build: enum / Action class compile
-> Code Flow: Parry 요청이 ActionData lookup을 거쳐 `CAction_Parry`로 연결되는지 확인
-> PIE: Parry 입력 시 Montage 재생 확인
-> Editor: Parry ActionData 설정 확인
-> Asset: `Montage_Parry` 연결 확인

문서화 필요 여부
-> Work Checklist: 구현 항목 반영
-> Verification Log: 입력 / ActionData / Montage 연결 검증 반영

### 구현 단위 2. Parry Montage / Window 연결

선행 조건
-> Parry Animation 입력 Asset 후보 확인
-> 기존 `UCAnimNotifyState_ExecutionInterventionWindow` / `WindowKey` 적용 방식 확인

목표
-> `Montage_Parry`에서 `WindowKey = Parry` 구간을 열고 닫는다.

수정 범위
-> `Montage_Parry` 신규 작성
-> Montage NotifyState 설정
-> 필요 시 WindowKey enum / name 등록 위치
-> `CAction_Parry` Montage 연결

비범위
-> 전용 `CAnimNotifyState_Parry` 신규 작성
-> 최종 Animation polish

위험
-> 기존 Window 모델이 Parry 성공 판정에 필요한 상태를 충분히 표현하지 못할 수 있음
-> Montage Notify 설정은 Codex가 직접 검증하기 어려울 수 있음

사용자 결정 필요 여부
-> 불필요
-> 기존 Window 모델로 표현 불가능한 근거가 확인될 때만 전용 NotifyState 후보 재검토

검증 기준
-> Build: WindowKey 코드 변경 시 compile
-> Code Flow: Window open / close 상태가 `UCCombatResolutionComponent`에서 조회 가능한지 확인
-> PIE: Parry Montage 구간에서 Window 적용 확인
-> Editor: NotifyState / WindowKey 설정 확인
-> Asset: `Montage_Parry` 생성 및 연결 확인

문서화 필요 여부
-> Verification Log: Editor / Asset 미검증 여부 기록
-> System Design Records: 후속 Architecture baseline과 충돌 시 보완

### 구현 단위 3. UCCombatResolutionComponent 최소 구성

선행 조건
-> `Actor::TakeDamage()` 진입부 확인
-> `CTakeDamageComponent` 기존 책임 확인
-> 후속 Architecture baseline 확인

목표
-> DamagePacket 유입 시 Health commit 전에 Parry 성공 여부를 판정한다.

수정 범위
-> 신규 `UCCombatResolutionComponent`
-> 소유 Actor component 등록
-> `Actor::TakeDamage()` 또는 현재 TakeDamage 진입부
-> `CTakeDamageComponent` fallback 호출 조건

비범위
-> Attacker Signal
-> Resource / Posture / Guard Gauge
-> S28 Policy / Gate 전체 리팩터링

위험
-> Damage 무효화와 fallback damage commit이 중복될 수 있음
-> 기존 Hit / Dead reaction 흐름이 회귀할 수 있음
-> Component 책임이 Parry 전용으로 고정되면 후속 확장성이 낮아질 수 있음

사용자 결정 필요 여부
-> 불필요
-> D20에서는 최소 Parry outcome 판정 계층으로 구현

검증 기준
-> Build: 신규 component compile
-> Code Flow: Parry 성공 시 `TakeDamageComponent->RequestTakeDamage()`가 호출되지 않는지 확인
-> Code Flow: Parry 실패 / 비Parry 상황에서는 fallback 경로 유지 확인
-> PIE: Parry Window 중 Damage 유입 시 Damage 무효화 확인
-> Editor: component attach / Blueprint 설정 확인
-> Asset: 해당 없음

문서화 필요 여부
-> System Design Records: 후속 Architecture baseline과 충돌할 경우 보완
-> Verification Log: Parry success / fallback 검증 결과 기록

### 구현 단위 4. Parry 성공 처리 / Reaction interrupt 연결

선행 조건
-> 기존 Action / Reaction interrupt 구조 확인
-> 기존 Feedback 실행 구조 확인

목표
-> Parry 성공 시 현재 Parry Action을 Parry Reaction으로 전환하고 기본 Feedback을 실행한다.

수정 범위
-> `UCCombatResolutionComponent`
-> `CAction_Parry`
-> Action / Reaction interrupt API 호출부
-> `CDamageFeedbackComponent`
-> `CReactionFeedbackComponent`

비범위
-> Attacker Reaction
-> Perfect / Normal Parry Feedback 구분
-> 최종 VFX / SFX polish

위험
-> Action Montage lifecycle과 Reaction interrupt timing이 충돌할 수 있음
-> Feedback 실행 시점이 damage 무효화보다 앞서거나 중복될 수 있음

사용자 결정 필요 여부
-> 불필요
-> 기존 interrupt API가 Parry Reaction을 표현하지 못하면 구조 선택 필요 항목으로 전환

검증 기준
-> Build: interrupt / feedback 호출 compile
-> Code Flow: Parry success result가 interrupt와 feedback으로 이어지는지 확인
-> PIE: Parry 성공 시 Action이 중단되고 Reaction / Feedback이 실행되는지 확인
-> Editor: Reaction / Feedback Asset 연결 확인
-> Asset: Parry Reaction 후보 Asset 확인

문서화 필요 여부
-> Verification Log: PIE / Code Flow 검증 결과 기록
-> PR Document: 변경 요약 반영

---

## 7. 구현 착수 판정

```yaml
상태
-> 조건부 진행 가능

판단
-> 계획차단 항목은 없음
-> 사용자 선택이 필요한 항목은 현재 없음
-> 선행 확인 단위 수행 후 실제 구현 착수 가능
-> Editor / Asset 설정은 구현 중 미검증 항목으로 남길 수 있음

구현 착수 전 조건
-> 기존 Action / TakeDamage / Window / Feedback 흐름 확인
-> Parry Animation 입력 Asset 또는 후보 폴더 존재 여부 확인
-> 후속 Architecture baseline 존재 여부 또는 신규 작성 여부 확인
-> `Montage_Parry` 구성 후보 확인
-> `Actor::TakeDamage()` Combat Resolution 선처리 위치 확인
```

---

## 8. 권장 실행 순서

1. 기존 구조 확인 및 연결 지점 확정
2. Parry Action 타입 / ActionData 연결
3. Parry Montage / Window 연결
4. `UCCombatResolutionComponent` 최소 구성
5. Parry 성공 처리 / Reaction interrupt 연결
6. Build / Code Flow 검증
7. PIE / Editor / Asset 검증

---

## 9. Work Checklist 작성 판단

```yaml
판단
-> 작성 권장

작성 시점
-> 실행 전
-> 이 Feature Work Planning 결과를 기준으로 작성

이유
-> D20은 신규 component, 신규 action, montage / notify / asset 설정, TakeDamage 진입부 변경을 포함함
-> Build / Code Flow / PIE / Editor / Asset 검증을 함께 추적해야 함
-> 후속 범위와 D20 범위를 명확히 관리해야 함

실행 후 처리
-> 새 Work Checklist를 만들지 않고 기존 D20 Work Checklist의 완료 상태 / 검증 상태 / 후속 범위를 업데이트
```

---

## 10. D20 후속 범위

```yaml
D20 후속 범위
-> Attacker Signal 송신 / 수신
-> Attacker 측 Combat Resolution 해석 및 처리
-> Attacker 체간 게이지 / ExecutionState / Reaction 처리
-> Subsystem / Interface 전달 방식 결정
-> Perfect Parry / Normal Parry 구분
-> Resource / Stamina / Posture / Guard Gauge 처리
-> Network Replication
-> 최종 VFX / SFX polish
-> S28 Policy / Gate 전체 리팩터링
```

---

## 11. 위험 요소

### 구조 위험

- `UCCombatResolutionComponent`가 Parry 전용 분기만 담으면 후속 Guard / Counter 확장 시 재설계가 필요할 수 있음.
- `TakeDamageComponent` fallback 조건이 불명확하면 damage commit이 중복될 수 있음.
- S28 Policy / Gate 전체 리팩터링을 함께 진행하면 D20 범위가 과도하게 커질 수 있음.

### 구현 위험

- 기존 `Actor::TakeDamage()` 흐름을 잘못 수정하면 모든 피격 흐름에 영향이 생김.
- ActionData / Montage / WindowKey 설정 중 하나만 누락되어도 PIE에서 Parry가 실패할 수 있음.
- Reaction interrupt API가 현재 구조에서 Player Parry Reaction을 바로 표현하지 못할 수 있음.

### 검증 위험

- Editor / Asset 설정은 Codex가 직접 완료 여부를 확인하지 못할 수 있음.
- PIE 검증 없이 Code Flow만으로는 Window timing / overlap timing을 확정할 수 없음.
- Montage Notify timing은 사용자가 Editor에서 최종 확인해야 할 가능성이 큼.

---

## 12. 사용자 결정 필요 항목

현재 Planning 확정을 막는 사용자 결정 항목은 없다.

다음 항목은 구현 중 구조 충돌이 확인될 경우 사용자 결정 항목으로 전환한다.

```yaml
EActionType / ECombatActionIntent 확장 방식
-> 기존 Action 설계와 충돌할 경우 선택 필요

전용 Parry NotifyState 필요 여부
-> 기존 ExecutionInterventionWindow로 표현 불가능할 경우 선택 필요

Reaction interrupt 표현 방식
-> 기존 Action / Reaction interrupt API로 표현 불가능할 경우 선택 필요
```

---

## 13. 검증 계획

### Build

- 신규 `UCCombatResolutionComponent` compile 확인
- 신규 `CAction_Parry` compile 확인
- enum / ActionData key / WindowKey 변경 compile 확인
- `Actor::TakeDamage()` 또는 현재 TakeDamage 진입부 변경 compile 확인

### Code Flow

- Parry input -> `CAction_Parry` 실행 흐름 확인
- `CAction_Parry` -> `Montage_Parry` 연결 확인
- `Montage_Parry` -> `ExecutionInterventionWindow` -> `WindowKey = Parry` 흐름 확인
- DamagePacket -> `UCCombatResolutionComponent` -> Parry 판정 흐름 확인
- Parry 성공 -> Damage 무효화 -> fallback 미호출 확인
- Parry 실패 / 일반 피격 -> 기존 fallback 유지 확인
- Parry 성공 -> Reaction interrupt -> Feedback 실행 흐름 확인

### PIE

- Parry 입력 시 Montage 재생 확인
- Parry Window 중 피격 시 Damage 무효화 확인
- Parry Window 밖 피격 시 기존 피격 처리 확인
- Parry 성공 시 Reaction interrupt 확인
- 기본 DamageFeedback / ReactionFeedback 확인

### Editor / Asset

- `Montage_Parry` 생성 확인
- `FActionData.Montage`에 `Montage_Parry` 연결 확인
- `UCAnimNotifyState_ExecutionInterventionWindow` 적용 확인
- `WindowKey = Parry` 설정 확인
- Parry Reaction / Feedback Asset 연결 확인

---

## 14. Commit 분리 후보

```yaml
Commit 1
-> Parry Action 타입 / ActionData / CAction_Parry 기본 연결
-> 검증 단위: Build + Action Code Flow

Commit 2
-> Montage_Parry / WindowKey / ExecutionInterventionWindow 연결
-> 검증 단위: Editor / Asset + PIE Window 확인

Commit 3
-> UCCombatResolutionComponent 최소 구성 및 TakeDamage 선처리 / fallback 연결
-> 검증 단위: Build + Code Flow + 일반 피격 fallback

Commit 4
-> Parry 성공 처리 / Reaction interrupt / Feedback 연결
-> 검증 단위: PIE Parry success flow
```

실제 Commit 분리는 구현 중 변경 규모와 검증 가능 단위에 맞춰 조정한다.

---

## 15. 문서화 필요 여부

```yaml
Work Checklist
-> 작성 후보
-> 이 Planning 결과를 기준으로 실행 전 작성 권장

Bug Report
-> 불필요
-> 구현 중 회귀나 오류가 발견될 경우 작성 후보

System Architecture
-> 즉시 작성하지 않음
-> D20 구현 후 현재 구조 설명 문서가 필요하면 후보

System Design Records
-> 후속 Architecture baseline과 충돌할 경우 보완
-> 신규 ADR / Architecture Issue Report는 현재 불필요

Engine Technique Document
-> 불필요

Engine Implementation Records
-> AnimNotify / Montage / Damage event 처리 이슈가 발생하면 후보

Verification Log
-> 필요
-> Build / Code Flow / PIE / Editor / Asset 검증과 미검증 항목 기록

PR Document
-> 필요
-> Branch 완료 시 변경 요약 / 검증 / 미검증 / 후속 범위 정리

Portfolio Technical Document
-> D20 완료 후 Parry 구조가 포트폴리오 설명 가치가 있을 때 후보
```

---

## 16. 미검증 / 확인 필요 항목

```yaml
미검증
-> `ECombatActionIntent::Parry` / `EActionType::Parry` 실제 추가 위치
-> `UCActionComponent.ActionDatas` 등록 방식
-> `Actor::TakeDamage()` 실제 수정 위치
-> `CTakeDamageComponent` fallback 최소 책임
-> 기존 WeaponActor Overlap / DamagePacket 유입 흐름
-> 기존 Action / Reaction interrupt API
-> 기존 DamageFeedback / ReactionFeedback 호출 방식
-> `Montage_Parry` 구성 가능 여부
-> Editor에서 WindowKey 설정 가능 여부

확인 필요
-> Parry Reaction으로 사용할 Asset 후보
-> Parry Feedback으로 사용할 기본 VFX / SFX / Camera / UI 후보
-> PIE에서 Window timing과 Damage overlap timing이 맞는지 여부
```
