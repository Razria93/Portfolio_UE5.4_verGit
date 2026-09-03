# UE5 Portfolio - Master Development Roadmap

> 목적: 현재 프로젝트가 전체 개발 흐름에서 어디에 있으며, 다음에 무엇을 어떤 순서로 진행할지 한눈에 확인하기 위한 상위 로드맵이다.

---

## 1. 문서 역할

이 문서는 개별 기능의 상세 설계서가 아니다.

| 문서 종류 | 책임 |
|---|---|
| `P02 Development Roadmap` | 전체 개발 순서, 의존성, 현재 단계, 남은 작업 규모 |
| `Sxx System Architecture` | 해당 시스템의 책임 경계, 상태·데이터 계약, 런타임 구조 |
| `Wxx WorkPlan` | 해당 브랜치의 구현 단계, 검증 항목, 완료 조건 |
| `Pxx Pull Request` | 실제 변경 범위, 커밋, 검증 결과, 리뷰 대응 기록 |

따라서 API 이름, 세부 상태 전이, 튜닝 값, 파일별 구현 계획은 이 문서에 중복 기록하지 않는다. 그런 내용은 각 작업 브랜치의 `Sxx` 또는 `Wxx` 문서에서 관리한다.

---

## 2. 현재 위치

```text
최근 완료
→ P60. Shared Combat Target and Evidence-Based Combat Participation

현재 보완
→ F07. Combat Participation Review Follow-up

현재 작업
→ R07. Resource 계층 정리
```

현재 브랜치:

```text
fix/combat-participation-review-followup
```

현재 작업의 상세 기준 문서:

- [P60 Shared Combat Target and Evidence-Based Combat Participation](../04_Pull_Request/P60_UE5_Portfolio_Pull_Request.md)
- [S33 Combat Target / Participation Architecture](../05_System_Architecture/S33_UE5_Portfolio_System_Architecture.md)
- [S34 Combat Participation Policy](../05_System_Architecture/S34_UE5_Portfolio_Combat_Participation_Policy.md)
- [S31 Enemy Death Lifecycle Architecture](../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)

---

## 3. 전체 개발 순서

### Phase A. 공통 전투 기반 정리

이 단계는 Player와 Enemy가 같은 전투 계약을 사용할 수 있도록 Target, Facing, Feedback, Death 경계를 정리한다.

| ID  | 작업                                | 상태  | 핵심 결과                                                                                                               |
| --- | --------------------------------- | --- | ------------------------------------------------------------------------------------------------------------------- |
| R01 | 공통 Combat Target 상태 기반            | 완료  | Player/Enemy의 Combat Target SoT, weak/EndPlay/revision 수명 계약을 공통 컴포넌트로 통합                                           |
| R02 | 피격 기반 Enemy Engage 진입             | 완료  | Perception/HitReactive Active Evidence를 공통 allocator로 통합하고, live HitReactive Evidence 기반 Extra Engage admission을 적용 |
| R03 | Action 구간 기반 Facing 보정            | 비채택 | 현재 Combat Target 기반 Focus/Facing과 directional locomotion이 자연스러운 경험을 제공하므로 별도 Action 구간 고정 정책은 적용하지 않음               |
| R04 | Enemy Focus 및 8Way 이동 정리          | 완료  | Combat Target 기반 Gameplay Focus/Facing, ControllerDesired와 directional locomotion presentation으로 이동·공격 방향을 정렬       |
| R05 | Player/Enemy 컴포넌트 계약 대칭화          | 완료  | 공통 Target/Action/Signal 수명 계약을 정리하고, 입력·AI 선택·표현·제거 정책의 비대칭은 역할 차이로 유지                                              |
| R06 | Enemy Death Lifecycle 컴포넌트화 검토·적용 | 완료  | Health·Reaction·Feedback·ACEnemy coordinator 경계를 확정하고, 현 규모에서는 Actor-owned coordinator를 유지                          |

Phase A 완료 기준:

```text
Player와 Enemy 모두 동일한 Target 계약 사용
→ 피격·AI 의도·플레이어 락온이 일관된 대상 문맥으로 수렴
→ 이동·공격·피격 후 전투태세가 같은 Target을 기준으로 동작
→ Target/Action/Signal의 공통 수명 계약과 Player·Enemy별 의도 정책이 분리됨
→ Health·Reaction·Feedback·Actor finalization의 Death 책임 경계가 명확함
```

### Phase A 후속 TODO / 재검토 조건

아래 항목은 Phase A 완료 조건이 아니다. 실제 요구가 생길 때만 별도 작업으로 다시 연다.

