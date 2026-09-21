# 넉백·공격 시작 방향 보정 통합 검토 가이드

## 1. 기준과 열람 순서

- 기준일: 2026-09-21. 브랜치: `feat/combat-knockback`, 분기 기준: `d4012433`.
- 소스 검토 완료 커밋: `b0291c0e` — 20개 파일. 테스트: `361aeeb2` — 4개 파일. 플레이어 BP: `4a1487a3` — 1개 파일.
- 이 문서는 통합 탐색·재현·후속 점검용이다. 세부 구현의 중복 설명은 아래 두 문서로 위임한다. PR/머지 완료 보고가 아니다.

| 알고 싶은 내용 | 읽을 문서 |
| --- | --- |
| 작업 범위, 코드 열람 순서, 검증 재현, 남은 확인 | 이 문서 |
| 피해별 데이터 전달, 넉백 시작·소유권·정리 | [35. 일반 Hit 지상 넉백](35_Gameplay_Combat_Knockback_Implementation.md) |
| 콤보별 방향 보정, Serial 0 문제, Tick 경계 | [36. 공격 시작 방향 보정](36_Gameplay_Attack_Start_Facing.md) |
| 기존 피해 판정과 Feedback의 책임 | [S38. Combat Signal](../../05_System_Architecture/S38_UE5_Portfolio_Combat_Signal_Architecture.md) |
| 기존 실행 판정·개입 | [S37. Execution Transition](../../05_System_Architecture/S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md) |
| 타깃 상태의 책임 | [S33. Combat Target](../../05_System_Architecture/S33_UE5_Portfolio_System_Architecture.md) |

## 2. 프로젝트 조사 범위

`Source/Portfolio`, `Source/PortfolioEditor`, Docs의 인덱스·기존 아키텍처·제작 기록을 대상으로 관련 심볼과 호출 경로를 검색했다. 브랜치에서 바뀐 런타임 파일 전부와 테스트, Player/Enemy 참조 구성, 실행 시작·종료, 타깃·카메라·AI Facing, Balance/처형, HitStop 경계를 대조했다.

이는 관련 변경·연결 경로의 전수 점검이다. 프로젝트의 모든 무관한 소스 줄, 모든 Blueprint 그래프, 모든 레벨 인스턴스를 완전 검증했다는 뜻은 아니다. 바이너리 애셋 diff는 LFS 포인터 변경만 보여 주므로 실제 속성 편집량으로 해석하지 않는다.

문서 조사 결과 기존 35/36은 개별 구현 설명은 갖췄지만 공통 진입 링크, 최종 커밋 범위, 최신 검증 재현과 코드 탐색 순서가 부족했다. 그래서 이 가이드만 추가하고 새로운 중복 아키텍처 문서나 아직 생성하지 않은 PR 기록은 만들지 않았다.

## 3. 파일별 코드 열람 지도

경로는 저장소 루트 기준이다. 줄 번호 대신 API명을 기준으로 찾으면 후속 배치 변경에도 추적할 수 있다.

| 순서 | 파일 | 읽을 대상 / 확인할 계약 |
| --- | --- | --- |
| 1 | `Source/Portfolio/Type/CCombatKnockbackTypes.h` | Spec.Speed/Duration, Context.Direction, 유한값·수평 정규화 조건 |
| 2 | `Type/CCombatDamageTypes.h`, `Type/CCombatSignalTargetTypes.h` | 공격별 설정과 대상이 확정한 실행 정보의 분리 |
| 3 | `Component/CCombatSignalTargetComponent.h/.cpp` | ResolveDamageKnockback: 피해 Commit·Outcome 확정 뒤 일반 Hit/Normal/지상 조건과 방향만 계산 |
| 4 | `Type/CReactionOrchestrationTypes.h`, `Type/CReactionDataTypes.h`, `Component/CReactionOrchestratorComponent.cpp` | Context → Candidate → ExecutionContext 복사, 조회만으로 이동하지 않음 |
| 5 | `Component/CReactionComponent.h/.cpp` | StartReaction, IsActiveReactionGeneration, StopReactionKnockback, ClearActiveReactionContext: 성공한 현재 실행에만 이동 연결 |
| 6 | `Component/CMovementComponent.h/.cpp` | Start/StopKnockback, CanApply/Update/ClearKnockback: 실제 RMS 이동과 자기 소스만 정리 |
| 7 | `Type/CCharacterComponentReferenceTypes.h`, `Character/Player/CPlayer.cpp`, `Character/Enemy/CEnemy.cpp` | BuildReferences에서 CharacterMovement를 구해 Movement로 직접 전달 |
| 8 | `Type/CActionDataTypes.h` | bFaceCombatTargetOnStart, 거리·각도 기본값과 공격별 설정 |
| 9 | `Action/CAction.cpp`, `Action/CAction_ComboAttack.cpp` | Start/ConsumeChain의 실제 시작 성공 및 이전 실행 재진입 방어 |
| 10 | `Component/CActionComponent.h/.cpp` | 내부 세대, HandleApplyActionStarted, ApplyPendingStartFacing, ClearPendingStartFacing |
| 11 | `Component/CMovementComponent.cpp` | TryFaceTarget: 지상·수평 거리·허용 각도 검사 후 Yaw만 한 번 변경 |
| 12 | `Source/PortfolioEditor/Private/Tests/CCombatKnockbackTests.cpp`, `CActionFacingTests.cpp` 및 각 Probe 헤더 | 대역 기반 생명주기 검사와 실제 CharacterMovement/애니메이션 이동 검사의 구분 |

