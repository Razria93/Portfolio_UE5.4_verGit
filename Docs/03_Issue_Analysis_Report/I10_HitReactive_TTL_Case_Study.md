# 피격 근거 TTL — Reaction 종료부터 유효 기간 계산

[대표 README](../../README.md#problem-solving) · [사례 목록](00_Issue_Analysis_Report_Index.md) · [영상 12와 공개 상태](../07_Portfolio_Documents/Portfolio_Production/Video_Catalog.md#video-12)

**핵심:** 적이 피격 동작을 끝내기도 전에 공격자 정보가 만료되는 문제를 줄이기 위해, 피격 수락 시점과 근거 유효 기간(TTL)의 시작 시점을 분리했습니다. 오래된 종료 이벤트가 새 피격의 타이머를 시작하지 않도록 ResultSerial도 확인합니다.

## 문제와 해결 구조

피격 수락과 동시에 1초 TTL을 시작하면 약 1.4초의 Hit Reaction 중 근거가 먼저 만료될 수 있습니다. 이 경우 종료 시 Target/Focus가 없어 공격자 방향을 적용하지 못합니다. 단순히 TTL을 길게 만드는 대신, 수락한 Reaction의 종료를 수명 계산의 기준으로 삼았습니다.

```text
피격 수락 → 공격자 + ResultSerial 기록 → Awaiting reaction
  ├─ Reaction 수락 → Completed / Interrupted / Ignored에서 해당 시리얼의 TTL 시작
  └─ Reaction 미수락 → 요청 결과 처리에서 TTL 시작
최신 시리얼과 일치하지 않는 종료 → 새 근거의 타이머를 시작하지 않음
```

[EnemyHitReactive](../../Source/Portfolio/Component/CEnemyHitReactiveComponent.cpp)의 수락·요청 결과·생명주기 콜백 → [Participation Component](../../Source/Portfolio/Component/CEnemyCombatParticipationComponent.cpp)의 전달 → [WorldSubsystem](../../Source/Portfolio/System/Combat/CWorldSubsystem_CombatParticipation.cpp)의 StartHitReactiveEvidencePostReactionTTL 순서로 확인할 수 있습니다. 전체 정책은 [S34](../05_System_Architecture/S34_UE5_Portfolio_Combat_Participation_Policy.md)에 있습니다.

## 영상으로 확인한 역사적 재현과 현행 경계 검사

| 근거 | 조건 | 확인 내용 |
| --- | --- | --- |
| 10월 1일 보존 환경을 10월 21일 재촬영 | TestRoom·마네킹·후방 100cm·타깃 선택 카메라. 양쪽 TTL 1초, Investigate 비활성. 월드 표시 전후 각 2회, HUD 전후 각 1회 | ImmediateTTL 비교 조건은 Reaction 중 만료 후 종료 시 Target/Focus 없음. 변경 조건은 Awaiting reaction 후 종료 시 TTL 1.0초·TargetFacing 적용 및 회전. [6회 시점별 자료](../98_Evidence/Combat_Case_Studies_20261021/ttl-historical-reshoot.json) |
| 10월 21일 소스의 컴포넌트 통합 검사 | 실제 HitReactive→Participation→WorldSubsystem, 합성 피격 패킷·종료 이벤트, TTL 1초, NullRHI/NoSound/Content 제외 | 4그룹·19시나리오 성공. 같은 공격자의 연속 시리얼, 중복 종료, A/B 독립 TTL, 완료·중단·무시·미수락, 파괴·종료 책임 검사. [최종 결과·대조군](../98_Evidence/Combat_Case_Studies_20261021/ttl-automation.json), [시점별 CSV](../98_Evidence/Combat_Case_Studies_20261021/ttl-timeline.csv) |
| 의도적 결함 대조 | WorldSubsystem의 시리얼 보호를 제거한 합성 변이. 역사적 수정 전 코드가 아님 | 동일 공격자 테스트 실패 후 복원본 전체 성공. [변이 패치](../98_Evidence/Combat_Case_Studies_20261021/ttl-negative-control.diff) |

재촬영의 수정 전은 보존 프로젝트의 ImmediateTTL 비교 정책입니다. 과거 pre-fix 커밋 바이너리를 그대로 실행했다는 뜻이 아닙니다. 당시 코드에는 현재 저장소의 NoCurrentSight 조건이 없으므로, 영상으로 현재 코드 전체의 동작을 증명하지 않습니다. Target/Focus 적용, 시야 재획득, 몸체 Yaw 회전도 각각 구분해 관찰했습니다.

## 발견된 잔여 문제 — 파괴된 공격자의 Pending 정리

**TTL 시리얼 동작은 확인했지만, 파괴 경로의 대기 항목 정리는 보완이 필요합니다.** 19시나리오 성공에는 현재 결함을 관찰하도록 작성한 검사도 포함됩니다.

| 시나리오 | Pending 대기 항목 | EvidenceRegistry 저장 항목 | 해석 |
| --- | --- | --- | --- |
| 공격자 8개 등록 후 파괴, 종료 콜백 및 2초 진행 | 8 | 0 | 유효하지 않은 WeakObject에서 조기 반환해 Pending 제거가 생략됨 |
| HitReactive 컴포넌트 종료 | 0 | 별도 책임 | Pending·리스너는 정리되지만 참여 근거 해제와는 책임이 다름 |
| HitReactive만 종료한 별도 실험 → Participation ClearAIController | 0 | 1→0 | 실제 Actor 전체 EndPlay 순서는 추가 통합 확인 필요 |

Registry가 0이므로 파괴된 공격자가 계속 전투 참여 중이라는 뜻은 아닙니다. 또한 중복 delegate 이벤트가 Pending 제거로 차단된 결과를, 공개 StartTTL API 자체의 멱등성 보장으로 확대하지 않습니다. CSV의 Registry는 저장된 전체 항목 수이며 활성 근거 수와 다를 수 있습니다. 없는 항목의 AStarted=1은 기본값이므로 실제 시작된 타이머로 해석하지 않습니다.

후속 작업은 유효하지 않은 공격자의 Pending 제거, 종료 콜백이 없는 파괴 경로, SignalTarget이 먼저 무효화된 Unbind 순서를 보완한 뒤 “반복 파괴 후 Pending 8→0” 회귀 조건과 실제 PIE를 확인하는 것입니다. **이 문서 PR에서 해당 게임 코드를 수정하거나 해결 완료로 표시하지 않았습니다.**

## 버전과 영상

컴포넌트 검사는 10월 21일 로컬 스냅샷에서 실행했습니다. 이 PR의 기반 코드와 전체 파일은 다르지만, 연결한 TTL 핵심 함수 7개는 본문이 일치합니다. [함수 대조](../98_Evidence/Combat_Case_Studies_20261021/key-function-comparison.json) · [소스·검증 범위](../98_Evidence/Combat_Case_Studies_20261021/README.md). 이 문서 정리에서는 Unreal 검사를 새로 실행하지 않았습니다.

최신 제작본은 **38.8초·1440p60 출력**의 TTL 포트폴리오 v2입니다. 승인된 재촬영을 기존 비교·진단·도식·정리 재생 형식에 반영했습니다. [기존 공개 영상 12](https://youtu.be/3wiZNXvtokM)는 **37초의 이전 버전**이며 최신 재촬영본으로 소개하지 않습니다. 최신본의 공개 주소는 확보되지 않았습니다. 세부 파일명·SHA-256·제작 검수 범위는 [영상 목록](../07_Portfolio_Documents/Portfolio_Production/Video_Catalog.md#video-12)에서 확인합니다.

다음 사례: [AI 인지 지연과 처리 비용의 반복 측정](I11_AI_Perception_Case_Study.md).
