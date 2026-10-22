# 순찰 전용 재현 장면

Project Stellar · UE 5.4.4 · 2026-10-22

## 목적과 범위

`/Game/00_UnitTest/PatrolReplay/L_PatrolReplay`는 실제 순찰 BT의 목표 선택,
이동과 장애물 우회를 확인하는 전용 장면이다. main `8b803431`에서 분리했다.
원본 SciFiMap의 순찰 장면 목적을 작은 독립 맵으로 재구성했으며, 원본 배치를
그대로 복제한 맵이나 완성된 보스전은 아니다.

공용 SciFiMap·TestRoom, PlayerStart 기본 맵 설정, 기존 BP·BT·EQS·몽타주와
AI 튜닝 값은 변경하지 않는다. 기존 BP_CEnemy를 맵에 배치하고 해당 인스턴스의
순찰 사용·경로·Loop 모드만 지정했다. 감지 반경은 500/1000이며 전투 참여
상한은 main의 Base 2 / Extra 3 / Total 5 / Alert 6 / Observe 12를 유지한다.

## 장면 구성

| 구성 | 값 / 역할 |
|---|---|
| GameMode | 기존 SciFiMap과 같은 GM_Portfolio |
| 바닥 | 50m × 50m, 윗면 Z=0 |
| 순찰 경로 | ACPatrolPath 1개, ACPatrolPoint 4개 |
| P0 | (-600, -600, 0) cm |
| P1 | (600, -600, 0) cm |
| P2 | (600, 600, 0) cm |
| P3 | (-600, 600, 0) cm |
| 중앙 장애물 | 중심 (0, 0, 150), 크기 400 × 400 × 300cm |
| 이동 경로 장애물 | 중심 (0, -600, 100), 크기 250 × 250 × 200cm |
| 적 시작 위치 | (-1100, -600, 100), 기존 BP_CEnemy, Loop |
| PlayerStart | (1900, 0, 100), 초기 순찰 감지 범위 밖 |
| NavMeshBounds | 중심 (0, 0, 200), 크기 4800 × 4800 × 900cm |

NavMesh는 에디터에서 빌드한 뒤 맵에 저장했다. 맵을 새 프로세스에서 다시
로드했을 때 네 지점 모두 투영되고 각 인접 지점 사이에 완전한 경로가 있어야 한다.
P0→P1 구간은 중앙의 작은 장애물을 우회한다. 직접 연결선은 설정 순서이며
실제 navigation path가 아니다.

## 직접 PIE로 확인

전용 맵을 열고 PIE를 시작하면 배치된 적이 기존 전체 BT를 통해 순찰한다.
에디터 Outliner에서 적을 관찰 대상으로 선택하거나 Debug Overlay의 Focus
선택 기능을 사용한다. 플레이어가 접근하면 자연스러운 인지가 순찰을 중단할 수 있다.
순찰만 볼 때에는 시작 위치에 머문다.

```text
Portfolio.DebugOverlay.HUDVisible 1
Portfolio.DebugOverlay.Patrol.Enabled 1
Portfolio.DebugOverlay.Patrol.DrawPoints 1
Portfolio.DebugOverlay.Patrol.DrawLinks 1
Portfolio.DebugOverlay.Patrol.RecordTrail 1
Portfolio.DebugOverlay.Patrol.DrawTrail 1
Portfolio.DebugOverlay.Patrol.DrawAcceptance 1
```

목표 순서와 적의 실제 위치 변화를 함께 확인한다. 기본 Loop는
P0→P1→P2→P3→P0이다. Reverse는 다음 순서로 확인한다.

1. PIE를 종료하고 전용 맵을 **Save Current Level As**로 별도 테스트 맵에 저장한다.
2. 저장한 복제 맵을 열고, 배치 Enemy에서 `PatrolConfig → PatrolMode`를 `Reverse`로 바꾼 뒤 저장한다.
3. 저장한 테스트 맵을 다시 열고 새 PIE를 시작한다. 시작 위치에 머물며
   P0→P1→P2→P3→P2→P1→P0 왕복과 실제 이동을 함께 확인한다.
4. 확인 후 PIE를 종료하고 원래 `L_PatrolReplay`를 다시 연다. 원래 맵의 Loop 설정은 유지한다.

실행 중인 Enemy의 `PatrolConfig`만 변경해도 BT 블랙보드에 자동 반영되지는 않는다.
설정은 컨트롤러 초기화 때 블랙보드에 복사되므로 **PIE 시작 전** 변경해야 한다.
기존 임시 Probe의 BT 재시작·블랙보드 직접 설정은 위 수동 절차와 구분한다.

## 무인 촬영과 완료 해석

