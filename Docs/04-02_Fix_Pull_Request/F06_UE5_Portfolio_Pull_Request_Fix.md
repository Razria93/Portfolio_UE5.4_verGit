# UE5 Portfolio Pull Request Fix

## 제목

**F06: Targeting Weak Target Lifecycle 보정**

## 날짜

**2026.08.11**

## 상태

- [x] **완료**

---

## 브랜치

- `fix/targeting-stale-target-lifecycle`

---

## 요약

이번 Fix PR에서는 `UCTargetingComponent`가 World에서 수명을 종료한 Target을 계속 유지하거나, 교체된 이전 Target의 늦은 lifecycle callback이 현재 Target을 해제할 수 있는 수명 경계 문제를 보정한다.

Actor의 정상적인 gameplay 수명 종료는 `OnEndPlay`로 관찰하고, 이미 Weak Object가 만료된 예외 경로는 `IsStale()` 검사로 보완해 Lock Assist와 Target HUD가 실제 Target 상태와 일치하도록 유지한다.

---

## 원인

- 기존 `OnDestroyed` 구독은 명시적 `Destroy()`만 관찰하므로 Streaming Level 제거처럼 Actor가 `EndPlay(RemovedFromWorld)`에 진입하지만 파괴되거나 GC되지 않는 경로를 처리하지 못했다.

- 이 상태에서는 `CurrentTarget.IsStale() == false`이고 Actor도 `IsValid()`일 수 있어, Health와 거리 검증이 통과하면 World에서 제거된 Target을 계속 유지할 가능성이 있었다.

- 만료 또는 EndPlay 정리 이벤트가 발행되지 않으면 `UCTargetLockAssistComponent`와 `UCTargetHUDPresenterComponent`가 Target 해제를 통지받지 못해 이전 정책을 유지할 수 있었다.

- lifecycle callback actor가 현재 Target인지 확인하지 않고 `CurrentTarget`을 초기화하면, 교체된 이전 Target의 지연 또는 재진입 callback이 새 Target을 해제할 가능성이 있었다.

---

## 변경 사항

- Target lifecycle의 주 구독을 `OnDestroyed`에서 `OnEndPlay`로 변경한다.

- `OnEndPlay`는 명시적 Destroy뿐 아니라 `RemovedFromWorld`, Level Transition, PIE 종료와 같은 Actor의 World 수명 종료 경로를 포괄한다.

- `HandleCurrentTargetEndPlay()`는 callback actor를 현재 `CurrentTarget`과 `HasSameIndexAndSerialNumber()`로 비교한다.

- Object Index와 Serial Number가 모두 일치하는 현재 Target의 EndPlay callback만 다음과 같이 처리한다.

```text
CurrentTarget Reset
ValidationElapsedTime Reset
OnTargetChanged(EndedTarget, nullptr)
```

- 교체된 이전 Target의 늦은 callback과 같은 주소를 재사용한 다른 UObject의 callback은 현재 Target을 변경하지 않는다.

- `ValidateCurrentTarget()`의 `CurrentTarget.IsStale()` 검사는 EndPlay callback을 놓쳤거나 Weak Object가 먼저 만료된 경우를 정리하는 fallback으로 유지한다.

- 만료된 Weak Target은 `ClearExpiredTarget()`에서 다음과 같이 정리한다.

```text
CurrentTarget Reset
ValidationElapsedTime Reset
OnTargetChanged(nullptr, nullptr)
```

- 명시적으로 Target이 없는 상태에서는 기존처럼 불필요한 변경 이벤트를 발행하지 않는다.

---

## 변경하지 않은 것

- Target 획득과 중앙 우선 점수 정책

- 좌우 Target Switching 정책

- 사망 또는 거리 초과 Target의 기존 `ClearTarget()` 경로

- Lock Assist camera와 movement rotation 정책

- Target HUD Marker의 projection 및 표시 정책

- Enemy Actor의 Dead 이후 Destroy 정책

---

## 검증 결과

- `git diff --check` 통과

- `PortfolioEditor Win64 Development` 빌드 성공

- UE 5.4 `AActor::Destroyed()`가 `RouteEndPlay(EEndPlayReason::Destroyed)`를 먼저 호출하므로 `OnEndPlay` 단일 구독으로 직접 Destroy까지 포괄하는 경로 확인

- Lock Assist와 Target HUD Presenter가 `OnTargetChanged` 수신 후 TargetingComponent의 현재 상태를 다시 조회하는 경로 확인

- 실제 Streaming Level 제거와 지연 EndPlay callback을 강제로 발생시키는 PIE 재현은 후속 Character Destroy Lifecycle 통합 검증 범위로 유지한다.

---

## 관련 문서

- `Docs/04_Pull_Request/P58_UE5_Portfolio_Pull_Request.md`

- `Docs/06_notes/task_briefs/W05_Player_Targeting/`

---
