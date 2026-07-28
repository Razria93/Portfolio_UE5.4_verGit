# Debug Overlay P0 최소 구현 설계

## 목적

P0 debug overlay는 제출 영상과 기술문서에서 전투 실행 흐름을 짧게 증명하기 위한 개발 전용 evidence overlay다.

이 overlay는 완성형 게임 HUD가 아니다. 화면을 꾸미는 기능보다 실제 코드에서 나온 상태와 최근 이벤트를 왜곡 없이 보여주는 것이 목적이다.

## 설계 원칙

- 기존 gameplay 로직에 영향을 주지 않는다.
- 표시값은 getter 기반 현재값과 debug hook 기반 최근값을 구분한다.
- 실제 코드에서 읽지 못하는 값은 `N/A`, `Pending`, `NotCaptured`로 표시한다.
- Shipping에서 동작하지 않도록 `#if !UE_BUILD_SHIPPING`과 console variable gate를 둔다.
- UMG/Slate 의존성을 추가하지 않고 `AHUD / Canvas Draw` 기반으로 시작한다.

## P0 표시 항목

| 섹션 | 항목 | 상태 | 표시 기준 |
| --- | --- | --- | --- |
| Player Execution | `ExecutionState` | Ready | `UCStateComponent::GetCurrentExecutionState()` |
| Player Execution | `ActiveAction` | Ready | `UCActionComponent::IsActive()`, `GetActiveActionType()`, `GetActiveActionIndex()` |
| Player Execution | `ActiveReaction` | Ready | `UCReactionComponent::IsActive()`, `GetActiveReactionType()` |
| Player Execution | `GuardOverlay` | Ready | `UCObservableOverlayComponent::WriteOverlaySnapshot()` 또는 `UCDefenseComponent` getter |
| Runtime LOD | `RuntimeLODTier` | Ready | `ACAIController::GetCurrentRuntimeLODTier()` |
| Target Combat | `HitWindow` | Ready / HookNeeded | 현재 weapon 조회 또는 최근 hit window event |
| Target Combat | `DefenseOutcome` | HookNeeded | 최근 `FCombatSignalTargetPacket` 또는 `FCombatResultPacket` |
| Target Combat | `FinalTakenDamage` | HookNeeded | 최근 `FCombatSignalTargetResult::FinalTakenDamage` |
| Target Combat | `DamageCommit` | HookNeeded | 최근 `CommittedDamage`, `bDamageCommitted` |
| Event Log | 최근 3~5줄 | HookNeeded | debug overlay snapshot store의 ring buffer |

## 화면 구성

P0 기본 레이아웃은 좌측 상단 텍스트 블록으로 둔다.

```text
[Player Execution]
ExecutionState:
ActiveAction:
ActiveReaction:
GuardOverlay:

[Target Combat]
HitWindow:
DefenseOutcome:
FinalTakenDamage:
DamageCommit:

[Runtime LOD]
RuntimeLODTier:

[Event Log]
1.
2.
3.
```

표시 문구는 간결하게 유지한다. 기술문서/제출 영상에서 읽을 수 있을 정도의 정보만 남기고, 게임 HUD처럼 장식하지 않는다.

## 데이터 소스 구분

### Getter 기반 현재값

HUD draw 시점에 read-only로 조회해도 되는 값이다.

- `ExecutionState`
- `ActiveAction`
- `ActiveReaction`
- `GuardOverlay`
- `RuntimeLODTier`

이 값들은 현재 상태이므로 매 프레임 조회해도 의미가 분명하다.

### Debug snapshot 기반 최근값

전투 처리 순간에만 존재하는 값이다. HUD가 매 프레임 직접 계산하거나 pipeline을 다시 실행하면 안 된다.

- `HitWindow` event
- `DefenseOutcome`
- `FinalTakenDamage`
- `DamageCommit`
- `EventLog`

이 값들은 기존 debug hook 또는 처리 지점에서 개발 전용 snapshot store에 기록하고, HUD는 그 결과만 읽는다.

## 최소 구현 구조

권장 구조는 세 부분으로 나눈다.

1. 개발 전용 snapshot store
   - `Core/Debug` 아래에 둔다.
   - 최근 combat packet summary와 event log ring buffer를 보관한다.
   - UObject 생명주기와 직접 묶지 않는다.
