# p.10 Death Lifecycle — Project Audit and Composition Plan

> 목적: `Death Lifecycle` 페이지가 Dead 이후의 표현·Facing 억제·종료 처리를 하나의 애니메이션 완료 이벤트로 축소하지 않고, 확인 가능한 수명주기 경계로 설명하도록 고정한다.

## 1. 페이지 핵심 주장

> Health의 Dead 전이 이후에도 Presentation·Facing 억제·Finalize·Destroy는 별도 수명주기로 관리한다.

- `Health`는 생명 상태를 Dead로 확정한다.
- `ACEnemy`의 Death Coordinator는 사망 표현과 최종 종료 흐름을 조율한다.
- Combat Target이 남아 있어도 Dead 상태에서는 Facing 정책이 이를 회전 입력으로 소비하지 않는다.
- Presentation 완료와 Fallback은 `FinalizeDeath` 요청으로 합류하며, Actor 제거는 다음 Tick의 종료 경로에서 확정된다.

## 2. 코드·문서 근거

| 범위 | 확인 근거 | 포트폴리오에 표현할 수 있는 사실 |
| --- | --- | --- |
| Dead 전이와 종료 흐름 | `Docs/05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md` | Health Dead → Presentation → Finalize / Destroy의 분리된 흐름 |
| Death Coordinator | `Source/Portfolio/Character/CEnemy.cpp` 및 관련 Death lifecycle 경로 | Presentation 시작·완료·Fallback을 종료 경로로 조율 |
| Dead 중 Facing 억제 | `Source/Portfolio/Component/CEnemyCombatTargetFacingComponent.cpp` | Dead 전이 시 deferred sync 취소, Gameplay Focus 해제, 이동 회전 모드 복구 |
| 늦은 Callback 재검사 | `ShouldSuppressCombatTargetFacing()` 및 deferred resolve 경로 | 이미 예약된 sync도 resolve 시점에 Dead 억제 조건을 재확인 |

## 3. 페이지 구성

### A. 작동 흐름 — Dead 전이부터 Presentation 완료까지

- 중심 시각: Dead Presentation 게임플레이 프레임 1장, 16:9.
- 오른쪽 흐름: `Dead 확정 → 전투 경로 정리 → Presentation 시작 → Finalize / Destroy`.
- 목적: 사망을 단일 Reaction이 아니라, 서로 다른 완료 시점을 가진 lifecycle로 읽히게 한다.

### B. 런타임 수명주기 관찰

| 관찰 항목 | 캡처에서 확인할 값 | 현재 상태 |
| --- | --- | --- |
| Dead Entry | Health state, death event, active execution 정리 | 신규 캡처 필요 |
| Facing Suppressed | Gameplay Focus, rotation mode, deferred sync 상태 | 신규 캡처 필요 |
| Finalize Route | presentation complete 또는 fallback, finalize request, destroy 흐름 | 신규 캡처 필요 |

### C. 대표 문제 해결 — Dead 상태의 늦은 Facing 재적용 차단

| 문제 | 해결 | 결과 |
| --- | --- | --- |
| Dead Reaction 중에도 남아 있는 Combat Target과 예약된 deferred sync가 Controller Facing을 다시 적용할 수 있었다. | Dead 전이에서 deferred sync를 취소하고 Gameplay Focus를 해제한다. 이후 resolve 시점에도 `ShouldSuppressCombatTargetFacing()`을 재확인한다. | Combat Target의 보관과 Facing 입력 소비를 분리해, 사망 표현 중 대상 추적 회전이 재개되지 않는다. |

## 4. 표현 제한

- 코드·구조 문서는 lifecycle과 facing 억제 경로를 뒷받침한다.
- 제출용으로는 실제 Dead Presentation, Focus/Rotation 값, Finalize 흐름의 신규 런타임 캡처가 필요하다.
- 캡처 전에는 "사망 표현 중 회전이 재개되지 않음을 시각적으로 검증했다"고 단정하지 않는다.
- Combat Target 삭제 자체를 Dead 페이지의 책임으로 표현하지 않는다. 타깃 정리와 참여 해제는 별도 lifecycle / participation 경계에서 다룬다.

## 5. 완료 기준

- [ ] Dead Presentation Hero 16:9 1장 확보
- [ ] Dead Entry Overlay 또는 Event History crop 확보
- [ ] Facing Suppressed(Dead) 상태 및 Focus/Rotation crop 확보
- [ ] Presentation 완료 또는 Fallback → Finalize → Destroy 흐름 crop 확보
- [ ] 위 네 증거가 Evidence Ledger와 Claim Freeze Log에 연결됨
- [ ] 페이지의 설명이 Dead 후 Target 삭제 또는 시각 검증 완료를 과장하지 않음
