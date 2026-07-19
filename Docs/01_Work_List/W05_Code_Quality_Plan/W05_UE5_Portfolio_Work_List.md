# UE5 Portfolio - Work List

## 제목

**W05: Code Quality 정리 계획**

## 날짜

**2026.06.28**

## 상태

- [x] **계획 확정**

---

## 브랜치

이 Work List는 하나의 대형 브랜치로 처리하지 않는다.

```yaml
우선 브랜치 순서
1. refactor/unreal-reference-safety-v1
2. refactor/character-component-reference-di
3. refactor/runtime-component-lookup-policy
4. refactor/ai-blackboard-key-registry
5. refactor/ai-update-interval-policy
6. chore/ai-profiling-test-assets
7. refactor/ai-runtime-lod-policy
8. feature/ai-alert-cap-comparison
9. feature/ai-observe-intent-state
10. feature/ai-combat-collision-profiling
11. feature/ai-combat-feedback-profiling
12. feature/ai-enemy-actor-tick-profiling
13. feature/ai-state-based-runtime-lod
14. refactor/debug-log-policy-v1
15. refactor/todo-status-cleanup
16. refactor/naming-typo-api-cleanup
17. refactor/api-const-consistency
18. refactor/tuning-constants-cleanup
19. refactor/type-header-helper-boundary
20. docs/pr-record-format-sweep

별도 후순위:
- refactor/enhanced-input-migration
```

---

## 1. Branch 목표

이번 작업 묶음은 신규 전투 기능을 추가하지 않고, 외부 리뷰 피드백과 코드 스캔에서 확인된 포트폴리오 신뢰도 리스크를 코드 품질 관점에서 줄인다.

현재 목표는 다음 세 가지다.

```text
1. 외부 리뷰에서 직접 지적된 UE C++ 기본기 리스크 제거
2. 코드 스캔에서 확인된 유지보수 / 확장성 리스크 정리
3. 이후 PR 단위 리뷰가 가능한 작은 브랜치 계획 고정
```

이번 W05의 1차 목표는 Blink / Repulse / ResultOut 기능 구현이 아니다.

```text
명시적 제외
-> Blink 실제 기능 구현
-> Repulse 실제 기능 구현
-> ResultOut 선행 일반화
-> UE TakeDamage route 제거
-> GAS 도입
-> 대규모 Player / Enemy 공통화
```

---

## 2. 완료 기준

W05 묶음은 다음 조건을 만족하면 코드 품질 1차 정리가 완료된 것으로 본다.

```yaml
완료 기준
- UObject 참조 정책이 코드에 반영되어 있다
- 필수 / 선택 컴포넌트 참조 정책이 정리되어 있다
- check / ensure / safe return / log 사용 기준이 정리되어 있다
- 조건 없는 debug print / log가 분류되어 있다
- 공용 Type 헤더 / local type / helper 분리 기준이 정리되어 있다
- 핵심 전투 흐름 주변 TODO가 제거되거나 Phase / 처리 계획이 명확하다
- 하드코딩 튜닝값이 constants / config / DataAsset 후보로 분류되어 있다
- 명백한 오타와 API 네이밍 불일치가 정리되어 있다
- const read-only API 일관성 점검이 완료되어 있다
- AI Blackboard key 수동 검증 구조의 후속 개선 방향이 정리되어 있다
- AI update interval은 구조 변경 전 측정 기준이 정리되어 있다
- 각 작업은 독립 PR로 리뷰 가능한 크기다
```

---

## 3. 필수 산출물

```yaml
Work List / Notes
- Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
- Docs/01_Work_List/00_Work_List_Index.md
- Docs/06_notes/N08_Code_Quality_Cleanup_Plan_Note.md
```

```yaml
후속 PR 문서
- 각 브랜치마다 Docs/04_Pull_Request/PXX_UE5_Portfolio_Pull_Request.md 작성
- PR 설명에는 "문제 / 수정 / 검증 / 문서 업데이트 여부"를 포함
```

---

## 4. 고정 작업 항목

이 섹션은 W05에서 다룰 작업 범위를 고정하기 위한 목록이다.

실제 진행 순서는 상단 `우선 브랜치 순서`와 하단 `PR 기준 작업 현황`을 따른다.

### 4.1 Unreal Reference Safety

**우선순위: P0**

**추천 브랜치**

```text
refactor/unreal-reference-safety-v1
```

**외부 리뷰에서 나온 내용**

- `UPROPERTY` 없는 `UObject*`는 GC 추적 문제로 면접 / 리뷰에서 바로 지적받을 수 있다.
- `UObject` 계열은 `UPROPERTY` / GC 기준으로 관리한다.
- 일반 C++ 객체는 `TSharedPtr` / `TUniquePtr` 대상이다.
- 약한 UObject 참조는 raw pointer보다 `TWeakObjectPtr`가 더 적절할 수 있다.