2~11행의 생략된 접두 경로는 `Source/Portfolio/`다. 각 Probe는 Editor 모듈의 transient 테스트 객체이며 게임플레이 애셋을 생성하지 않는다.

## 4. 두 기능이 만나는 경계

```text
공격 시작 성공 → Action에 현재 실행/타깃 예약
  → 다음 Action Tick → Movement.TryFaceTarget → CharacterMovement 이동

피해 수신 → Target의 Hit/수평 방향 확정 → Reaction 실제 시작 성공
  → Movement.StartKnockback → CharacterMovement 이동
  → 실행 종료 또는 적용 조건 상실 → 소유 넉백 정리
```

- Action과 Movement는 각각 CharacterMovement의 선행 Tick으로 등록된다. 두 컴포넌트 상호 간의 고정 순서를 새로 지정한 것은 아니다.
- 공격 방향은 단계 시작 시 보정하며, 넉백 방향은 피격 판정 때 확정한다. 연속 자동 추적·공격 거리 보정·적에게 붙여 주는 기능은 없다.
- `CTargetLockAssistComponent`는 ControlRotation을, `CEnemyCombatTargetFacingComponent`는 AI Focus와 기존 이동 회전 모드를 관리한다. 새 시작 보정이 이 책임을 인수하지 않는다.
- `CExecutionCollaborationComponent::AlignTargetExecutionFacing`은 기존 처형 전용 정렬이다. 일반 ComboAttack 보정과 통합하지 않았다.
- `CHitFeedbackComponent` → `CWorldSubsystem_CombatFeedback`의 HitStop은 Actor.CustomTimeDilation 및 복구 타이머를 사용한다. 넉백 이동 책임을 Feedback에 넣지 않았다. 조합 체감은 별도 PIE 항목이다.
- C++ 기본 넉백 Speed/Duration은 0, 방향 보정 활성값은 false다. 기능 코드가 존재한다고 모든 공격 데이터에서 활성화된 것은 아니다.

## 5. 설정과 문제 추적

| 현상 | 우선 확인할 곳 |
| --- | --- |
| 전혀 밀리지 않음 | 피격 대상이 아니라 공격자 DamageSpec의 Knockback 속도·시간, 실제 Outcome이 Hit인지, Hit 실행 성공 여부 |
| 일부 Hit만 밀리지 않음 | 지상/생존/Balance 제한, Hit 몽타주 및 현재 블렌드의 애니메이션 Root Motion |
| 콤보 후속타가 적을 보지 않음 | 각 ComboAttack 데이터의 활성값·거리·각도, 예약이 아니라 실제 ConsumeChain이 진행됐는지 |
| 첫 방향 보정 뒤 다시 회전함 | 지속 고정 기능이 아님. 비 Root Motion 공격의 기존 ControllerDesired/AI Focus 정책 확인 |
| 예상보다 이동량이 작음 | 충돌·경사, Reaction 조기 종료, Duration, 시간 배율. Speed × Duration은 무충돌·정상 유지 시 근삿값 |
| 이전 종료가 새 이동을 지움 | 외부 Serial이 아니라 Reaction 세대를 StopKnockback에 전달했는지, 실행 교체 시 세대 변경 확인 |

거리·각도는 보정 허용 여부를 결정한다. 최대 각도가 90도면 120도 차이를 90도로 잘라 회전하는 것이 아니라 보정을 생략한다. 넉백 Priority 500은 프로젝트 코드 상수이며 엔진 표준 우선순위는 아니다.

## 6. 검증 결과와 재현

2026-09-21 소스 `b0291c0e`와 테스트 `361aeeb2`, 사용자 저장 Stellar BP를 대상으로 실행했다.

