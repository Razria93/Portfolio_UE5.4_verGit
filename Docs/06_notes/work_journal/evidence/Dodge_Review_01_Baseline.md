# Dodge Review 01 — 기준점·재현·검증

## 식별과 범위

- 기준 태그: `dodge-review-01` (annotated tag, 최종 문서 커밋 대상).
- 브랜치: `feat/combat-dodge`. 작업 시작 HEAD: `35891fca`.
- 코드·에셋은 기존 작업 트리 상태를 보존한다. 이번 기준점 작업에서 연출 개선이나 게임플레이 코드를 수정하지 않는다.
- 최종 SHA 조회: `git rev-parse "dodge-review-01^{commit}"`.
- 로컬 커밋/태그까지만 수행한다. push, PR, merge, 원격 LFS 업로드는 수행하지 않는다.
- [협업 검토 기록](../J02_Dodge_Perfect_CounterRush_AI_Review.md), [구현 명세](../../N_Dodge_Perfect_CounterRush_Implementation.md), [최초 요청](Dodge_Original_Request.md).

## 환경과 재현 준비

- Windows, Unreal Engine 5.4, PortfolioEditor Win64 Development, `Portfolio.uproject`.
- 설정상 EditorStartupMap: `/Game/00_UnitTest/TestRoom.TestRoom`.
- GameDefaultMap은 `/Engine/Maps/Templates/OpenWorld`, GlobalDefaultGameMode는 엔진 기본 GameMode다. 실행만으로 Stellar 전투 장면이 보장되지 않는다.
- 확인 대상 플레이어: `/Game/01_Character/01_Player/BP_CPlayer_Stellar`. 기본 BP_CPlayer는 회피/Perfect 지원, 러시 데이터는 Stellar 우선이다.
- 입력 설정: `LeftShift` Dodge, `LeftMouseButton` ComboAction(조건부 CounterRush), `2` Sword. 실제 PIE 입력 활성/포커스와 빙의 Pawn을 확인한다.
- 촬영 맵의 실제 GameMode override, 빙의 Pawn, 적 공격 종류/배치/무기, 카메라 위치는 **아직 수동 확정하지 않았다**. TestRoom 경로를 촬영 완료 환경으로 오인하지 않는다.
- 동일 공격을 반복하는 적, 평지, 벽이 있는 촬영용 맵을 선택하고 아래 촬영 표에 실제 경로/설정을 기록한다. 필요한 맵 수정은 별도 준비 커밋으로 보존하며 기준 태그를 이동하지 않는다.

## 이번 실행 결과 — 2026-09-22 KST

| 검사 | 결과 | 근거와 한계 |
| --- | --- | --- |
| Editor Development 빌드 | 통과, exit 0, Target is up to date | 증분 빌드의 최신 여부 확인이며 clean rebuild는 아님 |
| Portfolio 자동화 | 50/50 Success, 프로세스 exit 0 | NullRHI, `DodgeReview01Tests.log`, Queue Empty 50 tests performed |
| 저장 에셋 검증 | Default/Stellar Verified persisted data, exit 0 | `CDodgeAssets`를 Apply 없이 실행. 0 errors / 0 warnings |
| 소스 diff 공백 검사 | 통과 | `git diff --check -- Source` |
| 이번 변경 에셋 LFS 무결성 | 10/10 일치 | 커밋 포인터의 SHA256과 로컬 LFS 객체 및 작업 파일을 각각 비교. 기존 전체 프로젝트 LFS/원격은 검사 범위 아님 |
| 실제 PIE·렌더링·전후 영상 | 미실시 | 시각 품질, 지형, 실제 BP/AnimBP 전체 경로 승인은 아님 |

최초 샌드박스 빌드는 사용자 UBT 로그 접근 권한 때문에 실패했다. 승인된 실행 환경에서 재실행해 통과했으며 소스 컴파일 오류는 아니었다. 전체 Git diff/LFS 필터도 승인된 실행에서 정상 조회했다.

과거 2026-09-21의 `DodgeVerifiedTests.log`, `DodgeFinalAssets.log`와 이번 결과를 구분한다. 이번 로그는 기본적으로 Saved 아래 로컬 산출물이다. 로그 파일 자체를 저장소에 넣지 않으므로 아래 해시와 발췌를 보존한다. 해시는 파일 보관을 대체하지 않으며 장기 증거가 필요하면 원본 로그를 별도 보관한다.

| 로컬 로그 | SHA256 |
| --- | --- |
| `Saved/Logs/DodgeReview01Tests.log` | `DFD08C5430FB906595CE2DA922D6DFD002DBD68A1AA5AF4C37ECAC230918F3D8` |
| `Saved/Logs/DodgeReview01Assets.log` | `E8228A8871ACD1F792C114899C2805F4374A8B7D6928AE5DB8AAFE4567B4997C` |

로그 발췌 (UTC):

```text
[2026.09.22-05.21.06:440] Automation Test Queue Empty 50 tests performed.
[2026.09.22-05.21.58:971] DodgeAssets Default: Verified persisted data
[2026.09.22-05.21.59:460] DodgeAssets Stellar: Verified persisted data
[2026.09.22-05.21.59:460] Success - 0 error(s), 0 warning(s)
```

