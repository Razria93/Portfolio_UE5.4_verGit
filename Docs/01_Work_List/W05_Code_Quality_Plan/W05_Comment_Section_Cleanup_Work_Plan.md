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
- UPROPERTY 변수 구간 섹션 주석 정리
- 코드가 그대로 설명되는 중복 주석 제거
- commented-out debug trace 잔존 여부 확인

제외 범위:
- API rename
- USTRUCT / header 구조 이동
- DataAsset 분리 구현
- UPROPERTY Category 재설계
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
- UPROPERTY 변수 구간은 섹션 주석보다 Category 기준으로 관리하는 편이 에디터 노출 의도와 맞음

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
- UPROPERTY Category 정리: 별도 category naming / editor exposure 정리
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

- [x] `CAIController` perception config DataAsset 후보 TODO 분류
- [x] `CPlayer` / `CEnemy` DeadFlag early return TODO 분류
- [x] `CActionFeedbackComponent` / `CReactionFeedbackComponent` loop support TODO 분류
- [x] `CWeaponComponent` deferred spawn TODO 분류
- [x] `CActionComponent` / `CReactionComponent` DataAsset build TODO 분류
- [x] `CCombatSignalSourceComponent` DamageSpecContainer DataAsset migration TODO 분류
- [x] `CHealthComponent` ResourceComponent extension / delegate broadcast TODO 분류
- [x] `CWeaponStructure` feedback / action execution context / 미분류 TODO 정리

### 4) 섹션 주석 양식 정리

- [x] 변수 / `UPROPERTY` 구간은 섹션 주석 대신 Category / 변수명 / struct 이름 기준으로 관리
- [x] `CWeaponActor.h`의 `// ===`, `/* === */`, 중복 `AnimNotify Events` 섹션 정리
- [x] `CAIController.h`의 `/* --- Asset --- */`와 `// Lifecycle` 스타일 혼용 정리
- [x] `CAnimInstance.h`의 RuntimeLOD 번호 주석을 의미 기반 섹션으로 정리
- [x] `CEnemy.h`의 RuntimeLOD 번호 주석을 의미 기반 섹션으로 정리
- [x] `CMovementComponent.h`의 RuntimeLOD 번호 주석을 의미 기반 섹션으로 정리
- [x] `CWeaponComponent.h` / `CStateComponent.h` / `CHealthComponent.h` 변수 섹션 주석 정리
- [x] `CActionComponent.h` / `CReactionComponent.h` 변수 섹션 주석 정리
- [x] `CPlayerFeedbackComponent.h` / `CHitFeedbackComponent.h` 변수 섹션 주석 정리
- [x] `CCombatSignalSourceComponent.h` / `CCombatSignalTargetComponent.h` 변수 섹션 주석 정리
- [x] `Core/Debug` / `Core/Profiling` helper 섹션명은 필요 시 최소 보정

### 5) 불필요한 설명 주석 제거

- [x] `CPlayer.cpp` 생성자 init 주석 중 코드 반복 설명 제거
- [x] `CEnemy.cpp` 생성자 init 주석 중 코드 반복 설명 제거
- [x] `CWeaponComponent.cpp` spawn 단계 번호 주석을 압축 또는 제거
- [x] `CBTServiceIntervalHelper.cpp`의 단순 return 설명 주석 제거
- [x] `CCombatSignalStructure.h`의 필드명 반복 UPROPERTY 주석 정리
- [x] `CWeaponStructure.h`의 `[NOTE] Temp`, 개인 체크리스트성 주석, 단순 단계 주석 정리

### 6) API / inline role comment 유효성 검토

- [x] `Incoming API`, `Active API`, `Getter`, `Setter` 같은 inline role comment가 실제 책임을 설명하는지 확인
- [x] 의미가 코드명과 중복되면 제거
- [x] 의미가 불명확하면 실제 역할 기준으로 수정
- [x] public API rename이 필요한 수준이면 이번 브랜치에서 수정하지 않고 네이밍 작업으로 분리

### 7) 잔존 trace 확인

- [x] commented-out `UE_LOG` 잔존 여부 확인
- [x] commented-out `DrawDebug` 잔존 여부 확인
- [x] commented-out `GEngine` 잔존 여부 확인
- [x] 임시 debug trace성 주석 잔존 여부 확인

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
- feedback data type 이동
- Health / ResourceComponent 확장

UPROPERTY / Editor 노출 정리:
- UPROPERTY Category naming 통일
- 변수 구간 분류는 Category 기준으로 재검토
- Category 변경에 따른 에디터 표시 / asset 영향 확인

