# UE5 Portfolio - Work List Draft

## 제목

**W03: Parry v1 구현 준비 워크리스트**

## 날짜

**2026.06.09**

## 상태

- [ ] **진행중**

---

## 브랜치

- `feature/codex-workflow`

---

## 1. Branch 목표

실제 W03 구현 Branch는 Stella Blade 스타일의 `Parry` v1을 구현하기 위한 최소 Combat Resolution 체계를 구축한다.

```yaml
목표
- Player 측 Parry Action 실행
- 기존 WindowKey 모델 기반 Parry Window Open / Close
- DamagePacket 유입 시 UCCombatResolutionComponent 선처리
- Parry 성공 시 Player Damage 완전 무효화
- Parry 성공 시 Reaction interrupt와 기본 Feedback 연결
- 기존 TakeDamage fallback 경로 유지
```

현재 AI Workflow Branch에서는 실제 구현을 하지 않고, `Work Brief -> Feature Work Planning -> Work List` 변환 흐름만 검증한다.

---

## 2. 입력 문서

- [x] `W03_UE5_Portfolio_Work_Brief.md` 작성
- [x] `W03_UE5_Portfolio_Feature_Work_Planning.md` 작성
- [x] W03 Work Brief 기준으로 Feature Work Planning 작성
- [x] Feature Work Planning 기준으로 Work List Draft 작성

---

## 3. W03 작업 범위

```yaml
포함 범위
- UCCombatResolutionComponent 신규 구성
- CAction_Parry 구현
- Parry Action 타입 / ActionData 연결
- Parry Animation 입력 Asset 확인 후 Montage_Parry 신규 작성
- Parry Window Open / Close 제어
- Parry 성공 판정
- Parry 성공 시 Damage 완전 무효화
- Parry Reaction interrupt
- 기본 DamageFeedback / ReactionFeedback 연결
- 기존 UCAnimNotifyState_ExecutionInterventionWindow / WindowKey 모델 활용 또는 확장
- 기존 TakeDamage fallback 경로 유지
```

---

## 4. W03 후속 범위

```yaml
후속 범위
- Attacker Signal 송신 / 수신
- Attacker 측 Combat Resolution 해석 및 처리
- Attacker 체간 게이지 / ExecutionState / Reaction 처리
- Subsystem / Interface 전달 방식 결정
- Perfect Parry / Normal Parry 구분
- Resource / Stamina / Posture / Guard Gauge 처리
- Network Replication
- 최종 VFX / SFX polish
- S28 Policy / Gate 전체 리팩터링
```

---

## 5. 확정된 결정

- 신규 컴포넌트명은 `UCCombatResolutionComponent`로 둔다
- W03에서는 Resource / Stamina / Posture / Guard Gauge 변화를 처리하지 않는다
- Parry 성공 시 Player Damage를 완전 무효화한다
- Parry Window는 기존 `UCAnimNotifyState_ExecutionInterventionWindow` / `WindowKey` 모델을 우선 활용한다
- W03의 기본 `WindowKey`는 `Parry`로 둔다
- `Actor::TakeDamage()` 진입부에서 `UCCombatResolutionComponent`를 먼저 호출하는 방향으로 계획한다
- 과도기 fallback으로 기존 `TakeDamageComponent->RequestTakeDamage()` 경로를 유지한다
- S28 Policy / Gate 전체 리팩터링은 W03 범위에 포함하지 않는다
- S26 / S27 / S28 Architecture baseline은 현재 PR 커밋 범위에 포함되지 않은 후속 기준 후보로 둔다

---

## 6. 선행 확인 체크리스트