**코드 스캔에서 확인한 내용**

- Header 전반에 raw `UObject` / `AActor` / Component 포인터가 넓게 존재한다.
- `CAction`, `CReaction`, `CAIController`, `CAnimInstance`, `CWeaponActor`, Component 계열 캐시 포인터가 주요 점검 대상이다.

**완료 조건**

- UObject 소유 / 참조 / 약한 참조 기준을 분류한다.
- `UPROPERTY`가 필요한 필드와 의도적으로 제외할 필드를 구분한다.
- `TWeakObjectPtr` 후보를 별도 기록한다.
- 빌드 성공.

---

### 4.2 Component Reference / Initialization Policy

**우선순위: P0**

**추천 브랜치**

```text
refactor/character-component-reference-di
```

**외부 리뷰에서 나온 내용**

- `check` 기반 컴포넌트 검증은 개발 빌드 / 릴리즈 빌드 동작 차이 때문에 원인 추적을 어렵게 만들 수 있다.
- UE 컴포넌트 초기화 순서는 기대대로 보장되지 않을 수 있다.
- `ensure` + log + safe return은 빠른 완화책이다.
- 필수 의존성은 dependency injection으로 명시하는 편이 더 설득력 있다.

**코드 스캔에서 확인한 내용**

- `check`, `FindComponentByClass`, `GetComponentByClass`가 주요 Component / Action / Reaction / Notify / AI 경로에 넓게 존재한다.
- 이미 `Injected` 명명 패턴이 일부 존재하므로 확장 여지가 있다.

**완료 조건**

- 필수 컴포넌트와 선택 컴포넌트를 구분한다.
- 생성자 / BeginPlay / Initialize / Lazy resolve 기준을 정리한다.
- `check`, `ensure`, warning log, safe return 기준을 정한다.
- 실제 수정은 Combat / Action / Reaction 핵심 경로부터 좁게 적용한다.

---

### 4.3 Debug / Logging Policy

**우선순위: P0**

**추천 브랜치**

```text
refactor/debug-log-policy-v1
```

**관련 문서**

```text
N22_Debug_Log_Policy_Work_Plan_Note.md
N23_Debug_Log_And_Diagnostic_Code_Policy_Note.md
```

**외부 리뷰에서 나온 내용**

- 조건 없는 debug print / log는 Shipping 빌드와 성능 측면에서 문제가 될 수 있다.
- Hot path 또는 AI update 경로에 로그가 있으면 성능 영향을 줄 수 있다.
- debug flag, console variable, build guard 등으로 필요할 때만 켜야 한다.

**코드 스캔에서 확인한 내용**

- `FLog::Log`, `Print...Info`, `SummaryInfo` 계열 함수가 Combat / Feedback / Reaction / AI 전반에 많다.
- 일부 로그는 오류성이고, 일부는 순수 debug dump다.
- 현재 로그는 단순 출력 중심이며, 출력 책임 / 메시지 양식 / build configuration 기준이 충분히 정리되어 있지 않다.
- 구조체 payload dump, subsystem summary, visual debug tool은 이 작업의 검토 대상이지만, 기능 추가가 필요하면 별도 feature 후보로 분리한다.

**완료 조건**

- Error / Warning / Debug dump / Temporary trace를 분류한다.
- debug message format과 prefix 기준을 정한다.
- debug print 책임 위치를 정한다.
- Shipping 빌드 포함 여부와 `#if` / build configuration / console variable / debug flag 사용 기준을 정한다.
- 최소한 Combat / Feedback / AI debug dump는 gate를 가진다.
- 시각적 debug tool / UI debug panel이 필요하면 별도 feature 브랜치 후보로 분리한다.
- 기능 동작 변경 없음.

---

### 4.4 Type Header / Helper Boundary 정리

**우선순위: P0 / P1**

**추천 브랜치**

```text
refactor/type-header-helper-boundary
```

**코드 스캔에서 확인한 내용**

- `Type/CWeaponStructure.h` 같은 도메인별 Type 헤더가 여러 구조체를 한 번에 포함하고 있어, 작은 타입 하나가 필요해도 무거운 헤더를 include하게 된다.
- 공유 type과 특정 모듈 내부에서만 쓰는 local context / request / result type의 경계가 충분히 분리되어 있지 않다.
- 도메인별 Type 헤더가 include dependency를 넓혀 빌드 의존성과 변경 영향 범위를 키울 수 있다.
- validation / formatting / initialization / clear 같은 반복 helper 중 공용화하거나 local helper로 유지할 후보가 존재한다.
- 프로젝트 전반에 기능상 필요가 낮아진 helper / legacy utility / dead code 후보가 남아 있을 수 있다.

