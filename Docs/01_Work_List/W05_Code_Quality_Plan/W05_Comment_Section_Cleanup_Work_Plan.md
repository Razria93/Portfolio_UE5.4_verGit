# UE5 Portfolio - Work List

## 제목

**W05: comment / section cleanup 작업 계획**

## 날짜

**2026.07.22**

## 상태

- [ ] **진행중**

---

## 브랜치

- `refactor/comment-section-cleanup`

---

## 1. 작업 목표

이번 작업은 코드 동작을 바꾸지 않고, 주석 / TODO / 섹션 구분 / 설명 문자열의 신뢰도를 정리한다.

목표는 리뷰어가 코드를 읽을 때 임시 trace, stale comment, 오타, 중복 설명 때문에 현재 구현 상태를 잘못 판단하지 않도록 만드는 것이다.

```text
핵심 목표
1. stale / 오타 / 잘못된 주석 정리
2. commented-out / temporary trace 잔존 여부 확인
3. TODO를 후속 작업 범주로 분류
4. 섹션 주석 양식 정리
5. 불필요한 설명 주석 제거
6. 작업 중 발견된 작은 보완 후보 기록
```

---

## 2. 작업 개요

이번 브랜치는 코드 품질 정리 중에서도 가벼운 문맥 정합성 정리에 한정한다.

```yaml
포함 범위:
- 오타 / 잘못된 단어 수정
- CVar help text와 실제 역할 정합성 수정
- 의미 없는 빈 TODO 제거 또는 후속 범주 명시
- 섹션 주석 스타일 정리
- 코드가 그대로 설명되는 중복 주석 제거
- commented-out debug trace 잔존 여부 확인

제외 범위:
- API rename
- USTRUCT / header 구조 이동
- DataAsset 분리 구현
- runtime behavior 변경
- DeadFlag / loop / spawn policy 구현
- RuntimeLOD CVar 위치 이동
```

---

## 3. 구조 / 비용 / 위험 검토

```yaml
구조 타당성:
- 코드 동작을 바꾸지 않는 품질 정리이므로 PR 단위로 독립 검토 가능
- TODO 분류는 이후 작업 카테고리와 연결되므로 장기 관리에 도움됨
- 섹션 주석 정리는 현재 debug / profiling helper 정리 흐름과 맞음

구현 비용:
- 대부분 주석 / 문자열 수정
- 일부 파일은 주석 양식만 수정
- Type 헤더 주석 정리는 양이 많을 수 있으므로 별도 커밋으로 분리 가능

위험:
- CVar help text 변경은 사용자-facing console help 문구가 바뀜
- API 이름, enum 이름, asset reference는 건드리지 않아야 함
- TODO를 삭제할 때 후속 작업 추적성이 사라지지 않도록 범주를 남겨야 함
```

---

## 4. 결정 항목

현재 사용자 결정이 필요한 항목은 없다.

다만 다음 항목은 이번 브랜치에서 구현하지 않고 후속 카테고리로 넘긴다.

```yaml
후속 처리:
- API rename: 네이밍 / 매개변수 작명 규칙
- Type 이동: 구조체 나누기 / 헤더 배치 규칙
- DataAsset 이동: 데이터 에셋 분리
- DeadFlag 동작 변경: 별도 gameplay correctness 작업
- RuntimeLOD CVar 위치 이동: RuntimeLOD config / policy 구조 정리
```

---

## 5. 이번 작업 범위

### 1) 오타 / 표현 정리

- [x] `Acitve API`를 `Active API`로 수정
- [x] `Delgate`를 `Delegate`로 수정
- [x] `Flag Toogle`을 `Flag Toggle`로 수정하거나 불필요하면 제거
- [x] `Deffered Spawn`을 `Deferred Spawn`으로 수정
- [x] `Seperate`를 `Separate`로 수정
- [x] `Stemina`를 `Stamina`로 수정
- [x] `BroadCast`를 `Broadcast`로 수정
- [x] `Injection Datas`를 `Injected Animation Data`로 수정
- [x] `FallBack`을 `Fallback`으로 수정하거나 불필요하면 제거
- [x] CVar 설명 문자열의 일반 명사 `Enemy` 표기를 `enemy` 또는 `ACEnemy` 기준으로 정리

