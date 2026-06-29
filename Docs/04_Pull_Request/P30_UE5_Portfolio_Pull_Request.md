# UE5 Portfolio Pull Request

## 제목

**P30: Runtime Component 조회 정책**

## 날짜

**2026.06.29**

## 상태

- [ ] 진행 중

---

## 브랜치

- `refactor/runtime-component-lookup-policy`

---

## 커밋

```text
TBD
```

---

## 요약

이번 PR은 P29에서 정리한 Character component reference DI 이후에도 남는 runtime lookup 경로를 분류하고, 유지 / 수정 / 문서화 기준을 정리한다.

핵심 목표는 모든 lookup 제거가 아니다.

```text
Character-owned required component
-> DI / injected reference

runtime target / external actor
-> request 시점 lookup 유지

AnimInstance owner / mesh-bound reference
-> animation lifecycle 기준 cache

AI Blackboard / BehaviorTree state
-> runtime query 유지
```

---

## 변경 배경

P29에서 Character가 소유한 component reference는 다음 흐름으로 정리했다.

```text
RecoverReferences
-> BuildReferences
-> InjectReferences
-> InitializeReferences
-> ValidateRequiredComponentReferences
```

하지만 프로젝트에는 다음처럼 runtime lookup이 자연스러운 영역이 남아 있다.

```text
- Notify / NotifyState routing
- AnimInstance owner / movement component cache
- AI BT Service / Decorator / Task의 Blackboard / Pawn / component query
- CombatSignal dynamic target resolve
- WeaponActor overlap target 해석
```

이번 PR은 이 영역을 “전부 DI로 바꾸는 작업”이 아니라, lookup이 필요한 이유와 실패 처리 기준을 명확히 하는 작업이다.

---

## 작업 범위

### 1. Notify / NotifyState routing

검토 기준:

```text
Notify는 timing trigger 이상을 알지 않는다.
Notify는 domain 판단을 하지 않는다.
Notify는 필요한 경우 component routing entry까지만 호출한다.
```

검토 대상:

```text
Source/Portfolio/Notify
```

진행 내용:

```text
Action Notify
-> 기존 ActionBase routing 유지

Reaction Feedback Notify
-> ReactionBase / ReactionStateBase 추가
-> UCReactionComponent notify routing API 호출로 정리

Health State Notify
-> HealthBase 추가
-> UCHealthComponent notify routing API 호출로 정리

Collision Window Notify
-> WeaponActor 직접 resolve 제거
-> UCActionComponent -> UCWeaponComponent -> ACWeaponActor 순서로 routing 정리
```

### 2. AnimInstance reference / cache

검토 기준:

```text
AnimInstance는 ActorComponent DI 흐름과 다르다.
TryGetPawnOwner() 기반 owner 확인은 animation lifecycle에 맞는 runtime lookup이다.
NativeUpdateAnimation에서는 invalid owner / component 상황을 safe return으로 처리한다.
```

검토 대상:

```text
Source/Portfolio/Character/CAnimInstance.*
```

진행 내용:

```text
UCAnimInstance
-> TryGetPawnOwner 기반 owner resolve 유지
-> component cache / clear / bind / unbind helper로 분리
-> movement / state parameter refresh helper로 분리
-> GetComponentByClass 사용을 FindComponentByClass<T>()로 정리
```

### 3. AI BehaviorTree lookup

검토 기준:

```text
Blackboard value 조회는 runtime query로 유지한다.
Service / Decorator / Task의 역할에 따라 lookup 실패 처리를 분리한다.
반복 lookup은 필요한 경우 local variable 또는 cache 기준을 둔다.
```

검토 대상:

```text
Source/Portfolio/AI
```

진행 내용:

```text
BT Service / Task
-> AI owner / Pawn runtime query 유지
-> 같은 함수 안의 반복 GetAIOwner()->GetPawn() 패턴을 local variable로 정리
-> owner component 조회는 FindComponentByClass<T>()로 통일

BT Decorator
-> Blackboard / Pawn / component 조건 조회 유지
-> 조회 실패 시 false 반환 정책 유지
```

### 4. WeaponActor runtime reference

검토 기준:

```text
WeaponActor는 Character component가 아니라 runtime actor다.
Owner / CombatSignalSource는 WeaponComponent가 생성 직후 주입한다.
Overlap target은 event payload에서 해석한다.
```

검토 대상:

```text
Source/Portfolio/Weapon
```

진행 내용:

```text
UCWeaponComponent
-> spawned WeaponActor owner로서 EndPlay cleanup 추가
-> runtime weapon state(collision / trail) 정리 후 spawned actor Destroy

ACWeaponActor
-> collision delegate bind/cache는 BeginPlay에서 유지
-> EndPlay / runtime state clear 경로에서 collision window와 trail을 명시적으로 정리
-> EndPlay에서 collision delegate unbind / collision cache clear
-> injected owner/source reference clear
```

후속 보완 대상:

```text
Actor / Component teardown cleanup
-> EndPlay delegate / timer / spawned actor cleanup 기준은 별도 refactor에서 처리
-> WeaponActor collision cleanup과 gameplay collision close API 분리 여부는 후속 작업으로 분리
```

### 5. Lookup 사용처 분류

대상 API:

```text
FindComponentByClass
GetComponentByClass
TryGetPawnOwner
GetOwner
GetController
Blackboard GetValueAs...
```

분류:

```text
DI 대상
runtime lookup 유지
cache 필요
safe return / reject 필요
후속 브랜치 분리
```

진행 내용:

```text
GetComponentByClass
-> Source/Portfolio C++ 코드 기준 잔여 사용처 없음

FindComponentByClass
-> Notify base/helper, AnimInstance cache, AI owner component query, CombatSignal target resolve처럼 runtime context가 필요한 경로에만 유지

Component order
-> Character reference 구성/주입 계열은 canonical component order를 기준으로 정렬
-> runtime gameplay flow는 domain 처리 순서를 우선

현재 예외
-> ACEnemy는 DefenseComponent를 아직 보유하지 않음
-> 적 방어 기능이 필요해지는 시점에 추가
-> CombatSignalTarget의 Defense reference는 현재 optional dependency로 유지
```

---

## 주요 파일

```text
Docs/06_notes/N12_Runtime_Component_Lookup_Policy_Note.md
Docs/04_Pull_Request/P30_UE5_Portfolio_Pull_Request.md
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/04_Pull_Request/00_Pull_Request_Index.md
```

코드 파일은 실제 검토 후 필요한 범위만 추가한다.

---

## 검증 계획

```text
- rg 기반 lookup 사용처 전수 스캔
- Notify / AnimInstance / AI / WeaponActor 경로별 코드 확인
- 변경이 발생한 경우 Unreal C++ build
- PIE에서 기본 combat loop smoke test
```

---

## 관련 문서

```text
Docs/06_notes/N12_Runtime_Component_Lookup_Policy_Note.md
Docs/06_notes/N10_Component_Reference_Validation_Policy_Note.md
Docs/06_notes/N11_Unreal_Blueprint_Native_Component_Reference_Mismatch_Note.md
Docs/02_Bug_Report/B14_UE5_Portfolio_Bug_Report.md
```

---

## 제외 범위

```text
- Blink 실제 기능 구현
- Repulse 실제 기능 구현
- ResultOut 구조 선행 일반화
- UE TakeDamage route 제거
- GAS 도입
- BehaviorTree 전체 재설계
- AnimBP 구조 재작성
```
