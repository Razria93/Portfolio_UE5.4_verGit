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

이번 Fix PR에서는 `UCTargetingComponent`가 `OnDestroyed` 없이 만료된 Weak Target을 정리하지 못하거나, 교체된 이전 Target의 늦은 Destroy callback이 현재 Target을 해제할 수 있는 수명 경계 문제를 보정한다.

Target이 사라지는 경로와 Destroy callback의 객체 정체성을 명시적으로 구분해 Lock Assist와 Target HUD가 실제 Target 상태와 일치하도록 유지한다.

---

## 원인

- `ValidateCurrentTarget()`이 `CurrentTarget.Get() == nullptr`인 경우 바로 반환해, 명시적인 null과 GC 또는 Level Streaming Unload로 만료된 Weak Target을 구분하지 못했다.

- 만료 정리 이벤트가 발행되지 않으면 `UCTargetLockAssistComponent`와 `UCTargetHUDPresenterComponent`가 Target 해제를 통지받지 못해 이전 정책을 유지할 수 있었다.

- `HandleCurrentTargetDestroyed()`가 callback actor가 현재 Target인지 확인하지 않고 `CurrentTarget`을 초기화해, 교체된 이전 Target의 지연 또는 재진입 callback이 새 Target을 해제할 가능성이 있었다.

---

## 변경 사항

- `ValidateCurrentTarget()`에서 `CurrentTarget.IsStale()`을 먼저 검사한다.

- 만료된 Weak Target은 `ClearExpiredTarget()`에서 다음과 같이 정리한다.

```text
CurrentTarget Reset
ValidationElapsedTime Reset
OnTargetChanged(nullptr, nullptr)
```

- 명시적으로 Target이 없는 상태에서는 기존처럼 불필요한 변경 이벤트를 발행하지 않는다.

- Destroy callback actor는 현재 `CurrentTarget`과 `HasSameIndexAndSerialNumber()`로 비교한다.

- Object Index와 Serial Number가 모두 일치하는 현재 Target의 Destroy callback만 `OnTargetChanged(DestroyedTarget, nullptr)`로 처리한다.

- 교체된 이전 Target의 늦은 callback과 같은 주소를 재사용한 다른 UObject의 callback은 현재 Target을 변경하지 않는다.

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

- Lock Assist와 Target HUD Presenter가 `OnTargetChanged` 수신 후 TargetingComponent의 현재 상태를 다시 조회하는 경로 확인

- 실제 Streaming Level Unload와 지연 Destroy callback을 강제로 발생시키는 PIE 재현은 후속 Character Destroy Lifecycle 통합 검증 범위로 유지한다.

---

## 관련 문서

- `Docs/04_Pull_Request/P58_UE5_Portfolio_Pull_Request.md`

- `Docs/06_notes/task_briefs/W05_Player_Targeting/`

---
