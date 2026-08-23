# UE5 Portfolio Pull Request Fix

## 제목

**F07: Combat Participation Review Follow-up 보정**

## 날짜

**2026.08.23**

## 상태

- [x] **완료**

---

## 브랜치

- `fix/combat-participation-review-followup`

---

## 요약

이번 Fix PR에서는 P60 Combat Participation 통합 후 리뷰에서 확인된 두 가지 정확성 문제를 보정한다.

첫째, Action 실행 종료 이벤트가 runtime 초기화 뒤에 현재 cache를 다시 읽어 실행 identity를 잃을 수 있었다. 둘째, Combat Participation class redirect가 C++ 타입명이 아닌 UE reflected class path를 사용해야 하는데, 기존 설정의 class name 경로가 이를 만족하지 않았다.

Combat Participation Evidence / Assignment 정책을 변경하지 않고, Action event 상관관계와 기존 serialized class reference 호환성만 바로잡는다.

---

## 원인

- `UCAction::Complete()`와 `UCAction::HandleActionStop()`은 `ClearRuntime()` 이후 `EmitActionEvent()`를 호출했다. `ClearRuntime()`은 `ActiveDataKey_Cached`와 `ActionRequestSerial_Cached`를 초기화하므로, `Completed / Interrupted / Ignored` 이벤트가 시작 이벤트와 같은 Action 실행 단위를 식별하지 못할 수 있었다.

- `UCAction::EmitActionEvent()`는 호출자가 전달한 index만 사용하고 Action type과 request serial은 mutable runtime cache에서 읽었다. 따라서 종료 뒤 event identity 전체가 cache 수명에 의존했다.

- `UCWorldSubsystem_CombatParticipation`, `UCEnemyCombatParticipationComponent`는 C++ 타입명이다. `/Script/Portfolio`의 reflected class path는 선행 `U`를 제외한 `CWorldSubsystem_CombatParticipation`, `CEnemyCombatParticipationComponent`이므로, 기존 redirect의 old/new name이 실제 class path와 일치하지 않았다.

---

## 변경 사항

- `EmitActionEvent()`가 `FActionDataKey`와 `ActionRequestSerial`을 명시적으로 받도록 변경했다.

- Action 시작, Combo chain window, chain event는 현재 실행 identity를 명시적으로 전달한다.

- `Complete()`와 `HandleActionStop()`은 `ClearRuntime()` 전에 action key와 request serial을 복사하고, 초기화 뒤에도 그 immutable snapshot으로 terminal event를 발행한다.

```text
ActionStarted
-> ActionType / ActionIndex / ActionRequestSerial

ActionCompleted / ActionInterrupted / ActionIgnored
-> 같은 ActionType / ActionIndex / ActionRequestSerial
```

- `DefaultEngine.ini`의 Combat Participation class redirect old/new name을 실제 reflected class path인 `CWorldSubsystem_...`, `CEnemy...` 형식으로 정정했다.

---

## 변경하지 않은 것

- Combat Participation Evidence, Candidate, Assignment, Extra admission 정책

- Investigate handoff, Action assignment lock, ReturnHome suppression 정책

- Action / Reaction Orchestrator의 요청·중재 정책

- Blueprint, UAsset, TestRoom 데이터

- P60의 Combat Participation 구조 문서 및 원본 PR 기록

---

## 검증 결과

- `git diff --check` 통과

- `PortfolioEditor Win64 Development` UHT / C++ 빌드 통과

- 모든 `EmitActionEvent()` 호출이 action key와 request serial을 명시 전달하는지 검색 확인

- generated reflection code와 기존 `C...` Core Redirect 명명 규칙을 대조해 Combat Participation redirect 대상 경로 확인

- 이전 class name이 저장된 별도 legacy asset fixture는 현재 프로젝트에 없으므로, 실제 redirect migration load 재현은 수행하지 않았다. 이번 보정은 reflected class path 정합성과 Editor build로 검증한다.

---

## 관련 문서

- `Docs/04_Pull_Request/P60_UE5_Portfolio_Pull_Request.md`

- GitHub PR #116

---
