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

### File Family Section Policy

섹션 주석은 모든 파일에 같은 모양으로 강제하지 않는다.
같은 성격 / 책임 / 파일 패턴을 가진 파일군끼리 같은 섹션 체계를 우선 사용한다.
파일 고유 책임은 고유 섹션으로 허용한다.

```text
공통 판단 기준:
-> 같은 계열 파일은 같은 책임명 섹션을 우선 사용한다.
-> 파일 고유 책임은 고유 섹션으로 허용한다.
-> .h와 .cpp는 함수 1:1이 아니라 책임 그룹 기준으로 동기화한다.
-> 로컬 흐름 설명은 섹션이 아니라 문장형 주석으로 작성한다.
-> 짧고 단일 책임인 UE adapter / Interface / Type cpp는 섹션을 생략할 수 있다.
-> 섹션은 같은 책임의 함수가 2개 이상 있거나, 파일 내 책임 그룹이 2개 이상일 때 우선 둔다.
-> 단, 프로젝트 계열 일관성을 위해 단일 함수라도 명시 섹션을 둘 수 있다.
```

#### Action / Reaction Base

```text
대상:
-> CAction.h / .cpp
-> CReaction.h / .cpp

권장 섹션:
-> Component Reference
-> Tick
-> Query
-> Decision
-> Observable Overlay
-> Notify
-> Intervention
-> State Transition
-> Intervention Match Helper

판단:
-> base class는 파생 클래스가 따르는 공통 책임 기준점이다.
-> Idle && No ActivePart, Default Action Case 같은 case 설명은 섹션이 아니라 문장형 로컬 설명으로 둔다.
```

#### Action / Reaction Derived

```text
대상:
-> CAction_*
-> CReaction_*

권장 공통 섹션:
-> Decision
-> Lifecycle
-> Notify
-> Observable Overlay
-> Intervention

파일 고유 섹션 예:
-> Chain Reservation
-> Chain Window
-> Weapon
-> Guard State Cleanup

판단:
-> 파생 클래스는 base와 공통 책임명을 맞춘다.
-> 파일 고유 로직은 고유 섹션으로 둔다.
-> Case 1, Another Case, GuardState Case 같은 라벨은 섹션으로 쓰지 않고 조건 설명 문장으로 둔다.
```

#### Component

```text
대상:
-> Component/*

권장 공통 섹션:
-> Component Reference
-> Lifecycle
-> Runtime Lifecycle
-> Query
-> Mutation
-> Runtime State
-> State Transition
-> Request / Entry
-> Receive
-> Resolve
-> Apply
-> Packet
-> Notify
-> Helper

도메인별 고유 섹션 예:
-> Data Resolve
-> Execution Entry
-> Decision Apply
-> Execution Operations
-> Matching
-> Runtime Key / Playback Key
-> Playback
-> Runtime LOD
-> Movement Arbitration
-> Movement Input
-> Movement Policy
-> Hit Window
-> AI Entry
-> Cue Helper

판단:
-> Component는 public / private API 책임이 섞이기 쉬우므로 섹션을 적극 적용한다.
-> Check / Query는 Query로 통일한다.
-> 단독 Mutation은 허용 가능하지만 더 구체적인 책임명이 있으면 구체명을 우선한다.
```

#### Controller

```text
대상:
-> CAIController
-> CPlayerController

권장 공통 섹션:
-> Lifecycle

PlayerController 권장 섹션:
-> Look Input
-> Move Input
-> Movement Dispatch
-> Action Input

AIController 권장 섹션:
-> Team
-> Config Setup
-> Blackboard Setup
-> Blackboard Runtime Value
-> Behavior Tree Runtime
-> Perception Event Callback
-> Runtime LOD Snapshot
-> Perception Candidate Audit
-> Blackboard / Engage Latency Audit

판단:
-> Controller는 UE lifecycle + input / perception / blackboard 책임이 분명하면 섹션을 둔다.
-> 짧은 controller라도 .h / .cpp가 같은 책임으로 나뉘면 같은 섹션명을 유지한다.
-> 큰 controller는 공통명보다 실제 책임을 드러내는 구체 섹션명을 우선한다.
```

