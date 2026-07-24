# UE5 Portfolio Pull Request

## 제목

**P47: Type Rename and Feedback Key Cleanup**

## 날짜

**2026.07.24**

## 상태

- [x] `FActionContext` 제거
- [x] hit / combat signal action identity 전달을 `FActionDataKey`로 통일
- [x] Feedback MatchKey / PlaybackKey 의미 분리
- [x] Action / Reaction feedback playback dedupe 기준 통일
- [x] 구조체 rename pass 적용
- [x] renamed USTRUCT / UPROPERTY CoreRedirects 추가
- [x] W05 Type Header Organization Work Plan 완료 상태 반영
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] PIE smoke 확인

## 브랜치

- `refactor/type-rename-feedback-key-cleanup`

## 요약

이번 PR은 P46 Type Header Organization 이후 남은 타입 의미 정리 범위를 처리한다.

작업 범위는 세 가지다.

```text
1. FActionContext 제거
2. Feedback MatchKey / PlaybackKey 구조 정리
3. 의미가 맞지 않던 USTRUCT 이름 rename
```

기존에는 `FActionContext`, `FActionFeedbackKey`, `FActionVFXExecutionKey`, `FTargetData`, `FDamageImpactInfo`처럼 이름만 봐서는 실제 생명주기와 역할을 판단하기 어려운 타입이 남아 있었다.

이번 PR에서는 단순 파일 분리에서 한 단계 더 나아가, 타입 이름이 `Data`, `Context`, `State`, `MatchKey`, `PlaybackKey`, `Resolution`의 의미를 직접 드러내도록 정리했다.

## 핵심 개념

```text
MatchKey
-> 어떤 feedback data를 선택할지 결정하는 key

PlaybackKey
-> 선택된 feedback data가 실제로 같은 playback인지 dedupe하는 key

Data
-> editor / asset / component config 입력값

State
-> tick / frame / phase를 넘어 누적되는 runtime state

Context
-> 한 실행 / 판정 / 요청 흐름에서 계산되어 전달되는 transient 묶음

Resolution
-> request를 해석한 결과
```

## 변경 배경

P46에서 Type 헤더는 파일 책임 기준으로 분리됐지만, 일부 타입명은 여전히 실제 역할과 맞지 않았다.

특히 feedback 쪽은 Action과 Reaction의 dedupe 기준이 달랐다.

```text
기존 Action feedback playback key
-> feedback match key + timing + trigger + asset + playback option

기존 Reaction feedback playback key
-> asset + playback option
```

이 구조에서는 Action feedback은 request identity가 다르면 같은 effect asset도 별도 playback으로 취급하고, Reaction feedback은 effect asset 기준으로 dedupe했다.

이번 PR에서는 두 도메인 모두 같은 정책을 사용하도록 정리했다.

```text
matching 단계
-> ActionType / ReactionType / DamageSpecKey / Timing / TriggerKey를 사용해 feedback data 선택

playback dedupe 단계
-> effect asset + playback condition 기준으로 중복 실행 여부 판단
```

## 변경 범위

### 1. `FActionContext` 제거

왜:

`FActionContext`는 이름상 action runtime context처럼 보였지만, 실제 역할은 hit / combat signal 경로에 실리는 action identity snapshot이었다.

어떻게:

`FActionContext`를 제거하고 `FActionDataKey`를 직접 사용하도록 변경했다.

```text
UCAction
-> UCWeaponComponent::PushActionDataKey
-> ACWeaponActor::SetLastActionDataKey
-> FHitContext.ActionDataKey
-> UCCombatSignalSourceComponent::BuildSpecKey
```

결과:

action execution context와 hit source action key의 책임이 분리됐다.

### 2. Feedback MatchKey / PlaybackKey 정리

왜:

`FeedbackKey`는 matching key인지 playback dedupe key인지 이름만으로 판단하기 어려웠고, `ExecutionKey`는 실제 실행 context가 아니라 playback dedupe identity였다.

어떻게:

타입명을 의미 기준으로 변경했다.

```text
FActionFeedbackKey -> FActionFeedbackMatchKey
FReactionFeedbackKey -> FReactionFeedbackMatchKey

FActionVFXExecutionKey -> FActionVFXPlaybackKey
FActionSFXExecutionKey -> FActionSFXPlaybackKey
FReactionVFXExecutionKey -> FReactionVFXPlaybackKey
FReactionSFXExecutionKey -> FReactionSFXPlaybackKey
```

Action playback key에서는 matching 단계 입력을 제거했다.

```text
제거:
ActionFeedbackMatchKey
ActionFeedbackTiming
TriggerKey

유지:
effect asset
play type
socket / transform
```

결과:

Action / Reaction feedback 모두 `effect asset + playback condition` 기준으로 playback dedupe를 수행한다.

### 3. 구조체 rename pass

왜:

일부 타입은 `Data`, `Info`, `Request`, `Context` suffix가 실제 역할과 맞지 않았다.

어떻게:

역할 기준으로 rename했다.

