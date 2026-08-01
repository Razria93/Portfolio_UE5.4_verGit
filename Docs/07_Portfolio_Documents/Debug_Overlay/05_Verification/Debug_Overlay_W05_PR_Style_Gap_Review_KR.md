# Debug Overlay W05 PR Style Gap Review

## 1. 목적

이 문서는 `Docs/04_Pull_Request`의 W05 코드 클린 PR 문서 형식을 기준으로, `feature/debug-overlay-evidence-plan` 브랜치의 debug overlay 문서가 PR 설명으로 재사용 가능한 수준인지 검토한다.

이번 문서는 코드 수정 계획이 아니라 문서 gap 정리다. Debug overlay 작업은 기능 구현과 코드 품질 정리가 함께 진행되었으므로, 과거 W05 PR처럼 변경 배경, 적용 규칙, 검증, 비목표, 후속 작업을 별도로 설명해야 한다.

## 2. 비교 대상

| 문서 | 기준 역할 |
| --- | --- |
| `P42_UE5_Portfolio_Pull_Request.md` | Debug log / diagnostic helper / CVar gate 분리 방식 |
| `P43_UE5_Portfolio_Pull_Request.md` | CVar ownership와 debug/profiling/runtime policy 분리 방식 |
| `P44_UE5_Portfolio_Pull_Request.md` | comment / section cleanup 설명 방식 |
| `P45_UE5_Portfolio_Pull_Request.md` | naming / API cleanup 설명 방식 |
| `P46_UE5_Portfolio_Pull_Request.md` | Type header 책임 분리 설명 방식 |
| `P47_UE5_Portfolio_Pull_Request.md` | type 의미 정리와 migration risk 설명 방식 |
| `P48_UE5_Portfolio_Pull_Request.md` | include order cleanup과 비범위 고정 방식 |
| `P49_UE5_Portfolio_Pull_Request.md` | API const / read-only contract 분류 방식 |
| `P50_UE5_Portfolio_Pull_Request.md` | 파일군별 section policy 설명 방식 |
| `P51_UE5_Portfolio_Pull_Request.md` | tuning constant 성격 분류와 보류 후보 기록 방식 |

## 3. W05 PR 문서 공통 패턴

P42~P51은 세부 주제가 달라도 다음 구조를 반복한다.

```text
제목 / 날짜 / 상태
-> 브랜치
-> 요약
-> 변경 배경
-> 주요 변경 또는 변경 범위
-> 주요 처리 흐름
-> 구현 결과
-> 테스트 방법
-> 검증 결과
-> 비범위 / 후속 작업
-> 관련 문서
-> 정리 또는 PR 설명 초안
```

핵심은 단순 변경 파일 목록이 아니라 다음 질문에 답하는 것이다.

- 왜 이 정리가 필요했는가.
- 어떤 W05 규칙을 적용했는가.
- 어떤 변경은 의도적으로 제외했는가.
- 어떤 검증으로 동작 변경이 없거나 debug-only 경계가 유지됨을 확인했는가.
- 후속 PR 또는 다음 작업으로 넘긴 판단은 무엇인가.

## 4. P42~P51 코드 클린 축 요약

| PR | 코드 클린 축 | Debug Overlay에 가져올 기준 |
| --- | --- | --- |
| P42 | Debug Log Policy | gameplay 본문은 hook 호출만 남기고 출력/저장은 helper가 담당한다. |
| P43 | CVar Ownership | debug output, profiling, runtime policy CVar를 섞지 않는다. |
| P44 | Comment / Section Cleanup | 코드 반복 설명보다 책임/정책/예외 설명만 남긴다. |
| P45 | Naming / API Cleanup | bool query와 mutation helper 이름을 역할에 맞게 분리한다. |
| P46 | Type Header Organization | snapshot/debug 전용 type은 gameplay shared type과 소유권을 분리한다. |
| P47 | Type Rename / Meaning Cleanup | 이름이 실제 생명주기와 의미를 드러내야 한다. |
| P48 | CPP Include Order Cleanup | `.cpp` include group 정리는 동작 변경 없이 별도 cleanup으로 처리한다. |
| P49 | API Const Consistency | read-only contract와 mutation API를 문서상 분류한다. |
| P50 | Section Comment Consistency | 같은 파일군은 같은 책임 섹션을 우선 사용한다. |
| P51 | Tuning Constants Cleanup | 숫자 literal은 내부 규칙값, tuning data, 외부 계약값, 유지 literal로 분류한다. |

## 5. Debug Overlay 현재 문서 상태

현재 Debug Overlay 문서는 구현 전 의사결정과 검증 기준은 비교적 촘촘하게 남아 있다.

확인된 장점:

- P0 / P0.5 / P1 범위가 단계별로 분리되어 있다.
- SnapshotStore, HUD, TargetComponent의 의도와 비목표가 별도 문서에 남아 있다.
- Evidence Map과 PIE 체크리스트가 실제 표시 가능 값과 `N/A`, `NotCaptured` 기준을 구분한다.
- `Debug_Overlay_P1_Code_Quality_Review_KR.md`가 LowRiskFix / DecisionNeeded / Later / NoIssue를 분류한다.

부족한 점:

