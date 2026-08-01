# Debug Overlay P0.5 Final Decision

## 1. 문서 목적

이 문서는 P0.5 debug overlay의 완료 범위와 제외 범위를 고정한다.

P0.5 overlay는 TestRoom PIE에서 제출 영상과 기술문서에 사용할 개발 전용 evidence를 확보하기 위한 화면이다. 완성형 게임 HUD, Shipping HUD, 성능 성공 주장용 UI가 아니다.

## 2. P0.5 완료 범위

P0.5에서 완료된 범위는 다음으로 본다.

| 구분 | 완료 기준 |
| --- | --- |
| 화면 구조 | `[Debug Overlay P0.5]` 아래 Player/Enemy 패널과 공통 recent block 표시 |
| Player 패널 | State, Action, Reaction, Guard, Movement, HP, Runtime LOD, AI 표시 |
| Enemy 패널 | State, Action, Reaction, Guard, Movement, HP, Runtime LOD, AI 표시 |
| 패널 색상 | Player blue tab, Enemy red tab |
| 현재값 compact | enum prefix 제거 |
| multi-field 표시 | `Guard`, `Movement`, `HP`는 pipe 문자로 구분 |
| Guard action | `Guard In`, `Guard Out`으로 표시 |
| Execution summary | `Action(...)`, `Reaction(...)` subject 포함 |
| EventLog | 현재 compact key/value format 유지 |
| Enemy 선택 | world scan fallback 기반 표시 |

## 3. P0.5 표시 정책

### 3.1 Current value

현재값은 HUD draw 시점에 actor component getter로 조회한다.

예시:

```text
State: Idle
Action: ComboAttack[1]
Reaction: Hit
Guard: Wants=true | Pose=true | CanGuard=true | CanParry=false | CanStart=false
Movement: Gait=Run | Speed=0.0 | Dir=0.0 | CanMove=true | Falling=false
HP: 5000.0/5000.0 | DeadState=Alive
Runtime LOD: N/A
AI: NotCaptured
```

### 3.2 Guard action

Guard action은 internal action index를 화면에 노출하지 않는다.

표시:

```text
Action: Guard In
Action: Guard Out
```

P0.5에서는 Guard Hold / Guard Hit / Guard Parry를 별도 action label로 표시하지 않는다. 해당 의미는 Guard 현재값, Reaction, Combat outcome으로 설명한다.

### 3.3 Execution summary

Execution summary는 domain과 subject를 함께 표시한다.

```text
Action(ComboAttack[1]) | Decision=Accept | Apply=Start | RejectReason=None
Reaction(Hit) | Decision=Accept | Apply=Intervene | RejectReason=None
Action(Guard In) | Decision=Accept | Apply=Start | RejectReason=None
Action(Guard Out) | Decision=Accept | Apply=Start | RejectReason=None
```

이 summary는 overlay Store의 recent summary와 EventLog에 사용한다. 기존 audit log 출력 format은 P0.5 완료 범위에 포함하지 않는다.

### 3.4 EventLog

P0.5에서는 EventLog 추가 축약을 하지 않는다.

현재 형식:

```text
Execution/DecisionResolved: Action(Guard In) | Decision=Accept | Apply=Start | RejectReason=None
Combat/CollisionDisabledIgnored: State=CollisionDisabledIgnored | HitWindow=0 | Collision=None | Reason=HitWindowNotOpened
```

판단:

- 캡처에서 필요한 event category, event name, key/value summary가 유지된다.
- 추가 축약은 정보 손실 또는 별도 filter 설계와 연결될 수 있다.
- EventLog category filter와 Player/Enemy별 EventLog 분리는 P1 후보로 둔다.

## 4. Enemy 표시 정책

P0.5 Enemy 패널은 target component 기반이 아니다. 현재는 world scan fallback으로 단일 enemy를 표시한다.

화면에 다음 정보를 함께 표시한다.

```text
EnemySource: WorldScanFallback
EnemyFallback: Selected=BP_CEnemy_C_1 Policy=FirstValid Count=1
```

