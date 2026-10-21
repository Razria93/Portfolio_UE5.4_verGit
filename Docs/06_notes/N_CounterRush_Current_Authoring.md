# 퍼펙트 닷지·카운터 러시 현재 저작 가이드

기준: 2026-10-21 D1 분리본 검증. 이전 작업 일지는 당시 증거이며 현재 사용법은 이 문서를 우선한다.

## 카운터 기회

`BP_CPlayer_Stellar` → DefenseComponent → DodgeTuning → **Use Montage Counter Window = true**.

처음에는 기존 CounterConnectStart 값을 옮겨 0.45초에 Open을 추가했다. 이후 사용자 편집을 반영한 2026-09-23 BodyGlow 검증 시점에는 **Counter Opportunity Open 0.667초 / CompleteReaction 1.150초**였다. 몸 발광 이행에서 이 사용자 타이밍은 변경하지 않았다.

- 시작점을 옮기려면 이 Open notify를 이동한다. Notify State가 아니다.
- 더 일찍 닫으려면 같은 `CAnimNotify_CounterOpportunity`를 추가하고 **Open=false**로 설정한다.
- 리액션 정상 완료·중단은 자동으로 기회를 닫는다. 닫기 notify가 없어도 기회는 남지 않는다.
- 이미 승인된 카운터 전환만 실행 도중 예약 권한을 유지한다. 시작 확정에서 한 번 소비되고 중단된 리액션의 실패한 전환 권한은 제거된다.
- 몽타주 모드에서는 CounterConnectStart/CounterGraceSeconds가 입력 자격에 관여하지 않는다. 두 필드는 기존 콘텐츠 호환 모드에만 사용한다.
- Open은 실행 시작 직후가 아니라 유효한 양의 시점에 배치한다. 시작 시점 0에서의 소급 이벤트 실행은 보장하지 않는다.
- 다른 플레이어 BP는 자동 이행하지 않았다. 해당 BP의 실제 몽타주에 marker를 배치한 뒤 모드를 켠다.

## 러시 거리·방향

| 위치 | 설정 | 의미 |
|---|---|---|
| DefenseComponent / DodgeTuning | CounterMaxDistance | 반격 대상 자격 거리 |
| ActionComponent / CounterRushTuning | MaxTravelDistance | 실제 이동 상한 |
| 동일 | ExtraGap | 양쪽 캡슐 반경 합에 더하는 여유 거리 |
| 동일 | Duration | 이동 시간 |
| 동일 | FacingMaxDistance | 카운터 시작 시 확정 대상 방향으로 회전 가능한 거리 |
| 동일 | FacingMaxAngle | 허용 회전 각도 |

Facing 기본값은 기존 동작인 600cm/180도를 유지했다. 허용거리와 이동거리를 늘리면 회전거리도 함께 검토한다. 회전 제한 밖이면 회전 없이 진행하며 Verbose 로그에 이유를 남긴다. 이동은 시작 시 확정 방향/위치를 사용하고 지속 추적하지 않는다. 루트 고정 복제본 및 CharacterMovement 충돌 처리는 유지한다.

## 발광·트레일

- 몸: `Body Glow (Point)`의 Start/End로 제어한다. 성공 시 자동 시작은 제거했다. 현재 Stellar 몽타주의 Start=1/60초(약 0.016667초), End=0.30초이며, Start의 fallback Fade Out은 0.18초다. 이는 현재 저작값이며 Notify 위치는 필요에 따라 조정할 수 있다.
- Start 노티파이: Start=true, Peak Weight(0~1), Fade In, Fade Out(누락/중단 시 fallback), Safety Timeout. End 노티파이: Start=false, Fade Out. Fade In/Out과 안전 만료는 실제 시간, 노티파이 위치는 몽타주 시간이다.
- Start 후 최대 강도를 유지하고 End에서 현재 강도부터 감소한다. 동일 실행의 중복 Start는 fade나 안전 만료를 재시작하지 않는다. 리액션 완료·중단·카운터 전환은 fallback fade로 몸을 해제한다. 사망은 피드백 컴포넌트 Tick에서 감지해 복원하고 EndPlay도 복원한다. 모든 시각 효과가 사망 호출 안에서 동시에 사라진다는 뜻은 아니다. 몸은 카운터에 인계하지 않는다.
- Perfect Body Material / Perfect Body Parts / Perfect Body Part Tag / Glow Weight Parameter는 ReactionFeedbackComponent에 유지된다. 최대 강도는 이제 Start의 Peak Weight로 조절한다. `PerfectBodyEnvelope`는 옛 에셋 이행 값을 읽기 위한 숨김 직렬화 필드일 뿐 런타임 제어에는 사용하지 않는다.
- 무기: 기존 WeaponGlow Start/End point notify, fade, 카운터 인계 및 안전 만료 유지.
- 몸/무기 머티리얼 및 사용자의 Fresnel 튜닝을 바꾸지 않았다.
- 트레일: 무기 BP가 참조하는 WeaponTrailDataAsset의 CounterRush 정의에서 선택한다. 현재 LightSaber_Loop 선택을 보존했다.
- 트레일 상대 위치/회전은 유효한 저작 설정이며 자동 테스트가 socket transform과 합성해 검증한다. width override는 실제 Niagara에 같은 이름의 float User 파라미터가 있어야 한다.
- 기존 Water는 기존 방식 유지. 새 부착 옵션을 무조건 모든 정의에 켜지 않는다.

