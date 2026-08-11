# UE5 Portfolio Pull Request

## 제목

**P57: Player Targeting and Lock-On Presentation**

## 날짜

**2026.08.11**

## 상태

- [x] 카메라 전방 기반 Player Target 선택·해제
- [x] 중앙 우선 후보 평가와 주기적 유효성 검사
- [x] 현재 타겟 Destroy delegate 수명 관리
- [x] 화면 공간 좌우 Target Switching
- [x] 락온 camera / movement rotation assist
- [x] 근거리 distance-based Pitch 완화
- [x] Socket 기반 Target Marker 표시
- [x] Targeting world debug와 Debug Overlay Focus 연동
- [x] Work List 구현 범위 마감 및 Destroy Lifecycle 연계 검증 이관
- [x] `git diff --check origin/main..HEAD` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] 사용자 PIE 검증 완료

## 브랜치

- Base: `main`
- Branch: `feat/player-targeting-component`
- Implementation HEAD: `e25d94cc docs(targeting): close target marker milestone`
- Merge Policy: 일반 Merge Commit

## 대표 스크린샷

이번 PR은 사용자 PIE에서 Target Marker, Debug Overlay, 락온 camera 동작을 확인했다. 별도 screenshot evidence 파일은 PR 문서에 추가하지 않고, 연결된 Widget / texture / socket asset과 Work List의 검증 기록을 근거로 둔다.

## 요약

이번 PR은 Player가 카메라 전방의 Enemy를 선택하고 유지하며, 화면 공간 기준으로 좌우 전환할 수 있는 락온 타게팅 기반을 구성한다.

선택된 타겟은 camera와 Player 회전 정책, 화면 Target Marker, Debug Overlay가 공유하는 단일 runtime source가 된다. Targeting의 선택 상태, 락온 보정, UI 표시, debug 표현은 각각 별도 component와 helper가 담당하도록 분리했다.

Dead 이후 Actor Destroy 정책과 Focus Locomotion asset 변경은 이번 PR에 포함하지 않는다. Destroy delegate 처리 구조는 구현했으며 실제 Destroy 경계 검증은 다음 Character Destroy Lifecycle 작업으로 이관한다.

## 변경 배경

기존 Debug Overlay Focus는 개발 중 Actor를 관찰하는 도구였지만, Player gameplay가 직접 소유하는 현재 전투 타겟은 없었다. 따라서 다음 기능이 하나의 상태를 공유하지 못했다.

- Player 입력에 따른 Enemy 선택과 해제
- 타겟 전환
- 락온 camera와 캐릭터 정면 제어
- Target Marker 표시
- 실제 Player Target을 기준으로 한 debug 관찰

이번 PR에서는 Debug Overlay Focus와 Player Target을 별도 상태로 유지하면서도 필요할 때 Player Target을 Live/Frozen Focus로 관찰할 수 있게 했다. 또한 TargetingComponent가 UI나 world debug를 직접 그리지 않도록 runtime 판단과 표현 책임을 분리했다.

## 주요 변경

### 1. Target 선택·유지·해제

대상:

```text
Source/Portfolio/Type/CTargetingTypes.*
Source/Portfolio/Component/CTargetingComponent.*
Source/Portfolio/Controller/CPlayerController.*
Config/DefaultInput.ini
```

변경 내용:

- `UCTargetingComponent`를 PlayerController의 default subobject로 추가했다.
- `MiddleMouseButton` 입력으로 현재 타겟을 선택하거나 해제한다.
- 카메라 viewpoint 기준 거리, View Cone, angle score, distance score를 계산한다.
- 후보 선택과 Debug Snapshot이 같은 Target Evaluation 결과를 사용한다.
- 현재 타겟은 `TWeakObjectPtr<ACEnemy>`로 보관한다.
- 일정 간격으로 생존과 최대 거리를 검증해 유효하지 않은 타겟을 해제한다.
- `SetCurrentTarget()`을 통해 타겟 변경과 `OnDestroyed` bind/unbind를 단일 경로로 처리한다.
- 직접 Destroy callback은 callback actor identity를 확인한 뒤 `OnTargetChanged(DestroyedEnemy, nullptr)` 계약을 유지한다.
- `MaxTargetAngleDegrees == 0`인 경우 0으로 나누지 않고 정확한 정면 방향만 angle score 1로 처리한다.

