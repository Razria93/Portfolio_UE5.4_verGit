# AI Combat Feedback Presentation Measurement Plan

## 목적

`Combat Feedback Presentation` 축이 40 / 80 Enemy 조건에서 frame budget에 어느 정도 영향을 주는지 분리 측정한다.

이 축은 전투 판정이나 damage result를 끄는 작업이 아니다.
전투 결과는 유지한 채, 그 결과를 표현하는 presentation 비용만 분리한다.

## 측정 축

측정 CVar:

```text
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback
```

값:

| Value | 의미 |
| ---: | --- |
| 0 | Enemy combat feedback presentation 실행 |
| 1 | Enemy combat feedback presentation skip |

## 포함 범위

`DisableEnemyCombatFeedback 1`에서 차단하는 항목:

- Enemy `ActionFeedback`
  - weapon trail
  - action VFX
  - action SFX
- Enemy `ReactionFeedback`
  - hit / guard / parry reaction VFX
  - hit / guard / parry reaction SFX
- Enemy `HitFeedback` presentation
  - hit VFX
  - hit SFX
  - camera shake request

단, HitFeedback은 맞은 actor의 feedback component에서 실행된다.
Enemy가 Player를 공격해 Player 쪽 `HitFeedback`이 실행되는 경우는 Enemy owner feedback이 아니므로 `DisableEnemyCombatFeedback 1`에서도 유지된다.

## 제외 범위

이번 축에서 유지하는 항목:

- attack montage
- action / reaction state transition
- hit window open / close
- overlap 후보 수집
- hit processing
- combat signal
- damage result
- hit stop

`HitStop`은 presentation처럼 보일 수 있지만 actor time dilation을 바꾸므로 timing / simulation에 영향을 준다.
따라서 이번 `Feedback Presentation` 축에서는 끄지 않는다.

## 계측 방식

Event 기반 feedback 호출은 단일 CSV stat으로 직접 찍으면 누락되거나 해석이 불안정할 수 있다.
따라서 event 지점에서는 counter만 누적하고, `CombatEngage` tick에서 flush counter를 CSV로 기록한다.

주요 카운터:

```text
PortfolioAI_ActionFeedback_Request_FlushCount
PortfolioAI_ActionFeedback_Skipped_FlushCount
PortfolioAI_ActionFeedback_Trail_FlushCount
PortfolioAI_ActionFeedback_TrailClear_FlushCount
PortfolioAI_ActionFeedback_VFX_FlushCount
PortfolioAI_ActionFeedback_SFX_FlushCount

PortfolioAI_ReactionFeedback_Request_FlushCount
PortfolioAI_ReactionFeedback_Skipped_FlushCount
PortfolioAI_ReactionFeedback_VFX_FlushCount
PortfolioAI_ReactionFeedback_SFX_FlushCount

PortfolioAI_HitFeedback_Request_FlushCount
PortfolioAI_HitFeedback_PresentationSkipped_FlushCount
PortfolioAI_HitFeedback_VFX_FlushCount
PortfolioAI_HitFeedback_SFX_FlushCount
PortfolioAI_HitFeedback_CameraShakeRequest_FlushCount
```

해석 기준:

- `Request`는 해당 feedback route에 진입한 횟수다.
- `Skipped`는 CVar로 presentation이 차단된 횟수다.
- `Trail / VFX / SFX / CameraShakeRequest`는 실제 presentation 실행 수다.
- `TrailClear`는 action cleanup 과정에서 trail off를 보장하기 위한 정리 호출 수다. trail presentation 실행 수와 분리해서 본다.
- `DisableEnemyCombatFeedback 1`에서 `Request`는 유지되고 `Skipped`가 증가하며 presentation 실행 카운터가 0 또는 크게 감소해야 한다.

## 기본 측정 조건

공통 조건:

- Capture Duration: 약 36초
- Analysis Window: first 3s / last 3s trimmed, middle 30s used
- Log State: `-noailogging`
- PIE: F11 fullscreen
- GC 이벤트 없음
- fixed camera

CVar:

```text
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback 0 / 1

Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode 0
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationRefreshCounter 0
Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 0
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 0
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit 0
Portfolio.AI.RuntimeLOD.EnemyMovementMode 0
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing 0
```

## 확인 항목

`DisableEnemyCombatFeedback 0`:

