# Debug Overlay Working Prompt

아래 프롬프트는 다음 작업 세션 시작용이다.

```text
현재 브랜치 `feature/debug-overlay-evidence-plan`에서 debug overlay evidence 작업을 이어간다.

목표:
이력서, 포트폴리오, 기술문서, 제출 영상에서 사용할 수 있는 개발 전용 debug overlay를 설계하고 최소 구현한다.
이 overlay는 완성형 게임 HUD가 아니라 Action / Reaction, CombatSignal / Damage, Enemy AI, Runtime LOD 실행 흐름을 증명하는 evidence 도구다.

우선 기준 문서를 읽어라.
- Docs/07_Portfolio_Documents/Debug_Overlay/README.md
- Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Plan_KR.md
- Docs/07_Portfolio_Documents/Debug_Overlay/02_Operation/Debug_Overlay_Operation_Guide_KR.md
- Docs/07_Portfolio_Documents/Debug_Overlay/03_Evidence_Map/Debug_Overlay_Evidence_Map_KR.md
- Docs/07_Portfolio_Documents/Debug_Overlay/04_Capture_Presets/Debug_Overlay_Capture_Presets_KR.md

진행 방식:
1. 현재 코드에서 overlay 표시 항목별 실제 근거 위치를 다시 확인한다.
2. Evidence map의 상태를 Ready / HookNeeded / ReviewNeeded / Exclude로 갱신한다.
3. 최소 구현 위치를 제안하되, 기존 gameplay 로직에 영향을 주지 않는 방향으로 설계한다.
4. 구현 전에는 변경 파일과 이유를 짧게 보고한다.
5. 구현한다면 P0 최소 overlay부터 진행한다.
6. 모든 debug overlay 코드는 `#if !UE_BUILD_SHIPPING` 또는 console variable gate로 보호한다.
7. 빌드 검증이 필요하면 Editor build를 실행하고 결과를 요약한다.

목표모드:
여러 턴으로 이어질 가능성이 크면 다음 목표로 목표모드를 사용한다.
`Debug overlay evidence workspace 기준으로 evidence map 확정, 최소 overlay 구현, 빌드 검증까지 완료한다.`

에이전트 활용:
초기에는 메인 에이전트가 직접 진행한다.
Action / Reaction, CombatSignal / Damage, Enemy AI / Runtime LOD 조사가 병렬로 커질 때만 하위 에이전트 활용을 제안한다.

주의:
- 기존 사용자 변경을 되돌리지 않는다.
- 현재 확인된 기존 변경 `Docs/04_Pull_Request/P51_UE5_Portfolio_Pull_Request.md`는 건드리지 않는다.
- 실제 코드에서 읽지 못하는 값은 성공 evidence처럼 표시하지 않는다.
- Shipping HUD처럼 보이게 만들지 않는다.
```

