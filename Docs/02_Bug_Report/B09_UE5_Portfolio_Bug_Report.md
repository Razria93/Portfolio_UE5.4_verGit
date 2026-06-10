# UE5 Portfolio Bug Report

## 제목

**B09: Action / Reaction intervention window 설정 누락 및 notify 구간 문제로 active execution이 중단되지 않는 문제**

## 날짜

**2026.05.22**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/orchestration-refactor`

---

## 요약

- Action / Reaction intervention 구조에서 active execution의 `Allow` 조건이 명확히 설정되지 않으면 incoming execution이 active execution을 중단하지 못하는 문제가 발생했다.

- 대표 증상은 공격 Action 중 HitReaction이 들어와도 action montage가 계속 진행되는 현상과, active HitReaction 중 새 HitReaction이 들어와도 기존 reaction이 교체되지 않는 현상이었다.

- 원인은 active side allow 정책 누락, override 내부의 Super 호출로 인한 판단 경로 혼선, montage 0 frame ~ last frame notify state 배치에 따른 filter cache 타이밍 문제였다.

- 최종 수정에서는 intervention 판단을 notify 주입 filter에서 `ActionData` / `ReactionData`의 data rule 기반 구조로 이전하고, notify는 allow window timing만 전달하도록 축소했다.

---

## 영향 범위

- Action / Reaction intervention 판단과 interrupt 가능성 판정

- HitReaction / DeadReaction 우선순위와 active action 중단 정책

---

## 환경

- 엔진: Unreal Engine 5.4

- 관련 브랜치:
  - `feature/orchestration-refactor`

- 관련 코드:
  - `Source/Portfolio/Action/CAction.cpp`
  - `Source/Portfolio/Action/CAction.h`
  - `Source/Portfolio/Reaction/CReaction.cpp`
  - `Source/Portfolio/Reaction/CReaction.h`
  - `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`
  - `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp`
  - `Source/Portfolio/Notify/CAnimNotifyState_ExecutionInterventionWindow.cpp`
  - `Source/Portfolio/Type/CWeaponStructure.h`

- 관련 에셋:
  - `Content/04_Montage/Sword/M_Attack_Sword_0.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_1.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_2.uasset`
  - `Content/04_Montage/Damaged/M_HitReact.uasset`

---

## 발생 조건

- incoming request는 intervention을 원하지만 active action 쪽 allow window나 정책이 분리되어 있지 않으면 발생한다.

- montage boundary의 notify가 timing과 policy를 함께 들고 있으면 재현된다.

---

## 재현 방법

1. Enemy가 `ComboAttack` 0 / 1 / 2를 순차적으로 실행하도록 한다.

2. Player가 피격하여 `HitReaction`이 발생하도록 한다.

3. Action 중 HitReaction이 들어올 때 active Action이 interrupt되는지 확인한다.

4. active `HitReaction` 중 다시 `HitReaction`이 들어올 때 기존 reaction이 교체되는지 확인한다.

5. intervention allow window 구간을 montage 0 frame ~ last frame과 1 frame ~ last - 1 frame 배치로 비교한다.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- Action 실행 중 HitReaction이 들어오면 active Action이 `Interrupted` stop reason으로 중단되어야 한다.

- active `HitReaction` 중 다시 `HitReaction`이 들어오면, active reaction이 중단 가능한 상태일 때 새 `HitReaction`으로 교체되어야 한다.

- intervention 판단은 incoming side의 want와 active side의 allow가 모두 성립할 때만 성공해야 한다.

**실제 결과**

- 공격 Action에 HitReaction 대상 allow 조건이 없으면 피격되어도 Action montage가 계속 진행되었다.

- active `HitReaction` 중 새 `HitReaction`이 들어왔을 때 incoming side want는 true였지만 active side allow가 false가 되어 request가 reject되었다.

- notify state를 montage의 0 frame부터 마지막 frame까지 배치하면 montage 전환 과정에서 filter begin/end 타이밍이 기대와 다르게 동작할 수 있었다.

- 1 frame부터 마지막 - 1 frame까지 배치하면 같은 조건에서 counterpart filter가 정상 유지되고 matching이 성공했다.

---

## 원인

### 1. Allow 판단은 active execution 쪽 책임이다

- `Want`는 incoming execution이 어떤 active execution을 중단하려는지 표현한다.

- `Allow`는 active execution이 어떤 incoming execution에 의해 중단될 수 있는지 표현한다.

- HitReaction이 Action을 interrupt하려면 incoming `HitReaction` 쪽 want와 active `Action` 쪽 allow가 모두 필요하다.

### 2. HitReaction 재진입도 active HitReaction의 allow가 필요하다

- active `HitReaction` 중 새 `HitReaction`이 들어오는 경우에도 active reaction 쪽 allow rule이 필요하다.

- incoming `HitReaction`이 interrupt를 원해도, active `HitReaction`이 해당 interrupt를 허용하지 않으면 intervention은 실패한다.

### 3. override 내부 Super 호출이 판단 경로를 흐리게 한다

- 특정 Reaction이 intervention 판단을 override하면서 `Super::MatchesWantIntervention()` 또는 `Super::MatchesAllowIntervention()`을 함께 호출하면 base filter matching 경로와 class 고정 정책이 섞인다.

- 이 구조에서는 로그상으로 override 정책과 base filter 정책 중 어느 경로로 통과했는지 구분하기 어렵다.

### 4. notify state가 policy와 timing을 함께 소유하고 있었다

- 기존 notify state는 window timing과 intervention filter policy를 함께 들고 있었다.

- montage 경계에 notify state를 배치할 때 begin/end 호출 타이밍이 흔들리면 filter cache 상태까지 함께 흔들릴 수 있었다.

---

## 수정

### 1. Intervention rule을 data 기반으로 이전

- `FExecutionInterventionWantRule`과 `FExecutionInterventionAllowRule`을 추가했다.

- `FActionData` / `FReactionData`에 `WantInterventionRules`와 `AllowInterventionRules`를 분리해 추가했다.

- `WantIntervention()`은 incoming data rule을 기준으로 active participant를 매칭한다.

- `AllowIntervention()`은 active data rule을 기준으로 incoming participant를 매칭한다.

- 실제 중단 결과는 `Interrupted`로 통합했다.

### 2. Allow window는 timing gate로 축소

- `CAnimNotifyState_ExecutionInterventionWindow`는 `WindowKey` begin / end 전달만 담당하도록 축소했다.

- active executor의 열린 window는 runtime set으로 관리한다.

- `AllowInterventionRules` 중 `Window` timing rule만 현재 열린 window key를 확인한다.

- notify class / file 이름은 asset 안정성을 위해 유지하되, 표시 이름은 allow window 의미에 맞춰 정리했다.

### 3. Reaction 특수 정책 정리

- `UCReaction_Hit::WantIntervention()`과 `UCReaction_Dead::WantIntervention()` override를 제거하고 data rule 방식으로 이전했다.

- Dead reaction의 force intervention 정책은 orchestrator 정책으로 유지했다.

- `UCReaction_Dead::AllowIntervention()`은 active dead reaction이 다른 incoming execution에 의해 끊기지 않도록 막는 terminal guard로 유지했다.

---

## 수정 기준

- incoming want와 active allow data rule을 분리한다.

- notify는 timing만 제공하고 policy는 data / gate 쪽에서 판단한다.

---

## 검증 결과

- `feature/orchestration-refactor` PR에서 Action / Reaction intervention 판단 구조가 data rule 기반으로 정리된 것을 확인했다.

- Action / Reaction intervention filter가 notify 주입 방식에서 `ActionData` / `ReactionData` rule 방식으로 이전된 것을 확인했다.

- `Cancelled` stop reason과 구 intervention window API 잔여 참조가 없는 것을 확인했다.

- `HitReaction`이 action interrupt 가능 구간에서 정상 개입하는 것을 확인했다.

- Dead reaction이 active action / reaction 여부와 관계없이 최종 우선 실행되는 것을 확인했다.

- collision / hit context / allow intervention window notify begin / end가 정상 동작하는 것을 확인했다.

---

## 회귀 방지 기준

- allow window에서 HitReaction이 active action을 interrupt해야 한다.

- Dead reaction 우선순위가 유지되어야 한다.

- `Cancelled` 또는 이전 intervention API 참조가 남지 않아야 한다.

---

## 관련 PR / 문서

- Issue Checklist: `D18_UE5_Portfolio_Issue_Checklist.md`

- PR: `P17_UE5_Portfolio_Pull_Request (KR).md`

---

## 비고

- 이 버그는 intervention 판단에서 incoming의 want와 active의 allow 책임을 분리해야 한다는 점을 확인한 사례다.

---