2. Canvas renderer
   - `AHUD::DrawHUD()`에서 snapshot을 읽어 텍스트로 표시한다.
   - 화면 draw 책임만 가진다.
3. 기존 debug hook 연결
   - `FExecutionOrchestratorDebug`, `FCombatSignalDebug`, `FCombatResultDebug`, `FAICombatBTDebug` 등은 store에 최근 이벤트를 기록하는 보조 역할만 한다.

gameplay component는 overlay 전용 상태를 소유하지 않는다.

## 파일 배치 후보

### Debug snapshot store

권장 후보:

```text
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp
```

역할:

- `FDebugOverlaySnapshotTypes.h`
  - event log entry, combat summary, execution summary 등 debug 전용 POD 타입 정의
- `FDebugOverlaySnapshotStore.h/.cpp`
  - `Record...`, `GetSnapshotCopy`, `Reset`, `ShouldCollect...` API 제공

### Canvas renderer

권장 후보:

```text
Source/Portfolio/Core/Debug/CDebugOverlayHUD.h
Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp
```

대안:

```text
Source/Portfolio/HUD/CDebugOverlayHUD.h
Source/Portfolio/HUD/CDebugOverlayHUD.cpp
```

P0에서는 debug 전용 성격이 강하므로 `Core/Debug` 배치가 더 일관적이다. 장기적으로 게임 HUD와 분리된 개발 도구 체계가 커지면 `HUD` 또는 `DebugOverlay` 폴더 분리를 검토한다.

### GameMode 연결 후보

현재 프로젝트에는 C++ `AHUD` 파생 클래스와 C++ `AGameMode` 파생 클래스가 확인되지 않았다.

현재 설정:

```text
Config/DefaultEngine.ini
GlobalDefaultGameMode=/Script/Engine.GameMode
```

연결 후보:

1. C++ GameMode 추가 후 전역 연결
   - `CPortfolioGameMode`를 만들고 `HUDClass = ACDebugOverlayHUD::StaticClass()`로 지정한다.
   - 필요 시 `PlayerControllerClass = ACPlayerController::StaticClass()`도 함께 명시한다.
   - `DefaultEngine.ini`의 `GlobalDefaultGameMode` 변경이 필요하다.
2. 테스트 맵 한정 GameMode 연결
   - `TestRoom` 등 검증 맵 World Settings 또는 BP GameMode에서 `HUDClass`만 지정한다.
   - 전역 GameMode 변경 리스크가 크면 P0 구현 검증에는 이 방식이 더 안전하다.

문서 기준 권장안은 2번이다. P0 evidence overlay는 제출 영상 촬영용이므로 먼저 검증 맵 한정 연결로 시작하고, 안정화 후 전역 연결을 검토한다.

## CVar 설계

표시와 수집 gate는 분리한다.

```text
Portfolio.DebugOverlay.Enabled
Portfolio.DebugOverlay.Collect
Portfolio.DebugOverlay.Preset
Portfolio.DebugOverlay.EventLogLimit
```

역할:

- `Enabled`
  - Canvas draw 표시 여부
- `Collect`
  - debug hook에서 snapshot store에 기록할지 여부
- `Preset`
  - P0/P1 또는 영상 preset 전환
- `EventLogLimit`
  - 화면에 표시할 최근 이벤트 줄 수

기존 `Portfolio.Debug.*Audit` cvar와 강하게 묶지 않는다. 로그는 끄고 overlay evidence만 수집하는 사용 흐름이 필요할 수 있기 때문이다.

## 데이터 흐름

### 현재값 조회 흐름

```text
ACDebugOverlayHUD::DrawHUD
-> Owning PlayerController
-> Player Pawn
-> UCStateComponent / UCActionComponent / UCReactionComponent / UCObservableOverlayComponent
-> Canvas text draw
```

현재값 조회는 player pawn 중심으로 제한한다. P0에서 매 프레임 모든 actor를 스캔하지 않는다.

### 최근 이벤트 기록 흐름

```text
Combat / Action / AI runtime event
-> Existing debug hook
-> FDebugOverlaySnapshotStore::Record...
-> Ring buffer / latest summary
-> ACDebugOverlayHUD::DrawHUD
```

store 조회는 copy 기반 API로 설계한다. HUD가 그리는 동안 기록이 갱신되어도 순회 문제가 생기지 않도록 한다.

## Hook 후보

### Action / Reaction