#### Behavior Tree Service / Task / Decorator

```text
BT Service 권장 섹션:
-> Lifecycle
-> Context Build
-> Context Compute
-> Blackboard Update
-> Blackboard Clear
-> Intent Decision
-> Intent Transition
-> Interval Defaults
-> Interval Selection
-> Profiling
-> Public API

BT Task / Decorator 판단:
-> 대체로 짧은 UE node adapter이므로 섹션 생략을 기본 허용한다.
-> ExecuteTask, CalculateRawConditionValue 같은 override 하나가 중심이면 함수 자체가 구조 역할을 한다.
-> 파일이 커지고 책임이 나뉘면 Execution, Validation, Blackboard Update, Request, Result 정도의 최소 섹션만 둔다.
```

#### AI RuntimeLOD Policy

```text
대상:
-> AI/RuntimeLOD/*Policy*

권장 섹션:
-> Policy Resolve
-> Runtime LOD
-> Query
-> Helper

판단:
-> RuntimeLOD policy 파일은 resolver보다 짧고 단일 정책 판단에 집중하는 경우가 많다.
-> 함수가 1~2개뿐이면 섹션 생략을 허용한다.
-> policy가 여러 입력을 조합하거나 CVar / config / tier decision을 함께 다루면 Runtime LOD / Policy Resolve 섹션을 둔다.
-> CAIRuntimeLODTierResolver처럼 context build와 tier resolve가 분리되면 Context Build / Tier Resolve / String Conversion처럼 책임을 더 구체화한다.
```

#### Character

```text
대상:
-> Character/*
-> Character/Player/*
-> Character/Enemy/*

권장 섹션:
-> Lifecycle
-> Component Reference
-> Input
-> Query
-> Component Query
-> AI Config Query
-> Runtime LOD
-> Tick
-> Damage
-> Combat Result
-> Movement Intent
-> Action Intent
-> Runtime State
-> Action Event Routing

판단:
-> Character 계열은 gameplay owner / actor 책임을 기준으로 섹션을 둔다.
-> Player와 Enemy가 공유하는 책임은 같은 섹션명을 사용한다.
-> Enemy처럼 AI config / Runtime LOD / intent routing이 추가되면 고유 섹션을 허용한다.
-> CAnimInstance는 Character 하위지만 animation refresh / gate / record 고유 구조가 강하므로 파일 고유 섹션을 유지한다.
```

#### System

```text
대상:
-> System/*
-> System/Combat/*

권장 섹션:
-> Lifecycle
-> Tick
-> Query
-> Request
-> Assignment
-> Assignment Build
-> Assignment Warmup
-> Assignment Apply
-> Assignment Lease
-> Runtime State
-> HitStop
-> Camera Shake

판단:
-> System / Subsystem 계열은 actor/component보다 runtime service 책임에 가깝다.
-> Request / Assignment / Runtime State처럼 외부 요청과 내부 상태 처리를 분리한다.
-> CombatFeedback처럼 feedback dispatch 책임이면 HitStop / Camera Shake 같은 도메인 섹션을 허용한다.
```

#### Weapon

```text
대상:
-> Weapon/*

권장 섹션:
-> Component Reference
-> Initial State
-> Lifecycle
-> Collision Component
-> Trail
-> Hit Context Provider Query
-> Hit Context Provider Mutation
-> Query
-> Mutation
-> Equip Notify Events
-> Collision Notify Events
-> Engine Delegate Events
-> Helper

판단:
-> Weapon actor는 combat collision / notify / hit context provider 책임이 섞이므로 섹션을 둔다.
-> WeaponComponent는 Component 계열 정책을 따르되 Weapon Actor / Runtime Lifecycle / Profiling 같은 고유 섹션을 허용한다.
-> Equip / Unequip action의 Weapon 섹션은 Action derived의 파일 고유 섹션으로 허용한다.
```