| 항목                      | 다시 여는 조건                                                            | 처리 방향                                                                                                                       |
| ----------------------- | ------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| Action 구간 Facing 보정     | 현재 Focus/Facing만으로 해결되지 않는 실제 조작감 또는 공격 연출 문제가 확인될 때                | R03을 별도 설계로 재개한다. 공격 종류별 회전 고정이 아니라 문제 상황과 필요한 presentation 계약부터 정의한다.                                                      |
| Player Death 정책         | Player 사망, Game Over, checkpoint, respawn 또는 spectator 규칙이 확정될 때    | `UCHealthComponent`의 공통 Dead 상태와 `UCCharacterFeedbackComponent`의 presentation capability를 사용하되, Player 최종 정책은 Player가 소유한다. |
| Death Lifecycle 컴포넌트 추출 | pooling, revive, 다수 Enemy 계열의 재사용, lifecycle variant 증가 중 하나가 발생할 때 | `ACEnemy` coordinator를 먼저 일반화하지 않는다. 재사용되는 상태 전이와 presentation 계약만 검증한 뒤 component extraction을 검토한다.                        |
| Component reference 분할  | 공통 reference bag의 actor 전용 capability가 실제 유지보수 비용이나 잘못된 의존성을 만들 때   | `FCharacterComponentReferences`를 기능별 capability context로 분할하는 리팩터링을 별도 범위로 검토한다.                                            |
| Player CombatResult policy 활성화 | Enemy가 Player에게 Parry를 적용하는 gameplay와 Player Balance/Collapse 정책이 확정될 때 | Player도 `ReceiveCombatResultPacket → UCCombatSignalTargetComponent` 공통 ingress만 사용한다. `UCBalanceComponent` 도입과 Player 전용 표현 정책은 해당 시점에 함께 구현한다. |

### Phase B. 전투 자원과 처형 기반

Phase A의 공통 대상 및 실행 계약이 안정된 뒤 전투 자원과 협업 실행을 확장한다.

| ID  | 작업             | 상태  | 핵심 결과                                                                |
| --- | -------------- | --- | -------------------------------------------------------------------- |
| R07 | Resource 계층 정리 | 완료  | Enemy Balance / Collapse lifecycle, authored asset 연결, 기본 PIE 흐름을 반영. 세부 outcome 회귀 매트릭스는 지속 검증 |
| R08 | Execution 시스템  | 구현 완료 / 검증 진행 | Standard / Lethal Action·Reaction pair, Balance / Death handoff, participant movement collision 정책을 반영 |

권장 의존성:

```text
R07 Resource 계약
→ R08 Execution
```

### Phase C. 전투 반응과 회피 확장

| ID | 작업 | 상태 | 핵심 결과 |
|---|---|---|---|
| R09 | Enemy Hit 정책 확장 | 대기 | 피격 종류와 전투 상태에 따른 반응·Engage 정책 정리 |
| R10 | Dodge 리팩터링 | 대기 | Normal Dodge와 Perfect Dodge 분리, Burst 획득 연결 |

### Phase D. 특수 전투 액션 확장

| ID | 작업 | 상태 | 핵심 결과 |
|---|---|---|---|
| R11 | Blink / Repulse / Counter | 대기 | Signal 기반 특수 Action·Reaction 흐름 구축 |
| R12 | Branch Attack | 대기 | 입력·상태·자원에 따른 공격 분기 확장 |

권장 의존성:

```text
Target / Facing / Resource / Dodge 기반
→ Blink·Repulse·Counter
→ Branch Attack
```

### Phase E. 플레이 경험과 관찰성 마감

| ID | 작업 | 상태 | 핵심 결과 |
|---|---|---|---|
| R13 | Enemy Status / Player Resource UI | 대기 | HP·Balance·Beta·Burst 상태를 이벤트 기반으로 표시 |
| R14 | Debug Overlay 운영성 확장 | 대기 | 섹션별 표시 제어와 심각도별 경고·주의 로그 정책 정리 |

UI와 Debug 작업은 기반 데이터 계약이 안정된 뒤 마감하는 것을 원칙으로 한다. 다만 앞선 기능을 검증하는 데 필요한 최소 Debug 표시는 각 기능 브랜치에서 함께 추가할 수 있다.

---

## 4. 전체 의존 관계

```text
Phase A
Combat Target Kernel
→ Player Target Migration
→ Enemy / BT Projection Migration
→ Provider / Consumer Cleanup
→ Hit Engage
→ Action Facing
→ Enemy Focus / 8Way
→ Component Contract Symmetry
→ Death Lifecycle Boundary

Phase B
→ Resource Contract
→ Execution

Phase C
→ Enemy Hit Policy
→ Dodge / Perfect Dodge

Phase D
→ Blink / Repulse / Counter
→ Branch Attack

Phase E
→ Enemy Status / Player Resource UI
→ Debug Overlay Operations
```