### 2) stale / 잘못된 설명 정리

- [x] `FCombatEngageDebug` CVar 설명을 warmup 한정 표현에서 warmup / rebuild summary 기준으로 수정
- [x] `FCombatFeedbackDebug` CVar 설명을 실제 request / channel / presentation / dispatch 역할에 맞게 수정
- [x] `FAIPerceptionDebug` CVar 설명의 candidate / latency / Blackboard / Engage 표현을 실제 출력 기준으로 수정
- [x] `CAIPerceptionProfiling` CVar 설명의 `Enemy` 표기를 정리
- [x] `CCombatCollisionProfiling` CVar 설명의 enemy / weapon actor 표현을 정리
- [x] `CCombatFeedbackProfiling` CVar 설명의 `Enemy` 표기를 정리
- [x] `CAIStateRuntimeLODPolicy`의 `Runtime LOD` / `RuntimeLOD` 표현을 문맥별로 통일
- [x] `CActionFeedbackComponent`의 `Architect Miss` 표현을 실제 의미가 드러나는 문구로 수정

### 3) TODO 분류

- [ ] `CAIController` perception config DataAsset 후보 TODO 분류
- [ ] `CPlayer` / `CEnemy` DeadFlag early return TODO 분류
- [ ] `CActionFeedbackComponent` / `CReactionFeedbackComponent` loop support TODO 분류
- [ ] `CWeaponComponent` deferred spawn TODO 분류
- [ ] `CActionComponent` / `CReactionComponent` DataAsset build TODO 분류
- [ ] `CCombatSignalSourceComponent` DamageSpecContainer DataAsset migration TODO 분류
- [ ] `CHealthComponent` ResourceComponent extension / delegate broadcast TODO 분류
- [ ] `CWeaponStructure` feedback / action execution context / 미분류 TODO 정리

### 4) 섹션 주석 양식 정리

- [ ] `CWeaponActor.h`의 `// ===`, `/* === */`, 중복 `AnimNotify Events` 섹션 정리
- [ ] `CAIController.h`의 `/* --- Asset --- */`와 `// Lifecycle` 스타일 혼용 정리
- [ ] `CAnimInstance.h`의 RuntimeLOD 번호 주석을 의미 기반 섹션으로 정리
- [ ] `CEnemy.h`의 RuntimeLOD 번호 주석을 의미 기반 섹션으로 정리
- [ ] `CMovementComponent.h`의 RuntimeLOD 번호 주석을 의미 기반 섹션으로 정리
- [ ] `Core/Debug` / `Core/Profiling` helper 섹션명은 필요 시 최소 보정

### 5) 불필요한 설명 주석 제거

- [ ] `CPlayer.cpp` 생성자 init 주석 중 코드 반복 설명 제거
- [ ] `CEnemy.cpp` 생성자 init 주석 중 코드 반복 설명 제거
- [ ] `CWeaponComponent.cpp` spawn 단계 번호 주석을 압축 또는 제거
- [ ] `CBTServiceIntervalHelper.cpp`의 단순 return 설명 주석 제거
- [ ] `CCombatSignalStructure.h`의 필드명 반복 UPROPERTY 주석 정리
- [ ] `CWeaponStructure.h`의 `[NOTE] Temp`, 개인 체크리스트성 주석, 단순 단계 주석 정리

### 6) API / inline role comment 유효성 검토

- [x] `Incoming API`, `Active API`, `Getter`, `Setter` 같은 inline role comment가 실제 책임을 설명하는지 확인
- [x] 의미가 코드명과 중복되면 제거
- [x] 의미가 불명확하면 실제 역할 기준으로 수정
- [x] public API rename이 필요한 수준이면 이번 브랜치에서 수정하지 않고 네이밍 작업으로 분리

### 7) 잔존 trace 확인

- [ ] commented-out `UE_LOG` 잔존 여부 확인
- [ ] commented-out `DrawDebug` 잔존 여부 확인
- [ ] commented-out `GEngine` 잔존 여부 확인
- [ ] 임시 debug trace성 주석 잔존 여부 확인

---

## 6. 제외 범위 / 후속 작업 범위

이번 브랜치에서 발견하더라도 다음 범위는 수정하지 않는다.