#### Type

```text
Type 헤더:
-> Type Header Section Taxonomy를 따른다.

Type cpp:
-> IsValidMinimal, hash/helper만 있는 짧은 cpp는 섹션 생략을 허용한다.
-> 구현 함수가 늘거나 helper 성격이 섞이면 Helper API, Data / Config, Runtime State 등 헤더 taxonomy와 맞춘다.
```

#### Core Debug / Profiling

```text
Core / Debug 권장 섹션:
-> Gate
-> Diagnostic Hook
-> Debug Dump
-> 도메인 고유 Diagnostic Hook

Core / Profiling 권장 섹션:
-> Gate
-> Counter
-> Service Tick Counter
-> Interval Preset Counter
-> Flush

판단:
-> Debug / Profiling 파일은 gameplay component식 Lifecycle / Query / Mutation을 강제하지 않는다.
-> 이미 Gate / Diagnostic Hook / Debug Dump 체계가 있으면 그 체계를 유지한다.
-> 짧은 helper / type 파일은 섹션 생략을 허용한다.
```

#### Notify / Interface / Blackboard / Patrol / Module

```text
Notify:
-> UE Notify adapter 성격이 강하면 섹션 생략을 기본 허용한다.
-> 길어지면 Notify Entry, Validation, Command Dispatch, Payload Build 정도만 사용한다.

Interface:
-> UInterface 계약 자체가 구조이므로 API가 적으면 섹션 생략을 허용한다.
-> 여러 계약 그룹이 생기면 Query, Request, Result, Event 정도로 최소화한다.

AI / Blackboard:
-> key namespace / registry / helper 구조 자체가 섹션 역할을 한다.
-> helper가 커지면 Key Lookup, Value Apply, Value Clear 정도를 사용한다.

AI / Patrol:
-> actor / data 책임이 작으면 섹션 생략을 허용한다.
-> 커지면 Lifecycle, Query, Patrol Point, Path Resolve, Debug 같은 도메인 섹션을 사용한다.

Module / Global:
-> Portfolio.cpp / .h, ProjectGlobal.h는 gameplay section policy 대상이 아니다.
-> include grouping, module macro, global dependency 관리가 우선이다.
```

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

## 13.2 Section Consistency Full Audit

섹션 주석 통일성 기준으로 `Source/Portfolio` 전체를 재스캔한 결과다.
이 목록은 PR 마감 전에 보완할 후보를 빠뜨리지 않기 위한 감사표이며, 실제 수정 여부는 파일별 책임 구조와 가독성 기준으로 다시 판단한다.

### Audit 기준

```text
기본:
-> 같은 책임이면 .h / .cpp 섹션명은 동일하게 둔다.
-> Lifecycle / Query / Component Reference / Runtime State / Request / Result 등 공통 책임은 공통 섹션명을 사용한다.
-> 파일 고유 책임은 고유 섹션으로 허용한다.

예외:
-> .cpp-only helper / anonymous namespace / local helper / 아주 작은 단일 책임 파일은 예외로 둘 수 있다.
-> include-only Type .cpp는 섹션화하지 않는다.
-> 로컬 분기 설명은 섹션처럼 보이지 않게 문장형 주석으로 둔다.
```

### 1) 큰 `.cpp`인데 섹션이 없는 후보

```text
Source/Portfolio/Component/CStateComponent.cpp
Source/Portfolio/Reaction/CReaction_BlockHit.cpp
Source/Portfolio/Type/CExecutionRuleTypes.cpp
Source/Portfolio/AI/RuntimeLOD/CAIRuntimeLODTierResolver.cpp
```

### 2) `.h / .cpp` 섹션 불일치 또는 한쪽 누락 후보