해석 기준:

| 상태 | 의미 | Evidence 사용 |
| --- | --- | --- |
| `Count=1` | 단일 enemy를 fallback으로 선택 | Enemy 상태 확인 evidence로 사용 가능 |
| `NoEnemy` | enemy actor를 찾지 못함 | Enemy system 성공 evidence로 사용하지 않음 |
| `Ambiguous(Count=N)` | 다중 enemy로 대상 확정 불가 | 특정 enemy evidence로 사용하지 않음 |
| `StaleEnemy` | cache 대상이 유효하지 않음 | 재확인 필요 |

Target Component 기반 enemy selection은 P1에서 검토한다. Blackboard 기반 선택은 Target Component가 없거나 AI 내부 상태를 보조 검증해야 할 때의 보조 후보로 둔다.

## 5. 제출 evidence로 사용할 수 있는 항목

| 항목 | 사용 가능 조건 |
| --- | --- |
| Overlay 표시 | `[Debug Overlay P0.5]`, Player/Enemy 패널이 보일 때 |
| Player current state | component getter 값이 `N/A`가 아닐 때 |
| Player action | `None`, `ComboAttack[n]`, `Guard In/Out` 등 실제 표시값 |
| Player reaction | `Hit`, `Parry` 등 실제 reaction 발생 시 |
| Guard | `Wants`, `Pose`, `CanGuard`, `CanParry`, `CanStart` 값이 표시될 때 |
| Movement | `Gait`, `Speed`, `Dir`, `CanMove`, `Falling` 값이 표시될 때 |
| HP | current/max HP와 DeadState가 표시될 때 |
| Recent Execution | subject 포함 summary가 갱신될 때 |
| Recent Combat | HitWindow, DefenseOutcome, Final/Commit 값이 갱신될 때 |
| EventLog | event 발생 후 3~5 lines가 표시될 때 |
| Enemy 상태 | `WorldScanFallback Count=1` 조건에서만 보조 evidence로 사용 |

## 6. 주의해서 사용해야 할 항목

| 항목 | 주의 |
| --- | --- |
| Runtime LOD | P0.5에서는 `N/A` 가능. 실제 tier 성공 evidence로 사용하지 않는다. |
| AI | `NotCaptured` 가능. AI task event가 실제 발생한 경우만 보조 evidence로 사용한다. |
| Enemy selection | world scan fallback이므로 Target Component 기반 evidence가 아니다. |
| EventLog | world 단위 공통 log이며 Player/Enemy별 분리 log가 아니다. |
| FPS/성능 | overlay 캡처를 성능 성공 주장으로 사용하지 않는다. |

## 7. P0.5 제외 범위

P0.5에서는 다음을 구현/검증 범위에서 제외한다.

- EventLog 추가 축약
- EventLog category filter
- Player/Enemy별 EventLog 분리
- Target Component 기반 enemy selection
- Runtime LOD 실제 tier hook 보강
- AI blackboard 상세 표시
- 다중 enemy cycling UI
- capture automation
- Shipping HUD화
- 전역 `GlobalDefaultGameMode` 변경
- `.umap`, `.uasset`, config, `Build.cs` 변경

## 8. P1 후보

P1 후보는 다음으로 둔다.

| 후보 | 목적 |
| --- | --- |
| Target Component 기반 enemy selection | gameplay/runtime truth인 현재 target 기준 enemy 표시 |
| Blackboard 기반 enemy 보조 검토 | Target Component가 없거나 AI 내부 target 확인이 필요할 때만 보조 후보로 사용 |
| Store subject 분리 | Player/Enemy별 recent summary/EventLog 분리 |
| EventLog category filter | Execution/Combat/AI 등 선택 표시 |
| Runtime LOD hook | 실제 tier/interval evidence 표시 |
| AI detail hook | blackboard intent, BT state, request result 상세 표시 |
| capture automation | preset별 반복 캡처 안정화 |
| 다중 enemy selector | 다중 enemy 환경에서 대상 전환 |

## 9. 운영 결정