상수 제거 / RuntimeLOD 구조 정리:
- CEnemy RuntimeLOD CVar 위치 이동
- RuntimeLOD policy/config 재구성

기능 수정:
- DeadFlag early return 동작 변경
- feedback loop support 구현
- deferred spawn 정책 구현
- AI combo branch의 action index 사용 정책
- CombatEngage coordinator naming / responsibility 재검토
- combat result UI feedback 추가 여부
```

---

## 7. 완료 기준

- [x] 전수조사 후보가 이번 브랜치 범위 / 후속 범위로 분류되어 있다
- [x] 오타와 stale comment가 코드 상태와 충돌하지 않는다
- [x] TODO는 구현 필요 여부와 후속 카테고리를 추적할 수 있다
- [x] 섹션 주석은 같은 파일 안에서 일관된 양식을 가진다
- [x] 단순히 코드 내용을 반복하는 주석이 줄어 있다
- [x] commented-out temporary trace가 남아 있지 않음을 확인했다
- [x] 코드 동작 변경 없이 diff가 주석 / 문자열 / 문서 중심으로 제한되어 있다

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
- 함수 구간 섹션 주석은 파일별 기존 스타일을 우선하되, 같은 파일 안에서는 혼용을 줄인다.
- 변수 / UPROPERTY 구간은 섹션 주석을 줄이고 Category / 변수명 / struct 이름으로 의미를 표현한다.

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
## 13. Comment Usage Rules

이번 브랜치 이후 주석은 "코드가 직접 말하지 못하는 이유 / 정책 / 예외 / 구역"만 남긴다.
단순히 코드 동작을 반복하는 주석, 임시 trace, stale 상태 설명, 장식성 banner는 제거 대상이다.

### 1) Engine / Template Header

UE 템플릿 copyright 주석은 유지한다.
그 외 파일 상단 설명 주석은 별도 필요가 없으면 추가하지 않는다.

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "Portfolio.h"
```

### 2) Debug Helper

Debug helper는 API 구역을 구분하기 위한 짧은 명사구 섹션만 허용한다.
출력 정책은 함수명, CVar, helper 구조로 표현하고 본문 설명 주석은 최소화한다.

```cpp
// Gate

bool FCombatSignalDebug::ShouldAuditCombatSignal()
{
	return CVarCombatSignalAudit.GetValueOnGameThread() != 0;
}

// Source Diagnostic Hook

void FCombatSignalDebug::RecordSourceRejectedForAudit(...)
{
	if (!ShouldAuditCombatSignal()) return;
}
```

### 3) Profiling Helper

Profiling helper는 gate / audit / counter / CSV 구역 구분에 사용한다.
counter 종류가 여러 개면 구체적인 counter 이름을 섹션명으로 쓴다.

```cpp
// Gate

bool CAIAnimationProfiling::ShouldAuditAnimationRefresh()
{
	return CVarAnimationRefreshAudit.GetValueOnGameThread() != 0;
}

// Counter

void CAIAnimationProfiling::RecordAnimationRefreshExecuted()
{
	CSV_CUSTOM_STAT_GLOBAL(AnimationRefreshExecuted, 1, ECsvCustomStatOp::Accumulate);
}
```

### 4) Header API Section

헤더 선언부는 함수 그룹을 읽기 쉽게 나눌 때만 섹션 주석을 둔다.
섹션명은 `Lifecycle`, `Component Reference`, `Query`, `Mutation`, `Runtime State`, `Notify Routing`처럼 짧은 명사구를 사용한다.
`API`, `Current`, `Used`, `Temp` 같은 상태성 표현은 사용하지 않는다.

```cpp
public:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Query
	bool IsActive() const;
	EActionType GetActiveActionType() const;

private:
	// Runtime State
	void ClearActionRuntimeState();
```

### 5) Implementation Section

cpp 구현부는 함수 그룹을 나눌 때만 섹션 주석을 둔다.
가능하면 헤더의 섹션명과 같은 용어를 사용한다.
함수 1개짜리 작은 그룹은 보통 섹션 주석을 두지 않는다.

```cpp
// Runtime Lifecycle

void UCActionComponent::InitializeActionRuntime()
{
	BuildActionRuntimeMaps();
}

void UCActionComponent::UninitializeActionRuntime()
{
	ClearActionRuntimeMaps();
}

// Notify Routing

void UCActionComponent::HandleActionNotify(...)
{
}
```

### 6) Algorithm / Step

순서나 우선순위가 의미 있는 fallback, priority matching, state decision에만 단계 주석을 사용한다.
번호는 실제 실행 순서나 우선순위를 나타낼 때만 쓴다.
장식성 separator 또는 block banner는 사용하지 않는다.