```text
Source/Portfolio/Action/CAction.h / .cpp
Source/Portfolio/Action/CAction_ComboAttack.h / .cpp
Source/Portfolio/Reaction/CReaction.h / .cpp
Source/Portfolio/Reaction/CReaction_BlockHit.h / .cpp
Source/Portfolio/Character/CAnimInstance.h / .cpp
Source/Portfolio/Character/Enemy/CEnemy.h / .cpp
Source/Portfolio/Character/Player/CPlayer.h / .cpp
Source/Portfolio/Component/CActionComponent.h / .cpp
Source/Portfolio/Component/CActionOrchestratorComponent.h / .cpp
Source/Portfolio/Component/CDefenseComponent.h / .cpp
Source/Portfolio/Component/CMovementComponent.h / .cpp
Source/Portfolio/Component/CReactionComponent.h / .cpp
Source/Portfolio/Component/CReactionFeedbackComponent.h / .cpp
Source/Portfolio/Component/CReactionOrchestratorComponent.h / .cpp
Source/Portfolio/Component/CStateComponent.h / .cpp
Source/Portfolio/Component/CWeaponComponent.h / .cpp
Source/Portfolio/Controller/CAIController.h / .cpp
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.h / .cpp
Source/Portfolio/System/Combat/CWorldSubsystem_CombatFeedback.h / .cpp
Source/Portfolio/Weapon/CWeaponActor.h / .cpp
```

### 3) AI / BehaviorTree 후보

```text
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.h / .cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateEngageContext.h / .cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.h / .cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateInvestigateContext.h / .cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTServiceIntervalHelper.h / .cpp
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_SelectPatrolPoint.h / .cpp
Source/Portfolio/AI/BehaviorTree/Task/*.h / .cpp
Source/Portfolio/AI/BehaviorTree/Decorator/*.h / .cpp
Source/Portfolio/AI/RuntimeLOD/CAIRuntimeLODTierResolver.h / .cpp
```

### 4) Core / Debug / Profiling 후보

```text
Source/Portfolio/Core/Debug/FAIPerceptionDebugTypes.h
Source/Portfolio/Core/Debug/FCombatEngageDebugTypes.h
Source/Portfolio/Core/Debug/FComponentReferenceHelper.h
Source/Portfolio/Core/Debug/FReferenceValidation.h
Source/Portfolio/Core/Profiling/CAIAnimationProfiling.h / .cpp
Source/Portfolio/Core/Profiling/CAIBehaviorTreeProfiling.h / .cpp
Source/Portfolio/Core/Profiling/CAIPerceptionProfiling.h / .cpp
Source/Portfolio/Core/Profiling/CAIStateRuntimeLODProfiling.h / .cpp
Source/Portfolio/Core/Profiling/CCombatCollisionProfiling.h / .cpp
Source/Portfolio/Core/Profiling/CCombatCollisionProfilingCounters.h / .cpp
Source/Portfolio/Core/Profiling/CCombatFeedbackProfiling.h / .cpp
```

### 5) Type 구현 `.cpp` 후보

```text
Source/Portfolio/Type/CExecutionRuleTypes.cpp
Source/Portfolio/Type/CExecutionTypes.cpp
Source/Portfolio/Type/CActionDataTypes.cpp
Source/Portfolio/Type/CReactionDataTypes.cpp
Source/Portfolio/Type/CActionKeyTypes.cpp
Source/Portfolio/Type/CReactionKeyTypes.cpp
Source/Portfolio/Type/CCombatHitTypes.cpp
```

include-only Type `.cpp`는 섹션화 실익이 낮으므로 기본 예외 후보로 둔다.

### 6) 공통명 흔들림 후보

```text
Check / Query vs Query
State Query vs Query
CameraShake vs Camera Shake
Delegate / Legacy delegate / Engine Delegate Events
Condition 단독 섹션
Mutation 단독 섹션
Profiling Event Sink
HitWindow
Entry for AI
```

### 7) 섹션처럼 잡히는 로컬 설명 후보