```yaml
네이밍 / 매개변수 작명 규칙:
- FObservableOverlayDebug / FExecutionOrchestratorDebug의 Handlings 계열 API 이름
- combat profiling API suffix 통일
- 책임명이 어색한 public API rename

구조체 나누기 / 헤더 배치 규칙:
- EBTServiceIntervalPreset 위치 이동
- CWeaponStructure large type 분리
- shared type / module-local type 재배치

데이터 에셋 분리:
- Action / Reaction data map build 이동
- DamageSpecContainer DataAsset migration
- AI perception config DataAsset migration

상수 제거 / RuntimeLOD 구조 정리:
- CEnemy RuntimeLOD CVar 위치 이동
- RuntimeLOD policy/config 재구성

기능 수정:
- DeadFlag early return 동작 변경
- feedback loop support 구현
- deferred spawn 정책 구현
```

---

## 7. 완료 기준

- [ ] 전수조사 후보가 이번 브랜치 범위 / 후속 범위로 분류되어 있다
- [ ] 오타와 stale comment가 코드 상태와 충돌하지 않는다
- [ ] TODO는 구현 필요 여부와 후속 카테고리를 추적할 수 있다
- [ ] 섹션 주석은 같은 파일 안에서 일관된 양식을 가진다
- [ ] 단순히 코드 내용을 반복하는 주석이 줄어 있다
- [ ] commented-out temporary trace가 남아 있지 않음을 확인했다
- [ ] 코드 동작 변경 없이 diff가 주석 / 문자열 / 문서 중심으로 제한되어 있다

---

## 8. 필수 문서 / 산출 대상

```yaml
Work List:
- Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
- Docs/01_Work_List/W05_Code_Quality_Plan/W05_Comment_Section_Cleanup_Work_Plan.md

후속 PR 문서:
- Docs/04_Pull_Request/P##_UE5_Portfolio_Pull_Request.md
```

---

## 9. 검증 기준

```yaml
정적 확인:
- rg로 TODO / 오타 / temporary trace 잔존 여부 확인
- git diff에서 기능 코드 변경 여부 확인
- git diff --check 통과

빌드:
- 주석 / 문자열만 변경한 경우 빌드는 선택
- header comment 정리 중 preprocessor / macro 주변을 건드린 경우 PortfolioEditor Development 빌드 권장

리뷰:
- 후속 작업으로 넘긴 항목이 삭제되지 않고 추적 가능한지 확인
- 문서와 코드 주석의 작업 범주가 충돌하지 않는지 확인
```

---

## 10. 진행 중 변경 관리 기준

- 동작 변경이 필요해 보이면 이번 브랜치에서 수정하지 않고 후속 후보로 기록한다.
- public API rename이 필요해 보이면 네이밍 작업으로 분리한다.
- USTRUCT / UPROPERTY / Blueprint exposure와 관련된 이름 변경은 이번 브랜치에서 하지 않는다.
- TODO 삭제는 구현 완료가 명확하거나 다른 문서 / 후속 범주로 추적 가능할 때만 허용한다.
- 섹션 주석은 파일별 기존 스타일을 우선하되, 같은 파일 안에서는 혼용을 줄인다.

---

## 11. PR 가능 조건

```yaml
PR 가능:
- 이번 문서의 체크리스트가 완료 또는 후속 범위로 명확히 분류됨
- diff가 주석 / 문자열 / 문서 중심임
- git diff --check 통과
- 필요 시 PortfolioEditor Development 빌드 통과

PR 보류:
- 기능 동작 변경이 섞임
- asset reference 위험이 있는 rename이 섞임
- TODO를 삭제했지만 후속 추적 경로가 사라짐
- header / macro 주변 수정 후 빌드를 확인하지 못함
```

---

## 12. Backlog 후보

- `refactor/naming-typo-api-cleanup`: API / 매개변수 / suffix 네이밍 통일
- `refactor/type-header-helper-boundary`: Type 헤더 분리와 helper boundary 정리
- `refactor/tuning-constants-cleanup`: 상수 / CVar / DataAsset 후보 정리
- `refactor/api-const-consistency`: read-only API const 정합성 정리
- `refactor/runtime-lod-config-policy`: RuntimeLOD CVar / policy / config 위치 재검토

---