```cpp
AController* ResolveInstigatorController(AActor* InAttacker, AActor* InDamageCauser)
{
	// 1) Prefer attacker-provided instigator.
	if (AController* controller = InAttacker->GetInstigatorController())
		return controller;

	// 2) Fall back to the attacker pawn controller.
	if (APawn* pawn = Cast<APawn>(InAttacker))
		return pawn->GetController();

	return nullptr;
}
```

### 7) Policy / Exception Reason

본문 주석은 기본적으로 "왜 이렇게 해야 하는지"를 설명할 때만 사용한다.
`[NOTE]`, `[Policy]` 같은 태그는 기본적으로 쓰지 않고, 한 줄 문장으로 작성한다.
2줄 이상 길어지면 함수 분리 또는 문서화를 먼저 검토한다.

```cpp
bool UCReactionOrchestratorComponent::CanAcceptReactionRequest(...)
{
	// DeadReaction may be requested after health and dead state have already been committed.
	if (RejectReason == EReactionRequestRejectReason::DeadState)
		return true;

	return false;
}
```

### 8) Type / Data Meaning

enum sentinel / wildcard처럼 이름만으로 의미가 부족한 값은 짧은 inline 주석을 허용한다.
field 의미 설명은 이름으로 표현하기 어려울 때만 허용한다.
payload 설명이 길어지면 struct 이름, 필드명, 문서로 이동한다.
UPROPERTY 바로 위에 변수 그룹처럼 보이는 섹션 주석은 지양한다.

```cpp
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None = 0,	// Invalid, unset
	Sword,

	All,		// Wildcard
	Max,		// Sentinel
};
```

```cpp
USTRUCT(BlueprintType)
struct FDamageRequestAmount
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	float RequestedDamage = 0.f; // Raw incoming damage before mitigation.

	UPROPERTY(Transient)
	float FinalTakenDamage = 0.f; // Damage after target-side final rules.
};
```

### 9) Sparse / One-off

주석이 한두 개만 있는 파일은 기본적으로 제거 후보로 본다.
단, 코드만으로 드러나지 않는 엔진 제약, interface wrapper, lifecycle 예외 같은 이유 설명은 유지할 수 있다.

```cpp
// Avoid if this is the only comment and it only repeats the function name.
void UCWeaponComponent::ClearWeaponRuntime()
{
}
```

```cpp
// UINTERFACE wrapper keeps the UObject and interface pointer together.
FObservableTargetRef targetRef;
```

---

## 13.1 Section Comment Follow-up Rules

### Rule / Work Plan Boundary

W05 문서는 코드 품질 정리의 정책 기준과 현재 프로젝트 적용 계획을 함께 기록한다.
AI 작업 시스템에는 반복 적용해야 하는 실행 체크리스트만 반영하고, 프로젝트의 세부 후보 판단은 W05 Work Plan에 남긴다.

```text
역할 분리:
-> W05 Work Plan: 현재 branch에서 적용할 정책, 후보, 완료 / 보류 판단을 기록한다.
-> AI Workflow Prompt: 이후 작업 세션에서 반복 적용할 실행 체크리스트를 제공한다.
-> .agents: 현재 repo에는 별도 agent rule 파일이 없으므로 이번 반영 대상에서 제외한다.
-> .codex: 현재 repo에는 별도 Codex rule 파일이 없으므로 이번 반영 대상에서 제외한다.
```

### Header / Source Section Synchronization

`.h`가 API 책임 단위로 섹션을 나누면 `.cpp`도 같은 책임 그룹 기준으로 섹션을 둔다.
목표는 선언 파일에서 본 책임 구조를 구현 파일에서도 빠르게 찾을 수 있게 하는 것이다.

```text
기본 원칙:
-> 공통 책임 섹션명은 프로젝트 전체에서 같은 이름을 사용한다.
-> .h와 .cpp에서 같은 책임을 다루면 같은 섹션명을 사용한다.
-> .cpp 섹션명과 순서는 가능하면 .h를 따른다.
-> 완전한 함수 단위 1:1 매칭은 강제하지 않는다.
-> 파일 고유 책임 섹션은 허용하되, .h와 .cpp 양쪽에 대응되는 구현이 있으면 같은 이름을 쓴다.
-> 구현 전용 helper / local namespace / static helper / 세부 pipeline 단계는 .cpp 전용 섹션으로 둘 수 있다.
-> 함수가 1~2개뿐인 작은 파일은 섹션 주석을 생략할 수 있다.
-> Unreal lifecycle / callback 순서처럼 호출 순서가 이해에 직접 영향을 주는 경우에만 구현 흐름을 우선한다.
-> 이 경우에도 같은 책임을 다루는 섹션명은 가능한 한 .h / .cpp에서 동일하게 유지한다.
```