- W05 PR 문서처럼 "작업 서사"로 이어지는 요약 문서가 없다.
- P42~P51 중 어떤 기준이 Debug Overlay 코드에 적용되었는지 한 곳에서 보이지 않는다.
- LowRiskFix cleanup은 반영되었지만, 어떤 W05 PR 축에 해당하는지 PR 설명으로 재사용하기 어렵다.
- DecisionNeeded / Later 항목이 P52 PR 후보의 비범위/후속 작업과 아직 직접 연결되어 있지 않다.
- 검증 결과가 구현 단계별 문서에 흩어져 있어 PR 초안에서 참조하기 어렵다.

## 6. Debug Overlay와 W05 축 매핑

| W05 축 | Debug Overlay 반영 상태 | 근거 |
| --- | --- | --- |
| P42 Debug Log Policy | 반영 | 기존 audit log와 overlay collect를 결합하지 않고 SnapshotStore 기록 경로를 분리했다. |
| P43 CVar Ownership | 반영 | `Portfolio.DebugOverlay.Enabled`, `Collect`, `Preset`, `EventLogLimit`은 기존 `Portfolio.Debug.*Audit` CVar와 분리했다. |
| P44 Comment / Section Cleanup | 일부 반영 | HUD / Store helper 섹션 정리는 반영했지만 PR 서사 문서에는 아직 매핑이 부족했다. |
| P45 Naming / API Cleanup | 반영 | `TrySelect...`, `TryGetSnapshotCopy`, `Append...` 계열로 low-risk rename을 적용했다. |
| P46 Type Header Organization | 반영 | `FDebugOverlaySnapshotTypes.h`와 SnapshotStore를 Core/Debug 소유 debug type으로 분리했다. |
| P47 Type Meaning Cleanup | 일부 반영 | `RecentCombatPair`, `ExecutionSummary`, `CombatSummary` 등 의미 중심 이름을 사용하나 subject ownership 분리는 Later다. |
| P48 CPP Include Order Cleanup | 반영 | 대상 `.cpp` include group 정리를 LowRiskFix로 처리했다. |
| P49 API Const Consistency | 일부 반영 | query/getter const 검토는 문서화했지만 `ResolveWorld`의 `const_cast`는 DecisionNeeded다. |
| P50 Section Comment Consistency | 반영 | Store API section과 HUD helper section을 책임 단위로 정리했다. |
| P51 Tuning Constants Cleanup | 일부 반영 | nearest radius, stale timeout, event limit은 internal policy constant로 유지한다. CVar/preset화는 Later다. |

## 7. PR 문서로 보강해야 할 gap

### 7.1 변경 배경

현재 문서는 "무엇을 구현할지"는 많지만, PR 독자가 처음 읽을 때 왜 debug overlay가 필요한지 한 문장으로 연결하는 요약이 부족하다.

P52 후보 문서에서는 다음 배경을 먼저 둔다.

```text
resume / 기술문서 evidence를 위해 runtime action, reaction, combat, AI 상태를 화면에서 검증 가능한 개발 전용 overlay로 노출한다.
```

### 7.2 주요 변경 축

기능 축과 코드 품질 축을 분리해서 써야 한다.

기능 축:

- SnapshotTypes / SnapshotStore
- debug hook 연결
- Canvas HUD
- Player/Enemy panel
- TargetComponent source chain

코드 품질 축:

- W05 debug log / CVar ownership
- W05 naming / include / section cleanup
- W05 shipping guard / no-op boundary
- W05 tuning constant 분류

### 7.3 검증 결과

PR 문서에는 실제 수행한 검증과 아직 수행하지 않은 검증을 분리해야 한다.

반영 가능:

- `PortfolioEditor Win64 Development` build 통과 이력
- `git diff --check` 통과 이력
- TestRoom PIE 동작 확인
- P0.5 / P1 체크리스트 작성

주의:

- FinalCandidate 촬영은 P1 이후로 미뤘으므로 완료 검증처럼 쓰지 않는다.
- Runtime LOD / AI detail은 아직 성공 evidence로 주장하지 않는다.

### 7.4 비범위 / 후속 작업

P52 후보 문서에는 다음을 명시해야 한다.

- 범용 target system / lock-on system은 구현하지 않았다.
- EventLog category filter와 Player/Enemy EventLog 분리는 P1 후속이다.
- Runtime LOD actual 표시와 AI current value 보강은 Later다.
- Shipping HUD화, UMG/Slate 전환, 최종 촬영/패키징은 제외한다.

## 8. 보강 우선순위

1. P52 PR 후보 문서를 작성해 P0/P0.5/P1 흐름을 하나의 PR 서사로 묶는다.
2. `Debug_Overlay_P1_Code_Quality_Review_KR.md`에 P42~P51 매핑 섹션을 추가한다.
3. Debug Overlay README에서 W05 review와 P52 후보 문서로 연결한다.
4. 이후 DecisionNeeded 항목을 별도 결정 문서로 분리한다.

## 9. 완료 기준

- 과거 W05 PR 문서 패턴이 Debug Overlay 문서에 반영된다.
- P52 후보 문서에서 기능 구현과 코드 품질 정리를 분리해 설명할 수 있다.
- Debug Overlay code quality review에서 W05 기준 반영 상태를 추적할 수 있다.
- 후속 작업 제안이 촬영보다 P1 DecisionNeeded / EventLog filter / Runtime LOD 보강으로 이어진다.
