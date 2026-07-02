# UE5 Portfolio - Code Quality PR Status Summary Note

## 목적

이 문서는 코드 품질 정리 작업의 현재 진행 상황과 다음 작업 순서를 PR 기준으로 공유하기 위해 작성한다.

## PR 진행 현황

```text
완료
- P28 Unreal Reference Safety 정책 정리
- P29 Character Component Reference DI 정리
- P30 Runtime Component Lookup 정책 정리
- P31 Component Lifecycle Cleanup 정책 정리
- P32 AI Blackboard Key Registry 정책 정리
- P33 AI Update Interval Profiling 정책 정리
- P34 AI Profiling Test Asset 분리

다음 작업
- P35 AI Runtime LOD 정책 정리

후속 작업
- P36 AI Perception LOD 정책 정리
- P37 AI Update LOD 정책 정리
- P38 Type Header / Helper Boundary 정리
- P39 Tuning Constants Cleanup
- P40 API Const Consistency
- P41 Debug Log Policy
- P42 Naming / Typo / API Cleanup
- P43 TODO Status Cleanup
- P44 PR Record Format Sweep
```

정리 기준은 다음과 같다.

```text
완료
-> 이미 PR merge까지 완료된 작업

다음 작업 / 후속 작업
-> 다음 PR로 진행할 작업

카테고리
-> 작업 성격이 같은 PR을 묶어 리뷰 흐름을 이해하기 쉽게 분류
```

---

## 완료

### 1. Unreal / Reference Safety

#### P28: Unreal Reference Safety 정책 정리

정리 내용:

```text
UObject / UPROPERTY / Transient 사용 기준 정리
raw pointer / smart pointer 사용 기준 정리
Unreal GC와 일반 C++ 객체 관리 기준 분리
```

의도:

```text
Unreal C++ reference safety 기준을 먼저 정리한다.
```

---

### 2. Component Reference / Runtime Lookup

#### P29: Character Component Reference DI 정리

정리 내용:

```text
Player / Enemy component reference 주입 흐름 정리
_Injected / _Cached 기준 정리
Blueprint native component stale reference 대응 기록
```

의도:

```text
Character가 소유한 필수 component reference를 명시적으로 주입하고,
Blueprint stale reference 문제에 대한 방어선을 만든다.
```

#### P30: Runtime Component Lookup 정책 정리

정리 내용:

```text
Notify / AnimInstance / AI / WeaponActor lookup 흐름 분류
Character-owned component DI와 runtime lookup 경계 정리
외부 target resolve와 owner component reference를 구분
```

의도:

```text
모든 lookup을 제거하는 것이 아니라,
DI 대상과 runtime resolve 대상을 구분해 구조적 설득력을 높인다.
```

---

### 3. Lifecycle / Cleanup

#### P31: Component Lifecycle Cleanup 정책 정리

정리 내용:

```text
BeginPlay / EndPlay cleanup 기준 정리
OnPossess / OnUnPossess cleanup 기준 정리
AIController / ActionComponent / ReactionComponent / WeaponComponent / subsystem teardown 흐름 정리
```

의도:

```text
runtime setup과 teardown의 짝을 명확히 하고,
delegate / timer / spawned actor / runtime cache 정리 책임을 드러낸다.
```

---

### 4. AI Blackboard 구조

#### P32: AI Blackboard Key Registry 정책 정리

정리 내용:

```text
Blackboard key spec / registry / validation 구조 정리
initial / custom / clear blackboard value helper 분리
Blackboard key 추가 시 누락 위험 감소
```

의도:

```text
BehaviorTree와 C++ 사이의 key 계약을 한 곳에서 확인하고 검증한다.
```

---

### 5. AI Profiling / Bottleneck 분석

#### P33: AI Update Interval Profiling 정책 정리

정리 내용:

```text
BT Service / Blackboard / CombatEngage update 경로 계측
1~200 Enemy 기준 CSV profiling 기록
Blackboard dirty write guard 적용
대량 Enemy 기준 병목 축 분류
후속 LOD 최적화 작업 순서 정리
```

의도:

```text
AI update interval을 감으로 조정하지 않고,
측정 결과를 바탕으로 runtime LOD / perception LOD / update LOD 후속 작업을 분리한다.
```

---

## 다음 작업

### 6. AI LOD / Performance 최적화

#### P35: AI Runtime LOD 정책 정리

계획 브랜치:

```text
refactor/ai-runtime-lod-policy
```

작업 범위:

