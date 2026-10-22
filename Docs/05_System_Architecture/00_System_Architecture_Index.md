# Project Stellar 시스템 구조 안내

[대표 README](../../README.md) · [목적별 문서 안내](../00_Documentation_Index.md) · [문제 해결 사례](../03_Issue_Analysis_Report/00_Issue_Analysis_Report_Index.md)

현재 동작을 이해하려면 아래 순서에서 관심 주제를 선택하세요. 과거 설계·리팩터링 기록은 하단에 보존하며, 현행 구조와 구분합니다.

## 추천 열람 순서

| 순서 | 질문 | 읽을 문서 |
| --- | --- | --- |
| 1 | 공격·피격이 겹칠 때 실행을 어떻게 전환하는가? | [S37 — 실행 판단·전환·정리](S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md) |
| 2 | 피해 사실 전달과 대상의 결과 판정은 어디서 나뉘는가? | [S38 — Combat Signal](S38_UE5_Portfolio_Combat_Signal_Architecture.md) |
| 3 | 대상 변경과 적의 참여는 어떻게 연결되는가? | [S33 — 공통 타깃](S33_UE5_Portfolio_System_Architecture.md) → [S34 — 참여 정책](S34_UE5_Portfolio_Combat_Participation_Policy.md) |
| 4 | 무너짐에서 처형·회복·사망으로 어떻게 이어지는가? | [S35 — Balance·Collapse](S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md) → [S36 — 처형 협업](S36_UE5_Portfolio_Execution_Collaboration_Architecture.md) → [S31 — 사망 생명주기](S31_UE5_Portfolio_System_Architecture.md) |
| 5 | 무기 소켓 전환과 손잡이 기준 회전을 어떻게 분리했는가? | [I04 — 문제 분석](../03_Issue_Analysis_Report/I04_UE5_Portfolio_Issue_Analysis_Report.md) → [S39 — 표현 좌표계](S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md) |