**1차 조회 대상**

```text
Source/Portfolio/Type/CWeaponStructure.h
Source/Portfolio/Type/CCombatSignalStructure.h
Source/Portfolio/Type/CActionOrchestrationStructure.h
Source/Portfolio/Type/CReactionOrchestrationStructure.h
Source/Portfolio/Type/CReactionFeedbackStructure.h
Source/Portfolio/Type/CWorldSubSystemStructure.h
Source/Portfolio/Type/CAIStructure.h
Source/Portfolio/Type/CCharacterComponentReferenceStructure.h
```

**완료 조건**

- Type 헤더를 shared public type / module-local type / private helper type 기준으로 분류한다.
- 작은 타입 하나를 위해 도메인 전체 Type 헤더를 include하는 구간을 목록화한다.
- 공용 Type 계층에 둘 type과 header-local / cpp-local namespace에 둘 type 기준을 정한다.
- 필요하면 무거운 Type 헤더를 용도별 header로 분리한다.
- forward declaration으로 충분한 곳과 full include가 필요한 곳을 구분한다.
- 반복 helper 후보를 validation / formatting / initialization / clear / conversion 기준으로 분류한다.
- 불필요하거나 사용처가 사라진 helper / utility / legacy code 후보를 목록화한다.
- 삭제 가능한 항목과 후속 검증이 필요한 항목을 구분한다.
- 기능 변경 없이 가능한 이동 / 분리만 우선 처리한다.
- USTRUCT / Blueprint exposure / serialization / asset reference 위험이 있는 type 이동은 별도 commit 또는 후속 브랜치로 분리한다.

---

### 4.5 TODO / 미완성 신호 정리

**우선순위: P0**

**추천 브랜치**

```text
refactor/todo-status-cleanup
```

**외부 리뷰에서 나온 내용**

- TODO가 많으면 미완성 포트폴리오처럼 보일 수 있다.
- TODO는 없애거나, Phase / 처리 계획 / 보류 이유가 보여야 한다.

**코드 스캔에서 확인한 내용**

- CombatSignalTarget, Health, Feedback loop, Weapon spawn, DataAsset 분리 등 핵심 흐름 주변 TODO가 존재한다.

**완료 조건**

- 핵심 runtime 경로 TODO는 제거하거나 명확한 Phase 주석으로 바꾼다.
- 구현 완료처럼 보이는 문서와 TODO 상태가 충돌하지 않게 정리한다.
- 실제 기능 구현이 필요한 TODO는 별도 브랜치 후보로 이동한다.

---

### 4.6 Hardcoded Tuning Value / Data-driven 정합성

**우선순위: P0 / P1**

**추천 브랜치**

```text
refactor/tuning-constants-cleanup
```

**외부 리뷰에서 나온 내용**

- Data-driven을 문서에서 주장하면서 튜닝값이 코드에 박혀 있으면 신뢰도가 떨어진다.
- 시야 범위, update interval, cooldown, movement speed 같은 값은 에디터 / 데이터에서 조정 가능한 편이 좋다.

**코드 스캔에서 확인한 내용**

- AI sight radius, movement speed, BT interval, combat mitigation multiplier 등이 코드에 직접 존재한다.

**완료 조건**

- 즉시 DataAsset으로 옮길 값과 constants로 둘 값을 분류한다.
- 최소한 의미 있는 상수명으로 모아 magic number 인상을 줄인다.
- DataAsset 전환이 큰 값은 후속 작업으로 분리한다.

---

### 4.7 Naming / Typo / API Consistency

**우선순위: P0 / P1**

**추천 브랜치**

```text
refactor/naming-typo-api-cleanup
```

**외부 리뷰에서 나온 내용**

- 명백한 오타는 포트폴리오 검증을 안 했다는 인상을 준다.
- Delegate / Destroy / Executor 같은 기본 단어 오타는 특히 눈에 띈다.

**코드 스캔에서 확인한 내용**

- `ReactionExcutor`, `Seperate`, `Deffered`, `InValid` 등이 확인됐다.
- 일부 API는 현재 책임명과 어색하게 맞물린다.

**완료 조건**

- 기능 변경 없이 오타와 명백한 네이밍 불일치를 정리한다.
- Blueprint / asset reference 위험이 있는 rename은 별도 확인 후 처리한다.
- `RequestAICombatSignalCue` 같은 책임명 불일치 후보는 별도 commit으로 분리한다.

---

### 4.8 Const / Read-only API Consistency

**우선순위: P1**