## 검증 명령

UE Editor 종료 후 `UnrealEditor-Cmd.exe Portfolio.uproject`에 아래 인수를 추가한다. 공통 `-unattended -nop4 -NullRHI -nosound` 사용.

```text
-run=CDodgeAssets -Verify
-run=CDodgeAssets -CounterOpportunityWindow
-run=CDodgeAssets -CounterOpportunityWindow -Verify
-run=CDodgeAssets -CounterTrailAttachment -Verify
-run=CDodgeAssets -BodyGlowPoints -Verify
-ExecCmds="Automation RunTests Portfolio." -TestExit="Automation Test Queue Empty"
```

`-run=CDodgeAssets -Verify`는 두 플레이어 BP의 실제 참조를 읽는 검사다. `-CounterOpportunityWindow`만 지정한 명령은 해당 이행의 dry-run이다. 카운터 이행에만 `-Apply`를 명시한다. 기존 marker가 있으면 중복 생성하지 않는다. 모든 대상 경로를 사전 검사하며 몽타주를 저장한 뒤 BP 모드를 활성화한다. 저장 중 I/O 실패까지 자동 rollback하는 트랜잭션은 아니므로 Apply 전 백업한다.

몸 발광 이행은 별도 `-BodyGlowPoints` dry-run으로 현재 슬로우를 확인한 뒤 `-Apply '-StartTime=0.01' '-EndTime=0.30'`처럼 명시한다. PowerShell에서는 소수값 인수를 인용한다. 기존 포인트가 있으면 재배치하지 않는다. 현재 도구는 Stellar BP가 참조한 PerfectDodge만 이행한다. 다른 몽타주는 Start/End를 직접 배치해야 한다. Start는 0보다 뒤, End는 실행 완료보다 앞에 둔다.

주의: `-CounterTrailAttachment -Apply`는 과거 Scifi 마이그레이션이다. 현재 LightSaber 설정을 확인할 때는 **Verify만** 사용한다. 옵션 없는 기본 실행과 `-Verify`는 현재 BP가 참조한 에셋을 읽기 전용으로 검사한다. 전체 legacy bootstrap 저작은 `-Apply -LegacyBootstrapApply`를 함께 지정해야 하며 현재 설정을 덮어쓸 수 있다. `-Apply -Verify` 동시 지정은 변경 전에 거절된다.

