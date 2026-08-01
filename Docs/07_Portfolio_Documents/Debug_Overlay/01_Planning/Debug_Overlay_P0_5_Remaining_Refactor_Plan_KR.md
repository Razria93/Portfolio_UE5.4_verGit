# Debug Overlay P0.5 Remaining Refactor Plan

## 1. 목적

이 문서는 P0.5 debug overlay가 TestRoom PIE에서 동작 확인된 이후, 최종 evidence로 넘어가기 전에 남은 보완점을 고정한다.

목표는 기능을 계속 늘리는 것이 아니라 최종 캡처 전 필수 작업, P0.5 보강 후보, P1 이후 항목, 지금 하지 않을 항목을 분리하는 것이다. Round1 evidence package는 최종 제출 후보가 아니라 1차 동작 확인 패키지로만 사용한다.

## 2. 기준 상태

현재 P0.5 overlay는 다음을 확인한 상태다.

- TestRoom PIE에서 `[Debug Overlay P0.5]` 표시 확인
- Player/Enemy panel 분리 표시 확인
- Player/Enemy panel의 공통 항목 표시 확인
  - State
  - Action
  - Reaction
  - Guard
  - Movement
  - HP
  - Runtime LOD
  - AI
- `Recent Execution`, `Recent Combat`, `Recent AI`, `Event Log` 표시 확인
- Enemy selection은 `WorldScanFallback` 기반
- EventLog 추가 축약은 현재 기준에서 진행하지 않음
- Runtime LOD는 아직 최종 evidence 주장 제외

Round1 패키지 위치:

- `Docs/98_Evidence/01_Screenshot/DebugOverlay/Round1`
- `Docs/07_Portfolio_Documents/Debug_Overlay/06_Evidence_Package/Debug_Overlay_P0_5_Evidence_Package_Round1_KR.md`

## 3. Round1 Evidence에서 확인된 보완점

| 항목 | 현재 상태 | 최종 캡처 전 판단 |
| --- | --- | --- |
| Overlay 동작 | PIE에서 정상 표시 | 유지 |
| Player/Enemy panel | stacked layout과 blue/red tab 확인 | 유지 |
| Enemy source | `WorldScanFallback` 표시 | P0.5에서는 유지, Target Component 구현 후 교체 |
| EventLog | 현재 compact key/value format으로 충분 | 추가 축약 보류 |
| Player/Enemy별 EventLog | world 단위 공통 log | P1 후보 |
| Runtime LOD | `N/A` 가능 | 최종 성공 evidence에서 제외 |
| 캡처 화면 | editor/taskbar/mouse tooltip 노출 가능 | 최종 후보 재촬영 필요 |
| 파일 구분 | Round1 패키지 생성 | final candidate는 별도 폴더 필요 |

## 4. 최종 캡처 전 필수 보완

최종 evidence 후보로 넘어가기 전에 다음은 반드시 처리한다.

1. 최종 캡처 화면 정리
   - mouse control tooltip 노출 제거
   - editor panel 또는 taskbar 노출 여부를 캡처 목적에 맞게 정리
   - PIE viewport만 캡처할지, editor 설정까지 함께 보여줄지 캡처별로 결정

2. final candidate 경로 분리
   - Round1은 1차 패키지로 보존
   - 최종 후보는 별도 경로에 저장
   - 권장 경로: `Docs/98_Evidence/01_Screenshot/DebugOverlay/FinalCandidate`

3. 최종 캡처 scene/action 순서 확정
   - Idle baseline
   - Walk / Run
   - Guard In
   - Guard Out
   - Player Hit
   - Enemy Hit
   - Block Hit
   - Parry
   - Enemy Stagger

4. 캡처별 evidence claim 범위 정리
   - 각 이미지가 무엇을 증명하는지 한 문장으로 고정
   - `N/A`, `NotCaptured`, `WorldScanFallback`을 성공 evidence처럼 과장하지 않음
   - Runtime LOD는 실제 tier 표시 전까지 최종 주장 제외

## 5. P0.5 보강 후보

| 후보 | 목적 | 현재 판단 |
| --- | --- | --- |
| Stagger Count | Parry 누적과 stagger trigger 조건 표시 | 코드 근거 있음. 표시 방식은 추가 결정 필요 |
| Enemy source/fallback 문구 | fallback evidence임을 명확히 표시 | 현재 유지. 문구 위치/간격만 필요 시 보완 |
| HP / Movement / Guard 가독성 | 캡처에서 한 줄 의미를 빠르게 읽도록 유지 | 현재 `|` 구분 유지 |
| Runtime LOD `N/A` 설명 | 최종 문서에서 오해 방지 | 문서 보강으로 처리 |

## 6. Stagger Count 검토 결과

### 6.1 코드 근거

Player와 Enemy 모두 parry 누적 count를 갖고 있다.