```text
Sync with ActionComponent
Based OwnerPawn
Based Perception
Based TargetActor
Absolute States
Context
Result
Reroll
determine left or right
Internal linkage
Candidate SpecKey
Resolve Executor
Delay for Warmup
Flag Toggle
Slow InActor
Restore InActor
Early-Return
Invalid
Legacy delegate
```

### 8) 단계형 / Case 주석 후보

```text
CAction.cpp: Idle && No ActivePart / No Idle && Has ActivePart
CAction.cpp: Default Action Case
CAction_Guard.cpp: Case 1 / Case 2 / Case Guard-in / Case Guard-out
CAction_Dodge.cpp: GuardState Case / Another Case
CReaction.cpp: Idle && No ActivePart / No Idle && Has ActivePart / Default Reaction Case
CReaction_Hit.cpp: GuardState Case / Another Case
CReaction_Dead.cpp: GuardState Case / Another Case
CReaction_Stagger.cpp: GuardState Case / Another Case
CReaction_BlockHit.cpp: Another Case
CBTTask_SelectPatrolPoint.cpp: Reroll / Reverse...
CBTService_UpdateAIIntentState.cpp: Engage -> Non-Engage / Investigate -> Non-Investigate
CCombatSignalTargetComponent.cpp: V1 hook only
```

### 9) 우선 검토 묶음

```text
1. CStateComponent.h / .cpp
2. CAIController.h / .cpp
3. CMovementComponent.h / .cpp
4. CWeaponActor.h / .cpp
5. CAnimInstance.h / .cpp
6. BT Service 계열
7. Core / Profiling 계열
8. Type 구현 .cpp 일부
9. CAction_ComboAttack.h / .cpp
10. CReaction_BlockHit.h / .cpp
```

### 10) 보완 판정표

아래 판정은 13.2 감사표를 기준으로 에이전트 교차 검토 후 정리한 실행 계획이다.
코드 수정 시 함수명, 시그니처, 접근 권한, 동작은 변경하지 않고 섹션 주석과 로컬 설명 주석만 정리한다.

#### Fix Now

```text
Action / Reaction:
-> CAction_ComboAttack.h / .cpp
-> CReaction_BlockHit.h / .cpp
-> CAction.h / .cpp
-> CReaction.h / .cpp

Component / Character / Controller / Weapon / System:
-> CStateComponent.h / .cpp
-> CMovementComponent.h / .cpp
-> CWeaponComponent.h / .cpp
-> CAIController.h / .cpp
-> CCombatSignalSourceComponent.h / .cpp
-> CWorldSubsystem_CombatFeedback.h / .cpp
-> CWeaponActor.h / .cpp

AI / RuntimeLOD / Profiling / Type:
-> CBTService_UpdateAIContext.h / .cpp
-> CBTService_UpdateEngageContext.h / .cpp
-> CBTService_UpdateAIIntentState.h / .cpp
-> CBTServiceIntervalHelper.h / .cpp
-> CAIRuntimeLODTierResolver.h / .cpp
-> CAIAnimationProfiling.h / .cpp
-> CAIBehaviorTreeProfiling.h / .cpp
-> CAIPerceptionProfiling.h / .cpp
-> CAIStateRuntimeLODProfiling.h / .cpp
-> CCombatCollisionProfiling.h / .cpp
-> CExecutionRuleTypes.cpp
-> CExecutionTypes.cpp
```

#### Optional

```text
-> CAction_Dodge.h / .cpp
-> CAction_Equip.h / .cpp
-> CAction_Unequip.h / .cpp
-> CReaction_Hit.h / .cpp
-> CReaction_Dead.h / .cpp
-> CReaction_Stagger.h / .cpp
-> CReaction_Parry.h / .cpp
-> CAnimInstance.h / .cpp
-> CEnemy.h / .cpp
-> CActionComponent.h / .cpp
-> CReactionComponent.h / .cpp
-> CActionOrchestratorComponent.h / .cpp
-> CBTService_UpdateInvestigateContext.h / .cpp
-> CBTTask_SelectPatrolPoint.h / .cpp
-> CCombatCollisionProfilingCounters.h / .cpp
-> CCombatFeedbackProfiling.h / .cpp
-> FAIPerceptionDebugTypes.h
-> Type 구현이 매우 짧은 CActionDataTypes.cpp / CReactionDataTypes.cpp / CActionKeyTypes.cpp / CReactionKeyTypes.cpp / CCombatHitTypes.cpp
```