**추천 브랜치**

```text
refactor/api-const-consistency
```

**외부 리뷰에서 나온 내용**

- 읽기 전용 함수인데 `const`가 들쭉날쭉하면 C++ 기본기 문제로 보일 수 있다.

**코드 스캔에서 확인한 내용**

- getter / query / resolver 계열 API 전수 점검이 필요하다.

**완료 조건**

- read-only API에 `const` 적용 기준을 맞춘다.
- 내부 상태 변경이 필요한 함수는 이름 / 역할을 확인한다.
- 빌드 성공.

---

### 4.9 AI Blackboard Key Registry

**우선순위: P1**

**추천 브랜치**

```text
refactor/ai-blackboard-key-registry
```

**외부 리뷰에서 나온 내용**

- Blackboard key를 여러 곳에서 수동 초기화 / 검증하면 키 추가 시 누락 위험이 크다.
- 정의 배열 / registry 형태로 한 곳에서 관리하는 편이 좋다.

**코드 스캔에서 확인한 내용**

- `CAIKey`와 `CAIController::ValidateBlackboardKeys`가 분리되어 있고, 검증 로직이 길게 수동 작성되어 있다.

**완료 조건**

- key 정의와 required key 검증 기준을 단일화한다.
- key 추가 시 수정 지점이 줄어든다.
- Behavior Tree runtime 동작 변경 없음.

---

### 4.10 AI Update Interval / Performance Audit

**우선순위: P1**

**추천 브랜치**

```text
refactor/ai-update-interval-policy
```

**외부 리뷰에서 나온 내용**

- `0.1s`, `0.2s` AI update interval은 적 수 증가 시 성능 리스크가 될 수 있다.
- 구조 변경 전 프로파일링 근거가 필요하다.

**코드 스캔에서 확인한 내용**

- AI BT Service 다수가 `bNotifyTick = true`와 짧은 interval을 사용한다.

**완료 조건**

- 현재 interval 목록을 정리한다.
- Enemy 수 증가 시 측정 기준과 raw CSV 보관 기준을 정리한다.
- 측정 결과를 바탕으로 interval / dirty flag / LOD / perception 최적화 후보를 분리한다.
- low-risk Blackboard dirty write guard는 적용하고, 큰 구조 변경은 후속 브랜치로 분리한다.

---

### 4.11 AI Profiling Test Asset 분리

**우선순위: P1**

**추천 브랜치**

```text
chore/ai-profiling-test-assets
```

**코드 스캔 / 리뷰에서 확인한 내용**

- 공유 gameplay asset에 profiling 전용 설정이 섞이면 일반 gameplay 조건과 측정 조건이 구분되지 않는다.
- P33 측정 중 `TestRoom`, `BP_CEnemy`, `BT_Idle` 같은 공유 asset에 profiling용 변경이 들어갔고, 이후 공유 asset 변경은 제외하는 방향으로 정리했다.
- P35~P37 최적화 작업 전에 재현 가능한 profiling 전용 asset이 필요하다.

**완료 조건**

- profiling 전용 Map / Enemy / BT 후보를 구성한다.
- 공유 gameplay asset을 오염시키지 않는 측정 환경을 만든다.
- Enemy count / 배치 / collision / perception / BT 조건을 profiling 전용 asset 기준으로 재현 가능하게 한다.
- P35~P37에서 사용할 극단 비교 테스트 조건을 asset 또는 문서 기준으로 고정한다.
- runtime LOD / perception LOD / update LOD 최적화 로직은 구현하지 않는다.

---

### 4.12 AI LOD / Performance 최적화

**우선순위: P1**

**추천 브랜치**

```text
refactor/ai-runtime-lod-policy
refactor/ai-perception-lod-policy
refactor/ai-update-lod-policy
```

**측정에서 확인한 내용**

- 대량 Enemy 상황에서 Frame / GameThread 총량 증가가 확인됐다.
- BT Service 비용은 Enemy 수 증가에 따라 커졌지만, 120 Enemy까지는 전체 Frame / GameThread 부하가 먼저 한계에 가까워졌다.
- 140 Enemy 이후 perception 인지 지연이 관찰됐고, 160 Enemy 이후 OUT OF MEMORY / runtime stress limit 성격이 강해졌다.
- 따라서 단순 interval 조정보다 runtime LOD / perception LOD / update LOD를 분리해 검증해야 한다.

**완료 조건**

- P34에서 고정한 profiling 전용 환경에서 시작한다.
- 구현 전에 극단 비교 측정을 먼저 수행한다.
  - AnimInstance off
  - WeaponActor off
  - Mesh hidden
  - Collision off
  - Perception active cap
  - BT Service interval 비교