| 구분 | 결과 | 한계 |
| --- | --- | --- |
| PortfolioEditor Win64 Development | 빌드 성공 | cook/패키징/다른 플랫폼은 미실행 |
| Portfolio. 자동 테스트 | 28개 Success, 종료 코드 0 | RenderPreview 1개는 NullRHI에서 본 검사 생략. 렌더 검증 아님 |
| 넉백 6개 | Ownership, ReactionLifecycle, DamagePipeline, Movement, ExternalLaunch, FrameRate 실행 성공 | 실제 전체 맵·모든 애니메이션 조합은 아님 |
| 방향 보정 4개 | Gates, Lifecycle, PlayerInputZeroSerial, RootMotion 실행 성공 | 첫 Stellar 몽타주를 복제하고 Notify를 제거한 격리 검사 포함 |
| RootMotion 격리 검사 | 약 66.19cm 이동, 몸 Yaw 45도, ControlRotation -30도 유지 | 실제 콤보 Notify/충돌 전체 재현은 아님 |
| 사용자 PIE 보고 | 콤보 시작부터 끝까지 적 방향으로 공격하는 동작 확인 | 모든 제외·취소·충돌 조건 확인으로 확장 해석하지 않음 |

최신 로그는 `Saved/Logs/KnockbackCommitValidation.log`이며 로컬 생성물이다. Git에는 자동 포함하지 않는다. 과거 `Saved/Audit` 자료는 이번 최신화 시 존재하지 않아, 35/36의 과거 애셋 조사와 이번 자동 검증을 구분했다.

경고: Regacy SK_Mannequin의 M_Man_Body/M_Man_ChestLogo 참조 누락과 콘솔 변수 반복 조회 경고가 남아 있다. 테스트 성공이 모든 기존 애셋 경고 해결을 뜻하지 않는다. 검토 완료 소스에는 줄 끝 공백 5곳이 남아 있으며 이번 문서 작업에서 수정하지 않았다.

에디터를 저장·종료한 뒤 저장소 루트에서 실행한다. 경로는 설치 위치에 맞춘다.

```powershell
$projectPath = Join-Path (Get-Location) 'Portfolio.uproject'
$enginePath = 'C:/Program Files/Epic Games/UE_5.4/Engine'
& "$enginePath/Build/BatchFiles/Build.bat" PortfolioEditor Win64 Development $projectPath -WaitMutex -NoHotReloadFromIDE
& "$enginePath/Binaries/Win64/UnrealEditor-Cmd.exe" $projectPath -unattended -nop4 -nosplash -nullrhi '-ExecCmds=Automation RunTests Portfolio.;Quit' '-TestExit=Automation Test Queue Empty' '-log=KnockbackCommitValidation.log'
```

`Portfolio.Combat.Knockback` 또는 `Portfolio.Combat.ActionFacing` 필터로 분리 실행할 수 있다. 실행 종료 코드뿐 아니라 로그의 실패 결과와 생략 항목을 함께 확인한다. 필요한 Stellar 애셋은 Git LFS로 받아야 한다.

## 7. 남은 점검과 범위 밖

- 실제 AI MoveTo 중단 콜백 재진입, HitStop과 이동·애니메이션 체감, 경사·계단·캐릭터 충돌, Root Motion 블렌드 전환은 구체적인 PIE 시나리오별 추가 확인이 필요하다.
- Movement 참조 재주입 테스트는 같은 캐릭터 참조를 재주입하는 경우다. 다른 Owner/Balance로 갈아끼우는 전반적 재바인딩 보장은 아니다. 현재 Balance 이벤트 RemoveAll은 새 참조 대입 뒤 실행되므로 이전 Balance 교체 시 구독 정리는 별도 검토 대상이다.
- Action의 CharacterMovement 직접 조회는 남아 있다. Movement의 직접 주입 변경을 전체 컴포넌트의 주입 방식 통일로 설명하지 않는다.
- 자동 테스트 성공을 네트워크 예측/복제, 멀티플레이, 성능 개선, 모든 상황의 무결함 보장으로 설명하지 않는다.
- 공중 발사·벽꿍·넘어짐·저항·슈퍼아머·Motion Warping·전용 DataAsset 전환은 구현 범위 밖이다.

## 8. 커밋 범위

| 커밋 | 범위 |
| --- | --- |
| `b0291c0e` | 사용자 검토 완료 런타임 소스 20개 |
| `361aeeb2` | 넉백·방향 보정 테스트 및 Probe 4개 |
| `4a1487a3` | 사용자 저장 BP_CPlayer_Stellar 1개 |
| 이 문서를 포함한 문서 커밋 | 35/36 최신화, 통합 가이드, 탐색 링크 및 S38의 연결 설명 |

SciFiMap은 사용자의 요청대로 커밋하지 않았다. 이번 준비 시 이미 Git 수정 목록에 없었으며 에이전트가 되돌리거나 저장하지 않았다. 플레이어 BP의 세부 속성 차이는 바이너리 diff로 완전 검증하지 않았고 사용자 저장본을 그대로 커밋했다. 푸시·PR·머지는 수행하지 않는다.
