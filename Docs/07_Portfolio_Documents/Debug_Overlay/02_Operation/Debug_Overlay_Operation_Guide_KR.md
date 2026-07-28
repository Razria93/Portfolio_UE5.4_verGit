# Debug Overlay Operation Guide

## 작업 브랜치

- `feature/debug-overlay-evidence-plan`

## 세션 운영

- 이 세션은 debug overlay evidence 작업의 계획, 문서, 구현, 검증을 다룬다.
- 기존 사용자 변경은 되돌리지 않는다.
- 현재 확인된 기존 변경:
  - `Docs/04_Pull_Request/P51_UE5_Portfolio_Pull_Request.md`
- 구현 전에 항상 실제 코드 위치와 표시 가능 여부를 확인한다.

## 진행 순서

1. 계획 문서 고정
2. evidence map 작성
3. 구현 위치 확정
4. 최소 overlay 구현
5. Editor build 검증
6. 영상 preset별 촬영 검증
7. 포트폴리오/기술문서에서 사용할 evidence 설명 정리

## 검증 기준

- 빌드가 통과해야 한다.
- overlay enable이 꺼져 있을 때 기존 동작이 변하지 않아야 한다.
- 표시 값은 실제 runtime state 또는 최근 event hook에서 온 값이어야 한다.
- 불확실한 값은 `Pending`, `N/A`, `NotCaptured`처럼 표현한다.

## Console Variable 원칙

예상 형태:

```text
Portfolio.DebugOverlay.Enabled
Portfolio.DebugOverlay.Preset
Portfolio.DebugOverlay.EventLogLimit
```

기존 debug cvar와 충돌하지 않도록 `Portfolio.DebugOverlay.*` 네임스페이스를 사용한다.

## 목표모드 사용 기준

목표모드는 작업 범위가 여러 턴에 걸쳐 이어질 때만 사용한다.

권장 목표:

```text
Debug overlay evidence workspace를 기준으로 계획 문서, evidence map, 최소 overlay 구현, 빌드 검증까지 완료한다.
```

## 에이전트 활용 기준

초기 문서/구현 계획 단계에서는 별도 에이전트가 필요하지 않다.

다만 다음 경우에는 하위 에이전트 활용을 검토한다.

- Action / Reaction, CombatSignal / AI RuntimeLOD를 병렬로 코드 조사해야 할 때
- 문서 evidence map과 구현 후보를 동시에 검토해야 할 때
- 빌드 오류 원인 범위가 넓어질 때

기본 방침은 메인 에이전트가 코드와 문서를 직접 읽고 판단하는 것이다.