이동 적용까지 살펴보려면 [일반 Hit 넉백·공격 시작 방향 보정 검토 가이드](../07_Portfolio_Documents/Portfolio_Production/37_Gameplay_Knockback_and_Facing_Review_Guide.md)를 이어서 읽습니다. 코드에서 시작하려면 [README 코드 지도](../../README.md#code-guide)를 이용하세요.

## 현재 시스템 문서

아래 분류는 기존 인덱스에서 현행 구조로 지정한 문서를 유지한 것입니다. 구현 상태와 런타임 검증·제출 캡처 확보 상태는 각 문서의 설명을 따릅니다. S37은 제출용 전환 캡처 대기, S36은 결과별 회귀·캡처 보완 상태입니다.

| ID | 제목 | 문서 | 현재 역할 | 분류 | 비고 |
| --- | --- | --- | --- | --- | --- |
| S31 | Enemy Dead / Presentation / Destroy 생명주기 설계 | [문서](S31_UE5_Portfolio_System_Architecture.md) | 구현·에셋·PIE와 동기화된 최신 계약 | Current System | Enemy Dead 구조 리뷰의 단일 기준 |
| S33 | 공통 Combat Target Kernel 및 의사결정 통합 설계 | [문서](S33_UE5_Portfolio_System_Architecture.md) | Player/Enemy 공통 Combat Target 최신 구조와 구현 Goal 상태 | Current System | Enemy Participation 세부 정책은 S34 참조 |
| S34 | Combat Participation Policy | [문서](S34_UE5_Portfolio_Combat_Participation_Policy.md) | Enemy Evidence, shared allocator, Assignment lifecycle, HitReactive/ExtraSlot 정책 | Current System | Evidence-centric runtime contract |
| S35 | Enemy Balance / Collapse Lifecycle 설계 | [문서](S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md) | Enemy Balance Count, Collapse In / Loop / Out, TTL, Count lock 및 Standard Execution의 Balance extension 경계 | Current System | R07 자연 Collapse lifecycle 기준. Execution의 최종 계약은 S36 참조 |
| S36 | Execution Collaboration Architecture | [문서](S36_UE5_Portfolio_Execution_Collaboration_Architecture.md) | Source/Target pair session, Standard Execution Down/Recovery, Lethal Death presentation handoff | Current System | 구현·authoring 반영 완료. outcome별 회귀 검증 기준 |
| S37 | Execution Transition and Intervention Architecture | [문서](S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md) | Action/Reaction 공통 Decision·Relationship·ApplyMode 계약 | Current System | 전환·개입 정책 경계와 runtime evidence 제한 |
| S38 | Combat Signal Architecture | [문서](S38_UE5_Portfolio_Combat_Signal_Architecture.md) | Source 사실 전달과 Target Outcome 확정 경계 | Current System | 일반 Signal과 Execution Pair의 책임 분리 |
| S39 | Weapon Presentation Pivot Architecture | [문서](S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md) | 캐릭터 장착 좌표와 무기 HandGrip 기준 Presentation 회전의 분리·합성 계약 | Current System | Pivot 수학, Action Pose Scope, 소켓 전환, 회전 모드, authoring 및 검증 기준 |

<details>
<summary>과거 설계·리팩터링 기록과 현행 구조 편입 검토 대상</summary>

당시 판단과 변경 과정을 보존합니다. `Current System 후보` 또는 `분리 후보`는 최신 코드와 대조가 필요한 기록이며, 위 현행 문서와 같은 확정 상태가 아닙니다. 기존 분류 후보와 비고를 유지합니다.

| ID | 제목 | 문서 | 현재 역할 | 분류 | 비고 |
| --- | --- | --- | --- | --- | --- |
| S01 | AIStateComp 방식 vs BB-BT 방식 비교 | [문서](S01_UE5_Portfolio_System_Architecture.md) | 설계 비교 | System Design Records | 현재 AI 구조 문서에 일부 흡수 후보 |
| S02 | 인지 / 입력 / 상태변환 / 액션 실행 구조 정리 | [문서](S02_UE5_Portfolio_System_Architecture.md) | 구조 설명 + 설계 원칙 | Current System / Records 분리 후보 | 문제 분석 섹션 분리 검토 |
| S03 | 액션 오케스트레이션 상태 모델 | [문서](S03_UE5_Portfolio_System_Architecture.md) | 구조 모델 | Current System 후보 | 최신 코드 정합성 확인 필요 |
| S04 | 액션 오케스트레이션 구현 계획 | [문서](S04_UE5_Portfolio_System_Architecture.md) | 구현 계획 | System Design Records | 완료 계획 일부 archive 후보 |
| S05 | AI 액션 이벤트 브리지 구조 | [문서](S05_UE5_Portfolio_System_Architecture.md) | 구조 설명 | Current System 후보 | P15 이후 구조와 대조 필요 |
| S06 | Shared Reaction Execution Pipeline 설계 | [문서](S06_UE5_Portfolio_System_Architecture.md) | 설계 기록 + 구조 후보 | System Design Records | 현재 reaction 구조 요약 흡수 후보 |
| S07 | Reaction Pending 제거와 AI BehaviorTree 역할 재정의 | [문서](S07_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S08 | Reaction Local Level과 Orchestration Level 역할 분리 | [문서](S08_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S09 | Combat Feedback 계층 구성 | [문서](S09_UE5_Portfolio_System_Architecture.md) | 구조 설명 + 설계 기록 | Current System / Records 분리 후보 | P14/P16 기준 대조 필요 |
| S10 | Action Orchestration 이전 Player / AI Action 실행 흐름 비대칭 분석 | [문서](S10_UE5_Portfolio_System_Architecture.md) | 문제 분석 | System Design Records |  |
| S11 | Action Request Entry와 Execution Pipeline 구조 결정 | [문서](S11_UE5_Portfolio_System_Architecture.md) | 구조 결정 | Current System / Records 분리 후보 |  |
| S12 | Action Orchestrator 내부 Request 처리 흐름 결정 | [문서](S12_UE5_Portfolio_System_Architecture.md) | 구조 결정 | Current System / Records 분리 후보 |  |
| S13 | Action Execution Decision 구조 도입 | [문서](S13_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S14 | Action Competition Arbitration 도입 필요성과 후속 방향 | [문서](S14_UE5_Portfolio_System_Architecture.md) | 설계 방향 | System Design Records / Archive 후보 | 최신 intervention 구조와 중복 확인 |
| S15 | 1차 Action Orchestration 결과와 2차 리팩터링 필요성 | [문서](S15_UE5_Portfolio_System_Architecture.md) | 회고 / 필요성 | System Design Records / Archive 후보 |  |
| S16 | 1차 Action Orchestration 리팩터링 시행착오와 구조적 결론 | [문서](S16_UE5_Portfolio_System_Architecture.md) | 회고 / 결론 | System Design Records / Archive 후보 |  |
| S17 | Damage Feedback과 Reaction Feedback 책임 재정의 | [문서](S17_UE5_Portfolio_System_Architecture.md) | 책임 재정의 | System Design Records |  |
| S18 | Action Orchestration Refactor의 Decision / Intervention 모델 전환 | [문서](S18_UE5_Portfolio_System_Architecture.md) | 구조 전환 기록 | System Design Records |  |
| S19 | 액션 / 리액션 실행 대칭화 구현 계획 | [문서](S19_UE5_Portfolio_System_Architecture.md) | 구현 계획 | System Design Records |  |
| S20 | 실행 계층 책임 분리 결정 | [문서](S20_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S21 | 실행 컨텍스트 / 스냅샷 책임 결정 | [문서](S21_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S22 | 실행 관계 분류 결정 | [문서](S22_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S23 | 실행 개입 디렉티브 구조 결정 | [문서](S23_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S24 | Intervention Rule / Interrupt 통합 리팩터링 계획 | [문서](S24_UE5_Portfolio_System_Architecture.md) | 리팩터링 계획 | System Design Records |  |
| S25 | 실행 개입 정책 결정 | [문서](S25_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records |  |
| S26 | 실행 몽타주 생명주기 결정 | [문서](S26_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records / Engine Technique 후보 | Montage lifecycle 설명 분리 검토 |
| S27 | 전투 판정 책임 분리 결정 | [문서](S27_UE5_Portfolio_System_Architecture.md) | 설계 결정 | System Design Records | Combat Resolution 도입 전 기록 |
| S28 | 실행 개입 키 윈도우 모델 | [문서](S28_UE5_Portfolio_System_Architecture.md) | 구조 모델 | System Design Records / Engine Technique 후보 | Notify window 설명 분리 검토 |
| S29 | 실행 개입 정책 / 게이트 리팩터링 | [문서](S29_UE5_Portfolio_System_Architecture.md) | 리팩터링 방향 | System Design Records |  |
| S32 | 공통 Combat Target 상태 및 의사결정 경계 설계 | [문서](S32_UE5_Portfolio_System_Architecture.md) | Player/Enemy 공통 Target SoT, Perception/BT/Blackboard 경계와 단계별 마이그레이션 | Historical Design | 구현 전 경계 설계 기록. 현재 구현 계약은 S33, Enemy Evidence 정책은 S34 참조 |

</details>