- 유의미한 측정 차이가 확인된 축부터 구현한다.
- runtime LOD, perception LOD, update LOD는 각각 별도 브랜치로 분리한다.
- 상세 계획은 `N18_AI_Performance_Bottleneck_And_LOD_Plan_Note.md`를 따른다.

---

### 4.13 Enhanced Input Migration

**우선순위: P2**

**추천 브랜치**

```text
refactor/enhanced-input-migration
```

**외부 리뷰에서 나온 내용**

- UE5.4 프로젝트에서 legacy input은 최신 엔진 흐름을 따르지 않는 인상을 줄 수 있다.

**코드 스캔에서 확인한 내용**

- `EnhancedInput` 모듈은 포함되어 있지만 실제 player input binding은 legacy `BindAxis` / `BindAction`을 사용한다.

**완료 조건**

- InputAction / MappingContext 기반으로 player input을 전환한다.
- 기존 이동 / 시점 / 장착 / 공격 / 가드 / 회피 입력 회귀 없음.
- 회귀 위험이 있으므로 P2로 분리한다.

---

## 5. 작업 원칙

```yaml
원칙
- 기능 추가보다 코드 품질 개선을 우선한다.
- 한 브랜치에는 하나의 리뷰 주제만 담는다.
- 기능 회귀 위험이 있는 변경은 P2로 분리한다.
- 코드 수정 후 관련 문서 업데이트 필요 여부를 확인한다.
- Prompt update가 필요한 경우 Docs/06_notes/prompt_updates에 별도 기록한다.
```

---

## 6. 검증 기준

```yaml
공통 검증
- Unreal C++ build 성공
- 기존 Guard / Parry / Hit / Reaction / Feedback 흐름 수동 확인
- CombatSignal damage route가 UE TakeDamage route 위에서 유지되는지 확인
- TimingCue delivery hook이 깨지지 않는지 확인
- 변경 브랜치별 PR 문서 작성
```

```yaml
문서 검증
- 외부 리뷰 지적 / 코드 스캔 결과 / 겹치는 내용이 PR 설명에 드러난다
- 구현 완료 / 보류 / Phase 2 항목을 섞지 않는다
- 새 기능 구현처럼 보이는 표현을 피한다
```

---

## 7. 후속 문서 업데이트 후보

```yaml
업데이트 후보
- Docs/06_notes/N08_Code_Quality_Cleanup_Plan_Note.md
- Docs/07_Portfolio_Documents/PF07_UE5_Portfolio_Document.md
- Docs/04_Pull_Request/00_PR_Document_Management/02_PR_Term_Usage_Map.md
- Docs/08_AI_Workflow/05_Prompt_Library/01_Prompt_Files/06_Review_Verification/Code_Review_Prompt (KR).md
```

Prompt update는 실제 코드 작업을 1회 이상 진행한 뒤 판단한다.

---

## 8. 진행 기록

### P28. Unreal Reference Safety v1

- 상태: 완료
- 브랜치: `refactor/unreal-reference-safety-v1`
- PR: `P28_UE5_Portfolio_Pull_Request.md`
- 결과: UObject / UPROPERTY / Transient / raw pointer / smart pointer 기준을 문서화하고 주요 runtime cache 필드의 reference safety 기준을 정리했다.

### P29. Character Component 참조 주입 / 복구

- 상태: 완료
- 브랜치: `refactor/character-component-reference-di`
- PR: `P29_UE5_Portfolio_Pull_Request.md`
- 결과: Character가 소유한 component reference를 `RecoverReferences -> BuildReferences -> InjectReferences` 흐름으로 정리하고, component / executor / weapon actor 참조 주입과 필수 reference validation 기준을 적용했다.
- 추가 결과: Blueprint stale native component reference 복구를 `FComponentReferenceHelper`로 분리하고, `BP_CEnemy` asset 갱신 결과를 기록했다.
- 관련 문서: `N10_Component_Reference_Validation_Policy_Note.md`, `N11_Unreal_Blueprint_Native_Component_Reference_Mismatch_Note.md`, `B14_UE5_Portfolio_Bug_Report.md`

### P30 분리 배경

다음 항목은 P29 범위에 포함하지 않고 P30에서 하나의 runtime lookup policy 작업으로 묶는다.

```text
refactor/runtime-component-lookup-policy
```

후속 범위:

```text
- Notify / NotifyState component lookup 정책
- AnimInstance component cache 기준
- BehaviorTree Service / Decorator component query 기준
- CombatSignal dynamic target lookup 허용 기준
- Runtime component lookup policy 문서화
```

### P30. Runtime Component Lookup Policy