### 2. 화면 공간 Target Switching

대상:

```text
Source/Portfolio/Type/CTargetingTypes.h
Source/Portfolio/Component/CTargetingComponent.*
Source/Portfolio/Controller/CPlayerController.*
Config/DefaultInput.ini
```

변경 내용:

- `MouseScrollUp / MouseScrollDown`을 Left / Right 전환 입력으로 연결했다.
- 현재 타겟과 후보를 screen position으로 투영해 방향을 판정한다.
- 생존, 거리, View Cone, Viewport 조건을 통과한 Enemy만 후보로 사용한다.
- 선택 방향에서 수평 거리, 수직 거리, 기존 FinalScore 순서로 가장 인접한 후보를 고른다.
- 방향 후보가 없으면 현재 타겟과 delegate binding을 유지한다.
- 전환 성공은 기존 `SetCurrentTarget()` 경로를 사용해 변경 event와 Destroy 구독 계약을 재사용한다.

### 3. Lock-On Assist와 movement rotation 책임 분리

대상:

```text
Source/Portfolio/Component/CTargetLockAssistComponent.*
Source/Portfolio/Component/CMovementComponent.*
Source/Portfolio/Component/CDefenseComponent.*
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Type/CTargetingTypes.h
```

변경 내용:

- `UCTargetLockAssistComponent`가 Target 변경, Possession, camera tracking과 rotation mode 연결을 담당한다.
- 비락온에서는 `OrientToMovement`, 락온에서는 `ControllerDesired` rotation mode를 사용한다.
- 락온 중 Player의 자유 Look Yaw / Pitch 입력을 억제한다.
- Guard는 Walk gait만 제어하고 rotation policy를 소유하지 않는다.
- movement gait override와 rotation mode 명령을 분리해 Guard와 Target Lock이 서로의 책임을 덮어쓰지 않게 했다.
- 근거리에서는 설정된 기준 Pitch를 사용하고, 원거리에서는 실제 Target Pitch까지 수평거리 기반 SmoothStep으로 연결한다.
- 타겟 해제, UnPossess, EndPlay에서는 Player rotation policy를 복구한다.

### 4. Socket 기반 Target Marker

대상:

```text
Source/Portfolio/UI/CTargetHUDWidget.*
Source/Portfolio/Component/CTargetHUDPresenterComponent.*
Source/Portfolio/Character/Enemy/CEnemy.*
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Portfolio.Build.cs
Content/08_UI/BP_CTargetHUDWidget.uasset
Content/08_UI/Textures/T_TargetMarkerDot.uasset
Content/01_Character/Mesh/Mesh/SKM_Quinn_Portfolio_Skeleton.uasset
```

변경 내용:

- `UCTargetHUDPresenterComponent`가 Target 변경 구독, Widget 수명, world-to-widget projection을 담당한다.
- 하나의 Target HUD Widget을 생성하고 타겟 전환 때 재생성하지 않고 ViewData만 갱신한다.
- `UCTargetHUDWidget`은 C++ ViewData를 저장하고 Blueprint 구현 event로 전달한다.
- Enemy는 `TargetMarker` socket 또는 fallback offset을 통해 표시 기준 world location을 제공한다.
- 타겟이 camera 뒤나 DPI 보정 Viewport 밖에 있으면 Marker만 숨기고 Target Lock은 유지한다.
- 화면 안으로 돌아오면 같은 Widget을 다시 표시한다.
- Runtime module의 UMG dependency를 명시적으로 추가했다.

### 5. Targeting Debug와 Focus 관찰