- Engage / Alert 상태 정상 유지
- attack montage 정상
- hit / guard / parry result 정상
- trail / Niagara / SFX / camera shake presentation 정상
- GC 이벤트 없음

`DisableEnemyCombatFeedback 1`:

- Engage / Alert 상태 정상 유지
- attack montage 정상
- hit / guard / parry result 정상
- hit processing / combat signal 유지
- Enemy trail / VFX / SFX / camera shake presentation 제거 또는 감소
- HitStop은 유지
- GC 이벤트 없음

## 측정 순서

1. 40 Enemy / FeedbackBaseline / `DisableEnemyCombatFeedback 0`
2. 40 Enemy / FeedbackDisabled / `DisableEnemyCombatFeedback 1`
3. 80 Enemy / FeedbackBaseline / `DisableEnemyCombatFeedback 0`
4. 80 Enemy / FeedbackDisabled / `DisableEnemyCombatFeedback 1`

40 Enemy에서 counter가 의도대로 분리되지 않으면 80 Enemy 측정 전에 코드 또는 에셋 조건을 먼저 점검한다.

## 분석 기준

우선 비교 지표:

- Frame p95
- Game p95
- CharacterMovement p95
- BT Tick p95
- feedback flush counter

주요 판단:

- presentation counter가 줄었는데 Frame / Game p95가 거의 같으면, 현재 조건에서 feedback presentation은 Runtime LOD v1 우선 축이 아니다.
- presentation counter와 Frame / Game p95가 같이 줄면, feedback presentation은 Representation LOD 후보로 유지한다.
- counter가 줄지 않으면 CVar 적용 범위 또는 feedback route 누락을 먼저 확인한다.

## 결과

대표 측정:

| Enemy | Case | CVar | CSV |
| ---: | --- | ---: | --- |
| 40 | FeedbackBaseline | 0 | `Profile(20260711_191545).csv` |
| 40 | FeedbackDisabled | 1 | `Profile(20260711_191931).csv` |
| 80 | FeedbackBaseline | 0 | `Profile(20260711_193111).csv` |
| 80 | FeedbackDisabled | 1 | `Profile(20260711_193802).csv` |

`Profile(20260711_192806).csv`는 80 Enemy baseline 계열이지만 action feedback counter가 대표 baseline보다 한 사이클 더 들어간 형태라 최종 대표값에서 제외한다.

### Frame / Game 비교

| Enemy | Case | Frame p95 | Game p95 | GPU p95 | CharacterMovement p95 | BT Tick p95 |
| ---: | --- | ---: | ---: | ---: | ---: | ---: |
| 40 | Baseline | 12.4115ms | 12.3616ms | 10.4383ms | 0.5052ms | 0.2125ms |
| 40 | Disabled | 12.2223ms | 12.2703ms | 10.0552ms | 0.5309ms | 0.2104ms |
| 80 | Baseline | 17.9264ms | 17.8648ms | 11.3880ms | 0.7770ms | 0.4195ms |
| 80 | Disabled | 18.3894ms | 18.4272ms | 10.9936ms | 0.9020ms | 0.4174ms |

### Feedback counter 비교

| Enemy | Case | Request | Skipped | Trail | TrailClear | VFX | SFX |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 40 | Baseline | 104 | 0 | 22 | 12 | 20 | 34 |
| 40 | Disabled | 102 | 102 | 0 | 6 | 0 | 0 |
| 80 | Baseline | 104 | 0 | 22 | 12 | 20 | 34 |
| 80 | Disabled | 104 | 104 | 0 | 6 | 0 | 0 |

### 해석

- `DisableEnemyCombatFeedback 1`에서 `ActionFeedback_Skipped`가 request 수와 동일하게 증가하고 `Trail / VFX / SFX`가 0으로 떨어졌다.
- `TrailClear`는 action cleanup에서 trail off를 보장하는 정리 호출이므로 presentation 실행 수로 보지 않는다.
- Player가 맞아서 Player component에서 실행되는 `HitFeedback / HitStop`은 유지된다.
- 40 / 80 Enemy 모두에서 feedback presentation 제거가 Frame / Game p95를 유의미하게 낮추지는 않았다.
- 따라서 `Combat Feedback Presentation`은 Runtime LOD v1의 핵심 병목 축이 아니라, 최하위 representation 단계에서 선택적으로 줄일 수 있는 후보로 둔다.