이 순서는 절대적인 시간 순서라기보다 구조적 의존 순서다. 서로 독립적인 작업을 병렬화하더라도, 선행 계약이 확정되지 않은 상태에서 후속 시스템이 임시 타입이나 임시 상태에 의존하지 않도록 한다.

---

## 5. 남은 작업 규모

현재 로드맵은 14개의 상위 작업 단위로 관리한다.

| 구간 | 전체 | 완료 | 진행 | 다음 | 대기 | 비채택 |
|---|---:|---:|---:|---:|---:|---:|
| Phase A. 공통 전투 기반 | 6 | 5 | 0 | 0 | 0 | 1 |
| Phase B. 자원·처형 | 2 | 0 | 1 | 0 | 1 | 0 |
| Phase C. 반응·회피 | 2 | 0 | 0 | 0 | 2 | 0 |
| Phase D. 특수 액션 | 2 | 0 | 0 | 0 | 2 | 0 |
| Phase E. UI·Debug | 2 | 0 | 0 | 0 | 2 | 0 |
| **합계** | **14** | **5** | **1** | **0** | **7** | **1** |

따라서 완료된 5개 항목과 비채택 R03을 제외하면, 현재 R07을 포함해 구현 대상으로 남은 상위 단계는 8개다. 각 상위 단계는 구현 전 프로젝트 조사 결과에 따라 하나 이상의 브랜치로 분할될 수 있다.

이 숫자는 작업량의 절대 시간 추정치가 아니라, 기능군의 남은 범위를 확인하기 위한 진행 지표다.

---

## 6. 브랜치별 문서화 규칙

각 상위 작업을 시작할 때 다음 순서로 문서를 구성한다.

```text
1. P02에서 해당 작업을 진행 상태로 변경
2. 프로젝트 전수조사
3. 필요한 Sxx 설계 문서 작성 또는 갱신
4. Wxx WorkPlan으로 구현·검증 단계 고정
5. 구현 및 검증
6. PR 문서에 실제 결과 기록
7. Merge 후 P02를 완료 처리하고 다음 작업 지정
```

P02에는 다음 내용만 갱신한다.

- 작업 상태
- 실제 수행 순서 변경
- 새로 발견된 상위 작업
- 상위 의존성 변경
- 해당 작업의 대표 `Sxx` / `Wxx` 링크

세부 API, 파일 목록, 상태 전이, 튜닝 정책은 이 문서에 옮기지 않는다.

---

## 7. 조회 방법

### 전체적으로 얼마나 남았는가

- `5. 남은 작업 규모` 확인

### 다음에 무엇을 해야 하는가

- `2. 현재 위치`와 `3. 전체 개발 순서` 확인

### 현재 브랜치에서 어떻게 구현해야 하는가

- 해당 행에 연결된 `Sxx` 및 `Wxx` 문서 확인

### 실제로 무엇을 변경하고 검증했는가

- 해당 작업의 Pull Request 문서 확인

---

## 8. 관련 문서

- [Documentation Index](../00_Documentation_Index.md)
- [System Architecture Index](../05_System_Architecture/00_System_Architecture_Index.md)
- [S35 Enemy Balance / Collapse Lifecycle](../05_System_Architecture/S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md)
- [S36 Execution Collaboration Architecture](../05_System_Architecture/S36_UE5_Portfolio_Execution_Collaboration_Architecture.md)
- [S31 Enemy Death / Presentation / Destroy](../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)
- [S33 Common Combat Target / Participation Architecture](../05_System_Architecture/S33_UE5_Portfolio_System_Architecture.md)
- [S34 Combat Participation Policy](../05_System_Architecture/S34_UE5_Portfolio_Combat_Participation_Policy.md)

---

## 9. 현재 결론

현재 프로젝트의 우선순위는 개별 전투 기능을 빠르게 추가하는 것이 아니라, 후속 기능이 공통으로 의존할 Target·Facing·Feedback·Death 기반을 먼저 고정하는 것이다.

```text
현재: R07 Resource 계층 및 R08 Execution Collaboration 구현 반영
→ S35의 Enemy Balance / Collapse lifecycle과 S36의 pair-session 계약을 기준으로 운영
→ Standard / Lethal outcome별 회귀 검증과 증적 캡처를 지속
그 이후: R09/R10 반응·회피 확장
후속: 특수 Action → UI/Debug
```

이 문서를 통해 현재 브랜치가 끝났을 때 다음 작업과 전체 잔여 범위를 바로 확인할 수 있어야 한다.
