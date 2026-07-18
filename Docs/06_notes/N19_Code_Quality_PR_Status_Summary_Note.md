# UE5 Portfolio - Code Quality PR Status Summary Note

## 목적

이 문서는 코드 품질 정리 작업의 현재 진행 상황과 후속 작업 순서를 PR 기준으로 공유하기 위해 작성한다.

`W05_Code_Quality_Plan`에서 시작된 작업은 중간에 AI Runtime LOD / profiling 축으로 확장되었고, P39~P41은 원래 예상했던 Type Header / Tuning / Const 작업이 아니라 AI 성능 측정과 Runtime LOD 정책 정리에 사용되었다.

따라서 이 문서는 현재 실제 PR 흐름을 기준으로 상태를 다시 정리한다.

---

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
- P35 AI Runtime LOD 정책 정리
- P36 AI AlertCap 비교 측정 및 Assignment Cap 제어 추가
- P37 AI Observe Intent 및 Investigate Lifecycle 정리
- P38 AI Combat Collision / Hit Window 비용 분리 측정
- P39 AI Combat Feedback Presentation 비용 분리 측정
- P40 AI Enemy Actor Tick 비용 분리 측정
- P41 AI State Runtime LOD tier snapshot 통합

후속 작업
- Debug Log Policy
- TODO Status Cleanup
- Naming / Typo / API Cleanup
- API Const Consistency
- Tuning Constants Cleanup
- Type Header / Helper Boundary 정리
- PR Record Format Sweep

별도 후순위
- Enhanced Input Migration
- Perception Active Budget / Cap
- Proxy / Dormant Actor 최적화
```

정리 기준은 다음과 같다.

```text
완료
-> 구현 / 검증 / PR 문서 작성까지 완료된 작업

후속 작업
-> 이후 PR로 진행할 작업

별도 후순위
-> 현재 코드 품질 sweep에서 바로 처리하지 않고, 별도 기능 / 성능 설계로 넘길 작업
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

현재 판단:

```text
완료된 항목으로 본다.
후속 작업에서 Blackboard key를 추가 / 제거할 경우 registry 계약을 같이 갱신해야 한다.
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

현재 판단:

```text
기본 audit은 완료된 항목으로 본다.
이후 P35~P41에서 BT interval / assignment cap / state tier 기반 Runtime LOD로 확장 검증했다.
```

---

### 6. AI LOD / Performance 최적화

#### P34: AI Profiling Test Asset 분리

정리 내용:

```text
공유 gameplay asset을 오염시키지 않는 AI profiling 전용 asset 흐름 정리
측정 맵 / 적 수 / 고정 카메라 / csvprofile 기준 정리
```

#### P35: AI Runtime LOD 정책 정리

정리 내용:

```text
Enemy mesh / animation / WeaponActor / perception / movement / BT service interval 축 분리 측정
CombatEngage assignment gate / lease / warmup 기반 Engage 2 / Alert 6 / Idle 계층 안정화
AIContext / AIIntentState interval split 정책 후보 정리
```

#### P36: AI AlertCap 비교 측정 및 Assignment Cap 제어 추가

정리 내용:

```text
EngageCap / AlertCap CVar 추가
AlertCap 6 / 40 비교 측정
Alert 후보 수 증가가 CharacterMovement cost를 증가시키는지 확인
```

결론:

```text
AlertCap 증가는 BT Tick보다 CharacterMovement p95에 더 크게 반영됐다.
Runtime LOD에서 movement 후보 수를 제한해야 하는 근거로 사용한다.
```

#### P37: AI Observe Intent 및 Investigate Lifecycle 정리

정리 내용:

```text
CombatRole 없는 인지 대상은 Observe로 대기
bCanInvestigate 제거
bShouldInvestigate / bIsInvestigating / bShouldEndInvestigate lifecycle 분리
40 / 80 Enemy smoke 측정
```

#### P38: AI Combat Collision / Hit Window 비용 분리 측정

정리 내용:

```text
Combat Collision / Hit Window event 계측
event count를 tick phase에서 FlushToCsv로 기록
DisableEnemyHitProcessing gate 추가
40 / 80 Enemy FullCombat vs HitProcessingDisabled 비교
EngageCap 4 stress 참고 측정
```

결론:

```text
HitProcessing / CombatSignal 차단은 정상 동작했지만 Frame / Game p95 개선은 작았다.
Combat Collision / HitProcessing은 Runtime LOD v1 우선 제어 후보로 보지 않고 후순위로 닫는다.
```

#### P39: AI Combat Feedback Presentation 비용 분리 측정

정리 내용:

```text
Enemy action feedback presentation gate 추가
Trail / VFX / SFX counter와 TrailClear cleanup counter 분리
40 / 80 Enemy 기준 FeedbackBaseline vs FeedbackDisabled 쌍 측정
```

결론:

```text
Combat Feedback Presentation은 기능적으로 분리 가능하다.
하지만 40 / 80 Enemy 조건에서 Frame / Game p95 개선은 유의미하지 않았다.
Runtime LOD v1 핵심 병목 축이 아니라 최하위 representation 단계의 선택 후보로 둔다.
```

#### P40: AI Enemy Actor Tick 비용 분리 측정

정리 내용:

```text
Enemy Actor Tick off mode 추가
Actor Tick이 Runtime LOD Movement guard를 소유하던 구조를 분리
Movement Runtime LOD 제어는 MovementComponent 책임으로 이동
40 / 80 Enemy 기준 Actor Tick off 효과 측정
```

결론:

```text
Enemy Actor Tick 자체 제거만으로 Frame / Game p95가 안정적으로 개선되지는 않았다.
측정 변동은 Movement / Animation 쪽과 더 강하게 연결되어 있다고 본다.
```

#### P41: AI State Runtime LOD tier snapshot 통합

정리 내용:

```text
Blackboard 기반 Runtime LOD tier snapshot 추가
Controller / Animation / BT interval 정책이 같은 tier snapshot을 사용하도록 정리
AIContext interval은 stale tier feedback을 피하기 위해 고정 주기로 유지
Animation Runtime LOD 정책을 별도 policy helper로 분리
```

결론:

```text
축별 계측에서 얻은 결과를 실제 Runtime LOD tier 정책으로 통합하는 1차 구조를 마련했다.
Dormant / Proxy / Perception Active Budget은 별도 장기 작업으로 남긴다.
```

---

## 후속 작업

### 1. Debug Log Policy

추천 브랜치:

```text
refactor/debug-log-policy-v1
```

관련 문서:

```text
N22_Debug_Log_Policy_Work_Plan_Note.md
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

