# UE5 Portfolio Pull Request

## 제목

**P52: Debug Overlay Evidence Tooling**

## 날짜

**2026.07.30**

## 상태

- [x] P0 SnapshotTypes / SnapshotStore 구현
- [x] 기존 debug hook에서 SnapshotStore 기록 연결
- [x] Canvas 기반 `ACDebugOverlayHUD` 구현
- [x] TestRoom 수동 연결용 `ACDebugOverlayGameMode` 추가
- [x] P0.5 Player / Enemy panel 분리
- [x] Movement / HP / Stagger Count 표시
- [x] compact display format 정리
- [x] P1 TargetComponent 기반 enemy source chain 추가
- [x] W05 Code Quality Review 작성
- [x] W05 LowRiskFix cleanup 반영
- [ ] P1 DecisionNeeded 항목 확정
- [ ] EventLog category filter 구현
- [ ] Runtime LOD actual 표시 보강
- [ ] P1 완료 후 FinalCandidate 촬영 / 패키징

## 브랜치

- `feature/debug-overlay-evidence-plan`

## 요약

이번 PR 후보는 resume / 기술문서 / 제출 영상에서 runtime evidence를 설명하기 위한 개발 전용 Debug Overlay를 추가한다.

완성형 gameplay HUD가 아니라, Action / Reaction, CombatSignal / Damage, Enemy AI 관련 현재값과 최근 event를 TestRoom PIE에서 확인하기 위한 evidence tooling이다. Overlay는 shipping product UI처럼 보이지 않게 유지하며, 실제 코드에서 읽을 수 없는 값은 `N/A`, `NotCaptured`, `Pending`으로 표시한다.

작업은 기능 구현과 W05 code quality 정리를 함께 포함한다.

```text
P0
-> SnapshotStore / debug hook / Canvas HUD

P0.5
-> Player/Enemy panel, Movement, HP, Stagger Count, compact 표시

P1
-> TargetComponent 기반 EnemySource chain
```

## 변경 배경

기술문서와 포트폴리오 문서에서 gameplay 구조를 설명하려면, 코드 경로만이 아니라 실제 runtime에서 어떤 판단과 결과가 발생했는지 보여주는 evidence가 필요하다.

기존 audit log는 Output Log 기반이라 제출 영상이나 스크린샷에 바로 쓰기 어렵고, 기존 gameplay HUD로 만들면 shipping HUD처럼 오해될 수 있다. 따라서 debug-only CVar gate 뒤에서만 동작하는 overlay를 별도 evidence tooling으로 구성했다.

또한 이 브랜치에서 추가된 debug overlay 코드는 P42~P51의 W05 코드 클린 기준과 충돌하지 않아야 한다. 특히 debug log policy, CVar ownership, include order, naming, section, tuning constant, shipping guard 경계를 함께 검토했다.

## 주요 변경

### 1. SnapshotTypes / SnapshotStore

무엇:

- `FDebugOverlaySnapshotTypes.h`
- `FDebugOverlaySnapshotStore.h/.cpp`

역할:

- World별 debug overlay snapshot cache를 제공한다.
- 현재 gameplay state의 owner가 아니라 evidence 표시를 위한 최근값 저장소다.
- event log는 고정 크기 ring buffer로 유지한다.
- snapshot 조회는 copy 반환 방식으로 처리한다.

주요 정책:

```text
World별 store
-> TObjectKey<UWorld> 기반 분리

Actor 장기 보관
-> raw pointer 금지
-> weak pair / actor name / frame / time 정보만 저장

Shipping
-> CVar 미선언 참조 없음
-> Record API no-op
-> Query API false 또는 empty
```

### 2. 기존 debug hook 연결

무엇:

- `FExecutionOrchestratorDebug.cpp`
- `FCombatSignalDebug.cpp`
- `FCombatResultDebug.cpp`
- `FAICombatBTDebug.cpp`

역할:

- 기존 audit log 출력 흐름을 바꾸지 않고 SnapshotStore에 최근 event를 기록한다.
- 기존 `ShouldAudit...()` CVar와 overlay collect CVar를 결합하지 않는다.

연결 항목:

```text
Execution
-> Action / Reaction decision

CombatSignal
-> HitWindow
-> Target accepted / rejected
-> CombatResult dispatch

CombatResult
-> CombatResult received

AI
-> Combat task success / reject
```

### 3. Canvas 기반 Debug Overlay HUD

무엇:

- `ACDebugOverlayHUD`
- `ACDebugOverlayGameMode`

역할:

- UMG / Slate 없이 `AHUD::DrawHUD()`와 Canvas Draw로 개발 전용 overlay를 표시한다.
- 전역 `GlobalDefaultGameMode`를 변경하지 않고 TestRoom 수동 연결을 기준으로 둔다.

표시 항목:

```text
[Debug Overlay P0.5]

[Player]
State / Action / Reaction / Stagger / Guard / Movement / HP / Runtime LOD / AI

[Enemy]
EnemySource / EnemyTarget or fallback
State / Action / Reaction / Stagger / Guard / Movement / HP / Runtime LOD / AI

[Recent Execution]
[Recent Combat]
[Recent AI]
[Event Log]
```

### 4. P0.5 표시 보강

무엇:

