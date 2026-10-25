# PIE 재현 시나리오 실행과 완료 판정

Project Stellar · UE 5.4.4 · 2026-10-23

## 목적과 범위

P2-B2는 main의 녹화·패널 내보내기 도구를 사용하는 에디터 전용 재현 입력이다. 게임플레이 코드, 공격 속도, AI 기본값, 몽타주, Blueprint, 맵은 변경하지 않는다. 자동 입력·임시 적·관찰 카메라를 사용한 재현이며 수동 플레이 기록이나 최종 포트폴리오 영상은 아니다.

## 지원 모드

| 명령 | 모드 | 준비 조건과 관찰 대상 |
|---|---|---|
| `Portfolio.Capture02.Start` | `parry` | 실제 적 AI 공격에 Guard 입력. 실제 Parry 리액션을 관찰해야 완료된다. |
| `Portfolio.Capture09.Start` | `equip`, `combo`, `combo2`, `combo3`, `execution`, `hit` | 장탈착·콤보·피격·처형. `combo`와 `combo3`는 3타, `combo2`는 2타다. |
| `Portfolio.Capture10.Start` | 위 6개 + `debug`, `overview`, `patrol` | 패널 선택·필터·Freeze/Live 관찰. `patrol`에는 3포인트 이상의 레벨 순찰 경로와 이동 가능한 NavMesh가 필요하다. |

명령 뒤에 선택적으로 `warmupSeconds`를 지정한다(0~30초). 수동 명령은 PIE 안에서 사용한다. 녹화는 별도로 시작해야 하며 `Stop`은 시나리오를 종료한다. 무인 실행기는 준비·녹화·PIE 종료를 함께 관리한다.

`patrol`은 `Structure10Output` 실행 인자가 필요하며 플레이어 배치·기존 적의 관찰 상태를 바꾸므로 일회성 무인 프로세스에서 사용한다. 일반 편집 세션에서 순찰 촬영 준비를 재사용하지 않는다.

처형 모드는 임시 적의 HP와 Collapse 준비에 승인된 전투 패킷을 사용한다. 실제 처형 요청·협업 세션은 실행하지만, 자연스러운 패링 누적으로 처형 조건에 도달했다는 증거는 아니다. 피격 모드는 임시 적의 AI를 멈추고 실제 무기 충돌을 사용한다. 순찰 모드는 레벨 경로를 사용하는 임시 적을 만들고 일회성 PIE 안에서 플레이어 위치·HP·관찰 대상을 설정한다.

## 별도 프로세스에서 실행

사용자 편집 중인 에디터에 무인 실행 인자를 전달하지 않는다. 아래는 PowerShell 예시이며 경로는 실제 분리 작업 공간과 새 출력 경로로 지정한다. 실행마다 출력 경로를 바꾼다.

```powershell
& 'C:/Program Files/Epic Games/UE_5.4/Engine/Binaries/Win64/UnrealEditor.exe' `
  '<workspace>/Portfolio.uproject' '/Game/SciFi/SciFiMap' `
  '-Structure09Mode=combo' '-Structure09Output=<new-output>/take' `
  '-Structure09Width=1280' '-Structure09Height=720' `
  '-Structure09IsolateActors' '-RenderOffscreen' `
  '-unattended' '-nop4' '-nosplash' '-NoLiveCoding' '-NoAutoSave'
