# System Architecture Index

이 문서는 System Architecture 문서의 식별 제목과 다음 분류 후보를 관리한다.
각 문서의 현재 역할은 표의 `다음 분류 후보` 열로 관리한다. `Current System`은 현재 runtime 계약의
정규 기준이고, `System Design Records`와 `Archive`는 당시의 설계·검증 기록으로 보존한다.

---

| ID | 제목 | 파일 | 현재 역할 | 다음 분류 후보 | 비고 |
| --- | --- | --- | --- | --- | --- |
| S01 | AIStateComp 방식 vs BB-BT 방식 비교 | `S01_UE5_Portfolio_System_Architecture.md` | 설계 비교 | System Design Records | 현재 AI 구조 문서에 일부 흡수 후보 |
| S02 | 인지 / 입력 / 상태변환 / 액션 실행 구조 정리 | `S02_UE5_Portfolio_System_Architecture.md` | 구조 설명 + 설계 원칙 | Current System / Records 분리 후보 | 문제 분석 섹션 분리 검토 |
| S03 | 액션 오케스트레이션 상태 모델 | `S03_UE5_Portfolio_System_Architecture.md` | 구조 모델 | Current System 후보 | 최신 코드 정합성 확인 필요 |
| S04 | 액션 오케스트레이션 구현 계획 | `S04_UE5_Portfolio_System_Architecture.md` | 구현 계획 | System Design Records | 완료 계획 일부 archive 후보 |
| S05 | AI 액션 이벤트 브리지 구조 | `S05_UE5_Portfolio_System_Architecture.md` | 구조 설명 | Current System 후보 | P15 이후 구조와 대조 필요 |
| S06 | Shared Reaction Execution Pipeline 설계 | `S06_UE5_Portfolio_System_Architecture.md` | 설계 기록 + 구조 후보 | System Design Records | 현재 reaction 구조 요약 흡수 후보 |
| S07 | Reaction Pending 제거와 AI BehaviorTree 역할 재정의 | `S07_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S08 | Reaction Local Level과 Orchestration Level 역할 분리 | `S08_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S09 | Combat Feedback 계층 구성 | `S09_UE5_Portfolio_System_Architecture.md` | 구조 설명 + 설계 기록 | Current System / Records 분리 후보 | P14/P16 기준 대조 필요 |
| S10 | Action Orchestration 이전 Player / AI Action 실행 흐름 비대칭 분석 | `S10_UE5_Portfolio_System_Architecture.md` | 문제 분석 | System Design Records |  |
| S11 | Action Request Entry와 Execution Pipeline 구조 결정 | `S11_UE5_Portfolio_System_Architecture.md` | 구조 결정 | Current System / Records 분리 후보 |  |
| S12 | Action Orchestrator 내부 Request 처리 흐름 결정 | `S12_UE5_Portfolio_System_Architecture.md` | 구조 결정 | Current System / Records 분리 후보 |  |
| S13 | Action Execution Decision 구조 도입 | `S13_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S14 | Action Competition Arbitration 도입 필요성과 후속 방향 | `S14_UE5_Portfolio_System_Architecture.md` | 설계 방향 | System Design Records / Archive 후보 | 최신 intervention 구조와 중복 확인 |
| S15 | 1차 Action Orchestration 결과와 2차 리팩터링 필요성 | `S15_UE5_Portfolio_System_Architecture.md` | 회고 / 필요성 | System Design Records / Archive 후보 |  |
| S16 | 1차 Action Orchestration 리팩터링 시행착오와 구조적 결론 | `S16_UE5_Portfolio_System_Architecture.md` | 회고 / 결론 | System Design Records / Archive 후보 |  |
| S17 | Damage Feedback과 Reaction Feedback 책임 재정의 | `S17_UE5_Portfolio_System_Architecture.md` | 책임 재정의 | System Design Records |  |
| S18 | Action Orchestration Refactor의 Decision / Intervention 모델 전환 | `S18_UE5_Portfolio_System_Architecture.md` | 구조 전환 기록 | System Design Records |  |
| S19 | 액션 / 리액션 실행 대칭화 구현 계획 | `S19_UE5_Portfolio_System_Architecture.md` | 구현 계획 | System Design Records |  |
| S20 | 실행 계층 책임 분리 결정 | `S20_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S21 | 실행 컨텍스트 / 스냅샷 책임 결정 | `S21_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S22 | 실행 관계 분류 결정 | `S22_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S23 | 실행 개입 디렉티브 구조 결정 | `S23_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S24 | Intervention Rule / Interrupt 통합 리팩터링 계획 | `S24_UE5_Portfolio_System_Architecture.md` | 리팩터링 계획 | System Design Records |  |
| S25 | 실행 개입 정책 결정 | `S25_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records |  |
| S26 | 실행 몽타주 생명주기 결정 | `S26_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records / Engine Technique 후보 | Montage lifecycle 설명 분리 검토 |
| S27 | 전투 판정 책임 분리 결정 | `S27_UE5_Portfolio_System_Architecture.md` | 설계 결정 | System Design Records | Combat Resolution 도입 전 기록 |
| S28 | 실행 개입 키 윈도우 모델 | `S28_UE5_Portfolio_System_Architecture.md` | 구조 모델 | System Design Records / Engine Technique 후보 | Notify window 설명 분리 검토 |
| S29 | 실행 개입 정책 / 게이트 리팩터링 | `S29_UE5_Portfolio_System_Architecture.md` | 리팩터링 방향 | System Design Records |  |
| S31 | Enemy Dead / Presentation / Destroy 생명주기 설계 | `S31_UE5_Portfolio_System_Architecture.md` | 구현·에셋·PIE와 동기화된 최신 계약 | Current System | Enemy Dead 구조 리뷰의 단일 기준 |
| S32 | 공통 Combat Target 상태 및 의사결정 경계 설계 | `S32_UE5_Portfolio_System_Architecture.md` | Player/Enemy 공통 Target SoT, Perception/BT/Blackboard 경계와 단계별 마이그레이션 | Historical Design | 구현 전 경계 설계 기록. 현재 구현 계약은 S33, Enemy Evidence 정책은 S34 참조 |
| S33 | 공통 Combat Target Kernel 및 의사결정 통합 설계 | `S33_UE5_Portfolio_System_Architecture.md` | Player/Enemy 공통 Combat Target 최신 구조와 구현 Goal 상태 | Current System | Enemy Participation 세부 정책은 S34 참조 |
| S34 | Combat Participation Policy | `S34_UE5_Portfolio_Combat_Participation_Policy.md` | Enemy Evidence, shared allocator, Assignment lifecycle, HitReactive/ExtraSlot 정책 | Current System | Evidence-centric runtime contract |
| S35 | Enemy Balance / Collapse Lifecycle 설계 | `S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md` | Enemy Balance Count, Collapse In / Loop / Out, TTL, Count lock 및 미래 Execution 경계 | Current System (C++ 구현 완료; asset/PIE 검증 대기) | R07 구현 기준. 과거 S30은 설계 근거로만 참조 |

---
