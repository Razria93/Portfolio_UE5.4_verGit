# Debug Overlay Evidence Plan

## 목표

Action RPG 전투 시스템의 실행 흐름을 이력서, 포트폴리오, 기술문서, 제출 영상에서 설명할 수 있도록 개발 전용 debug overlay를 설계한다.

이 작업의 목적은 완성형 게임 HUD 제작이 아니다. 실제 코드의 Action / Reaction, CombatSignal / Damage, Enemy AI, Runtime LOD 흐름을 화면에서 짧게 증명하는 evidence를 만드는 것이다.

## 고정 원칙

- Shipping 기능처럼 포장하지 않는다.
- 기존 전투 동작에 영향을 주지 않는다.
- 표시 값은 실제 코드에서 읽을 수 있는 값만 사용한다.
- 불확실한 값은 성공 evidence처럼 표시하지 않는다.
- `#if !UE_BUILD_SHIPPING` 및 console variable gate를 기본 전제로 둔다.
- 기존 debug hook과 profiling 구조를 우선 활용한다.

## 1차 분석 대상

- Action / Reaction
  - `UCActionOrchestratorComponent`
  - `UCReactionOrchestratorComponent`
  - `UCActionComponent`
  - `UCReactionComponent`
  - `FExecutionOrchestratorDebug`
- Observable Overlay / Guard
  - `UCObservableOverlayComponent`
  - `FObservableOverlayDebug`
  - `UCDefenseComponent`
- CombatSignal / Damage
  - `UCCombatSignalSourceComponent`
  - `UCCombatSignalTargetComponent`
  - `FCombatSignalDebug`
  - `FCombatResultDebug`
- Enemy AI / Runtime LOD
  - `ACAIController`
  - `UCBTTask_StartCombatAction`
  - Blackboard keys
  - `AI/RuntimeLOD/*`
  - `Core/Profiling/*`

## P0 표시 후보

- `ActionState`
- `ReactionState`
- `CurrentMontage`
- `OverlayHandling`
- `DefenseOutcome`
- `DamageCommit`
- `FinalDamage`
- `RuntimeLODTier`
- recent event log 3-5 lines

## P1 표시 후보

- `ActionRequest`
- `ReactionRequest`
- `ApplyMode`
- `HitWindow`
- `DamageSpecKey`
- `BT State`
- `Blackboard Intent`
- `AI Request`
- `BT Interval`
- `DistanceToPlayer`
- `CSV Capture`

## 제외 또는 주의 항목

- 내부 경로, private 문서명, raw CSV 전체 경로
- FPS 최적화 성공처럼 보이는 문구
- Counter 완성 기능처럼 보이는 표시
- 화면을 과도하게 가리는 HUD
- 실제 코드에서 직접 읽지 못한 값을 확정값처럼 표시하는 것

## 구현 방향

1. 코드 기준 evidence map을 먼저 작성한다.
2. P0 항목 중 안정적으로 읽을 수 있는 값만 최소 overlay로 구현한다.
3. 기존 debug hook에서 최근 이벤트 ring buffer를 갱신한다.
4. 상태값은 overlay draw 시점에 read-only로 조회한다.
5. Console variable로 overlay enable 및 preset을 전환한다.
6. Editor build 통과 후 영상 preset별로 표시 신뢰도를 검증한다.

## 최소 구현 후보

- 개발 전용 `UCDebugOverlayComponent` 또는 `ADebugOverlayActor`
- Canvas debug draw 또는 `AHUD::DrawHUD`
- 별도 UMG는 후순위

## 산출물

- 계획 문서
- 운영 문서
- evidence map
- 영상 preset 문서
- 최소 debug overlay 구현
- 검증 결과 및 촬영 가이드