- [ ] 기존 `CActionComponent`의 Action request / ActionData lookup / Action 실행 방식 확인
- [ ] 기존 `CAction`의 Montage 재생 / 종료 / interrupt 대응 구조 확인
- [ ] 기존 `UCAnimNotifyState_ExecutionInterventionWindow`의 `WindowKey` open / close 전달 방식 확인
- [ ] 현재 `Actor::TakeDamage()` 또는 TakeDamage 진입부 확인
- [ ] 기존 `CTakeDamageComponent`의 Health commit / reaction / feedback 처리 책임 확인
- [ ] 기존 `CDamageFeedbackComponent` / `CReactionFeedbackComponent` 호출 방식 확인
- [ ] 기존 `CWeaponActor` / Weapon overlap / DamagePacket 유입 흐름 확인
- [ ] Parry Animation 입력 Asset 또는 `Content/03_Animation/GuardAndParry/` 후보 폴더 존재 여부 확인
- [ ] `Montage_Parry` 구성 가능 여부 확인

---

## 7. 구현 단위 체크리스트

### 구현 단위 1. Parry Action 타입 / ActionData 연결

- [ ] `ECombatActionIntent::Parry` 또는 관련 intent 확장 위치 확인
- [ ] `EActionType::Parry` 또는 관련 action type 확장 위치 확인
- [ ] `UCActionComponent.ActionDatas`에 Parry ActionData 추가
- [ ] 신규 `CAction_Parry` 작성
- [ ] Parry 입력 요청이 `CAction_Parry` 실행으로 연결되는지 확인
- [ ] Parry ActionData에 `Montage_Parry` 연결

### 구현 단위 2. Parry Montage / Window 연결

- [ ] Parry Animation 입력 Asset 후보 기반으로 `Montage_Parry` 구성
- [ ] `Montage_Parry`에 `UCAnimNotifyState_ExecutionInterventionWindow` 설정
- [ ] `WindowKey = Parry` 설정
- [ ] Parry Window open / close 상태가 코드에서 조회 가능한지 확인
- [ ] 전용 `CAnimNotifyState_Parry` 신규 작성이 필요 없는지 확인

### 구현 단위 3. UCCombatResolutionComponent 최소 구성

- [ ] 신규 `UCCombatResolutionComponent` 작성
- [ ] 소유 Actor에 `UCCombatResolutionComponent` 등록
- [ ] `Actor::TakeDamage()` 또는 현재 TakeDamage 진입부에 Combat Resolution 선처리 삽입
- [ ] Parry 성공 시 `TakeDamageComponent->RequestTakeDamage()` fallback 미호출 처리
- [ ] Parry 실패 / 비Parry 상황에서는 기존 fallback 경로 유지
- [ ] Damage 무효화와 fallback damage commit 중복 방지

### 구현 단위 4. Parry 성공 처리 / Reaction interrupt 연결

- [ ] Parry 성공 결과를 Reaction interrupt로 연결
- [ ] 기존 Action / Reaction interrupt API로 Parry Reaction을 표현할 수 있는지 확인
- [ ] DamageFeedback 기본 실행 연결
- [ ] ReactionFeedback 기본 실행 연결
- [ ] Feedback 실행 시점이 Damage 무효화 이후로 정리되는지 확인

---

## 8. 검증 기준

### Build

- [ ] 신규 `UCCombatResolutionComponent` compile 확인
- [ ] 신규 `CAction_Parry` compile 확인
- [ ] enum / ActionData key / WindowKey 변경 compile 확인
- [ ] TakeDamage 진입부 변경 compile 확인

### Code Flow

- [ ] Parry input -> `CAction_Parry` 실행 흐름 확인
- [ ] `CAction_Parry` -> `Montage_Parry` 연결 확인
- [ ] `Montage_Parry` -> `ExecutionInterventionWindow` -> `WindowKey = Parry` 흐름 확인
- [ ] DamagePacket -> `UCCombatResolutionComponent` -> Parry 판정 흐름 확인
- [ ] Parry 성공 -> Damage 무효화 -> fallback 미호출 확인
- [ ] Parry 실패 / 일반 피격 -> 기존 fallback 유지 확인
- [ ] Parry 성공 -> Reaction interrupt -> Feedback 실행 흐름 확인

### PIE

