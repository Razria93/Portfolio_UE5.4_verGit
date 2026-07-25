# UE5 Portfolio Pull Request

## 제목

**P49: API Const Consistency**

## 날짜

**2026.07.25**

## 상태

- [x] ReadOnly API const 사용 규칙 정리
- [x] ReadOnly / Non-ReadOnly / 보류 후보 전수 감사
- [x] 명확한 ReadOnly member API const 1차 적용
- [x] ReadOnly Param const 1차 재스캔
- [x] local const / Others 사용 정책 정리
- [x] 의미 없는 local bool / scalar / enum const 선별 정리
- [x] ReadOnly member function const 잔여 / 보류 후보 재검증
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] PIE smoke 확인

## 브랜치

- `refactor/api-const-consistency`

## 요약

이번 PR은 `Source/Portfolio`의 C++ API에서 ReadOnly 성격이 명확한 함수와 읽기 전용 입력 파라미터의 `const` 사용 기준을 정리한다.

목표는 모든 곳에 기계적으로 `const`를 붙이는 것이 아니라, 호출자가 "이 API는 owner 상태를 바꾸지 않는 조회 / 계산 / 조립 함수"라고 신뢰할 수 있는 범위에만 `const` 계약을 부여하는 것이다.

작업은 세 단계로 나누었다.

```text
1. 규칙 정리
-> ReadOnly API / ReadOnly Param / local const 사용 기준 문서화

2. 코드 적용
-> 명확한 ReadOnly member API에 const 적용
-> 의미 없는 local const 임시값 정리

3. 보류 후보 재검증
-> ReadOnly Param / member function 잔여 후보를 다시 스캔
-> 정책 변경이 필요한 항목은 코드 변경 없이 보류 사유 문서화
```

## 변경 배경

ReadOnly 성격의 getter / query / compute API에 `const`가 일관되게 붙어 있지 않으면, 외부 리뷰에서 함수의 side effect 여부를 코드만 보고 판단하기 어렵다.

반대로 `const`를 무리하게 넓히면 UE reflection, Blueprint 노출 signature, delegate, override, runtime cache, audit 기록, non-const UObject pointer graph 같은 영역에서 불필요한 리스크가 생긴다.

이번 PR에서는 다음 원칙을 고정했다.

```text
ReadOnly API
-> owner member 상태를 바꾸지 않는 조회 / 계산 / 조립 API에만 member const를 사용한다.

ReadOnly Param
-> TArray / TMap / FString / FText / 큰 project struct 입력이 읽기 전용이면 const&를 사용한다.

local const
-> 단순 임시값에는 남발하지 않는다.
-> snapshot / decision basis / static key / literal reason처럼 스코프 안에서 의미가 고정되는 값에만 허용한다.
```

## 변경 범위

### 1. ReadOnly API const 규칙 문서화

왜

`const` 적용 기준이 코드 관례로만 남아 있으면 이후 pass에서 UFUNCTION, override, delegate, audit 기록 함수까지 무리하게 확장될 수 있다.

어떻게

`W05_Naming_Rules.md`에 ReadOnly API, ReadOnly Param, local const 사용 기준을 기록했다.

결과

const 적용 범위와 제외 범위가 다음 기준으로 분리됐다.

```text
적용:
-> 외부에서 내부 상태를 조회하는 ReadOnly API
-> 읽기 전용 큰 입력 파라미터
-> 의미가 고정되는 local snapshot / gate / static constant

제외:
-> UFUNCTION / Blueprint 노출 signature
-> delegate signature
-> override signature
-> UPROPERTY / serialized USTRUCT field
-> audit / debug / profiling 기록 함수
-> lazy initialization / cache mutation 함수
-> const_cast가 필요한 변경
-> UObject pointer-to-const 대량 적용
```

### 2. ReadOnly member API const 적용

왜

일부 query / compute 계열 함수가 owner 상태를 바꾸지 않는데 member `const`가 빠져 있었다.

어떻게

명확한 ReadOnly 함수에만 선언 / 정의 `const`를 맞췄다.

```text
UCWeaponComponent
-> GetCurrentWeaponType
-> GetWeaponActor

UCCombatSignalTargetComponent
-> CanReceiveCombatSignal

UCBTService_UpdateAIContext
-> ComputeHomeMetricContext
-> ComputeAlertRangeContext
-> ComputeReactionContext
-> ComputeDeadContext

ACAIController
-> SelectTopPriority
```

결과

ReadOnly query / compute API의 호출 계약이 명확해졌고, 하위 호출 const 정합성도 함께 맞췄다.

### 3. local const 정책 정리와 선별 cleanup

왜

local `const`가 단순 임시 bool / int / enum까지 퍼지면 "정말 의미가 고정되는 값"과 "그냥 한 번 쓰는 임시값"이 구분되지 않는다.

어떻게

프로젝트 전체 const 사용을 다음 기준으로 재분류했다.

```text
ReadOnly API
ReadOnly Param
Others
```

`Others` 중 단순 임시값만 선별 정리했다.

```text
정리:
-> 단순 bool result 임시값: bStarted / bReserved / bRequested / bApplied
-> Notify trigger 비교용 단순 active type / index 임시값
-> Investigate index 증가용 단순 int32 임시값

유지:
-> previous state / threshold / cooldown / latency / match tier
-> guard phase / reaction type / execution state snapshot
-> 복잡한 조건식에 이름을 붙인 branch gate
-> static / namespace constant
-> UObject / AActor pointer-to-const 기존 사용
```

결과

local `const`는 전면 제거 대상이 아니라, 의미 고정이 있는 값에만 남기는 기준으로 정리됐다.

### 4. ReadOnly Param const 재스캔

왜

