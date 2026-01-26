# Reaction 파이프라인 구현: UCReaction / UCReactionComponent / AnimNotifyState_Reaction

## 제목

✨ feat: Reaction 파이프라인 구현 (#24)

## 요약

- `FReactionDataKey` + `FReactionData` 기반의 `Data-Driven` 리액션 선택 구조를 추가함.

- `UCReactionComponent`에서 리액션 타입/데이터 해결 → 재생/교체 → 종료 후 복구까지의 파이프라인을 구성함.

- `UCReaction` 기본 `Executor`를 통해 `Validate/Begin/Stop/End` 및 `Montage` 종료 처리 흐름을 정리함.

- `UCAnimNotifyState_Reaction`으로 `Interruptible/Cancelable/Immune Window`를 통해 `Reaction`에 대한 간섭을 제어할 수 있도록 라우팅함.

- `Hit/Dead Reaction` 정책을 구현해 `Dead`는 중단/취소 불가, `Hit`는 `Dead`에 의해 인터럽트를 허용하도록 구성함.

- `State` 시스템에 `Reaction` 모드를 추가해 상태 전이를 명시화함.


---

## 완료 항목

### 1. Reaction 데이터/Executor 등록 구조 구성

- `FReactionDataKey` / `FReactionData`로 리액션 키-데이터 구조 정의.

- `UCReactionComponent`에서 키 기반 데이터 조회 + `Executor` 재사용 캐시 구성.

### 2. Reaction 파이프라인 및 상태/이동 제어

- `ProcessReaction` 흐름에서 타입/데이터 해결 후 재생 또는 교체 처리.

- 반응 중 이동 제한 및 `State`를 `Reaction`으로 전환, 종료 시 `Idle`로 복구.

### 3. Executor 기반 생명주기 정리

- `UCReaction`에서 `Montage` 재생/종료 바인딩 및 종료 라우팅을 표준화함.

- `Stop` 시 `End(true)`를 통해 강제 종료/정리 경로를 보장함.

### 4. AnimNotifyState 기반 Reaction Window 라우팅

- `Notify Begin/End`에서 `Reaction Window`를 열고 닫아 정책 게이트로 사용함.

### 5. Hit/Dead Reaction 정책 구현

- `Hit`은 `Dead` 인터럽트를 허용하고, 그 외는 `Window` 상태를 따른다.

- `Dead`는 인터럽트/캔슬 불가로 고정함.


---

## 테스트 방법

1. `UCReactionComponent`에 `ReactionDatas`/`ReactionClasses가` 설정되어 있는지 확인.

2. `FTakeDamageResult`의 `Hit`/`Death` 플래그로 `RequestReaction`이 호출되는지 확인.

3. `Reaction Montage` 재생/종료 시 이동/상태 전이가 정상인지 확인.

4. `Reaction Window` 구간에서 `Interruptible`/`Cancelable` 동작이 반영되는지 확인.


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-reaction`

- 이슈: `#24`


---

## 노트

- `ApplyDamageSpecKey` + `ReactionType` 기반 매핑으로 리액션 선택을 데이터화하여 확장성을 확보함.

- Window 기반 정책으로 인터럽트/캔슬 제어를 분리해 Executor의 책임을 단순화함.


---