[재현 시나리오 안내](N_Portfolio_Replay_Scenarios.md)의 별도 프로세스 실행을
사용하되 맵 인자를 이 전용 맵으로 바꾼다. 출력 경로는 매 실행 새로 지정한다.

```powershell
& 'C:/Program Files/Epic Games/UE_5.4/Engine/Binaries/Win64/UnrealEditor.exe' `
  '<workspace>/Portfolio.uproject' '/Game/00_UnitTest/PatrolReplay/L_PatrolReplay' `
  '-Structure10Mode=patrol' '-Structure10Output=<new-output>/take' `
  '-Structure10Preset=<preset-file>' `
  '-Structure10Width=1280' '-Structure10Height=720' `
  '-PatrolRunX=1900' '-PatrolRunY=0' `
  '-PatrolCamX=1600' '-PatrolCamY=-2000' '-PatrolCamZ=2100' `
  '-PatrolCamYaw=130' '-PatrolCamPitch=-40' '-PatrolCamFov=70' `
  '-ReplayAllowSilentAudio' '-RenderOffscreen' `
  '-unattended' '-nop4' '-nosplash' '-NoLiveCoding' '-NoAutoSave'
```

프리셋 파일에는 위 관찰 설정과 `Portfolio.Capture09.RecordFPS 30`,
`t.MaxFPS 60`을 한 줄씩 넣는다. 이벤트까지 볼 경우 수집·이벤트 표시를
별도로 켠다. DrawTrail만 켜서는 이동 기록 수집이 시작되지 않는다.
무음 허용은 순찰 영상의 명시적 정책이며 효과음 검증을 의미하지 않는다.

기존 촬영기는 배치된 적을 숨기고 해당 경로를 사용하는 임시 적을 만든다.
플레이어 배치·관찰 카메라를 바꾸므로 일반 편집 세션이 아닌 일회성 무인
프로세스에서 실행한다. `PatrolCaptureSight`는 전달하지 않아 감지 반경을
보정하지 않는다. AI·공격 속도·시간 배율 튜닝을 프리셋에 넣지 않는다.

현재 `patrol` 촬영기의 종료는 32초 경과 기준이다. 성공 표식은 내보내기
계약과 촬영 종료를 확인하지만 BT 이동·지점 도착 자체를 판정하지 않는다.
따라서 이동 근거는 BT 상태·실제 변위·목표 순서·도착 거리 기록과 함께 확인한다.
기존 SciFiMap의 `NoPatrolPath` 실패는 그대로 유지된다.

## 실제 BT 이동 검증

2026-10-22, 별도 UE 5.4.4 에디터의 실제 렌더링 PIE에서 저장된 전용 맵을
다시 로드했다. 1280×720, 게임 FPS 상한 60이며 고정 프레임률 보증은 아니다.
임시 관측 코드는 위치를 이동시키거나 MoveTo 성공을 주입하지 않았다.
Loop는 저장된 배치와 기존 BT를 사용했고 Reverse는 PIE 안에서 모드와 초기
인덱스/방향만 설정해 기존 BT를 다시 시작했다.

| 조건 | 관측 결과 |
|---|---|
| Loop | 목표 선택 0→1→2→3→0→1, 누적 이동 4957.41cm |
| Reverse | 목표 선택 0→1→2→3→2→1→0→1, 누적 이동 7127.10cm |
| 실제 이동 | BT_Idle의 Move To, Moving 상태와 양수 속도를 함께 관측 |
| 도착 | 이전 지점의 2D 거리가 200cm 이내인 상태에서 다음 목표 선택 |
| 경로 | 네 인접 구간 완전 경로, P0→P1 길이 1249.15cm / 직선 1200cm |
| 설정 | 감지/상실 반경 500/1000 유지, 기존 에셋 해시 동일 |

마지막 목표 선택은 다음 순찰 구간의 시작 확인이다. 예를 들어 Loop의 마지막
P1 선택을 P1 재도착으로 세지 않는다. 도착 거리는 MoveTo의 기본 허용 반경과
캡슐 반경 포함 정책의 결과이며, 지점 좌표에 정확히 일치해야 한다는 뜻은 아니다.

임시 맵 생성·관측 코드는 검증 후 제거했다. 이 기록은 순찰과 경로 탐색의
근거이며 순찰→전투→수색→복귀 전체 전환, 모든 프레임 조건, 영상의 연속
시청·청취를 검증했다는 뜻은 아니다.

## 최종 분리본 회귀 검증

- 임시 코드 제거 후 기본 Unity 및 Adaptive Unity를 끈 전체 Unity Editor 빌드 통과.
- 전체 자동화 98개: 일반 성공 96, 기존 경고 동반 성공 2, 실패·미실행 0.
- `CDodgeAssets -Verify` 오류·경고 0. 처형 몽타주 검사는 308개 패키지 로드,
  검사 오류·경고 0이며 로딩 중 기존 legacy 머티리얼 참조 경고는 남는다.
- 최종 빌드의 실제 `Capture10 patrol` 실행에서 성공 표식과 정상 내보내기를 확인했다.
  요청·반환·저장 JPEG 1029개가 일치하고 전부 디코딩됐다. CSV 상태도 모두 정상이다.
- PIE 창 요청은 1280×720이고, 기존 recorder의 출력 기본값에 따라 저장된 JPEG는
  1920×1080이다. 녹화 요청 30fps, 게임 FPS 상한 60으로 실행했다.
- WAV는 48kHz stereo 16-bit, 34.88초의 무음 PCM이다. 명시적 무음 허용으로
  내보내기는 성공했으며 효과음·음성 또는 시청각 동기화 성공을 주장하지 않는다.
- 기존 SciFiMap에서는 `FAIL NoPatrolPath`와 실패 표식만 생성됐으며 성공 표식은
  없었다. 이 실행의 녹화 파일이 정상이어도 시나리오 성공으로 승격되지 않았다.

순찰 모드에서 별도 패널 PNG 내보내기는 검증하지 않았다. 최종 촬영에서는
HUD·순찰 지점·실제 이동 궤적이 포함된 게임 프레임을 확인했다.

## R1 안내 정정과 커밋 경계 재검증

2026-10-23, Reverse 수동 안내를 PIE 시작 전 별도 테스트 맵 설정으로 정정했다.
안내와 같은 에디터 상태 전환을 임시 에디터 API 자동화로 실행했다. 전용 맵을
Save As한 복사본을 다시 열고 배치 Enemy의 모드를 Reverse로 저장한 뒤,
새 에디터 프로세스에서 해당 맵을 로드해 실제 PIE를 시작했다.
메뉴 클릭 자체를 수동 검수한 기록은 아니다.

이번 관측 코드는 블랙보드를 직접 쓰거나 BT를 중단·재시작하지 않았다.
저장된 Enemy 설정과 초기 블랙보드가 모두 Reverse인 것을 확인하고,
기존 컨트롤러 초기화 및 전체 BT의 실제 Move To를 관측했다.

| 실제 PIE 조건 | 결과 |
|---|---|
| 별도 저장 Reverse | 목표 0→1→2→3→2→1→0→1, 이동 7120.66cm, 31개 검사 통과 |
| 원래 저장 Loop | 목표 0→1→2→3→0→1, 이동 4961.83cm, 27개 검사 통과 |
| 이전 지점 도착 | Reverse 7회, Loop 5회. 마지막 목표 선택은 추가 도착으로 세지 않음 |
| 경로와 설정 | 네 점 투영·네 인접 완전 경로, 감지 반경 500/1000 유지 |

첫 자동화 시도는 Save As 이후 복사본을 다시 열지 않아 후속 저장이 거부됐고,
Loop가 남은 복사본을 Reverse 검사에서 실패로 판정했다. 복사본 재열기와
저장 성공 로그 확인을 추가한 뒤 위 결과를 얻었다. 원래 Loop 맵은 수정하지 않았다.

임시 관측 코드·모듈 의존성·Reverse 테스트 맵을 제거한 뒤, 커밋 없이 작업 파일을
각 경계로 구성해 다음 검사를 실행했다. C1은 새 맵만, C2는 맵과 문서 2개다.

| 경계 | 기본 Unity / 전체 Unity | 전체 자동화 | 에셋 검사 |
|---|---|---|---|
| C1 | 통과 / 통과 | 98개: 성공 96, 경고 동반 성공 2, 실패·미실행 0 | Dodge 오류·경고 0, 몽타주 308개 패키지 감사 오류·경고 0 |
| C2 | 통과 / 통과 | 98개: 성공 96, 경고 동반 성공 2, 실패·미실행 0 | Dodge 오류·경고 0, 몽타주 308개 패키지 감사 오류·경고 0 |

몽타주 로딩 중 기존 legacy 머티리얼 참조 경고는 남는다. 전체 Unity는
`-DisableAdaptiveUnity` 조건이다. 결과 기록을 추가한 이 문단은 실행 후 문서만
갱신했으며 링크·diff·범위 검사를 다시 수행했다. 기존 에셋 1,439개와 원래 Loop
맵 해시는 유지됐다. 이번 재검증에서 녹화 내보내기·전체 전투 전환을 다시
실행한 것은 아니며, 위 2026-10-22 결과와 구분한다.
