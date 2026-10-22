# 가드 종료 중 피격 누락 — 정리 요청의 반복 수락

[대표 README](../../README.md#problem-solving) · [사례 목록](00_Issue_Analysis_Report_Index.md) · [영상 11과 공개 상태](../07_Portfolio_Documents/Portfolio_Production/Video_Catalog.md#video-11)

**핵심:** 가드 종료를 피격이 중단하는 순간, 이미 비워진 가드 상태를 다시 정리하려다 피격 실행까지 거부되는 문제를 해결했습니다. 상태 정리는 반복해도 수락하도록 하고, 전체 가드 상태 정리와 화면·이동 Overlay 정리의 책임을 구분했습니다.

## 증상과 원인

GuardOut 도중 피해는 적용됐지만 Hit Reaction이 시작되지 않는 경로를 추적했습니다. 피격 판단 시 준비한 ClearGuardState 요청을 실제 적용하기 전에, 기존 GuardOut의 중단 처리가 가드 런타임 상태를 먼저 비웠습니다. 이후 “가드 상태가 남아 있어야 정리 가능”이라는 수락 조건 때문에 정리가 거부되고 ReactionExecutionFailed로 이어졌습니다.

```text
피격 판단: GuardOut 중단 + ClearGuardState 준비
→ GuardOut 중단 콜백이 가드 상태를 먼저 정리
→ 준비한 ClearGuardState 적용
   이전 조건: 상태 없음 → 거부 → Hit 시작 누락
   수정 조건: 이미 정리된 상태도 수락 → Hit 시작
```

## 선택한 해결책과 코드

정리 요청은 원하는 종료 상태로 수렴하는 연산으로 취급합니다. CanApplyOverlayHandling은 ClearGuardState / ClearGuardOverlay를 현재 잔여 상태와 무관하게 수락합니다.

| 정리 요청 | 책임 |
| --- | --- |
| ClearGuardState | 다음 가드 시작 허용, 가드 의도·자세 종료, Guard/Parry Window 닫기, 가드 이동속도 Override 제거 |
| ClearGuardOverlay | 가드 의도와 시작 허용 상태를 보존하고, 자세·Window·이동속도 Override만 정리 |

[Defense 구현](../../Source/Portfolio/Component/CDefenseComponent.cpp)에서 CanApplyOverlayHandling → ApplyOverlayHandling → ClearGuardState / ClearGuardOverlay 순서로 읽으면 됩니다. [Reaction Orchestrator](../../Source/Portfolio/Component/CReactionOrchestratorComponent.cpp)와 [S37 실행 전환](../05_System_Architecture/S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md)은 판단과 실제 적용의 시점 차이를 설명합니다.

## 검증 결과 — 2026-10-21

| 단계 | 확인한 범위 | 결과와 근거 |
| --- | --- | --- |
| 경계 자동화 | 가드 없음/In/Hold/Out × 직접 정리/완료/중단 × Run/Sprint = 24조건, 조건별 25회 State 정리. 별도로 Overlay 책임과 Hit 판단 후 중단 순서 검사 | 최종 UE 자동화 3개 테스트 성공. [24조건 CSV](../98_Evidence/Combat_Case_Studies_20261021/guard-24-conditions.csv), [결과·대조군](../98_Evidence/Combat_Case_Studies_20261021/guard-automation.json) |
| 실패 대조 | 과거의 두 수락 조건만 복원한 대조군. 역사적 커밋 전체 재현은 아님 | HitSnapshotAfterInterruption 검사 실패, 수정 조건 복원 후 전체 3개 성공. [대조 패치](../98_Evidence/Combat_Case_Studies_20261021/guard-negative-control.patch) |
| 실제 전투 통합 | AI의 공개 행동 요청 → 실제 무기 충돌 → 피해 판정 → GuardOut 중단 → Hit Montage → 복귀 → 다음 GuardIn/Hold | HP 5000→4700, Run 600→GuardWalk 400→Run 600, 다음 가드 시작과 종료 확인. [통합 이벤트·검사](../98_Evidence/Combat_Case_Studies_20261021/guard-integration.json) |

통합 검증은 원본 BP_CEnemy에 HandleAICombatAction(ComboAttack)을 요청한 통제 시나리오입니다. 피해를 직접 주입하지 않았지만 BT가 자율적으로 공격을 선택한 전체 상황을 검사한 것은 아닙니다. 경계 자동화의 180/420/620 속도는 테스트 fixture 값이며 실제 게임의 400/600 값과 구분합니다.

Sprint를 800으로 한 번 설정하고 키를 계속 누르지 않은 시도에서는 “800 복원” 기대가 실패했습니다. 회전 모드 변경 시 입력 재평가가 기본 보행을 Run으로 갱신하므로, 같은 무입력 조건에서 600 복원을 기대한 재검증은 성공했습니다. [실패 기록](../98_Evidence/Combat_Case_Studies_20261021/guard-sprint09-excerpt.json)을 보존하며 **Sprint 키를 계속 누른 상태의 800 복원까지 검증했다고 주장하지 않습니다.**

## 근거의 적용 범위와 남은 확인

이 결과는 PPT 제작 과정에서 10월 21일 로컬 소스를 격리 복사해 실행한 기록입니다. 해당 496개 Source 파일과 이 PR의 기반 소스는 162개가 다릅니다. 위 가드 핵심 함수 3개는 본문이 같지만, 그 사실만으로 PR 전체의 런타임 검증을 대신하지 않습니다. [함수 대조](../98_Evidence/Combat_Case_Studies_20261021/key-function-comparison.json) · [전체 소스 대조·재현 범위](../98_Evidence/Combat_Case_Studies_20261021/README.md)

경계 자동화는 실제 컴포넌트를 쓰되 생명주기 콜백을 주입합니다. 별도의 통합 촬영이 실제 충돌·몽타주 경로를 보완합니다. 엔진 시작 시 Content 제외에 따른 HUD 리소스 오류가 있었으므로 “엔진 로그 전체 무오류”라는 의미도 아닙니다. 일반 수동 PIE와의 시각적 동등성, 키를 유지한 Sprint 복원, 전체 오디오 청취는 별도 확인 항목입니다.

공개된 [기존 설명 영상 11](https://youtu.be/wm57Nbvs8OI)은 약 30초입니다. 최신 통합 검증용 약 16.4초 촬영본 두 개는 별도 근거이며, 이 공개 영상을 교체한 제출본이 아닙니다.

다음 사례: [TTL 시작 시점을 Reaction 종료에 맞추기](I10_HitReactive_TTL_Case_Study.md).