대상:

```text
Source/Portfolio/Core/Debug/FTargetingDebug.*
Source/Portfolio/Core/Debug/CDebugOverlayHUD.*
Source/Portfolio/Core/Debug/FDebugOverlayFocusRuntimeHelper.*
Source/Portfolio/Core/Debug/FDebugOverlayViewDataTypes.h
Source/Portfolio/Core/Debug/FDebugOverlayViewDataBuilder.*
Source/Portfolio/Core/Debug/FDebugOverlayTextFormatter.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/
```

변경 내용:

- `UCTargetingComponent`에서 world debug rendering을 제거하고 read-only Snapshot만 제공한다.
- `FTargetingDebug`가 Range Sphere, Selected Target Sphere, View Line, World Debug Text를 개별 CVar로 제어한다.
- Debug Overlay에 distance, Dot, angle/distance/final score와 범위 판정을 표시한다.
- PlayerTarget Live는 CurrentTarget 변경을 Overlay Focus에 계속 동기화한다.
- PlayerTarget Frozen은 마지막으로 동기화한 Focus를 유지한다.
- Frozen 대상이 파괴되면 FocusActor만 안전하게 비운다.
- Manual / Outliner / RecentCombat Focus는 PlayerTarget driver가 아닐 때 자동 동기화로 덮어쓰지 않는다.
- Editor panel을 Overlay Options, Targeting Display Options, Focus Options 책임으로 정리했다.
- Editor CVar access는 console object pointer를 장기 cache하지 않고 안전한 조회 경로를 사용한다.

### 6. Blueprint / asset adapter

대상:

```text
Content/00_UnitTest/TestRoom.umap
Content/01_Character/01_Player/BP_CPlayer.uasset
Content/01_Character/02_Enemy/BP_CEnemy.uasset
Content/01_Character/Mesh/Mesh/SKM_Quinn_Portfolio.uasset
Content/01_Character/Mesh/Mesh/SKM_Quinn_Portfolio_Skeleton.uasset
Content/02_Controller/01_Player/BP_CPlayerController.uasset
Content/08_UI/BP_CTargetHUDWidget.uasset
Content/08_UI/Textures/T_TargetMarkerDot.uasset
```

변경 내용:

- PlayerController Blueprint에 Target HUD Widget class와 Lock Assist tuning을 연결했다.
- Target HUD Widget에 전체 화면 Canvas와 중앙 pivot Marker Image를 구성했다.
- Enemy Skeleton에 `TargetMarker` socket을 추가했다.
- TestRoom에서 다중 Enemy 선택, 전환, camera와 Marker 동작을 검증할 수 있게 asset을 갱신했다.

## 주요 처리 흐름

### 최초 선택과 해제

```text
MiddleMouseButton
-> ACPlayerController::PressTargetLock()
-> UCTargetingComponent::ToggleTargetLock()

CurrentTarget 없음
-> World Enemy 탐색
-> 공용 Target Evaluation
-> 최고 점수 후보 선택
-> SetCurrentTarget(NewTarget)

CurrentTarget 있음
-> ClearTarget()
-> SetCurrentTarget(nullptr)
```

### 타겟 전환

```text
Mouse Wheel
-> SwitchTarget(Direction)
-> Current / Candidate screen projection
-> 생존 / 거리 / View Cone / Viewport / 방향 검사
-> 수평 거리 / 수직 거리 / FinalScore 비교
-> SetCurrentTarget(BestSwitchTarget)
```

### 상태 공유

```text
SetCurrentTarget()
-> 이전 Target OnDestroyed unbind
-> 새 Target OnDestroyed bind
-> OnTargetChanged(PreviousTarget, NewTarget)
   -> TargetLockAssist: camera / rotation policy 갱신
   -> TargetHUDPresenter: Marker 표시 상태 갱신
   -> PlayerTarget Live Focus: Overlay Focus 갱신
```

### Target Marker