| 대상 | 파일 | 현재 구조 |
| --- | --- | --- |
| Player | `Source/Portfolio/Character/Player/CPlayer.h` | `ParryStaggerThreshold`, `ParryResultCount` |
| Player | `Source/Portfolio/Character/Player/CPlayer.cpp` | `HandleParryCombatResult()`에서 count 증가, threshold 도달 시 stagger request 후 reset |
| Enemy | `Source/Portfolio/Character/Enemy/CEnemy.h` | `ParryStaggerThreshold`, `ParryResultCount` |
| Enemy | `Source/Portfolio/Character/Enemy/CEnemy.cpp` | `HandleParryCombatResult()`에서 count 증가, threshold 도달 시 stagger request 후 reset |
| Debug hook | `Source/Portfolio/Core/Debug/FCombatResultDebug.cpp` | `RecordParryStackUpdatedForAudit()`가 `Count=%d/%d`, `StaggerReady` audit log 출력 |

### 6.2 현재 상태 분류

`Stagger Count`는 현재 overlay 표시 기준으로 `HookNeeded`에 가깝다.

이유:

- count 자체는 실제 gameplay code에 존재한다.
- Player/Enemy 모두 같은 개념을 가진다.
- 하지만 현재 HUD에서 바로 읽을 public getter는 확인되지 않았다.
- `RecordParryStackUpdatedForAudit()`는 audit log만 출력하고 SnapshotStore에는 아직 기록하지 않는다.

### 6.3 표시 방식 후보

P0.5에서 표시한다면 다음 중 하나를 선택한다.

| 방식 | 예시 | 장점 | 리스크 |
| --- | --- | --- | --- |
| current panel getter | `Stagger: 1/3` | Player/Enemy 현재 상태와 자연스럽게 결합 | `ACPlayer`, `ACEnemy` getter 추가 필요 |
| recent combat hook | `ParryStack: Count=1/3 | Ready=false` | 실제 parry event 시점 evidence에 적합 | SnapshotStore API/summary 확장 필요 |
| 둘 다 표시 | panel + recent/event | 가장 명확 | P0.5 범위가 커짐 |

권장:

- P0.5 보강으로 진행한다면 먼저 `ACPlayer`, `ACEnemy`에 읽기 전용 getter를 추가하고 panel에 `Stagger: Count/Threshold`를 표시한다.
- Recent/EventLog까지 연결하는 작업은 SnapshotStore 확장이 필요하므로 별도 작업으로 분리한다.

### 6.4 표시 정책

표시할 경우 다음 정책을 따른다.

- Player/Enemy 모두 같은 위치에 표시한다.
- `Reaction` 값과 별도 라인으로 둔다.
- 권장 라벨: `Stagger: 1/3`
- count가 없거나 대상 actor가 없으면 `Stagger: N/A`
- threshold 도달 후 reset되는 값이므로 “누적 총량”이 아니라 “현재 parry stack”으로 설명한다.
- 실제 코드에서 읽지 못하는 경우 성공 evidence로 표시하지 않는다.

## 7. P1로 넘길 항목

다음은 P0.5 최종 캡처 전 필수 작업이 아니다.

- Target Component 기반 enemy selection
- Player/Enemy별 EventLog 분리
- EventLog category filter
- Runtime LOD 실제 tier hook 및 최종 evidence
- Store subject 분리
- AI blackboard/BT detail 표시
- 다중 enemy selector
- final capture automation

## 8. 지금 하지 않을 항목

이번 잔여 리팩터링 계획 단계에서는 다음을 하지 않는다.

- 코드 구현
- Shipping HUD처럼 보이는 UI 개선
- UMG/Slate 전환
- `.umap`, `.uasset`, config, `Build.cs` 변경
- EventLog 추가 compact 작업
- 기존 gameplay 흐름 변경
- Round1 파일을 final candidate로 승격

## 9. 최종 캡처로 넘어가는 조건

최종 캡처로 넘어가기 전에 다음이 충족되어야 한다.

| 조건 | 기준 |
| --- | --- |
| P0.5 보강 범위 결정 | Stagger Count를 구현할지, P1로 넘길지 결정 |
| Stagger Count 분류 | `Ready`, `HookNeeded`, `P1` 중 하나로 확정 |
| 캡처 preset 확정 | scene/action 순서와 캡처 목적 확정 |
| 경로 분리 | Round1과 final candidate 폴더 분리 |
| evidence claim 확정 | 각 이미지가 주장하는 범위가 문서와 일치 |
| 화면 정리 | tooltip/editor/taskbar 노출 기준 결정 |

## 10. 권장 다음 작업

다음 작업은 `Stagger Count 표시 구현 여부 결정 및 최소 설계`로 둔다.

권장 진행:

1. `ACPlayer`, `ACEnemy`에 getter 추가만으로 HUD 표시가 가능한지 확인한다.
2. panel 표시만 할지, SnapshotStore recent/event까지 확장할지 결정한다.
3. P0.5에 포함한다면 Player/Enemy panel에 `Stagger` 라인을 추가한다.
4. P1로 넘긴다면 final capture에서는 현재 `Reaction: Stagger`와 `Recent Combat` evidence만 사용한다.

## 11. 결론

P0.5 overlay는 현재 1차 evidence 확보에 충분히 동작한다.

최종 캡처 전 가장 중요한 결정은 `Stagger Count`를 P0.5에 추가할지 여부다. 현 코드 기준 count의 원천은 존재하지만, HUD에서 읽기 위한 getter 또는 SnapshotStore hook이 아직 필요하므로 즉시 성공 evidence로 표시해서는 안 된다.