재실행 명령 (`<Project>`는 Portfolio.uproject 절대 경로):

```text
Build.bat PortfolioEditor Win64 Development -Project=<Project> -WaitMutex -NoHotReloadFromIDE
UnrealEditor-Cmd.exe <Project> -unattended -nop4 -nosplash -NullRHI -nosound -ExecCmds="Automation RunTests Portfolio." -TestExit="Automation Test Queue Empty" -AbsLog=<UniqueTestLog>
UnrealEditor-Cmd.exe <Project> -run=CDodgeAssets -unattended -nop4 -nosplash -NullRHI -nosound -AbsLog=<UniqueAssetLog>
```

검증 시 `-Apply`를 넣지 않는다. 각 재실행에는 다른 로그명을 사용해 이번 증거를 덮어쓰지 않는다. 현재 에셋 검증기는 선택된 참조와 속성만 검사하며 모든 모션/머티리얼 품질을 검증하지 않는다.

## 커밋 분류와 에셋 보존

1. 전투 코드·실행/Notify 소유권·초기 Feedback·관련 테스트: 새 타입과 시작 훅이 상호 의존하므로 같은 커밋.
2. HUD 상태 표시와 테스트: 전투 코드 다음의 별도 커밋.
3. Editor 에셋 제작/검증 도구: 별도 커밋.
4. 현재 연결 BP·Perfect/Rush 몽타주·루트 고정 시퀀스·오버레이·트레일: 별도 커밋.
5. Lucy 회피/Perfect 모션 후보 두 개: 현재 작업 자료 보존용 별도 커밋. Content 문자열 참조 검색에서 자기 파일 외 사용처가 확인되지 않았으며 Asset Registry 전체 참조 검증은 아니다. 기존 실행 모션으로 교체하지 않는다.
6. 문서와 인덱스: 마지막 커밋, 기준 태그 대상.

| 커밋 | 범위 |
| --- | --- |
| `0e5d98d4` | 전투·실행/Notify 소유권·초기 Feedback·관련 테스트 |
| `2fe38ef8` | HUD와 관련 테스트 |
| `e4dea3bc` | Editor 에셋 제작/검증 도구 |
| `17ed9861` | 현재 연결 에셋 |
| `aca95fed` | Lucy 모션 후보 별도 보존 |
| `dodge-review-01^{commit}` | 이 문서를 포함한 마지막 문서 커밋 |

분리된 각 중간 커밋을 독립 worktree에서 빌드하지는 않았다. 이번 빌드·테스트는 분리 전과 동일한 최종 소스/에셋 내용에 대한 통합 검증이며, 에셋 의존 테스트는 에셋 커밋 이전 시점의 기능 완료를 뜻하지 않는다.

`FCombatKnockbackDebug.cpp`의 기존 수정은 CVar의 외부 이름이 아닌 내부 변수 이름에 Knockback 접두사를 추가한 것이다. 이름 충돌 위험을 줄이는 변경으로 전투 기반 커밋에 포함하되, 당시 수정 원인은 별도 증거 없이 단정하지 않는다. 공용 SciFiSword 트레일에는 CounterRush_Trail 연결이 있어 활성 에셋 커밋에 포함한다.

`.uasset`은 Git LFS 대상이다. 새 머신에서는 태그와 필요한 LFS 객체를 모두 확보해야 한다. 로컬 객체 검증은 원격 업로드 완료를 뜻하지 않는다. 태그를 추후 이동하지 않으며, 비교 실행은 진행 중 작업을 덮어쓰지 않는 별도 worktree를 권장한다.

## 촬영 및 수동 검증 표

공통 기록: 태그/SHA, 촬영용 맵 준비 커밋(있는 경우), 맵·GameMode·Pawn·적·무기 경로, 공격 종류, 거리, PlayRate, 시간 배율, 해상도/FPS/그래픽 설정, 카메라, 파일명과 타임코드. 아래 항목은 모두 미촬영·수동 미검증이다.

| ID | 재현 | 비교 관찰 |
| --- | --- | --- |
| V01 | 공격 범위 밖 회피, 일반 무적 창 안 접촉 | 헛회피에서 성공 연출 없음; 일반 회피와 Perfect 구분 |
| V02 | 저스트 성공 후 입력하지 않음 | 몸·비네트·슬로우 종료와 반격 기회 만료. 개선 후 무기 페이드 비교 |
| V03 | 저스트 성공 후 연결 시점 이후 공격 입력 | 유한 돌진+단발 공격. 개선 후 몸과 무기 종료 시점 차이 |
| V04 | 가까운 적/최대 거리 근처의 적에 러시 | 고정 간격과 캡슐 기반 개선 비교, 실제 이동 거리 |
| V05 | 벽을 사이에 두거나 벽 가까이에서 러시 | 충돌 정지, 후속 미끄러짐, 목표/실제 디버그 |
| V06 | 효과 도중 중단·사망·재실행 | 배율/오버레이/비네트 복원; 개선 후 무기 교체도 확인 |

초기 상태의 제한: 대표 메시 하나의 임시 오버레이, 무기 발광 없음, 슬로우 보간/시작 Notify 없음, 비네트 강도 0.85 순간 적용, 중심 간 StopDistance, 러시 전용 이동 디버그 없음. 이 제한은 이번에 수정하지 않는다.