```

`StructureNNMode`와 `StructureNNOutput`이 모두 있어야 해당 무인 실행기가 활성화된다. `StructureNNPreset`은 선택적 콘솔 명령 텍스트 파일이다. `Structure10ExportPanel`은 Structure10의 패널 PNG 내보내기를 켠다. 인자는 한 실행기의 조합만 사용한다. `RenderOffscreen` 실행도 실제 PIE·게임 렌더링·오디오·파일 내보내기를 수행하며 수동 화면 조작 검증과는 구분한다.

이벤트·필터 관찰에는 프리셋에서 수집과 표시를 명시적으로 켠다. 기본값에서는 이벤트 수집이 꺼져 있으므로 PNG가 생성됐다는 사실만으로 이벤트 내용까지 검증했다고 판단하지 않는다.

```text
Portfolio.Capture09.RecordFPS 30
t.MaxFPS 60
Portfolio.DebugOverlay.CaptureEnabled 1
Portfolio.DebugOverlay.HUDVisible 1
Portfolio.DebugOverlay.EventLog.Visible 1
```

## 완료 판정

`RecorderBusy=0`과 프로세스 종료 코드 0만으로 성공이라고 판정하지 않는다. UE 5.4.4 Windows에서는 실패 요청 후 정상 정리 경로가 종료 코드 0으로 끝나는 경우도 관찰됐다.

무인 실행기는 다음 조건을 모두 확인한 뒤 `<output>.succeeded.txt`를 쓴다.

1. 시나리오 자체가 완료됐다. 조기 녹화 종료·실행 거부·플레이어 사망·EndPIE는 완료가 아니다.
2. 녹화 큐와 내보내기가 끝났다.
3. `record_done.txt`와 `complete.txt`의 `stop_reason`, `frames`, `audio`가 일치한다.
4. `stop_reason=requested_stop`, `frames=complete`, `manifest=written`이다. 드롭·타임아웃·기하 변경 종료를 성공으로 허용하지 않는다.
5. `requested > 0`, `requested = returned = saved`, `pending_at_finalize=0`이며 CSV와 WAV가 존재한다.
6. 기본 오디오 판정은 `valid_pcm_nonzero`다.

장탈착은 Equip/Unequip 요청 수락뿐 아니라 실제 장비 종류 전환을 각각 관찰해야 한다. 준비 단계에서 해제되지 않거나, 장착이 거부되거나, 수락 후에도 장비가 바뀌지 않으면 실패한다. 콤보는 `combo2`의 두 번째 액션 인덱스, `combo`/`combo3`의 세 번째 인덱스에 도달한 뒤 액션이 종료되어야 한다. 첫 액션의 비활성화만으로 목표 타수 도달을 대신하지 않는다. 이 판정은 해당 액션 단계의 실행 관찰이며, 모든 타격의 충돌 성공이나 마지막 액션의 모든 Notify 전달을 보증하지 않는다.

09/10 `execution`은 같은 협업 세션의 치명 처형 커밋 이벤트, 플레이어 액션의 정상 완료, 표적 리액션의 정상 완료를 모두 관찰해야 한다. 취소·중단 이벤트가 있거나 어느 근거라도 빠지면 실패한다. 현재 fixture는 치명 처형을 요청하므로 이 판정은 일반 처형 전체의 검증이 아니다. 커밋 뒤 중단이 게임플레이상 협업 종료로 허용되더라도 완전한 처형 촬영으로 인정하지 않는다. HUD 이벤트 수집이나 표시 여부에 의존하지 않고 기존 런타임 이벤트를 에디터 전용 observer에서 구독한다. Stop/WorldCleanup에서는 구독을 해제한다.

09/10 `hit`은 플레이어의 콤보 액션이 활성인 동안 해당 fixture가 수락한 전투 패킷에서 실제 확정 피해가 양수인 경우를 최소 한 번 요구한다. 콤보 애니메이션만 끝나거나 무기 충돌이 전달되지 않으면 `HitNotObserved`로 실패한다. 모든 타격의 성공이나 정확히 4타를 완료 조건으로 요구하는 것은 아니다.

일회성 무인 촬영의 `IsolateActors`는 배경 적뿐 아니라 재귀 부착 actor와 직접 소유 actor를 함께 숨기고 충돌을 비활성화한다. 따로 스폰되는 무기를 포함하며, 플레이어와 촬영 fixture는 격리 대상에서 제외한다. 일반 편집 월드·순찰 장면은 저장하지 않는다.

실패는 `<output>.failed.txt`와 에디터 로그에 남는다. 외부 실행자는 새 출력 경로에서 성공 표식이 있고 실패 표식은 없는지 함께 검사해야 한다. 성공 표식에는 실제 내보내기 상태를 포함한다. 실패 표식 쓰기까지 불가능한 파일 시스템 오류는 에디터 로그로 확인한다.

### 수동 중단과 관찰 선택 복원

02는 자신이 넣은 Guard 입력을 기억하고 정상 완료·살아 있는 플레이어의 실패·수동 Stop에서 한 번 해제한다. 기존 Guard 입력이 이미 유지 중이면 해당 입력을 가져오지 않고 실패한다. 사망 시에는 자신이 소유한 Guard 상태만 기존 Defense 정리 API로 해제하며 새 Guard-Out 액션을 요청하지 않는다. WorldCleanup에서는 새 입력이나 상태 변경을 하지 않는다. 살아 있는 플레이어는 Guard-Out 전환이 끝나면서 자세와 이동 제약이 정리되므로 Stop 호출 순간에 몽타주가 모두 종료된다는 의미는 아니다.

10 debug는 자신이 마지막으로 선택한 A/B actor·source·driver가 그대로일 때 이전 관찰값을 복원한다. 뒤에 사용자가 다른 대상을 선택한 경우 덮어쓰지 않는다. 시나리오가 선택한 전투 대상 B도 복원 대상에 포함한다. Freeze/Live를 바꾼 CVar는 기존 값·우선순위를 보존하고, 사용자가 그 뒤 바꾼 값은 유지한다. 이벤트 표시·필터 명령 전체를 일괄 되돌리는 기능은 아니다.

이 도구는 통제된 재현 입력용이다. 진행 도중 동일한 대상·같은 설정값을 사용자가 다시 지정하는 입력까지 별도의 소유권 변경으로 판별하지 않는다. 수동 입력과 자동 시나리오를 동시에 조작하는 전수 조합은 보장하지 않는다.

### 의도적인 무음 촬영

효과음이 없는 장탈착 촬영처럼 무음이 허용되는 작업에만 `-ReplayAllowSilentAudio`를 명시한다. 이 경우 `silent_pcm`도 완료로 허용하지만 해당 상태는 성공 표식에 그대로 남는다. WAV 실패·누락·타임아웃은 이 옵션으로 허용하지 않는다. 무음 촬영의 완료는 효과음 재생이나 오디오 동기화 검증을 뜻하지 않는다.

### 마지막 프레임의 종료 시점

패링의 정상 완료는 마지막 Slate 프레임 제출 뒤에 녹화를 멈춘다. 월드 타이머에서 바로 정지하면 다음 Slate 프레임을 기다리던 요청이 누락될 수 있어, fixture를 유지한 채 PostTick에서 렌더 제출을 한 번 정리한 뒤 기존 RecordStop을 호출한다. 프레임별 동기화나 녹화 백엔드는 변경하지 않는다.

## 재현 조건과 별도 장면 검토

패링의 `Structure02GuardDelay`는 적 공격 시작 관찰부터 Guard 입력까지의 지연(초)이며 기본값은 **0.75초**다. 재현 입력만 바꾸며 공격·Guard 에셋, 판정 코드, 몽타주 재생 속도는 바꾸지 않는다. 다른 공격·장면·프레임 조건에서는 다시 검증한다.

PR #132 검증 당시 공용 RateScale 1.0·적 첫 공격 PlayRate 0.5 기준에서 기존 0.65초 입력은 패링 창의 끝에 너무 가까웠다. Guard-In은 몽타주 0.45초에서 시작하고 0.616667초의 SwitchToGuard에서 패링 창을 닫으므로 약 167ms의 여유가 있다. 적 공격은 실제 0.5배 재생으로 충돌이 약 0.8초 이후에 도착한다. HUD 수집·표시를 켠 20fps 상한 PIE에서는 Guard 입력 200ms 뒤 충돌이 관찰돼 BlockHit와 HP 150 감소가 발생했다. HUD 자체의 전투 상태 변경 결함으로 판단하지 않는다. 기존 실패 프레임도 BlockHit와 HP 4850을 보여준다.

최종 0.75초 기본값에서 실제 PIE 패링을 8회 확인했다: HUD 켬/60fps 상한 3회, HUD 끔/60fps 상한 2회, HUD 켬/30fps 상한 1회, HUD 켬/20fps 상한 2회. 모두 실제 Parry, HP 5000 유지, 적 Balance 1, Guard 해제와 정상 export를 확인했다. 녹화 요청은 30fps이며 게임 FPS 상한은 실제 고정 프레임률을 보장하지 않는다. 동일 최종 코드에 0.65초만 명시한 20fps 대조 실행에서는 다시 실패하고 Guard가 정리됐다.

이 0.75초 값은 공용 RateScale 1.0·적 첫 공격 PlayRate 0.5·당시 임시 적 배치의 재현용 설정이다. 임의의 프레임 정지·저사양·다른 공격·모든 옵션 조합에서 성공을 보장하지 않으며, 실제 Parry를 관찰하지 못하면 기존 실패 판정을 유지한다. 반복 성공을 위해 자동 재시도하거나 실패 표식을 숨기지 않는다.

### G5-C1 적 전용 속도 후보 재현

최종 G5-C1은 공용 `M_Attack_Sword_0`의 RateScale을 기준값 **1.0**으로 유지하고, `BP_CEnemy`의 `ComboAttack` 인덱스 0 **PlayRate만 0.5→0.35**로 변경한다. 공용 몽타주를 0.7로 변경했던 선행 후보는 기존 `BP_CPlayer`에도 영향을 주므로 복원했다. 두 플레이어 BP와 공용 몽타주는 main과 동일하며, 다른 적 액션 데이터도 유지한다.

`Structure02GuardDelay` 기본 0.75초는 적 PlayRate 0.5 기준용이다. **최종 적 후보 0.35에는 실행 인수 `-Structure02GuardDelay=1.07`을 명시한다.** 콘솔 프리셋 CVar나 `warmupSeconds`가 아니다. 이 인수는 자동 Guard 입력 시점만 바꾸며 패링 판정·Guard 창을 변경하지 않는다.

별도 에디터 프로세스 PowerShell 예시다. 경로 자리표시자는 실제 경로로 바꾸고 새 출력 경로를 사용한다. 프리셋은 RecordFPS 30, t.MaxFPS 60, CaptureEnabled/HUDVisible/EventLog.Visible 1이다.

```powershell
& 'C:/Program Files/Epic Games/UE_5.4/Engine/Binaries/Win64/UnrealEditor.exe' `
  '<workspace>/Portfolio.uproject' '/Game/SciFi/SciFiMap' `
  '-Structure02Mode=parry' '-Structure02Output=<new-output>/take' `
  '-Structure02Preset=<preset.txt>' '-Structure02GuardDelay=1.07' `
  '-Structure02Width=1280' '-Structure02Height=720' `
  '-Structure02IsolateActors' '-RenderOffscreen' `
  '-unattended' '-nop4' '-nosplash' '-NoLiveCoding' '-NoAutoSave'
