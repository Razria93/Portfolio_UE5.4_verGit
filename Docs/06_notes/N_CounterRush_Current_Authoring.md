# 퍼펙트 닷지·카운터 러시 현재 저작 가이드

기준: 2026-09-23 감사 후 보완. 이전 작업 일지는 당시 증거이며 현재 사용법은 이 문서를 우선한다.

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

- 몸: `Body Glow (Point)`의 Start/End로 제어한다. 성공 시 자동 시작은 제거했다. 현재 Stellar 몽타주의 Start=0.01초, End=0.30초는 초기 배치이며 자유롭게 옮길 수 있다.
- Start 노티파이: Start=true, Peak Weight(0~1), Fade In, Fade Out(누락/중단 시 fallback), Safety Timeout. End 노티파이: Start=false, Fade Out. Fade In/Out과 안전 만료는 실제 시간, 노티파이 위치는 몽타주 시간이다.
- Start 후 최대 강도를 유지하고 End에서 현재 강도부터 감소한다. 동일 실행의 중복 Start는 fade나 안전 만료를 재시작하지 않는다. 리액션 완료·중단·카운터 전환은 fallback fade로 몸을 해제한다. 사망·EndPlay는 즉시 복원한다. 몸은 카운터에 인계하지 않는다.
- Perfect Body Material / Perfect Body Parts / Perfect Body Part Tag / Glow Weight Parameter는 ReactionFeedbackComponent에 유지된다. 최대 강도는 이제 Start의 Peak Weight로 조절한다. `PerfectBodyEnvelope`는 옛 에셋 이행 값을 읽기 위한 숨김 직렬화 필드일 뿐 런타임 제어에는 사용하지 않는다.
- 무기: 기존 WeaponGlow Start/End point notify, fade, 카운터 인계 및 안전 만료 유지.
- 몸/무기 머티리얼 및 사용자의 Fresnel 튜닝을 바꾸지 않았다.
- 트레일: 무기 BP가 참조하는 WeaponTrailDataAsset의 CounterRush 정의에서 선택한다. 현재 LightSaber_Loop 선택을 보존했다.
- 트레일 상대 위치/회전은 유효한 저작 설정이며 자동 테스트가 socket transform과 합성해 검증한다. width override는 실제 Niagara에 같은 이름의 float User 파라미터가 있어야 한다.
- 기존 Water는 기존 방식 유지. 새 부착 옵션을 무조건 모든 정의에 켜지 않는다.

## 검증 명령

UE Editor 종료 후 `UnrealEditor-Cmd.exe Portfolio.uproject`에 아래 인수를 추가한다. 공통 `-unattended -nop4 -NullRHI -nosound` 사용.

```text
-run=CDodgeAssets -CounterOpportunityWindow
-run=CDodgeAssets -CounterOpportunityWindow -Verify
-run=CDodgeAssets -CounterTrailAttachment -Verify
-run=CDodgeAssets -BodyGlowPoints -Verify
-ExecCmds="Automation RunTests Portfolio." -TestExit="Automation Test Queue Empty"
```

첫 명령은 dry-run. 카운터 이행에만 `-Apply`를 명시한다. 기존 marker가 있으면 중복 생성하지 않는다. 모든 대상 경로를 사전 검사하며 몽타주를 저장한 뒤 BP 모드를 활성화한다. 저장 중 I/O 실패까지 자동 rollback하는 트랜잭션은 아니므로 Apply 전 백업한다.

몸 발광 이행은 별도 `-BodyGlowPoints` dry-run으로 현재 슬로우를 확인한 뒤 `-Apply '-StartTime=0.01' '-EndTime=0.30'`처럼 명시한다. PowerShell에서는 소수값 인수를 인용한다. 기존 포인트가 있으면 재배치하지 않는다. 현재 도구는 Stellar BP가 참조한 PerfectDodge만 이행한다. 다른 몽타주는 Start/End를 직접 배치해야 한다. Start는 0보다 뒤, End는 실행 완료보다 앞에 둔다.

주의: `-CounterTrailAttachment -Apply`는 과거 Scifi 마이그레이션이다. 현재 LightSaber 설정을 확인할 때는 **Verify만** 사용한다. 옵션 없는 legacy bootstrap 검증은 현재 편집된 콘텐츠의 범용 검증이 아니다.

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
