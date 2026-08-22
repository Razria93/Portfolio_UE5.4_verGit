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
→ Enemy Death Presentation / Destroy Lifecycle

현재 진행
→ R01. 공통 Combat Target 상태 기반

다음 작업
→ R02. 피격 기반 Enemy Engage 진입
```

현재 브랜치:

```text
feature/combat-target-provider
```

현재 작업의 상세 기준 문서:

- [S32 Common Combat Target Architecture](../05_System_Architecture/S32_UE5_Portfolio_System_Architecture.md)
- 브랜치에서 별도 생성하는 Combat Target Migration WorkPlan

---

## 3. 전체 개발 순서

### Phase A. 공통 전투 기반 정리

이 단계는 Player와 Enemy가 같은 전투 계약을 사용할 수 있도록 Target, Facing, Feedback, Death 경계를 정리한다.

| ID | 작업 | 상태 | 핵심 결과 |
|---|---|---|---|
| R01 | 공통 Combat Target 상태 기반 | 진행 | Player/Enemy의 전투 대상 SoT와 수명 계약을 Character 공통 컴포넌트로 통합 |
| R02 | 피격 기반 Enemy Engage 진입 | 다음 | 피격한 공격자를 유효한 전투 대상으로 반영하고 Engage 유지 |
| R03 | Action 구간 기반 Facing 보정 | 대기 | 공격 초반에는 목표를 부드럽게 추적하고 타격 구간에서는 방향 고정 |
| R04 | Enemy Focus 및 8Way 이동 정리 | 대기 | Alert/Engage 이동과 공격 방향이 현재 전투 대상에 일치 |
| R05 | Player/Enemy 컴포넌트 계약 대칭화 | 대기 | Targeting·Feedback 등 한쪽에만 존재하는 책임을 공통 계약으로 정리 |
| R06 | Enemy Death Lifecycle 컴포넌트화 검토·적용 | 대기 | Character에 집중된 사망 생명주기 책임의 독립 여부 확정 |

Phase A 완료 기준:

```text
Player와 Enemy 모두 동일한 Target 계약 사용
→ 피격·AI 의도·플레이어 락온이 일관된 대상 문맥으로 수렴
→ Action이 필요한 구간에서만 회전 보정
→ 이동·공격·피격 후 전투태세가 같은 Target을 기준으로 동작
→ Feedback와 Death 책임 경계가 양측 캐릭터에서 일관됨
```

### Phase B. 전투 자원과 처형 기반

Phase A의 공통 대상 및 실행 계약이 안정된 뒤 전투 자원과 협업 실행을 확장한다.

| ID | 작업 | 상태 | 핵심 결과 |
|---|---|---|---|
| R07 | Resource 계층 정리 | 대기 | Enemy Balance와 Player Beta/Burst 자원의 소유·이벤트 계약 확정 |
| R08 | Execution 시스템 | 대기 | 일반/즉사 처형의 Action·Reaction 협업 실행 |

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

| 구간 | 전체 | 진행 | 다음 | 대기 |
|---|---:|---:|---:|---:|
| Phase A. 공통 전투 기반 | 6 | 1 | 1 | 4 |
| Phase B. 자원·처형 | 2 | 0 | 0 | 2 |
| Phase C. 반응·회피 | 2 | 0 | 0 | 2 |
| Phase D. 특수 액션 | 2 | 0 | 0 | 2 |
| Phase E. UI·Debug | 2 | 0 | 0 | 2 |
| **합계** | **14** | **1** | **1** | **12** |

즉 현재 브랜치를 포함해 14개의 상위 단계가 남아 있으며, 현재 작업 완료 후에는 13개의 후속 단계가 남는다. 각 상위 단계는 구현 전 프로젝트 조사 결과에 따라 하나 이상의 브랜치로 분할될 수 있다.

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
- [S30 Balance / Collapse / Executionable](../05_System_Architecture/S30_UE5_Portfolio_System_Architecture.md)
- [S31 Enemy Death / Presentation / Destroy](../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)
- [S32 Common Combat Target Architecture](../05_System_Architecture/S32_UE5_Portfolio_System_Architecture.md)

---

## 9. 현재 결론

현재 프로젝트의 우선순위는 개별 전투 기능을 빠르게 추가하는 것이 아니라, 후속 기능이 공통으로 의존할 Target·Facing·Feedback·Death 기반을 먼저 고정하는 것이다.

```text
현재: R01 Combat Target Foundation
다음: R02 Hit Engage
그 이후: Facing → Focus/8Way → Contract Symmetry → Death Boundary
후속: Resource → Execution → Hit/Dodge → Special Actions → UI/Debug
```

이 문서를 통해 현재 브랜치가 끝났을 때 다음 작업과 전체 잔여 범위를 바로 확인할 수 있어야 한다.