```

2026-10-23 최종 적 전용 후보에서 기존 재현 도구를 실행했다. 아래는 분리본 검증 기록이며 PR 게시·병합 상태와 구분한다.

| 적 첫 공격 데이터 | GuardDelay | 조건 | 실제 PIE 결과 |
|---|---:|---|---|
| 기준 PlayRate 0.5 | 0.75초 | 정상 | 실제 Parry·완료·정상 export |
| 기준 PlayRate 0.5 | 0.75초 | 단일 120ms | Parry 미관찰·실패 판정·성공 마커 없음 |
| 기준 PlayRate 0.5 | 0.75초 | 단일 200ms | Parry 미관찰·실패 판정·성공 마커 없음 |
| 후보 PlayRate 0.35 | 1.07초 | 정상 | 실제 Parry·완료·정상 export |
| 후보 PlayRate 0.35 | 1.07초 | 단일 120ms | 실제 Parry·완료·정상 export |
| 후보 PlayRate 0.35 | 1.07초 | 단일 200ms | Parry 미관찰·실패 판정·성공 마커 없음 |
| 후보 PlayRate 0.35 | 기본 0.75초 | 정상 실패 대조 | Parry 미관찰·실패 판정·성공 마커 없음 |

성공은 `VERIFIED_REAL_PARRY`, HP 5000·Balance 1, `TAKE_COMPLETE`, `<output>.succeeded.txt`의 `scenario=complete`·`export=verified`로 확인한다. 종료 코드 0만으로 대신하지 않는다. 실패 조건에 자동 재시도·입력 보정·판정창 확대를 적용하지 않았다.

패링 지연은 첫 몽타주 위치 0.38 이상·0.4 미만을 관찰한 뒤 단일 시뮬레이션 간격을 120ms/200ms로 주고 기존 시간 설정으로 복구한 시험이다. 정상 실행은 실제 프레임 간격을 사용한다. 이 임시 지연 도구는 제품에서 제거했다. 측정한 지점·배치·입력값의 결과이며 다른 프레임 정지 위치나 모든 FPS 조합의 보장은 아니다.

### 콤보 충돌창과 플레이어 불변성

콤보 전환 전에 이전 충돌창을 닫고, 종료 콜백에서 액션이 취소·교체되면 이전 전환을 중단한다. 지연된 이전 인덱스 NotifyEnd 차단과 같은 창의 중복 타격 방어를 유지한다.

공용 RateScale 1.0을 유지한 채 적 데이터 PlayRate 0.5·0.35 각각에서 정상 1/60초 시뮬레이션, 첫 몽타주 위치 0.515 이상 첫 샘플 뒤 단일 120ms·200ms를 비교했다. 여섯 실제 BT/PIE 조건 모두 창 ID 0·1·2, 확정 피해 300·500·700, 최종 HP 3500이었다. 고정 시뮬레이션 간격은 벽시계 60fps 보장이 아니다.

두 적 데이터 구성에서 `BP_CPlayer`와 `BP_CPlayer_Stellar`를 각각 실제 PIE에 생성하여 기본 입력 API로 첫 공격을 실행했다. 기존 BP_CPlayer는 PlayRate 1.0 × RateScale 1.0, BP_CPlayer_Stellar는 PlayRate 1.0 × RateScale 0.8을 유지했고, 몽타주 진행·충돌창·완료 기록이 동일했다. 에셋 해시도 main과 일치한다. 이는 이번 적 속도 변경에 대한 불변성 검사이며 모든 플레이어 행동의 전수 시험은 아니다.

두 구성에서 기본 Unity 빌드·전체 자동화 102개·에셋 감사가 통과했다. 회귀 테스트 `Portfolio.Combat.ComboCollision.Transition`과 `Portfolio.Combat.ComboCollision.ReentrantClose`는 충돌창·전환·취소·실행 교체 경계를 검사한다. 실제 명중은 별도 BT/PIE로 확인했으며 Capture09/10 완료 판정을 명중 횟수 판정으로 바꾸지 않았다.

### G5-C1 커밋 경계 재검증

2026-10-23 C1(충돌창 코드·테스트 3개, 기준 적 PlayRate 0.5)과 C2(적 BP 추가, PlayRate 0.35)를 각각 실제 파일 경계로 구성했다. 두 경계 모두 기본 Unity/Adaptive Unity Editor 빌드·전체 자동화 102개(일반 100, 기존 경고 동반 2)·CDodgeAssets Verify·몽타주 감사 오류 0을 확인했다. 임시 PIE 관찰 도구를 제거한 뒤 각 경계의 기본 빌드도 다시 통과했다. 같은 102개 테스트의 반복 실행이다.

각 경계에서 실제 BT 콤보의 정상·120ms·200ms 조건을 다시 실행했고 모두 창 ID 0·1·2, 피해 300·500·700, 최종 HP 3500이었다. 두 플레이어의 첫 공격 기록도 경계 간 동등했다. 콤보와 패링의 지연 적용 위치·시간 조건은 위 선행 검증과 동일한 절차를 사용했다.

| 경계·조건 | GuardDelay | 실제 PIE 결과 |
|---|---:|---|
| C1-parry-normal | 0.75초 | 실제 Parry·완료·정상 export |
| C1-parry-hitch120 | 0.75초 | Parry 미관찰·실패 판정·성공 마커 없음 |
| C1-parry-hitch200 | 0.75초 | Parry 미관찰·실패 판정·성공 마커 없음 |
| C2-parry-normal | 1.07초 | 실제 Parry·완료·정상 export |
| C2-parry-hitch120 | 1.07초 | 실제 Parry·완료·정상 export |
| C2-parry-hitch200 | 1.07초 | Parry 미관찰·실패 판정·성공 마커 없음 |
| C2-parry-default075 | 0.75초 | Parry 미관찰·실패 판정·성공 마커 없음 |

위 표는 이번 경계별 각 조건 1회 기록이다. 선행 표의 실패·성공 이력을 대체하지 않는다. 실제 프레임 간격과 지연 진입 위치에는 실행별 차이가 있으므로 단일 성공을 지연 내성 개선으로 단정하지 않는다. 실패 원인의 완전한 분리, 모든 지연 위치·FPS·다수 적·장시간 플레이는 미검증이다. 패링 판정창·기본 입력값을 수정하거나 실패를 숨기기 위한 재시도를 하지 않았다.

C3는 이 결과를 반영하는 문서 2개만 더한다. 실행 코드·에셋은 C2와 동일하므로 실제 PIE 근거는 C2 결과를 적용하며, 문서 경계에서 빌드·자동화·에셋·문서 링크를 별도로 확인한다. 커밋·PR 게시·병합 완료를 뜻하지 않는다.

기존 SciFiMap에서는 순찰 경로 전제조건이 충족되지 않아 `NoPatrolPath` 실패를 확인했다. 공용 맵은 유지하며, 순찰에는 별도의 `/Game/00_UnitTest/PatrolReplay/L_PatrolReplay`를 사용한다. 장면 구성·실행 인자·실제 Loop/Reverse BT 이동 검증은 [순찰 전용 재현 장면](N_Patrol_Replay_Scene.md)을 따른다. 현재 `patrol` 촬영기는 32초 경과로 종료하므로 성공 표식만으로 지점 도착을 입증하지 않는다. 순찰→전투→수색→복귀 전체 전환은 이 장면의 순찰 이동 검증과 구분한다.

## 검증 범위

`Portfolio.Capture.Replay.CompletionContract`는 정상·중단·드롭·타임아웃·오디오 정책·메타데이터 불일치·누락·중복 필드·프레임 집계를 검사한다. 전체 자동화와 에셋 검사 외에 실제 PIE에서 정상 시나리오와 의도적으로 만든 녹화 실패를 구분해 기록한다. 기존 D1/P2-A/G3/G4/P2-B1 구현은 main의 상태로 유지한다.

`Portfolio.Capture.Replay.ScenarioGoal`은 장착/해제 관찰 누락, 콤보 미시작·1타/2타 종료와 정상 2타/3타 목표를 구분한다. 실제 장탈착 요청 거부, 수동 Guard 중단, A/B 관찰 복원과 사용자 선택 보존은 별도의 실제 PIE 검사로 확인한다. 빌드·단위 테스트만으로 수동 종료의 런타임 동작을 검증했다고 표시하지 않는다.

JPEG 디코딩·WAV PCM·메타데이터 검사는 파일 유효성 근거다. 연속 영상 감상, 모든 모드·옵션 조합, 수동 입력, 청취 및 전투 이벤트별 시청각 동기화까지 검증했다는 의미는 아니다.

### PR #132 P2 리뷰 보완 검증

09/10 공통 완료 판정과 배경 무기 격리를 보완했다. 에디터 모듈의 observer만 추가했고 게임플레이 런타임·몽타주·Blueprint·맵과 기존 02 패링 입력은 유지했다.

- 기본 Unity와 Adaptive Unity를 끈 전체 Unity 빌드 통과. 전체 자동화 98개(일반 성공 96 + 기존 경고 2), 회피 에셋·처형 몽타주 검사 통과.
- 실제 UE 5.4.4 PIE 12조건: 09/10 각각 정상 처형·정상 피격, 커밋 전 대상 해제·액션 중단, 커밋 후 액션 중단, 무타격. 정상 4조건은 성공 export, 나머지 8조건은 실패 표식만 생성되고 TAKE_COMPLETE가 없었다.
- 정상 처형은 동일 세션의 커밋·양쪽 정상 완료, 정상 피격은 실제 확정 피해 패킷 4회를 각각 확인했다. 무타격은 임시 표적의 충돌만 비활성화한 검증 조건이다.
- 12조건에서 배경 적 소유 무기 1개가 숨김/충돌 비활성이고 플레이어·fixture 무기는 숨겨지지 않았음을 확인했다. 종료 뒤 임시 표적 제거·액션과 협업 종료·이전 전투 대상 복원을 함께 검사했다.
- HUD 수집·표시·이벤트 로그는 기본 켬, 커밋 전 액션 중단 2조건에서는 모두 끔으로 검사했다. 녹화 요청 30fps, 게임 상한 60fps, 1280×720의 별도 에디터 PIE이며 고정 프레임 실행 보증은 아니다.
- 결함 주입과 상태 확인에 사용한 임시 PIE driver는 최종 빌드·커밋에서 제외했다. 정상/실패 조건 판정과 녹화 파일 검사는 연속 시청·청취 검증과 구분한다.

### 2026-10-25 패링 재현 실패 진단 보강

기준은 main `59bfac10`이며 에디터 재현 도구의 관찰·실패 설명과 완료 조건을 보강했다. 무기 충돌, 패링 판정, 공격·Guard 에셋, 몽타주 속도, 기본 GuardDelay 0.75초는 유지한다. 현재 적 첫 공격 PlayRate 0.35의 비교 실행은 `-Structure02GuardDelay=1.07`을 명시한다. 기본값의 조용한 변경이나 자동 재시도·입력 보정은 하지 않는다.

관찰 대상은 이번 fixture가 시작한 **첫 ComboAttack 인덱스 0의 action generation**이다. 같은 적의 후속 인덱스나 다음 공격, 다른 적·무기의 결과는 첫 타 결과에 합산하지 않는다. 기존 `OnWeaponActorBeginOverlap` 이벤트에서 판정 직전 `CanParry()`를 읽고, 같은 프레임·무기 충돌창의 `OnCombatSignalTargetAccepted` 패킷과 대조한다. 에디터 observer는 읽기 전용이며 종료·수동 중단·월드 정리 때 구독을 해제한다.

| 진단 | 의미와 해석 범위 |
|---|---|
| `NoContact` | 첫 공격의 무기 충돌창 시작·종료를 관찰했으나 fixture 무기→플레이어 접촉 이벤트가 없었다. 패링 판정 거부의 증거가 아니다. |
| `ContactOutsideParryWindow` | 접촉 직전 패링 창이 닫혀 있었고, 일반 방어 정책의 패링 가능한 피해가 실제로 비패링 처리됐다. |
| `GateMismatch` | 일반 정책·생존 대상·패링 가능한 피해에서 접촉 직전 창과 실제 Parry 결과가 모순된다. 원인 확정이 아닌 추가 조사 대상이다. |
| `ParryConfirmed` | 같은 첫 공격의 접촉 직전 창 열림과 실제 Parry 패킷이 일치한다. 촬영 완료·export는 별도로 검사한다. |
| `ContactUnresolved` | 접촉은 있으나 대응하는 accepted 패킷을 확보하지 못했다. 접촉 없음이나 판정 버그로 단정하지 않는다. |
| `OtherDefensePolicy` | 패링 불가 피해, 특수 입력 정책, 회피 등 별도 방어 정책이 개입했다. |
| `IncompleteObservation` | 첫 공격·충돌창 종료·적절한 시점의 무기 구독을 확인하지 못했다. 미완료를 빗나감으로 판정하지 않는다. |

로그의 `[ParryDiagnostic]`에는 공격 generation·인덱스·몽타주 인스턴스·실제 PlayRate/RateScale과 접촉 시각·패링 상태를 기록한다. 최종 `DIAGNOSTIC`과 성공/실패 표식의 `Diagnostic=`에는 분류·Guard 입력 시각·입력 지연·접촉 시각·충돌창 번호가 남는다. `WindowOpenedAt`/`WindowClosedAt`은 **무기 충돌창** 시각이다. 패링 창의 모든 전환 시각을 계측한 필드가 아니다. `-1`은 미관찰 시각/충돌창이다. `ParryOpenAtContact`는 `Contact=1`일 때만 유효하며, 접촉이 없을 때 기본값 0을 창 닫힘의 증거로 해석하지 않는다.

실패 표식은 기존 실패 이유를 유지한 채 진단을 추가한다. 녹화 초기화/수동 중단 등의 실패 이유를 전투 진단으로 대체하지 않는다. 성공은 기존 실제 Parry 리액션에 더해 완전한 관찰에서 지정한 첫 공격의 접촉 직전 창 열림·일반 정책·실제 Parry 결과가 일치해야 하며, 정상 export까지 통과해야 한다. `GateMismatch`, `OtherDefensePolicy`, 불완전 관찰과 미확정 접촉은 성공 완료로 진행하지 않는다. 후속 콤보의 성공으로 첫 타 실패를 덮어쓰지 않는다.

UE 5.4.4 실제 SciFiMap PIE, 기존 transient fixture·실제 적 BT/무기 충돌, HUD 수집·표시·이벤트 로그 켬, 녹화 요청 30fps·게임 상한 60fps 조건이다. 아래 표는 R1·R2 보완 전 정상·120·200ms 각 2회와 0.75초 대조 1회의 기록이다. 보완 후 재검증은 다음 절에 별도로 기록한다.

| 조건 | GuardDelay | 시나리오 성공 | 첫 공격 진단 |
|---|---:|---:|---|
| 정상 | 1.07초 | 2/2 | ParryConfirmed |
| 120ms 단일 지연 | 1.07초 | 2/2 | ParryConfirmed |
| 200ms 단일 지연 | 1.07초 | 0/2 | NoContact |
| 정상 프레임·기존 입력 대조 | 0.75초 | 0/1 | ContactOutsideParryWindow |

200ms 조건은 기존 첫 접촉 누락과 실패를 유지했다. 이후 콤보의 Guard/BlockHit는 별개이며 `NoContact`를 덮어쓰지 않았다. 0.75초 대조의 실제 접촉은 닫힌 패링 창에서 발생해 `ContactOutsideParryWindow`로 분류됐다. 따라서 진단 개선의 통과를 게임플레이 지연 문제가 해결됐다는 뜻으로 쓰지 않는다.

R1·R2 보완 전에는 기본 Unity 설정의 Development Editor 빌드, 전체 자동 테스트 107개(일반 성공 105·경고 포함 성공 2·실패 0), CDodgeAssets 검사(오류·경고 0)가 통과했다. 추가 `Portfolio.Capture.Replay.ParryDiagnostic` 테스트는 미완료·접촉 없음·미해결 접촉·별도 정책·창 밖 접촉·양방향 판정 불일치·성공과 generation/후속 인덱스 분리를 확인한다. **실제 PIE에서 GateMismatch는 발생하지 않았다.** 그 분류는 합성 관찰 입력을 사용한 자동 테스트로 검증했으며 실제 판정 결함 재현으로 표시하지 않는다.

지연 주입은 저장소 외부의 기존 진단 플러그인으로 적 첫 몽타주 0.38~0.4초 구간 뒤 한 프레임의 게임 델타를 120/200ms로 바꾼 뒤 복구했다. 제품 기능이 아니고 네트워크 지연·실제 OS/GPU 정지와 같지 않다. 모든 프레임 지점·FPS·다수 적·수동 입력·장시간 플레이는 미검증이다. 이번에는 Stellar 공격을 재실행하지 않았으며 기존 지연 첫 타 빗나감과 패링 실패 이력을 유지한다. 영상 연속 감상·청취 검증은 수행하지 않았다.

이 절은 커밋 전 분리본에서 수행한 검증 기록이다. 검증 시점의 상태와 이후 커밋·PR 게시·병합 상태를 구분한다.


#### 2026-10-25 R1·R2 완료 조건·재접촉 보완과 재검증

- R1: harness는 observer의 단일 `CanComplete()` 조회로 완료 조건을 확인한다. 첫 인덱스 0, 유효 generation, 접촉·승인 결과, 일반 방어 정책, 열린 패링 창과 Parry 일치, 유효한 구독/대상 참조가 모두 필요하다. 불완전 구독이나 양방향 `GateMismatch`는 실제 Parry 리액션이 있어도 성공 조건이 아니다. 기존 리액션·export 조건은 유지한다.
- R2: 접촉이 있었다는 기록은 유지하고 첫 승인 결과 전까지 접촉 프레임·충돌창·창 열림 상태·시각을 갱신한다. 같은 공격의 이후 재접촉을 최신 pending 스냅샷과 연결한다. 다른 generation/인덱스/프레임/충돌창의 결과는 거부하고, 승인된 결과는 중복 패킷·후속 접촉으로 덮어쓰지 않는다. 기존 source/target/weapon 필터도 유지한다.
- 기본 Unity Development Editor 빌드 통과. 전체 자동 테스트 109개(일반 성공 107·경고 포함 성공 2·실패 0), CDodgeAssets 오류·경고 0. 추가한 `Portfolio.Capture.Replay.ParryCompletion`과 `ParryRecontact`가 통과했다.
- 완료 회귀는 정상 승인, 불완전 관찰, 양방향 불일치, 별도 정책, 무접촉·미확정 접촉·다른 인덱스/미식별 공격을 검사했다. 재접촉 회귀는 거부 또는 콜백 누락 뒤 다음 프레임 접촉, 최신 창 상태, 다른 generation/인덱스/프레임/충돌창 배제, 승인 후 중복·후속 접촉 동결을 검사했다. 실제 observer가 호출하는 상태 전이 함수를 합성 이벤트 순서로 실행한 자동 테스트다. UObject delegate 전체 경로의 결함 주입 PIE로 표시하지 않는다.
- 보완 후 실제 SciFiMap PIE 7회: 정상 1.07초 2/2, 120ms 1.07초 2/2는 `ParryConfirmed`와 정상 export. 200ms 1.07초 0/2는 `NoContact`, 기존 0.75초 정상 대조 0/1은 `ContactOutsideParryWindow`와 실패 표식만 생성됐다. 실제 target accepted 로그와 첫 공격 진단을 대조했고, 전 조건에서 소유 Guard 입력 해제를 확인했다.
- 실제 PIE에서 `GateMismatch` 또는 미승인 뒤 승인 재접촉 경계를 강제로 만들지는 않았다. 두 경계는 위 자동 회귀 테스트 근거이며, PIE는 기존 정상·지연 절차의 회귀 확인이다. 기존 200ms 패링 실패·Stellar 지연 첫 타 빗나감·델타 주입 방식·모든 FPS/지연 시점 미검증·영상 연속 시청/청취 미검증은 그대로다.

무기 충돌·패링 판정·게임플레이 런타임·에셋·Config는 수정하지 않았다. 이 절의 빌드·테스트·PIE는 커밋 전 보완본에서 실행했다. P79의 C1 구현·테스트와 C2 문서 구성으로 정리하며, 게시·리뷰·병합 상태는 `fix/parry-replay-diagnostics` 브랜치의 GitHub PR을 기준으로 확인한다.