- `FExecutionOrchestratorDebug::RecordActionExecutionResultForAudit`
- `FExecutionOrchestratorDebug::RecordReactionExecutionResultForAudit`

기록 후보:

- decision
- apply mode
- active/incoming participant 요약
- overlay handling 요청

### CombatSignal / Damage

- `FCombatSignalDebug::RecordWeaponCollisionWindowForAudit`
- `FCombatSignalDebug::RecordTargetAcceptedForAudit`
- `FCombatSignalDebug::RecordTargetRejectedForAudit`
- `FCombatSignalDebug::RecordCombatResultDispatchForAudit`
- `FCombatResultDebug::RecordCombatResultReceivedForAudit`

기록 후보:

- hit window open/close
- defense outcome
- final taken damage
- committed damage
- result dispatch/receive

### Enemy AI / Runtime LOD

- `FAICombatBTDebug::RecordCombatActionTaskSucceededForAudit`
- `FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit`
- `CBTServiceIntervalHelper::GetAIIntentStateInterval()` 내부 interval selection 직후

기록 후보:

- AI combat request intent/result
- runtime LOD tier
- AI intent interval preset/value

P0에서는 AI request와 interval은 event log 보조로만 둔다. 성능 최적화 성공처럼 보이게 표시하지 않는다.

## Snapshot store 타입 초안

구현 전 개념 타입은 다음 정도로 제한한다.

```text
FDebugOverlayEventEntry
- FrameNumber
- WorldTimeSeconds
- Category
- EventName
- OwnerName
- SourceName
- TargetName
- Summary

FDebugOverlayCombatSummary
- bHasRecentCombat
- DefenseOutcome
- FinalTakenDamage
- bDamageCommitted
- CommittedDamage
- HitWindowId

FDebugOverlayAISummary
- bHasRecentAIRequest
- AIIntent
- RequestResult
- RuntimeLODTier
- IntervalPreset

FDebugOverlaySnapshot
- CombatSummary
- AISummary
- RecentEvents
```

actor raw pointer를 장기 보관하지 않는다. 필요하면 `TWeakObjectPtr` 또는 이름/시간/frame snapshot으로 축약한다.

## 위험 요소

### Shipping 노출

- 모든 저장/표시 구현은 `#if !UE_BUILD_SHIPPING`로 보호한다.
- shipping public API는 no-op 또는 false 반환으로 둔다.

### Gameplay 흐름 오염

- HUD는 gameplay 판단을 수행하지 않는다.
- debug hook은 기존 결과를 기록만 한다.
- overlay를 위해 damage, action, AI decision을 재계산하지 않는다.

### CSV counter 오염

- `BT Interval`은 polling으로 재계산하지 않는다.
- interval 선택 시점에 최근값을 기록한다.
- CSV profiler helper가 없는 상태에서 `CSV Capture`를 성공 상태처럼 표시하지 않는다.

### 현재값과 최근값 혼동

- 현재값 섹션과 최근 이벤트 섹션을 분리한다.
- `DefenseOutcome`, `FinalTakenDamage`, `DamageCommit`은 현재 상태가 아니라 최근 combat result라고 표시한다.

### GameMode 연결 리스크

- 전역 `GlobalDefaultGameMode` 변경은 기본 pawn/controller/HUD 흐름을 바꿀 수 있다.
- P0 구현 검증은 맵 한정 GameMode 연결을 우선 검토한다.

## 구현 전 체크리스트

- C++ `AHUD` class 추가 위치 확정
- GameMode 연결을 전역으로 할지, 테스트 맵 한정으로 할지 결정
- `Core/Debug` snapshot store 파일명 확정
- store cvar와 draw cvar 분리 여부 확정
- store reset 시점 결정
  - PIE 시작/종료
  - world 변경
  - overlay collect cvar off
- event log capacity 기본값 결정
- player pawn 기준 component 조회 실패 시 `N/A` 처리 정책 확정
- 최근 combat summary가 없을 때 `NotCaptured` 처리 정책 확정
- build dependency 추가가 필요 없는지 재확인

## P0 구현 시작 조건

다음 조건이 충족되면 P0 구현으로 넘어간다.

- 이 문서의 파일 배치와 CVar 이름에 이견이 없다.
- HUD 연결 방식을 결정했다.
- snapshot store가 기록할 최소 summary 타입을 확정했다.
- P0에서 제외할 항목을 확정했다.