#### Leave Justified

```text
-> CPlayer.h / .cpp
-> CDefenseComponent.h / .cpp
-> CReactionFeedbackComponent.h / .cpp
-> CReactionOrchestratorComponent.h / .cpp
-> CWorldSubsystem_CombatEngage.h / .cpp
-> 작은 BT Task / Decorator 단일 책임 파일
-> include-only Type .cpp
-> FCombatEngageDebugTypes.h
-> FComponentReferenceHelper.h
-> FReferenceValidation.h
```

### 11) 파일별 보완 방향

```text
CAction_ComboAttack.h / .cpp
-> .h / .cpp에 Decision, Chain Reservation, Lifecycle, Notify, Chain Window, Chain Consume, Chain Query 섹션 추가
-> Sync with ActionComponent는 섹션이 아니라 로컬 설명으로 유지하거나 제거

CReaction_BlockHit.h / .cpp
-> Decision, Lifecycle, Intervention, Observable Overlay 섹션 추가
-> Another Case 주석은 문장형 로컬 설명으로 변경

CAction.h / .cpp
-> .h에 Intervention Match Helper 섹션을 추가해 .cpp와 대응
-> Idle / Default Action Case 주석은 문장형 로컬 설명 후보

CReaction.h / .cpp
-> .h에 Intervention Match Helper 섹션을 추가해 .cpp와 대응
-> Idle / Default Reaction Case 주석은 문장형 로컬 설명 후보

CStateComponent.h / .cpp
-> Check / Query와 Query를 Query로 통합
-> Component Reference, Health State Sync, Query, State Transition 섹션으로 .h / .cpp 동기화

CMovementComponent.h / .cpp
-> Check / Query는 Query로 통합
-> Dispatch / Movement Mode / Mutation은 실제 책임에 맞춰 Runtime LOD, Movement State, Query, Runtime State 쪽으로 흡수
-> determine left or right는 문장형 로컬 주석으로 변경

CWeaponComponent.h / .cpp
-> Check / Query를 Query로 통일

CAIController.h / .cpp
-> API 순서 재배치 없이 섹션명만 보정
-> Runtime LOD Query / Runtime LOD State / Perception Profiling / Perception Candidate Audit / Blackboard / Engage Latency Audit 기준으로 정리
-> Init AIPerceptionComp는 Perception Component Setup으로 변경

CCombatSignalSourceComponent.h / .cpp
-> HitWindow를 Hit Window로 표기
-> Entry for AI는 Entry로 흡수하거나, AI Entry처럼 짧은 명사구로 통일

CWorldSubsystem_CombatFeedback.h / .cpp
-> CameraShake를 Camera Shake로 통일
-> Slow InActor / Restore InActor는 문장형 로컬 설명으로 변경

CWeaponActor.h / .cpp
-> Early-Return / Invalid / Legacy delegate는 섹션처럼 보이지 않게 로컬 설명으로 낮춤
-> Delegate 계열 섹션명은 Engine Delegate Events 기준으로 유지

BT Service 계열
-> UpdateAIContext: Lifecycle, Config, Context Build, Context Compute, Blackboard Write, Blackboard Clear
-> UpdateEngageContext: Lifecycle, Context Build, Context Compute, Blackboard Write, Blackboard Clear
-> UpdateAIIntentState: Lifecycle, Intent Decision, State Transition
-> IntervalHelper: Console Variable, Mode Query, Runtime LOD, Interval Preset, Profiling, Public API

CAIRuntimeLODTierResolver.h / .cpp
-> Enum, Runtime Context, Runtime LOD Resolve, String Conversion 섹션 추가

Core / Profiling 짧은 .cpp
-> .h의 Gate / Counter / Service Tick Counter / Interval Preset Counter 섹션을 .cpp에도 그대로 반영

Type 구현 .cpp
-> CExecutionRuleTypes.cpp: Helper API, Validation, Match
-> CExecutionTypes.cpp: Validation, Query
```