- [ ] Parry 입력 시 Montage 재생 확인
- [ ] Parry Window 중 피격 시 Damage 무효화 확인
- [ ] Parry Window 밖 피격 시 기존 피격 처리 확인
- [ ] Parry 성공 시 Reaction interrupt 확인
- [ ] 기본 DamageFeedback / ReactionFeedback 확인

### Editor / Asset

- [ ] `Montage_Parry` 생성 확인
- [ ] `FActionData.Montage`에 `Montage_Parry` 연결 확인
- [ ] `UCAnimNotifyState_ExecutionInterventionWindow` 적용 확인
- [ ] `WindowKey = Parry` 설정 확인
- [ ] Parry Reaction / Feedback Asset 연결 확인

---

## 9. 문서화 필요 여부

- [ ] Work List 공식 문서 승격 여부 결정
- [ ] Verification Log 작성
- [ ] PR Document 작성
- [ ] 후속 Architecture baseline과 충돌할 경우 System Design Records 보완
- [ ] W03 완료 후 System Architecture 현재 구조 설명 필요 여부 판단
- [ ] Portfolio Technical Document 작성 가치가 있는지 후속 판단

---

## 10. 미검증 / 확인 필요 항목

```yaml
미검증
- ECombatActionIntent::Parry / EActionType::Parry 실제 추가 위치
- UCActionComponent.ActionDatas 등록 방식
- Actor::TakeDamage() 실제 수정 위치
- CTakeDamageComponent fallback 최소 책임
- 기존 WeaponActor Overlap / DamagePacket 유입 흐름
- 기존 Action / Reaction interrupt API
- 기존 DamageFeedback / ReactionFeedback 호출 방식
- Parry Animation 입력 Asset 존재 여부
- Montage_Parry 구성 가능 여부
- Editor에서 WindowKey 설정 가능 여부
```

```yaml
확인 필요
- Parry Reaction으로 사용할 Asset 후보
- Parry Feedback으로 사용할 기본 VFX / SFX / Camera / UI 후보
- PIE에서 Window timing과 Damage overlap timing이 맞는지 여부
```

---

## 11. PR 가능 조건

실제 W03 구현 Branch는 다음 조건을 만족하면 PR 가능한 상태로 본다.

- [ ] W03 작업 범위의 구현 단위가 완료된다
- [ ] Build 검증 완료
- [ ] Code Flow 검증 완료
- [ ] PIE 검증 완료
- [ ] Editor / Asset 검증 결과 또는 미검증 항목 기록 완료
- [ ] Verification Log 작성 완료
- [ ] PR Document 작성 완료
- [ ] W03 후속 범위가 분리되어 있다
- [ ] 구현 중 후속 Architecture baseline과 충돌한 항목이 있으면 문서 보완 후보로 기록된다

---

## 12. AI Workflow 검증 결과

- [x] 자연어 요청을 `W03 Work Brief`로 정리했다
- [x] `W03 Work Brief`를 `W03 Feature Work Planning`으로 변환했다
- [x] `W03 Feature Work Planning`을 `W03 Work List Draft`로 변환했다
- [x] Work Brief의 확정된 결정 / 검토필요 항목 / 후속 범위가 Work List Draft에 반영됐다
- [x] Feature Work Planning의 선행 확인 단위 / 작업 단위 / 검증 기준이 Work List Draft에 반영됐다
- [x] 실제 코드 / 런타임 / 에셋 검증 항목은 완료 처리하지 않는다

---

## 13. 비고

- 이 문서는 AI Workflow 구축 Branch에서 작성한 W03 Work List Draft다.
- 실제 Parry 구현은 별도 Branch에서 진행한다.
- 실제 구현 Branch에서는 이 Draft를 `Docs/01_Work_List/W03_Parry/W03_UE5_Portfolio_Work_List.md`로 승격하거나, 구현 Branch 상황에 맞게 재작성한다.
- 현재 문서의 `[x]`는 AI Workflow 문서 변환 검증 항목에만 사용한다.
- 실제 코드 / Asset / Editor / PIE 관련 항목은 모두 미완료 상태로 유지한다.
