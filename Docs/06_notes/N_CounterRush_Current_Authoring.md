# 퍼펙트 닷지·카운터 러시 현재 저작 가이드

기준: 2026-09-23 감사 후 보완. 이전 작업 일지는 당시 증거이며 현재 사용법은 이 문서를 우선한다.

## 카운터 기회

`BP_CPlayer_Stellar` → DefenseComponent → DodgeTuning → **Use Montage Counter Window = true**.

실제 연결된 `M_PerfectDodge_B_Root_Stellar`에 **Counter Opportunity Open** point notify를 0.45초에 추가했다. 기존 CounterConnectStart 값을 옮긴 것으로, 다른 사용자 notify와 아트 설정은 변경하지 않았다. 현재 CompleteReaction은 1.133초다.

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

- 몸: 성공 즉시 시작하는 ReactionFeedbackComponent의 PerfectBodyEnvelope 유지. 이번에 몸 시작점을 notify로 옮기지 않았다.
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
-ExecCmds="Automation RunTests Portfolio." -TestExit="Automation Test Queue Empty"
```

첫 명령은 dry-run. 카운터 이행에만 `-Apply`를 명시한다. 기존 marker가 있으면 중복 생성하지 않는다. 모든 대상 경로를 사전 검사하며 몽타주를 저장한 뒤 BP 모드를 활성화한다. 저장 중 I/O 실패까지 자동 rollback하는 트랜잭션은 아니므로 Apply 전 백업한다.

주의: `-CounterTrailAttachment -Apply`는 과거 Scifi 마이그레이션이다. 현재 LightSaber 설정을 확인할 때는 **Verify만** 사용한다. 옵션 없는 legacy bootstrap 검증은 현재 편집된 콘텐츠의 범용 검증이 아니다.

테스트 완료 후:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Validation/Test-AutomationLog.ps1 -LogPath Saved/Logs/RefinementFinalRegression.log
```

종료 코드 0 또는 실행 개수만으로 성공을 판단하지 않는다. 위 스크립트는 실패·미완료·0개 실행을 거절한다.

## 사용자 PIE 확인

1. 기존 Dodge → PerfectDodge 판정/리액션이 유지되는지.
2. 0.45초 전 카운터는 불가, 이후 리액션 종료 전 가능, 종료 후 불가인지.
3. 카운터 전환 때 무기 발광이 끊기지 않고 공격 End/취소 시 부드럽게 해제되는지.
4. 현재 거리/방향에서 공격이 맞고 벽을 통과하지 않는지.
5. LightSaber 트레일 폭·색상·방향이 기존 사용자 설정대로인지.

NullRHI 자동화 통과는 GPU 표시나 최종 조작감에 대한 사용자 승인과 다르다.