```text
Enemy 거리 / 중요도 기준 runtime LOD 산출
WeaponActor / collision / movement / mesh / component tick 비활성 효과 검증
AnimInstance off / WeaponActor off / Mesh hidden / Collision off 극단 비교 측정
```

관련 문서:

```text
N18_AI_Performance_Bottleneck_And_LOD_Plan_Note.md
N20_AI_Profiling_Test_Asset_Plan_Note.md
```

의도:

```text
Enemy 수 증가 시 Character / mesh / weapon / movement / collision runtime cost를 줄인다.
```

---

#### P36: AI Perception LOD 정책 정리

계획 브랜치:

```text
refactor/ai-perception-lod-policy
```

작업 범위:

```text
AI Perception 활성 대상 수 제한
거리 / 중요도 기반 sight activation 또는 interval 조정
Perception active cap 극단 비교 측정
```

의도:

```text
대량 Enemy 상황에서 perception 부하와 인지 지연을 줄인다.
```

#### P37: AI Update LOD 정책 정리

계획 브랜치:

```text
refactor/ai-update-lod-policy
```

작업 범위:

```text
BT Service / Blackboard update / CombatEngage rebuild 주기를 LOD와 연결
service interval 0.1s / 0.2s / 0.5s 비교 측정
dirty flag / event-driven 전환 범위 검토
```

의도:

```text
AI update 비용을 거리 / 중요도 / 전투 참여도에 맞춰 단계적으로 줄인다.
```

---

### 7. Code Quality Sweep

#### P38: Type Header / Helper Boundary 정리

계획 브랜치:

```text
refactor/type-header-helper-boundary
```

작업 범위:

```text
Type/CWeaponStructure.h 같은 도메인 Type 헤더의 include 범위 점검
공유 type / local type / transient request / result type 분류
작은 type 하나를 위해 무거운 도메인 Type 헤더를 include하는 구간 목록화
전역 Type 계층에 둘 type과 header-local / cpp-local type 기준 정리
validation / formatting / debug dump / initialization / clear helper 분리 기준 정리
```

의도:

```text
공유 Type 헤더의 include 범위를 줄이고,
type 위치와 helper 책임을 명확히 해 빌드 의존성과 변경 영향 범위를 줄인다.
```

#### P39: Tuning Constants Cleanup

계획 브랜치:

```text
refactor/tuning-constants-cleanup
```

작업 범위:

```text
AI radius / interval / threshold / combat tuning value 정리
constants / config / DataAsset 분류
```

#### P40: API Const Consistency

계획 브랜치:

```text
refactor/api-const-consistency
```

작업 범위:

```text
read-only API const 정합성 점검
불필요한 mutable 접근 정리
```

#### P41: Debug Log Policy

계획 브랜치:

```text
refactor/debug-log-policy-v1
```

작업 범위:

```text
hot path log / debug dump / error log 분류
build config 또는 debug flag 기준 정리
debug message format / prefix 기준 정리
debug print 책임 위치 정리
전처리 / console variable / debug flag 사용 기준 정리
시각적 debug tool / UI debug panel 필요 여부 분류
```

의도:

```text
무조건 출력하는 로그를 줄이는 수준을 넘어,
디버그 정보의 책임 위치와 빌드별 포함 기준을 명확히 한다.
시각적 디버그 툴은 feature 성격으로 별도 분리한다.
```

#### P42: Naming / Typo / API Cleanup

계획 브랜치:

```text
refactor/naming-typo-api-cleanup
```

작업 범위:

```text
오타 정리
include casing 정리
API naming 불일치 정리
```

#### P43: TODO Status Cleanup

계획 브랜치:

```text
refactor/todo-status-cleanup
```

작업 범위:

```text
핵심 runtime 경로 TODO 정리
Phase / 보류 / 후속 작업 상태 명확화
```

---

### 8. Documentation / PR Record

#### P44: PR Record Format Sweep

계획 브랜치:

```text
docs/pr-record-format-sweep
```

작업 범위:

```text
PR 문서 형식 통일
KR / EN 혼용 정리
표 가독성 정리
커밋 기록 방식 정리
```

---

## 현재 우선순위

가장 가까운 다음 작업은 P35다.

```text
P35
-> P34에서 고정한 profiling 환경에서 runtime LOD 축을 검증하고 구현한다.

P36~P37
-> perception LOD / update LOD 축은 P35 이후 별도 브랜치로 진행한다.
```

각 축은 구현 전에 유의미한 성능 차이를 만드는지 먼저 확인한 뒤 진행한다.