P0.5 완료 이후 제출 evidence 준비는 다음 순서로 진행한다.

1. TestRoom에서 overlay 수동 연결 상태를 확인한다.
2. `Portfolio.DebugOverlay.Enabled 1`을 설정한다.
3. `Portfolio.DebugOverlay.Collect 1`을 설정한다.
4. `Portfolio.DebugOverlay.EventLogLimit 5`를 설정한다.
5. Idle / Guard In / Guard Out / ComboAttack / Hit / Parry / Enemy action 장면을 캡처한다.
6. 각 캡처가 어떤 evidence 항목을 증명하는지 Capture Presets 문서에 맞춰 분류한다.
7. `N/A`, `NotCaptured`, fallback 상태는 성공 evidence로 과장하지 않는다.

## 10. 후속 작업 묶음

P0.5 이후 작업 제안은 다음 묶음을 기준으로 한다.

### 묶음 A: Evidence 패키징

목표:

- 실제 제출 자료로 사용할 overlay 캡처 세트를 수집하고 정리한다.

포함 작업:

- Idle / Guard In / Guard Out / ComboAttack / Hit / Parry / Enemy Action 캡처 선별
- 기존 Bandicam 캡처 중 채택본/폐기본 분리
- `Docs/98_Evidence/01_Screenshot/DebugOverlay/` 하위 파일명 정리
- 필요 시 `Docs/98_Evidence/02_Video/DebugOverlay/` 하위 영상 클립 정리
- Evidence Package 문서 작성

권장 산출물:

- `Docs/07_Portfolio_Documents/Debug_Overlay/06_Evidence_Package/Debug_Overlay_P0_5_Evidence_Package_KR.md`

### 묶음 B: 포트폴리오 문서 연결

목표:

- Debug overlay evidence를 실제 포트폴리오/기술문서 문맥에 연결한다.

포함 작업:

- PF02 Combat Data Pipeline에 연결할 combat/parry evidence 후보 정리
- PF03 Action & Reaction Execution에 연결할 action/reaction evidence 후보 정리
- PF04 Enemy AI Combat Behavior에 연결할 enemy state/action evidence 후보 정리
- overlay가 "주장"이 아니라 runtime evidence임을 설명하는 문장 작성

### 묶음 C: P1 설계

목표:

- P0.5에서 제외한 확장 항목을 P1 후보 설계로 분리한다.

포함 작업:

- Target Component 기반 enemy selection 설계
- Store subject 분리 설계
- Player/Enemy별 EventLog 분리 설계
- EventLog category filter 설계
- Runtime LOD 실제 tier hook 설계
- AI blackboard/BT detail 표시 설계
- 다중 enemy selector 설계
- capture automation 설계

### 묶음 D: P0.5 최종 검수

목표:

- 문서와 실제 PIE 화면이 서로 맞는지 확인하고 P0.5 작업을 닫는다.

포함 작업:

- README / Operation / Capture / Verification / Final Decision 용어 충돌 확인
- 실제 PIE 화면과 체크리스트 항목 일치 확인
- `N/A`, `NotCaptured`, `WorldScanFallback` 설명 일관성 확인
- 필요 시 문서 index 보강

### 권장 진행 순서

다음 작업 제안은 기본적으로 아래 순서를 따른다.

```text
묶음 A: Evidence 패키징
묶음 B: 포트폴리오 문서 연결
묶음 D: P0.5 최종 검수
묶음 C: P1 설계
```

단, 실제 캡처 파일이 아직 충분하지 않으면 묶음 A를 먼저 진행한다. 포트폴리오 본문 작성이 우선이면 묶음 B를 먼저 진행할 수 있다.

## 11. 최종 결론

P0.5 debug overlay는 제출용 evidence 확보에 필요한 최소 가독성 개선과 Player/Enemy 상태 비교를 제공하는 상태로 본다.

현 시점에서 추가 EventLog 축약은 진행하지 않는다. 다음 단계는 실제 캡처 세트 수집과 문서 내 evidence 매핑이다.