- 상태: 완료
- 브랜치: `refactor/runtime-component-lookup-policy`
- PR: `P30_UE5_Portfolio_Pull_Request.md`
- 목표: P29 이후에도 남는 runtime lookup 경로를 Notify / AnimInstance / AI / WeaponActor 기준으로 분류하고, DI 대상과 runtime query 유지 대상을 구분한다.
- 관련 문서: `N12_Runtime_Component_Lookup_Policy_Note.md`
- 제외 범위: Blink / Repulse / ResultOut 구현, UE TakeDamage route 제거, GAS 도입, BehaviorTree 전체 재설계

검토 순서:

```text
1. Notify / NotifyState component routing 확인
2. AnimInstance owner / component cache 확인
3. AI BT Service / Decorator / Task lookup 확인
4. WeaponActor runtime reference 확인
5. FindComponentByClass / TryGetPawnOwner / Blackboard 조회 사용처 분류
6. 필요한 코드 수정만 반영
7. 문서와 PR 기록 업데이트
```

### P31. Component Lifecycle Cleanup Policy

- 상태: 완료
- 브랜치: `refactor/component-lifecycle-cleanup-policy`
- PR: `P31_UE5_Portfolio_Pull_Request.md`
- 목표: P29~P30 이후 남은 actor / component lifecycle cleanup 기준을 정리하고, `BeginPlay` / `EndPlay` / delegate / timer / spawned actor / runtime cache 정리 정책을 고정한다.
- 관련 문서: `N13_Component_Lifecycle_Cleanup_Policy_Note.md`, `N14_Dead_Destroy_And_Execution_Cleanup_Followup_Note.md`
- 제외 범위: Dead 이후 Actor Destroy 구현, Action / Reaction 실행 종료 정책 재설계, montage stop 정책 변경, Guard / Reaction runtime cleanup 재설계, 모든 injected reference 일괄 null 처리

준비 단계 조회 결과:

```text
Lifecycle hook 보유 파일: 11개
Delegate / Timer 사용 파일: 8개
SpawnActor / NewObject 생성 경로: 3개
Runtime cleanup 명명 사용처: 약 20개+
```

우선 검토 대상:

```text
1. ACAIController
   -> perception delegate bind / unbind
   -> TargetDataMap / ControlledPawn_Cached cleanup

2. UCWorldSubsystem_CombatFeedback
   -> hit stop timer handle cleanup
   -> cached time dilation restore policy

3. UCActionComponent / UCReactionComponent
   -> executor object ownership
   -> active runtime context teardown 기준
   -> execution 흐름 영향도가 크면 문서화 후 후속 분리

4. 후속 분리 후보
   -> Dead Destroy Flow
   -> Execution Runtime Cleanup Boundary
```

검증 기준:

```text
- rg 기반 lifecycle / delegate / timer / spawn 사용처 전수 확인
- git diff --check
- PortfolioEditor Win64 Development 빌드
- PIE 기본 combat loop smoke test
```

### P32. AI Blackboard Key Registry

- 상태: 완료
- 브랜치: `refactor/ai-blackboard-key-registry`
- PR: `P32_UE5_Portfolio_Pull_Request.md`
- 목표: `CAIKey` 정의를 spec 기반으로 정리하고, blackboard required key 검증 기준과 initial / clear runtime value 흐름을 정리하여 키 추가 / 삭제 시 누락 위험을 줄인다.
- 관련 문서: `N15_AI_Blackboard_Key_Registry_Policy_Note.md`, `N16_AI_Blackboard_Key_Contract_Decision_Note.md`
- 제외 범위: BehaviorTree asset 재설계, BT Service / Task 행동 로직 변경, AI update interval 튜닝, Enhanced Input migration

준비 단계 조회 결과:

```text
핵심 파일:
1. Source/Portfolio/AI/Blackboard/CAIKey.h
2. Source/Portfolio/AI/Blackboard/CAIKeyTypes.h
3. Source/Portfolio/AI/Blackboard/CAIKeyFactory.h
4. Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h
5. Source/Portfolio/AI/Blackboard/CAIBlackboardValueHelper.h
6. Source/Portfolio/Controller/CAIController.*
7. Source/Portfolio/AI/BehaviorTree/Service/*
8. Source/Portfolio/AI/BehaviorTree/Task/*
9. Source/Portfolio/AI/BehaviorTree/Decorator/*
10. Source/Portfolio/Component/CCombatSignalSourceComponent.cpp

blackboard key 직접 사용 파일:
23개
```

우선 검토 대상:

