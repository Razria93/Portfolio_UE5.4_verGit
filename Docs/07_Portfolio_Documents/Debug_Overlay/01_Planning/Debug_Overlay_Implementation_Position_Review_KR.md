# Debug Overlay 구현 위치 검토

## 목적

P0 최소 debug overlay를 어디에 구현하는 것이 기존 gameplay 로직에 가장 적은 영향을 주는지 검토한다.

이번 문서는 구현 전 판단 문서이며, 아직 코드 변경은 하지 않는다.

## 전제

- overlay는 개발 전용 evidence 도구다.
- 완성형 게임 HUD가 아니다.
- 기존 전투 로직, AI 로직, damage pipeline의 책임을 UI 표시 책임과 섞지 않는다.
- 표시값은 read-only 조회 또는 개발 전용 최근 이벤트 snapshot에서 가져온다.
- 모든 구현은 `#if !UE_BUILD_SHIPPING` 또는 `Portfolio.DebugOverlay.*` console variable gate를 전제로 한다.

## 후보 비교

| 후보 | 장점 | 단점 | 판정 |
| --- | --- | --- | --- |
| `AHUD / DrawHUD` | Canvas text draw에 적합하다. UMG/Slate 의존성 추가 없이 화면 표시 가능하다. 개발용 overlay 성격과 맞다. | GameMode/HUD class 연결이 필요할 수 있다. 현재 프로젝트에 기존 HUD class가 확인되지 않았다. | 1순위 후보 |
| 개발 전용 `AActor` | level에 배치하거나 spawn하여 독립적으로 관리 가능하다. gameplay component와 분리된다. | 화면 draw는 결국 HUD/Canvas 또는 debug draw 경로가 필요하다. actor lifecycle과 표시 대상 선정 정책을 추가해야 한다. | 2순위 후보 |
| 개발 전용 `UActorComponent` | player/enemy에 붙여 대상별 snapshot 수집에 좋다. 기존 component reference pattern과 맞다. | 화면 전체 overlay를 그리는 책임까지 맡기면 component 책임이 흐려진다. 여러 actor에 붙으면 중복 표시 위험이 있다. | 수집 보조 후보 |
| 기존 Debug helper 확장 | 기존 `FExecutionOrchestratorDebug`, `FCombatSignalDebug`, `FAICombatBTDebug` hook을 재사용하기 쉽다. | helper가 화면 draw까지 맡으면 static debug formatter 책임을 넘어선다. | event 기록 보조 후보 |
| UMG widget | 보기 좋게 만들기 쉽다. | 현재 `Portfolio.Build.cs`에 UMG/Slate 의존성이 없다. 제출용 debug overlay치고 변경 범위가 커진다. | 후순위 |

## 권장 구조

최소 구현은 다음 구조가 가장 안전하다.

1. `Core/Debug` 아래 개발 전용 overlay snapshot 저장소를 둔다.
   - 최근 Action / Reaction decision
   - 최근 CombatSignal / Damage packet summary
   - 최근 AI request / Runtime LOD summary
   - 최근 event log ring buffer
2. 화면 표시는 `AHUD::DrawHUD` 기반 class 또는 Canvas draw entry에서 담당한다.
3. gameplay component는 기존 상태 getter만 제공하고, overlay 전용 상태를 소유하지 않는다.
4. 기존 debug helper는 화면을 직접 그리지 않고 최근 이벤트 기록만 보조한다.

## 표시 대상 선정

초기 P0 overlay는 다음 대상만 다룬다.

- Player
  - `ACPlayerController::GetPawn()` 기준 player pawn
  - player의 `UCStateComponent`, `UCActionComponent`, `UCReactionComponent`, `UCObservableOverlayComponent`
- Target Combat
  - 최근 `FCombatSignalTargetPacket` 또는 `FCombatResultPacket` 기준
- Enemy AI / Runtime LOD
  - P0에서는 최근 event log 중심
  - P1에서 focused enemy 선정 정책 검토

Enemy AI target 선정은 다음 중 하나를 후속 결정한다.

- player가 바라보는 enemy
- 최근 AI request를 발생시킨 enemy
- 가장 가까운 enemy
- combat engage target

## P0 최소 구현 제안

P0는 다음만 화면에 표시한다.

```text
[Player Execution]
ExecutionState:
ActiveAction:
ActiveReaction:
GuardOverlay:

[Target Combat]
DefenseOutcome:
DamageCommit:
FinalTakenDamage:

[Runtime LOD]
RuntimeLODTier:

[Event Log]
1.
2.
3.
```

## 구현 전 필요한 보강

- `CurrentMontage`
  - active data montage name만 표시할지, 실제 playing montage를 표시할지 결정한다.
  - 정확한 playing montage가 필요하면 `UCAction` / `UCReaction`에 개발 전용 getter 또는 debug hook이 필요하다.
- `ApplyMode`, `OverlayHandling`
  - decision result 최근값 저장소가 필요하다.
- `DamageSpecKey`, `DefenseOutcome`, `FinalTakenDamage`, `DamageCommit`
  - `FCombatSignalDebug` 또는 target component 처리 지점에서 최근 combat summary를 저장해야 한다.
- `AI Request`, `BT Interval`
  - `FAICombatBTDebug`와 `CBTServiceIntervalHelper`에 최근값 저장 hook이 필요하다.

## 결론

P0 구현 위치는 `AHUD / Canvas Draw`를 1순위로 둔다.

다만 데이터 수집은 HUD가 직접 모든 system을 뒤지는 방식이 아니라, `Core/Debug`의 개발 전용 snapshot 저장소와 기존 debug hook을 통해 분리한다.

이 방식은 기존 gameplay 로직의 책임을 유지하면서 제출 영상용 evidence overlay를 만들 수 있다.