### 2. TODO Status Cleanup

추천 브랜치:

```text
refactor/todo-status-cleanup
```

작업 범위:

```text
핵심 runtime 경로 TODO 정리
Phase / 보류 / 후속 작업 상태 명확화
문서의 후속 작업 후보와 코드 주석 상태 일치
```

### 3. Naming / Typo / API Cleanup

추천 브랜치:

```text
refactor/naming-typo-api-cleanup
```

작업 범위:

```text
오타 정리
include casing 정리
API naming 불일치 정리
Blueprint / asset 영향 rename은 별도 판단 후 분리
```

### 4. API Const Consistency

추천 브랜치:

```text
refactor/api-const-consistency
```

작업 범위:

```text
read-only API const 정합성 점검
불필요한 mutable 접근 정리
query / resolver / getter API const 적용
```

### 5. Tuning Constants Cleanup

추천 브랜치:

```text
refactor/tuning-constants-cleanup
```

작업 범위:

```text
AI radius / interval / threshold / combat tuning value 정리
constants / config / DataAsset 후보 분류
대규모 DataAsset 전환은 후속 작업으로 분리
```

### 6. Type Header / Helper Boundary 정리

추천 브랜치:

```text
refactor/type-header-helper-boundary
```

작업 범위:

```text
공유 Type 헤더 include 범위 점검
local type / shared type / helper type 위치 기준 정리
validation / formatting / debug dump / initialization / clear helper 분리 기준 정리
```

### 7. PR Record Format Sweep

추천 브랜치:

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

## 별도 후순위

```text
Enhanced Input Migration
-> 입력 시스템 구조 변경 성격이 강하므로 코드 품질 1차 sweep과 분리한다.

Perception Active Budget / Cap
-> Dormant / wake-up 정책과 연결되는 성능 설계 작업으로 분리한다.

Proxy / Dormant Actor 최적화
-> 단순 측정 축이 아니라 실제 Runtime LOD 적용 단계이므로 장기 작업으로 분리한다.
```

---

## 현재 우선순위

현재는 AI Runtime LOD 측정과 1차 정책 통합이 P41까지 완료된 상태다.

다음 작업은 남은 code quality sweep으로 복귀한다.

```text
1. Debug Log Policy
2. TODO Status Cleanup
3. Naming / Typo / API Cleanup
4. API Const Consistency
5. Tuning Constants Cleanup
6. Type Header / Helper Boundary
7. PR Record Format Sweep
```

현재 우선순위는 `Debug Log Policy`다.