```text
1. CAIKey.h
   -> key category namespace는 유지
   -> key name / key type / required 기준은 `FAIBlackboardKeySpec`로 결합

2. CAIKeyTypes.h / CAIKeyFactory.h / CAIKeyRegistry.h / CAIBlackboardValueHelper.h
   -> key spec 타입, 생성 helper, 전체 등록 / 검증 / value 적용 책임 분리

3. ACAIController
   -> required key validation은 registry 순회로 이동
   -> InitializeBlackboardValues는 helper common 초기화 / controller custom 초기화 / helper pending 검증 흐름으로 정리
   -> fixed / owner / clear 공통 적용은 helper로 분리
   -> custom value source 적용은 controller에 유지

4. BT Service / Task / Decorator
   -> CAIKey를 직접 읽고 쓰는 runtime 사용처
   -> 이번 작업에서는 로직 변경 없이 registry 기준과 맞는지 확인
```

검증 기준:

```text
- CAIKey spec / registry / ValidateRequiredKeys 사용처 정적 확인
- InitializeBlackboardValues / ClearBlackboardValues 누락 여부 확인
- git diff --check
- PortfolioEditor Win64 Development 빌드
- PIE AI basic loop smoke test
```

### P33. AI Update Interval Profiling 정책 정리

- 상태: 완료
- 브랜치: `refactor/ai-update-interval-policy`
- PR: `P33_UE5_Portfolio_Pull_Request.md`
- 목표: Enemy AI의 BehaviorTree Service / Task polling / CombatEngage subsystem update 경로를 전수 조사하고, AI 수 증가 시 비용을 측정할 수 있는 profiling 기준을 정리한다.
- 관련 문서: `N17_AI_Update_Interval_Profiling_Policy_Note.md`
- 관련 기록: `Docs/07_Profiling/AI_Update_Interval/CSV/MANIFEST.md`
- 제외 범위: BehaviorTree asset 재설계, AI 행동 로직 변경, full dirty flag 구조 도입, event-driven Blackboard update 전환, 대규모 AI LOD / batch manager 구현

사전 조사 결과:

```text
BT Service:
1. UCBTService_UpdateAIContext
   -> Interval 0.1s
   -> perception top target / home metric / alert range / engage assignment / reaction-dead state 갱신

2. UCBTService_UpdateEngageContext
   -> Interval 0.1s
   -> engage range / combat cooldown / combat action 가능 여부 갱신

3. UCBTTask_SelectPatrolPoint
   -> ExecuteTask
   -> patrol index / patrol location / patrol reverse 갱신

4. UCBTService_UpdateInvestigateContext
   -> Interval 0.1s
   -> investigate timeout 확인

5. UCBTService_UpdateAIIntentState
   -> Interval 0.2s
   -> Blackboard 기반 AI intent state 결정

BT Task polling:
1. UCBTTask_WaitDeadState
2. UCBTTask_WaitEndCombatAction
3. UCBTTask_WaitEndReaction

Subsystem:
1. UCWorldSubsystem_CombatEngage
   -> RebuildInterval 0.1s
   -> target별 engage role assignment rebuild
```

측정 기준:

```text
- Enemy Count: 1 / 10 / 20 / 40 / 60 / 80 / 100 / 120 / 140 / 160 / 180 / 200
- State: Idle / Patrol / Engage / Boundary / Dirty Write Guard
- Duration: 30s per case
- Stats: stat unit, stat game, stat ai, stat behavior
- Capture: csvprofile start / csvprofile stop
- Run mode: PIE fullscreen, -noailogging
```

완료된 작업:

```text
1. AI update interval / polling 경로 문서화
2. CSV profiling scope 추가
3. 1 / 10 / 20 / 40 / 60 / 80 / 100 / 120 / 140 / 160 / 180 / 200 Enemy 측정 기록
4. raw CSV archive / manifest 정리
5. Blackboard dirty write guard 적용
6. dirty write guard 전후 120 Enemy 비교 측정
```

검증 기준:

```text
- rg 기반 tick / interval / polling 사용처 전수 확인
- git diff --check
- PortfolioEditor Win64 Development 빌드
- PIE AI smoke test
```

후속 분리:

```text
0. chore/ai-profiling-test-assets
   -> 공유 gameplay asset을 오염시키지 않는 profiling 전용 Map / Enemy / BT 구성
   -> P35~P37 최적화 구현 전에 측정 환경과 극단 비교 테스트 기준 고정

1. refactor/ai-runtime-lod-policy
   -> Enemy 수 증가 시 Character / mesh / weapon / movement runtime cost를 줄이는 축

2. refactor/ai-perception-lod-policy
   -> AI Perception 활성 대상 수와 감지 주기를 거리 / 중요도 기준으로 제한하는 축

3. refactor/ai-update-lod-policy
   -> BT Service / Blackboard update / CombatEngage rebuild 주기를 LOD와 연결하는 축

4. refactor/type-header-helper-boundary
   -> Type header include boundary와 helper 분리 기준 정리

5. refactor/tuning-constants-cleanup
   -> LOD / profiling 과정에서 드러난 threshold / interval / radius 값을 constants 또는 config 후보로 정리
```

