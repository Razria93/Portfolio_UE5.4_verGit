# Debug Overlay 운영 가이드

## 고정 정책

- 이 브랜치의 문서는 기본적으로 한국어(KR)로 작성한다.
- 작업 단위가 끝나면 권장 커밋 메시지 형식으로 자동 커밋한다.
- 작업 종료 시 항상 다음 작업을 짧게 제안한다.
- 진행 프롬프트는 파일로 만들지 않고 채팅에서 직접 제안한다.
- 기존 사용자 변경은 되돌리지 않는다.
- 구현 전에 실제 코드 위치와 표시 가능 여부를 먼저 확인한다.
- 에이전트 활용이 유효하다고 판단되면 적극적으로 사용한다.

## 작업 브랜치

- `feature/debug-overlay-evidence-plan`

## 작업 범위

이 세션은 debug overlay evidence 작업의 계획, 문서, 구현, 검증을 다룬다.

목표는 완성형 게임 HUD가 아니라 이력서, 포트폴리오, 기술문서, 제출 영상에서 사용할 수 있는 개발 전용 evidence overlay를 만드는 것이다.

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
- Shipping 기능처럼 보이거나 동작하지 않도록 개발 전용 gate를 둔다.

## Console Variable 원칙

예상 형태:

```text
Portfolio.DebugOverlay.Enabled
Portfolio.DebugOverlay.Preset
Portfolio.DebugOverlay.EventLogLimit
```

기존 debug cvar와 충돌하지 않도록 `Portfolio.DebugOverlay.*` 네임스페이스를 사용한다.

## 목표모드 사용 기준

목표모드는 작업 범위가 여러 턴에 걸쳐 이어질 때 사용한다.

권장 목표:

```text
Debug overlay evidence workspace 기준으로 evidence map 확정, 최소 overlay 구현, 빌드 검증까지 완료한다.
```

## 에이전트 활용 기준

에이전트는 불필요한 작업에 형식적으로 사용하지 않는다. 다만 다음 조건에 해당하면 적극적으로 사용한다.

- 조사 범위가 Action / Reaction, CombatSignal / Damage, Enemy AI / Runtime LOD처럼 독립적인 도메인으로 나뉠 때
- 메인 작업이 문서 통합, 구현 판단, 코드 변경을 진행하는 동안 병렬 코드 조사가 가능할 때
- 구현 전 근거 수집, 위험 검토, 후보 비교처럼 병렬 검토가 품질을 높일 때
- 빌드 오류 원인 범위가 넓고 여러 후보를 동시에 확인해야 할 때
- 테스트/검증 로그 분석을 구현 작업과 분리해 진행할 수 있을 때

에이전트에 맡기는 작업은 명확한 범위와 산출물을 가져야 한다.

- 읽기 전용 조사인지, 코드 수정 작업인지 명확히 구분한다.
- 코드 수정 작업은 파일 범위를 분리한다.
- 최종 판단과 문서 반영은 메인 에이전트가 직접 수행한다.
- 하위 에이전트 결과는 그대로 확정하지 않고 코드 근거와 함께 검토한다.