TArray / TMap / FString / FText / project struct 입력이 by-value로 남아 있는지 확인해야 했다.

어떻게

Component / Action / Reaction / Weapon / CombatSignal / AI / System / Core 범위를 재스캔했다.

결과

```text
-> 새로 const&로 바꿀 대형 by-value 입력 후보 없음
-> TArray / TMap / FString / FText 계열 입력은 이미 const&이거나 Out / InOut 참조
-> FDamageEvent는 UE TakeDamage 계열 signature와 CombatSignal damage forwarding 경로라 유지
-> FAIStimulus는 perception UFUNCTION callback signature라 제외
-> FName은 작은 값 타입이고 넓은 notify / trigger / collision key 경로라 이번 pass에서 변경하지 않음
```

### 5. ReadOnly member function 보류 후보 재검증

왜

이름상 ReadOnly처럼 보이는 `Resolve*`, `Find*`, `Build*` 계열 중 일부는 실제로는 audit 기록, cache mutation, pointer graph 조립 책임이 있다.

어떻게

비-const로 남은 Get / Is / Has / Can / Should / Find / Resolve / Build / Make / Calculate / Compute 계열 header 선언을 재스캔했다.

결과

```text
신규 명확한 ReadOnly member const 적용 후보:
-> 없음

보류:
-> UCActionComponent::ResolveActionData
-> UCReactionComponent::ResolveReactionData
   사유: data map 조회 성격은 있지만 실패 경로 audit 기록 수행

-> UCActionComponent::BuildActionExecutorReferences
-> UCReactionComponent::BuildReactionExecutorReferences
-> ACPlayer::BuildReferences
-> ACEnemy::BuildReferences
   사유: non-const FCharacterComponentReferences pointer graph 조립 계약

Non-ReadOnly 유지:
-> ResolveActionExecutor / ResolveReactionExecutor
   사유: executor lazy 생성 / cache mutation

-> FindActionExecutor / FindReactionExecutor
   사유: invalid cached executor 제거

-> CAnimInstance::ShouldRefreshAnimationParameters
   사유: RuntimeLODAnimationRefreshElapsed 갱신 및 profiling / audit 조건
```

## 주요 처리 흐름

```text
ReadOnly API 후보 스캔
-> ReadOnly / Non-ReadOnly / 보류 분류
-> 명확한 ReadOnly member API const 적용
-> build 검증
-> local const 사용 현황 재분류
-> 의미 없는 단순 local const 선별 정리
-> ReadOnly Param const 재스캔
-> ReadOnly member function 보류 후보 재검증
-> 보류 정책 문서화
```

## 구현 결과

```text
main...HEAD
-> 20 files changed, 804 insertions(+), 37 deletions(-)

commit range
-> 60d05407 docs(w05): define readonly api const rules
-> 2cbe3877 docs(code-quality): audit readonly api const candidates
-> 89bb0d78 refactor(api): const readonly component queries
-> 5b1dfed6 refactor(api): const readonly ai context queries
-> 98b681e7 docs(code-quality): record readonly api const pass
-> e61c2c2c refactor(api): trim noisy local const temporaries
-> 65364f9f docs(api): record readonly param const audit
```

변경함:

```text
-> ReadOnly API const 규칙 문서화
-> API const 후보표 작성
-> 명확한 ReadOnly member API const 적용
-> local const 사용 정책 문서화
-> 의미 없는 local const 임시값 선별 정리
-> ReadOnly Param / member const 잔여 후보 재스캔 결과 문서화
```

변경하지 않음:

```text
-> UFUNCTION / Blueprint 노출 API signature
-> delegate signature
-> override signature
-> UPROPERTY / serialized USTRUCT field
-> audit 기록 함수를 ReadOnly 예외로 허용하는 정책 변경
-> FCharacterComponentReferences non-const pointer graph 계약 변경
-> UObject pointer-to-const 대량 적용
-> const_cast 사용
```

## 테스트 방법

```text
1. Get / Is / Has / Can / Should / Find / Resolve / Build 계열 API 스캔
2. ReadOnly Param by-value 후보 재스캔
3. local const / Others 분류 재스캔
4. git diff --check
5. PortfolioEditor Win64 Development build
6. PIE smoke 실행
```

## 검증 결과

### Static check

```text
git diff --check
Result: Pass
```

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### Scans

```text
ReadOnly API candidate scan
Result: Completed

ReadOnly Param by-value scan
Result: No new large by-value input candidate

ReadOnly member function follow-up scan
Result: No new clear const candidate

local const / Others scan
Result: Meaningless simple temporaries cleaned up selectively
```

### PIE

```text
PIE smoke
Result: Pass
```

## 비고 / 후속 작업

- audit 기록 함수를 ReadOnly 예외로 허용할지는 현재 정책상 보류한다.
- `FCharacterComponentReferences` pointer graph 계약 변경은 const pass가 아니라 별도 설계 검토로 분리한다.
- Blueprint / override signature 변경은 이번 PR에서 제외한다.
- 다음 코드 품질 작업 후보는 tuning constants cleanup이다.

## 관련 문서

- Work List: `W05_UE5_Portfolio_Work_List.md`
- Naming Rules: `W05_Naming_Rules.md`
- API Const Work Plan: `W05_API_Const_Consistency_Work_Plan.md`
- Previous PR: `P48_UE5_Portfolio_Pull_Request.md`

## 정리

이번 PR은 ReadOnly API / Param / local const 사용 기준을 정리하고, 명확한 ReadOnly API에만 `const` 계약을 적용했다.

남은 후보는 단순 누락이 아니라 audit 기록, runtime cache mutation, pointer graph 계약, UE signature 안정성처럼 정책 결정이 필요한 항목이므로 이번 PR에서는 코드 변경 없이 보류 사유를 문서화했다.