공통 책임 섹션명은 아래 이름을 우선 사용한다.

```text
// Lifecycle
// Runtime Lifecycle
// Component Reference
// Query
// Mutation
// State Transition
// Runtime State
// Request
// Entry
// Notify
// Notify Routing
// Feedback
// Result
```

파일 고유 책임은 짧은 명사구로 둔다. 예를 들어 CombatSignal 파일의 `Receive`, `Resolve`, `Send`, `Animation Refresh Audit`, `Movement Arbitration`, `Camera Shake`, `Overlay Snapshot`처럼 해당 파일의 실제 책임을 드러내는 이름은 허용한다.

### Type Header Section Taxonomy

Type 헤더에서 타입 역할이 3개 이상으로 나뉘면 아래 taxonomy를 우선 사용한다.
타입이 1~2개뿐인 작은 Type 헤더는 섹션 주석을 생략할 수 있다.

```text
// Enum
// Key / Identifier
// Data / Config
// Runtime State
// Runtime Context
// Request
// Candidate
// Payload
// Resolution
// Result
// Packet
// Runtime Key / Playback Key
// Reserved Pipeline Scaffold
// Helper API
```

pipeline 단계가 type taxonomy보다 더 명확한 파일은 `Request`, `Candidate`, `Payload`, `Resolution`, `Result`, `Packet`을 우선 사용한다.
feedback / playback처럼 domain 의미가 필요한 경우 `Runtime Key / Playback Key`처럼 구체적인 섹션명을 허용한다.

### Step Comment Style

단계형 주석은 fallback 순서, policy gate, priority matching처럼 순서 자체가 의미를 가질 때만 사용한다.
번호 깊이는 한 단계까지만 허용하고, `2-3-1` 같은 중첩 번호는 의미 있는 문장형 주석으로 바꾼다.

```cpp
// Gate: already dead.
// Gate: parry intercept.
// Gate: target-side defense policy.
```

```cpp
// Preferred: engine-provided instigator.
// Fallback: causer-provided instigator.
// Final fallback: causer owner as instigator.
```

피하는 형태:

```cpp
// 2-3-1) Case 03-01
// [Policy] ...
// NOTE: ...
```

---

## 14. P3 Final Decision

P3 항목은 이번 브랜치에서 코드 수정하지 않고, 유지 또는 후속 작업으로 이관한다.

### 1) Type / Data 주석

`CAIStructure.h`, `CWeaponStructure.h`에 남은 Type / Data 의미 주석은 현재 브랜치에서 추가 정리하지 않는다.

판단:
- enum의 `None`, `All`, `Max` 설명은 sentinel / wildcard 의미를 설명하므로 유지한다.
- `FDamageRequestAmount`의 damage 단계 설명은 `RequestedDamage`, `MitigatedDamage`, `FinalTakenDamage`, `CommittedDamage`의 파이프라인 의미를 구분하므로 유지한다.
- `FAIBlackboardUpdateContext`의 field group 주석과 `FOverlapContext`의 actor/component alias 주석은 구조체 분리 / 헤더 배치 규칙 작업에서 다시 판단한다.

후속 범위:
- 구조체 나누기 / 헤더 배치 규칙
- 네이밍 / 매개변수 작명 규칙
- UPROPERTY Category 정리

### 2) TODO 유지 판단

남은 TODO 6개는 코드 위치에 정책 후속 작업을 표시해야 하므로 유지한다.

유지 목록:
- `CPlayer.cpp`: `TODO(Gameplay)` Dead actor TakeDamage route 정책
- `CEnemy.cpp`: `TODO(Gameplay)` Dead actor TakeDamage route 정책
- `CCombatSignalTargetComponent.cpp`: `TODO(CombatPolicy)` target-side defensive gates
- `CCombatSignalTargetComponent.cpp`: `TODO(CombatPolicy)` mitigation policy
- `CCombatSignalTargetComponent.cpp`: `TODO(CombatPolicy)` final damage policy
- `CCombatSignalTargetComponent.cpp`: `TODO(CombatPolicy)` resource commit order

판단:
- TODO가 실제 구현 위치와 직접 연결되어 있다.
- 문서로만 옮기면 정책을 구현할 때 놓칠 가능성이 크다.
- 이번 브랜치의 주석 규칙상 허용되는 code-local TODO로 본다.