```text
FTrailFeedbackData -> FActionTrailFeedbackData
FActionCombatSignalCueRequest -> FActionCombatSignalCueResolution
FPatrolPointData -> FPatrolPointSnapshot
FTargetData -> FTargetPerceptionState
FAIContext -> FAIBlackboardUpdateContext
FDamageImpactInfo -> FHitImpactContext
EDamageImpactInfoSource -> EHitImpactContextSource
FDamageAmount -> FDamageRequestAmount
```

결과:

타입 이름이 runtime state, snapshot, hit impact context, request amount, resolution 같은 실제 역할을 직접 표현한다.

### 4. W05 작업 계획 갱신

왜:

P46에서 후속 후보로 남겼던 항목 일부를 이번 브랜치에 포함했다.

어떻게:

`W05_Type_Header_Organization_Work_Plan.md`의 feedback key model과 rename 후보를 완료 상태로 갱신했다.

결과:

다음 후속 작업은 feedback 구조 / rename이 아니라 RuntimeLOD config 정리 같은 별도 범위로 좁혀진다.

### 5. CoreRedirects 추가

왜:

`USTRUCT` / `UPROPERTY` rename은 C++ build가 통과해도 기존 Blueprint / component default / asset serialized data가 새 이름으로 자동 매핑되지 않을 수 있다.

어떻게:

`Config/DefaultEngine.ini`의 `[CoreRedirects]`에 이번 rename에 대한 `StructRedirects`, `EnumRedirects`, `PropertyRedirects`를 추가했다.

결과:

기존 action hit context, feedback component data, combat hit / damage event payload field, AI runtime struct reference가 에디터 로드 시 새 타입 / 프로퍼티 이름으로 매핑될 수 있게 했다.

## 주요 처리 흐름

```text
Action / Reaction feedback request 생성
-> FeedbackMatchKey + Timing + TriggerKey로 data matching
-> matched data 선택
-> FeedbackPlaybackKey 생성
-> effect asset + playback condition 기준으로 duplicate playback 차단
-> presentation 실행
```

```text
Weapon hit overlap
-> FHitContext.ActionDataKey
-> FHitContext.HitImpactContext
-> CombatSignalSource payload / context
-> FDefaultDamageEvent
-> CombatSignalTarget payload / context
-> CombatResult packet
```

## 구현 결과

```text
Source/Portfolio/Type
-> feedback key / playback key / AI state / combat hit context type rename

Source/Portfolio/Component
-> action / reaction feedback matching and playback dedupe API 갱신
-> combat signal source / target context field 갱신

Source/Portfolio/AI
-> patrol point snapshot, target perception state, blackboard update context 이름 반영

Source/Portfolio/Core/Debug
-> feedback / combat signal debug formatter의 renamed field 반영

Docs
-> W05 work plan과 P47 PR 문서 갱신
```

## 테스트 방법

```text
1. main 대비 diff와 변경 파일 범위 확인
2. 이전 타입명 잔여 검색
3. FeedbackKey / ExecutionKey 잔여 검색
4. git diff --check 실행
5. PortfolioEditor Win64 Development build 실행
6. Editor load / Blueprint compile 확인
7. renamed feedback component data 유지 확인
8. PIE smoke 실행
9. feedback playback duplicate 로그와 combat hit 경로 확인
```

## 검증 결과

### Static check

```text
Source 기준 이전 타입명 잔여 검색
Result: Pass

Source 기준 FeedbackKey / ExecutionKey 잔여 검색
Result: Pass

git diff --check
Result: Pass
```

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### PIE

```text
PIE smoke
Result: Pass
```

### Editor data migration

```text
CoreRedirects
Result: Added

Editor load / Blueprint compile / renamed feedback data 유지 확인
Result: Pass
```

## 비범위 / 후속 작업

이번 PR은 타입명과 feedback key model 정리에 집중한다.

다음 항목은 별도 브랜치 / PR에서 처리한다.

```text
CEnemy RuntimeLOD CVar / config 정리
-> RuntimeLOD config / DataAsset 분리 작업에서 별도 처리

Feedback data asset migration
-> 이번 PR의 renamed component data migration은 확인 완료
-> feedback data를 별도 DataAsset으로 분리하는 설계는 후속 작업에서 별도 판단

AI context 하위 context 분리
-> FAIBlackboardUpdateContext 내부 aggregate를 더 나눌지는 별도 구조 작업에서 판단
```

## 관련 문서

- Work List: `W05_UE5_Portfolio_Work_List.md`
- Type Header Rules: `W05_Type_Header_Organization_Rules.md`
- Type Header Work Plan: `W05_Type_Header_Organization_Work_Plan.md`
- Previous PR: `P46_UE5_Portfolio_Pull_Request.md`

## 정리

이번 PR은 P46에서 파일 책임만 분리한 뒤 남아 있던 타입 의미 문제를 정리한다.

`FActionContext`를 제거하고, feedback matching과 playback dedupe 기준을 분리했으며, 실제 역할과 맞지 않던 구조체 이름을 정리했다. CoreRedirects를 추가해 renamed data migration을 보완했고, build / PIE smoke / editor data migration 확인까지 완료했다.