### 12) 적용 순서

```text
1. 공통명 표기 흔들림 정리
   -> Check / Query, CameraShake, HitWindow, Entry for AI

2. 섹션 없는 중간 크기 파일 정리
   -> CStateComponent, CAction_ComboAttack, CReaction_BlockHit, CExecutionRuleTypes, CAIRuntimeLODTierResolver

3. 큰 파일 섹션명 보정
   -> CAIController, CMovementComponent, CWeaponActor

4. BT Service 계열 정리

5. Core / Profiling .h / .cpp 동기화

6. Type 구현 .cpp 섹션 보강

7. Optional 후보 선별
```

### 13) Applied Result

```text
Fix Now applied:
-> CAction.h / .cpp, CReaction.h / .cpp
-> CAction_ComboAttack.h / .cpp
-> CReaction_BlockHit.h / .cpp
-> CStateComponent.h / .cpp
-> CMovementComponent.h / .cpp
-> CWeaponComponent.h
-> CAIController.h / .cpp
-> CCombatSignalSourceComponent.h / .cpp
-> CCombatSignalTargetComponent.cpp
-> CWorldSubsystem_CombatEngage.cpp
-> CWorldSubsystem_CombatFeedback.h / .cpp
-> CWeaponActor.cpp
-> BT Service: UpdateAIContext, UpdateEngageContext, UpdateAIIntentState, IntervalHelper
-> CAIRuntimeLODTierResolver.h / .cpp
-> Core/Profiling: CAIAnimation, CAIBehaviorTree, CAIPerception, CAIStateRuntimeLOD, CCombatCollision
-> Type implementation: CExecutionRuleTypes.cpp, CExecutionTypes.cpp

Optional applied:
-> CAction_Dodge.h / .cpp
-> CReaction_Hit.h / .cpp
-> CReaction_Dead.h / .cpp
-> CReaction_Stagger.h / .cpp
-> CReaction_Parry.h / .cpp
-> CEnemy.h / .cpp repeated section meaning clarified
-> CActionComponent.cpp / CReactionComponent.cpp / CReactionOrchestratorComponent.cpp local section-like comments clarified
-> CPlayerController.h / .cpp reviewed and retained: Lifecycle / Look Input / Move Input / Movement Dispatch / Action Input already match.

Build blocker fixed:
-> CHealthComponent.h had duplicate InitializeHealth(float, float, EMaxHPUpdatePolicy) declarations around the State Transition section.
-> The duplicate declaration outside the section was removed; the section-owned declaration remains.

Explicitly left unchanged:
-> CAnimInstance.h / .cpp: file-specific Condition / Query / Gate / Record structure is retained.
-> CBTService_UpdateInvestigateContext.h / .cpp: excluded by implementation decision.
-> CBTTask_SelectPatrolPoint.h / .cpp: excluded by implementation decision.
-> CCombatCollisionProfilingCounters.h / .cpp: excluded by implementation decision.
-> CCombatFeedbackProfiling.h / .cpp: excluded by implementation decision.
-> FAIPerceptionDebugTypes.h: excluded by implementation decision.
-> Short Type implementation .cpp files are excluded.

Applied policy:
-> Shared responsibility sections use shared names such as Query, Lifecycle, Component Reference, Runtime LOD, Request, Result, Packet, Helper.
-> File-specific sections remain allowed when they express a responsibility unique to that file.
-> .h and .cpp use matching responsibility names where both sides expose the same responsibility.
-> Local flow explanations are written as sentences, not section labels.
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