```text
CurrentTarget
-> Enemy::GetTargetMarkerWorldLocation()
-> TargetMarker socket 또는 fallback offset
-> ProjectWorldLocationToWidgetPosition()
-> DPI 보정 Viewport 검사
-> FTargetMarkerViewData
-> UCTargetHUDWidget
-> Blueprint Visibility / Canvas Slot Position
```

## 변경 파일 범위

핵심 변경 범위:

```text
Config/DefaultInput.ini
Content/00_UnitTest/TestRoom.umap
Content/01_Character/
Content/02_Controller/01_Player/BP_CPlayerController.uasset
Content/08_UI/
Docs/06_notes/task_briefs/W05_Player_Targeting/
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/
Source/Portfolio/Character/Enemy/CEnemy.*
Source/Portfolio/Component/CTargetingComponent.*
Source/Portfolio/Component/CTargetLockAssistComponent.*
Source/Portfolio/Component/CTargetHUDPresenterComponent.*
Source/Portfolio/Component/CMovementComponent.*
Source/Portfolio/Component/CDefenseComponent.*
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Core/Debug/
Source/Portfolio/Type/CTargetingTypes.*
Source/Portfolio/UI/CTargetHUDWidget.*
Source/Portfolio/Portfolio.Build.cs
```

이번 PR에 포함하지 않은 Focus Locomotion 백업 asset:

```text
Content/03_Animation/00_Idle/00_Unarmed/
Content/03_Animation/Dodge/Lucy/
Content/03_Animation/ABP_Character.uasset
Content/05_BlendSpace/BS_Unarmed_Lucy.uasset
```

## 테스트 방법

### Static check

```text
git diff --check origin/main..HEAD
```

### Build

```text
PortfolioEditor Win64 Development
```

### PIE

```text
1. MiddleMouseButton으로 중앙 Enemy를 선택하고 다시 눌러 해제한다.
2. 여러 Enemy 사이에서 Mouse Wheel Up/Down으로 좌우 전환한다.
3. 선택 방향에 후보가 없을 때 기존 Target이 유지되는지 확인한다.
4. 타겟 사망과 거리 초과에서 자동 해제되는지 확인한다.
5. Targeting Debug 각 표시와 Overlay Details를 개별로 전환한다.
6. PlayerTarget Live/Frozen과 Manual Focus 보존을 확인한다.
7. 락온 중 camera, 8Way rotation, Guard gait와 Look 억제를 확인한다.
8. 근거리/원거리 이동에서 Pitch 연결을 확인한다.
9. Target Marker의 선택, 전환, 해제, 화면 밖 숨김과 재진입 표시를 확인한다.
10. DPI Scale 0.67 환경에서 Marker와 Widget 좌표가 일치하는지 확인한다.
```

## 검증 결과

사용자 PIE 확인 완료:

```text
- MiddleMouseButton Target 선택·해제
- 중앙 우선 후보 선택과 표시 score 일치
- Mouse Wheel 좌우 전환
- 방향 후보 부재 시 현재 Target 유지
- Target 사망·거리 초과 자동 해제
- Debug Snapshot과 실제 선택 score 일치
- Range Sphere / Selected Sphere / View Line / World Text 개별 표시
- PlayerTarget Live / Frozen
- Frozen Focus 대상 파괴 시 Focus 해제
- Manual Focus 보존
- 비락온과 락온 movement rotation policy 전환
- Guard gait와 Target Lock rotation 책임 분리
- 락온 Look 입력 억제와 해제 후 복구
- 근거리 Pitch 안정화와 원거리 Target Pitch 연결
- Socket 기반 Target Marker 추적
- Marker 선택 / 전환 / 해제
- 화면 밖 Marker 숨김, Lock 유지, 재진입 표시
- DPI Scale 0.67 환경의 Marker 위치
```

후속 Character Destroy Lifecycle 작업으로 이관한 통합 검증:

```text
- 현재 Target 직접 Destroy event cardinality
- 사망 해제 후 Destroy 중복 event 방지
- A -> B 전환 뒤 이전 Target A Destroy 시 B 유지
- stale Destroy callback identity guard
- MaxTargetAngleDegrees = 0 자동화 경계
```

## Scope Guard

이번 PR에서 하지 않은 것:

- Dead 이후 Enemy Actor Destroy 정책
- Destroy 검증만을 위한 임시 gameplay command 또는 test harness
- Focus Locomotion 1D/8Way asset 정책
- Enemy AI Focus와 공격 대상 연결
- Player / Enemy 공격 대상의 CurrentTarget 강제 적용
- Enemy 피격 후 Engage 전환 정책
- Enemy Name / HP / Balance Status HUD
- Player BU / BE / HP / SH HUD
- Line Of Sight 기반 후보 제외
- Target Switching wrap-around
- Tick 기반 상시 후보 cache
- Off-screen edge clamp / 방향 indicator
- 자유 시점 lock-on offset과 shoulder camera
- 실제 Merge 및 Ready for Review 전환

## 리스크 / 리뷰 포인트

- `OnDestroyed` bind/unbind와 callback actor identity가 Target 변경 event를 중복 발행하지 않는지 확인한다.
- 최초 선택과 전환은 입력 시점에 `TActorIterator<ACEnemy>`를 사용하며 Tick마다 전체 Enemy를 탐색하지 않는다.
- Validation Tick은 현재 Target만 검사한다.
- Targeting score와 Debug Snapshot이 같은 계산 경로를 사용하는지 확인한다.
- Guard는 gait만, Target Lock은 rotation mode만 제어하는 책임 분리가 유지되는지 확인한다.
- Target HUD Presenter가 Enemy mesh/socket 구조를 직접 해석하지 않고 Enemy API를 통해 world location을 받는지 확인한다.
- Player Target과 Debug Overlay Focus가 별도 상태로 유지되는지 확인한다.
- Binary asset 변경이 Targeting/Marker 연결 범위 안에 한정되는지 확인한다.

## 후속 작업

다음 branch에서 우선 처리:

1. Character Destroy Lifecycle과 이관된 Destroy 경계 통합 검증
2. `backup/focus-locomotion-policy` 기준 비포커스 1D / 포커스 8Way Locomotion 정책
3. Enemy Focus, 공격 Target, 피격 후 Engage 정책
4. Enemy Status HUD의 Name / HP / Balance 표현

## 관련 문서

- `Docs/06_notes/task_briefs/W05_Player_Targeting/README.md`
- `Docs/06_notes/task_briefs/W05_Player_Targeting/TB_W05_01_Player_Targeting_Component_v1.md`
- `Docs/06_notes/task_briefs/W05_Player_Targeting/TB_W05_02_Player_Targeting_Debug_Observability_v1.md`
- `Docs/06_notes/task_briefs/W05_Player_Targeting/TB_W05_03_Player_Target_Switching_v1.md`
- `Docs/06_notes/task_briefs/W05_Player_Targeting/TB_W05_04_Player_Target_Lock_Assist_v1.md`
- `Docs/06_notes/task_briefs/W05_Player_Targeting/TB_W05_05A_Player_Target_Marker_v1.md`

## 대표 커밋

```text
06ae55a4 feat(targeting): add player target acquisition component
595d60f7 refactor(targeting): extract runtime debug snapshot and drawing
569e752e feat(debug-overlay): add player-target live and frozen focus
a3a89af8 feat(debug-overlay-editor): add targeting debug controls
f60b7ed1 fix(targeting): handle target destruction and stabilize evaluation
dee4ec6c feat(targeting): add screen-space target switching
c66e0f12 feat(targeting): add lock-on camera and movement assist
25694047 feat(targeting): refine distance-based lock-on pitch
c98cf925 feat(targeting): add socket-anchored target marker UI
cf0959a5 feat(targeting): finalize target marker presentation
```