테스트 완료 후:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Validation/Test-AutomationLog.ps1 -LogPath Saved/Logs/RefinementFinalRegression.log
```

종료 코드 0 또는 실행 개수만으로 성공을 판단하지 않는다. 위 스크립트는 실패·미완료·0개 실행을 거절한다.

## 사용자 PIE 확인

1. 기존 Dodge → PerfectDodge 판정/리액션이 유지되는지.
2. 현재 Counter Opportunity Open 전 카운터는 불가, 이후 리액션 종료 전 가능, 종료 후 불가인지.
3. 카운터 전환 때 무기 발광이 끊기지 않고 공격 End/취소 시 부드럽게 해제되는지.
4. 현재 거리/방향에서 공격이 맞고 벽을 통과하지 않는지.
5. LightSaber 트레일 폭·색상·방향이 기존 사용자 설정대로인지.
6. Body Glow Start/End를 이동하면 몸 발광 시점이 바뀌는지, 조기 카운터·중단 시 몸은 부드럽게 사라지고 무기는 기존 인계를 유지하는지.

NullRHI 자동화 통과는 GPU 표시나 최종 조작감에 대한 사용자 승인과 다르다.

## 플레이어 사망 처리와 최신 검증

플레이어 사망 이벤트는 Dead 리액션 데이터가 없어도 활성 액션을 취소하고 활성 PerfectDodge 리액션도 종료한다. 일반 Dodge와 PerfectDodge는 사망 시 소유 몽타주를 블렌드 시간 0으로 정지하고 메시/Movement의 잔여 루트 모션과 수평 속도를 정리한다. 생존 중 종료 블렌드, 수직 낙하 속도와 이동 모드는 유지한다. CounterRush는 공격 충돌과 소유 이동 소스를 종료한다.

2026-10-21 D1 최종 작업 트리에서 UE 5.4 Win64 Development Editor 빌드, Portfolio 자동화 71개(경고 동반 통과 2개 포함), CDodgeAssets -Verify를 통과했다. 실제 렌더링 PIE 자동 시나리오 23개도 통과했다. 일반 회피 사망 6개(초반·중반·자연 블렌드아웃 × 치명 TakeDamage/직접 TryKill), 퍼펙트 회피 사망 4개, 카운터러시 사망 3개에서 각각 사망 후 8초 동안 추가 수평 이동 0.0000cm와 추가 공격 피해 0건을 확인했다. 일반 회피 생존 중 중단/비치명 HP 변경, 정상 카운터러시 명중 3회, 연출과 Actor EndPlay 정리도 포함했다.

PIE는 실제 BP·AnimBP·몽타주·무기를 사용하는 자동 실행이며 적 AI는 통제했다. 자연 AI 전체 전투, 물리 키보드/패드 입력, Shipping/cook/package와 네트워크 전수 검증은 아니다. 과거 일지의 테스트 수와 결과는 당시 기록으로 유지한다.

## 2026-10-21 커밋 경계 최종 검증 — 넉백 빌드 호환성 수정 포함

넉백 디버그 변수명 변경을 제외했던 검증에서는 기본 Unity 빌드의 C2374 중복 정의가 발생했다. 현재는 보존한 변경을 C1의 빌드 호환성 수정으로 다시 포함해 해결했다. `FActionFacingDebug.cpp`와 `FCombatKnockbackDebug.cpp`를 같은 `Module.Portfolio.9.cpp`에서 함께 컴파일하는 경우도 통과했다. 콘솔 변수 이름이나 넉백 계산·게임플레이 동작은 바꾸지 않는다.

| 경계 | 변경 경로 | 기본 Unity 빌드 | 전체 Unity 추가 검사 | 자동화 | 에셋 검사 |
|---|---:|---|---|---:|---|
| C1 기능·에셋·핵심 회귀·빌드 호환성 | 149 | 통과 | 통과 | 56개 통과 | 캐릭터/BP 및 몽타주 감사 통과 |
| C1+C2 검증 도구·추가 회귀 | 164 | 통과 | 통과 | 71개 통과 | CDodgeAssets -Verify 및 몽타주 감사 통과 |
| C1+C2+C3 문서 포함 | 181 | 통과 | 통과 | 71개 통과 | CDodgeAssets -Verify 및 몽타주 감사 통과 |

전체 Unity 추가 검사는 `-DisableAdaptiveUnity`로 변경 파일도 Unity 묶음에서 제외하지 않고 수행했다. `-DisableUnity`는 사용하지 않았으며 프로젝트/전역 빌드 설정 파일은 수정하지 않았다. 테스트는 각 경계의 전체 Unity 빌드 산출물로 실행했다. 기존 경고는 C1의 레거시 Mannequin 머티리얼 참조와 C2부터 포함되는 TimeEffects 테스트 월드 CleanupWorld 호출이다.

C1에는 CDodgeAssets가 아직 없으므로 기존 CExecutionMontageAudit를 `/Game/01_Character`와 `/Game/04_Montage`에 각각 실행했다. C2/C3는 두 경로를 함께 감사해 308패키지·52몽타주·169Notify, 로드 실패/규칙 오류/규칙 경고 0건을 확인했다. 모든 검사는 읽기 전용이며 기존 에셋을 저장하지 않았다.

이 검증은 커밋을 생성하지 않고 파일을 보관·복원해 구성한 경계별 작업 트리에서 수행했다. 최종 구성은 C1 149개 + C2 15개 + C3 17개이며 넉백 파일을 제외하지 않는다. 현재 기본 빌드 차단 문제는 해소됐다. 문서의 결과 보강은 C3 테스트 이후의 문서 변경이며 해당 테스트의 소스/에셋은 그대로다.

실제 PIE 23개 결과는 일반 회피 사망 수정 직후의 이전 실행 기록이며 이번 경계 검사에서 다시 실행하지 않았다. 현재 런타임 소스와 기존 에셋이 그 검증본과 같음을 확인했다. Shipping/cook/package·네트워크·수동 입력 전체 보스전은 이 검증 범위가 아니다.