- Player / Enemy panel 분리
- blue / red tab
- 배경 박스와 글자 크기 조정
- Movement, HP, Stagger Count 추가
- enum prefix 제거
- multi-field line `|` 구분
- Guard action 의미 중심 표시

결과:

- Player와 Enemy의 현재 상태를 같은 순서로 비교할 수 있다.
- Stagger Count는 현재 parry stack으로만 표시하며 누적 통계처럼 주장하지 않는다.
- EventLog 추가 축약은 P0.5에서 보류했다.

### 5. P1 TargetComponent source chain

무엇:

- `UCDebugOverlayTargetComponent`
- `ACPlayerController` debug Exec command
- HUD enemy source chain
- Store recent combat pair

역할:

- Enemy panel이 단순 world scan이 아니라 명시 target source를 우선 사용할 수 있게 한다.

source chain:

```text
TargetComponent
-> RecentCombatTarget
-> WorldScanFallback
```

Exec command:

```text
DebugOverlaySelectTarget
DebugOverlaySelectNearestTarget
DebugOverlayClearTarget
```

비목표:

- 범용 target system이 아니다.
- lock-on, target cycling, combat action target 강제는 구현하지 않는다.
- 브랜치 마감 후 필요하면 범용 target component로 리팩터링한다.

## W05 Code Quality 반영

| W05 기준 | 반영 내용 |
| --- | --- |
| P42 Debug Log Policy | audit log와 overlay collect를 분리하고, gameplay hook은 record 호출만 수행한다. |
| P43 CVar Ownership | `Portfolio.DebugOverlay.*` CVar를 기존 debug audit / profiling CVar와 분리했다. |
| P44 Comment / Section Cleanup | HUD / Store helper section을 책임 단위로 정리했다. |
| P45 Naming / API Cleanup | `Try...`, `Append...`, `TryGetSnapshotCopy` naming을 적용했다. |
| P46 Type Header Organization | debug overlay snapshot type을 `Core/Debug` 소유 type으로 분리했다. |
| P48 Include Order Cleanup | HUD / Store / PlayerController `.cpp` include group을 W05 기준으로 정리했다. |
| P49 API Const Consistency | getter/query const 여부와 DecisionNeeded 항목을 문서화했다. |
| P50 Section Comment Consistency | Store API section과 HUD helper section을 파일군 책임 기준으로 정리했다. |
| P51 Tuning Constants Cleanup | trace distance, nearest radius, stale timeout, event limit을 internal policy constant로 분류했다. |

## 변경 파일 범위

```text
Docs/07_Portfolio_Documents/Debug_Overlay/*
Docs/04_Pull_Request/P52_UE5_Portfolio_Pull_Request.md

Source/Portfolio/Core/Debug/*
Source/Portfolio/Controller/CPlayerController.h/.cpp
Source/Portfolio/Character/Player/CPlayer.h
Source/Portfolio/Character/Enemy/CEnemy.h
```

## 검증

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### Static check

```text
git diff --check
Result: Pass
```

### PIE

```text
Map:
/Game/00_UnitTest/TestRoom

Result:
P0.5 overlay 표시 확인
Player / Enemy panel 표시 확인
Movement / HP / Stagger Count 표시 확인
Recent Execution / Combat / AI / EventLog 갱신 확인
TargetComponent / RecentCombatTarget / WorldScanFallback source chain 체크리스트 작성
```

주의:

- Round1 / Round1_StaggerCount는 임시 검증 evidence다.
- 최종 제출용 FinalCandidate 촬영은 P1 완료 후 별도 진행한다.
- Runtime LOD actual value와 AI current detail은 아직 성공 evidence로 주장하지 않는다.

## 비범위 / 후속 작업

이번 PR 후보에서 완료하지 않은 항목:

```text
EventLog category filter
Player/Enemy Recent/EventLog 분리
Runtime LOD actual 표시
AI current value 보강
TargetComponent selection method 세분화
DebugOverlayTargetComponent shipping reflection 정책 결정
RecentCombat stale/not matched 문구 축약 여부 결정
FinalCandidate 촬영 / 패키징
포트폴리오 본문 연결
```

명시적 비목표:

```text
Shipping HUD화
UMG / Slate 전환
범용 lock-on system
combat action target 강제
AI target selection 변경
기존 audit log format 변경
성능 성공 주장
```

## 관련 문서

- `Docs/07_Portfolio_Documents/Debug_Overlay/README.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Work_Order_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Scope_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Quality_Review_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_W05_PR_Style_Gap_Review_KR.md`
- `Docs/04_Pull_Request/P42_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P43_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P44_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P45_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P46_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P48_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P49_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P50_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P51_UE5_Portfolio_Pull_Request.md`

## 정리

이번 PR 후보는 debug overlay를 제출용 evidence tooling으로 구성하고, P0/P0.5/P1로 단계화된 구현과 W05 code quality cleanup을 하나의 설명 가능한 흐름으로 묶는다.

현재 상태는 최종 촬영 전 기능 검증과 코드 품질 정리를 진행한 단계다. 이후에는 P1 DecisionNeeded 항목을 확정하고, EventLog filter / Runtime LOD / AI 보강을 닫은 뒤 FinalCandidate 촬영으로 넘어간다.