사전 검증 기준:

```text
- P34는 P35~P37의 선행 준비 작업이다.
- P34에서는 최적화 로직을 구현하지 않고 profiling 전용 asset과 측정 조건을 고정한다.
- 각 최적화 브랜치는 구현 전 극단 비교 측정을 먼저 수행한다.
- AnimInstance off / WeaponActor off / Mesh hidden / Collision off / Perception cap 같은 비교로 유효성을 확인한다.
- 유의미한 측정 차이가 확인된 축부터 구현한다.
- 상세 계획은 N18_AI_Performance_Bottleneck_And_LOD_Plan_Note.md를 따른다.
```

## 9. PR 기준 작업 현황

### 완료

```text
Unreal / Reference Safety
- P28: Unreal Reference Safety 정책 정리

Component Reference / Runtime Lookup
- P29: Character Component Reference DI 정리
- P30: Runtime Component Lookup 정책 정리

Lifecycle / Cleanup
- P31: Component Lifecycle Cleanup 정책 정리

AI Blackboard 구조
- P32: AI Blackboard Key Registry 정책 정리

AI Profiling / Bottleneck 분석
- P33: AI Update Interval Profiling 정책 정리
- P34: AI Profiling Test Asset 분리

AI LOD / Performance 최적화
- P35: AI Runtime LOD 정책 정리
  -> Enemy mesh / animation / WeaponActor / perception / movement / BT service interval 축 분리 측정 완료
  -> CombatEngage assignment gate / lease / warmup 기반 Engage 2 / Alert 6 / Idle 계층 안정화
  -> AIContext / AIIntentState interval split 정책 후보 정리
- P36: AI AlertCap 비교 측정 및 Assignment Cap 제어 추가
  -> AlertCap 6 / 40 비교로 Alert 후보 수가 CharacterMovement cost를 증가시키는 축임을 확인
- P37: Observe Intent / Investigate Lifecycle 정리
  -> CombatRole 없는 인지 대상은 Observe로 대기
  -> Investigate 진입 / 실행 / 종료 요청 flag 분리
- P38: AI Combat Collision / Hit Window 비용 분리 측정
  -> HitWindow / Overlap / HitProcessing / CombatSignal route 계측
  -> HitProcessing 차단은 동작하지만 Frame / Game p95 개선이 작아 Runtime LOD v1 우선 후보에서 제외
- P39: AI Combat Feedback Presentation 비용 분리 측정
  -> Enemy action feedback presentation gate / Trail / VFX / SFX counter 분리
  -> 40 / 80 Enemy 기준 Frame / Game p95 개선이 제한적이므로 최하위 representation 후보로 유지
- P40: AI Enemy Actor Tick 비용 분리 측정
  -> Enemy Actor Tick off 효과를 분리 측정
  -> ACEnemy Tick 자체보다 Movement / Animation 변동이 Frame / Game p95에 더 크게 관여함을 확인
- P41: AI State Runtime LOD tier snapshot 통합
  -> Blackboard 기반 Runtime LOD tier snapshot을 controller / animation / BT interval 정책에 연결
  -> AIContext interval은 stale tier feedback을 피하기 위해 고정 주기로 유지
```

### 진행 중

```text
없음
```

### 진행 예정

```text
Code Quality Sweep
- Debug Log Policy
  -> hot path log / debug dump / error log 분류
  -> build config / CVar / debug flag 기준 정리
- TODO Status Cleanup
  -> 핵심 runtime 경로 TODO 제거 또는 Phase / 후속 작업 후보로 분류
- Naming / Typo / API Cleanup
  -> 명백한 오타와 API naming 불일치 정리
  -> Blueprint / asset 영향 rename은 별도 판단
- API Const Consistency
  -> read-only API const 정합성 점검
- Tuning Constants Cleanup
  -> AI / Combat radius, interval, threshold 값을 constants / config / DataAsset 후보로 분류
- Type Header / Helper Boundary 정리
  -> 공유 Type 헤더와 helper 책임 경계 점검

Documentation / PR Record
- PR Record Format Sweep

Deferred Performance Follow-up
- Perception Active Budget / Cap 검토
- Proxy / Dormant Actor 최적화 검토
- Dormant / wake-up 정책 구현

별도 후순위
- Enhanced Input Migration
```

상세 공유용 정리는 `N19_Code_Quality_PR_Status_Summary_Note.md`를 따른다.
