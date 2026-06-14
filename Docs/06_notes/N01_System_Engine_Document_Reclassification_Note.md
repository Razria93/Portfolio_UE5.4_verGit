# System / Engine Document Reclassification Note

## 1. 목적

본 문서는 다음 브랜치에서 `System Architecture`, `Engine Technique Document`, `System / Engine Records`, `Portfolio Document`를 분리하기 위한 1차 분류표다.

현재 브랜치에서는 문서 파일명과 인덱스 정리까지만 수행했다.
System / Engine 역할 재분류, 파일 이동, 본문 재작성은 다음 브랜치에서 수행한다.

---

## 2. 분류 기준

```yaml
System Architecture
-> 현재 시스템 구조 / 책임 경계 / 실행 흐름 / 데이터 계약 설명

Engine Technique Document
-> Unreal Engine 기능 / API / 엔진 시스템 사용 방식 설명

System Design Records
-> 시스템 구조 설계 과정 / 구조 문제 / 책임 경계 이슈 / 선택 이유 기록

Engine Implementation Records
-> 엔진 기능 사용 결정 / 설정 / API 동작 이슈 / 엔진 사용상 문제 기록

Portfolio Document
-> 내부 문서를 기반으로 외부 독자에게 보여줄 포트폴리오용 해설 문서

Archive
-> 현재 기준으로 재사용 가치가 낮거나, 다른 문서에 흡수된 과거 판단 / 중간 산출물
```

---

## 3. 권장 폴더 구조 초안

```text
Docs/05_System_Architecture/
  00_System_Architecture_Index.md
  01_Current_System/
  02_System_Design_Records/
  archive/

Docs/06_Engine_Technique/
  00_Engine_Technique_Index.md
  01_Engine_Techniques/
  02_Engine_Implementation_Records/
  archive/

Docs/07_Portfolio_Documents/
  00_Portfolio_Document_Index.md
  PFxx_UE5_Portfolio_Document.md
  img/
  archive/
```

`Portfolio Document`는 `System Architecture` / `Engine Technique Document`와 같은 원천 문서 레벨이 아니라, 내부 문서를 바탕으로 만든 제출용 해설 문서로 둔다.

---

## 4. System Architecture 문서 1차 분류

EN / KR 쌍은 같은 문서 단위로 분류한다.

| ID  | 현재 문서 / 주제                                                             | 권장 분류                                      | 권장 위치                                             | 유지할 내용                                                          | 이동 / 흡수 후보                                            | Archive 후보                 | 사용자 판단 필요                            |
| --- | ---------------------------------------------------------------------- | ------------------------------------------ | ------------------------------------------------- | --------------------------------------------------------------- | ----------------------------------------------------- | -------------------------- | ------------------------------------ |
| S01 | AIStateComp 방식 vs BB-BT 방식 비교                                          | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | AI 상태 표현 방식 비교, 최종 구조 결정                                        | 현재 채택된 BB / BT 구조 요약은 Current System으로 흡수 가능          | 없음                         | 현재 AI 구조 문서에 얼마나 흡수할지                |
| S02 | Perception / Input / State Transition / Action Execution 구조            | Current System + 일부 Design Record          | `05_System_Architecture/01_Current_System`        | Player / Enemy 실행 구조 대응, perception / input / state / action 흐름 | `Problems`, `Design Principles`는 Design Record로 분리 후보 | 없음                         | 문제 분석 섹션을 유지할지 분리할지                  |
| S03 | Action Orchestration State Model                                       | Current System                             | `05_System_Architecture/01_Current_System`        | Action orchestration 상태 모델, 상태 전이 기준                            | 설계 판단 배경이 있으면 Design Record로 분리                       | 없음                         | 최신 action 구조와 일치 여부 확인 필요            |
| S04 | Action Orchestration Implementation Plan                               | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | 구현 계획, 작업 순서, 책임 분리 의도                                          | 완료된 현재 구조 요약은 Current System으로 흡수 가능                  | 완료된 계획 상세는 archive 후보      | 계획 문서를 유지할지 압축할지                     |
| S05 | AI Action Event Bridge                                                 | Current System                             | `05_System_Architecture/01_Current_System`        | AI 판단 결과가 action 실행으로 이어지는 bridge 구조                            | 구현 배경은 Design Record 후보                               | 없음                         | P15 이후 구조와 용어 정합성 확인 필요              |
| S06 | Reaction Orchestration Core / Shared Reaction Execution Pipeline       | System Design Record + Current System 후보   | `05_System_Architecture/02_System_Design_Records` | Reaction execution pipeline 설계 기준                               | 현재 reaction execution 구조는 Current System으로 흡수 후보      | 과거 decision 표현은 archive 후보 | EN/KR 파일명이 서로 다른 점 정리 필요             |
| S07 | Reaction Pending 제거 / AI BehaviorTree 역할 재정의                           | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | pending 제거 이유, AI 관찰 책임 재정의                                     | 현재 BehaviorTree 역할 요약은 Current System 후보              | 없음                         | EN/KR 파일명 정합성 정리 필요                  |
| S08 | Reaction execution policy / local-level orchestration design           | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | reaction 실행 정책과 orchestration level 설계 판단                       | 최신 reaction policy 요약은 Current System 후보              | 없음                         | 최신 P16 구조와 비교 필요                     |
| S09 | Combat Feedback 계층 구성                                                  | Current System + Design Record             | `05_System_Architecture/01_Current_System`        | Combat feedback 계층, shared / local feedback 책임                  | 도입 이유와 후속 안내는 Design Record 또는 archive 후보             | 후속 구조 안내가 오래됐으면 archive 후보 | P14/P16 feedback 기준과 대조 필요           |
| S10 | Action Orchestration 이전 Player / AI Action 실행 흐름 비대칭 분석                | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | 기존 player / AI 실행 비대칭 분석                                        | 현재 구조 설명은 S11 이후 Current System으로 흡수                  | 분석 완료 문서는 archive 후보 가능    | 유지 가치 판단 필요                          |
| S11 | Action Request Entry와 Execution Pipeline 구조 결정                         | Current System + Design Record             | `05_System_Architecture/01_Current_System`        | Action request entry, execution pipeline 구조                     | 결정 배경은 Design Record 후보                               | 없음                         | P15 정식 PR 문서와 용어 맞춤 필요               |
| S12 | Action Orchestrator 내부 Request 처리 흐름 결정                                | Current System + Design Record             | `05_System_Architecture/01_Current_System`        | Action orchestrator request 처리 흐름                               | decision 배경은 Design Record 후보                         | 없음                         | 현재 코드와 request result 값 대조 필요        |
| S13 | Action Execution Decision 구조 도입                                        | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | execution decision 도입 이유와 책임                                    | 현재 decision 구조 요약은 Current System 후보                  | 없음                         | P15/P17 이후 용어 변화 반영 필요               |
| S14 | Action Competition Arbitration 도입 필요성과 후속 방향                           | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | action 경쟁 / arbitration 필요성                                     | 후속 방향은 archive 또는 Backlog 후보                          | 오래된 후속 방향은 archive 후보      | 최신 intervention 구조와 중복 여부 확인         |
| S15 | 1차 Action Orchestration 결과와 2차 리팩터링 필요성                                | System Design Record / Archive 후보          | `05_System_Architecture/02_System_Design_Records` | 1차 결과, 2차 리팩터링 필요성                                              | 확정된 현재 구조는 S16~S18 또는 Current System으로 흡수             | 회고성 내용은 archive 후보         | History로 둘지 Record로 둘지               |
| S16 | 1차 Action Orchestration 리팩터링 시행착오와 구조적 결론                              | System Design Record / Archive 후보          | `05_System_Architecture/02_System_Design_Records` | 시행착오, 구조적 결론                                                    | 결론만 Design Record에 남기고 과정은 archive 후보                 | 시행착오 상세는 archive 후보        | History 체계 생성 여부                     |
| S17 | Damage Feedback과 Reaction Feedback 책임 재정의                              | Current System + Design Record             | `05_System_Architecture/01_Current_System`        | DamageFeedback / ReactionFeedback 책임 경계                         | 재정의 이유는 Design Record 후보                              | 없음                         | P14/P16 용어 기준과 대조 필요                 |
| S18 | Action Orchestration Refactor의 Decision / Intervention 모델 전환           | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | decision / intervention 모델 전환 이유                                | 현재 intervention 모델 요약은 Current System 후보              | 없음                         | P17과 통합 여부 검토 필요                     |
| S19 | Action / Reaction 실행 대칭화 구현 계획                                         | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | action / reaction 실행 대칭화 구현 계획                                  | 완료된 대칭 구조는 Current System으로 흡수 후보                     | 완료된 작업 순서는 archive 후보      | 구현 완료 여부 기준 확인 필요                    |
| S20 | Execution Layer Responsibility Decision                                | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | decision / intervention / component / executor 책임 경계            | 확정 책임 표는 Current System 후보                            | 없음                         | P17 문서와 중복 정리 필요                     |
| S21 | Execution Context Snapshot Decision                                    | Current System + Design Record             | `05_System_Architecture/01_Current_System`        | snapshot 책임과 실행 시점 context 구성                                   | decision 이유는 Design Record 후보                         | 없음                         | 코드 기준 최신 snapshot 필드 확인 필요           |
| S22 | Execution Relationship Decision                                        | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | active / incoming 관계, request 처리 관계 판단                          | 현재 relationship 모델은 Current System 후보                 | 없음                         | P17 intervention 용어와 정합성 확인          |
| S23 | Execution Intervention Directive Decision                              | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | directive 구조와 중단 지시값 판단                                        | 현재 directive 구조는 Current System 후보                    | 없음                         | P17 directive 설명과 중복 정리 필요            |
| S24 | Interrupt Unification Plan                                            | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | cancel / interrupt 결과 통합 판단                                     | 현재 stop reason 기준은 Current System 후보                  | 시행착오 섹션은 archive 후보        | P17 interrupt / interrupted 용어와 정합성 확인 |
| S25 | Execution Intervention Policy Decision                                 | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | intervention policy와 기본 정책 판단                                   | 현재 policy 구조는 Current System 후보                       | 시행착오 섹션은 archive 후보        | P17과 중복 정리 필요                         |
| S26 | Execution Montage Lifecycle Decision                                   | System Design Record + Engine Technique 후보 | `05_System_Architecture/02_System_Design_Records` | montage lifecycle 책임 분리 판단                                      | montage lifecycle 사용 방식은 Engine Technique 후보          | 시행착오 섹션은 archive 후보        | System / Engine 경계 판단 필요             |
| S27 | Combat Resolution Responsibility Decision                              | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | combat resolution 위치 / 책임 / dispatch 판단                         | 현재 combat resolution 구조는 Current System 후보            | 예시 API가 오래됐으면 archive 후보   | 아직 구현 전 계획인지 확인 필요                   |
| S28 | Execution Intervention Key Window Model                                | System Design Record + Engine Technique 후보 | `05_System_Architecture/02_System_Design_Records` | key window model, notify authoring model                        | notify authoring 세부는 Engine Technique 후보              | 시행착오 섹션은 archive 후보        | Notify model을 Engine Technique로 분리할지 |
| S29 | Execution Intervention Policy / Gate Refactor                          | System Design Record                       | `05_System_Architecture/02_System_Design_Records` | policy / gate 분리, executor 판단 흐름                                | 확정된 policy / gate 구조는 Current System 후보               | 구현 순서 제안은 archive 후보       | 다음 구현 브랜치와 연결 방식 확인                  |

---

## 5. Portfolio Documents 1차 분류

| ID | 현재 문서 / 주제 | 권장 분류 | 권장 위치 | 유지할 내용 | 이동 / 흡수 후보 | Archive 후보 | 사용자 판단 필요 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| PF00 | Portfolio Overview | Portfolio Document | `07_Portfolio_Documents` | 프로젝트 개요, 링크, 문서 목록 | 내부 문서 인덱스 성격은 Documentation Index와 연결 | 없음 | 없음 |
| PF01 | Project Summary | Portfolio Document | `07_Portfolio_Documents` | 기술 스택, 구현 기능, 핵심 설계 포인트 요약 | 시스템 구조 상세는 System Architecture로 흡수 후보 | 오래된 Troubleshooting 요약은 별도 문서 후보 | 제출용 문서로 유지할 압축 수준 |
| PF02 | Combat Data Pipeline | Portfolio Document + System Architecture 흡수 후보 | `07_Portfolio_Documents` | 포트폴리오용 combat data pipeline 해설 | 책임 경계, ApplyDamage / TakeDamage / Feedback / Reaction 흐름은 System Architecture 후보 | 문제 정의 / 1차 프로젝트 비교는 Record 또는 archive 후보 | 시스템 문서로 끌고 올 범위 |
| PF03 | Action & Reaction Execution | Portfolio Document + System Architecture 흡수 후보 | `07_Portfolio_Documents` | action / reaction execution pipeline 해설 | request / orchestrator / component / executor 구조는 System Architecture 후보 | 문제 정의 / 재구성 서술은 Record 후보 | P15~P17 구조와 통합 범위 |
| PF04 | Enemy AI Combat Behavior | Portfolio Document + System Architecture 흡수 후보 | `07_Portfolio_Documents` | Enemy AI combat behavior 해설 | BehaviorTree / Blackboard / AIState 흐름은 System Architecture 후보 | 문제 정의는 Record 후보 | Engine Technique와 System Architecture 경계 |
| PF05 | Data-Driven Design | Portfolio Document + Engine Technique 후보 | `07_Portfolio_Documents` | data-driven design 포트폴리오 해설 | DataAsset / Data Container / key resolve / fallback lookup은 Engine Technique 후보 | 남은 과제는 Backlog 후보 | Engine Technique로 분리할 세부 범위 |
| PF06 | Troubleshooting | Portfolio Document / Issue Summary 후보 | `07_Portfolio_Documents` 또는 별도 `Troubleshooting Summary` | 제출용 troubleshooting 사례 요약 | 개별 재현/원인/수정/검증은 Bug Report / Issue Report 후보 | 오래된 사례는 archive 후보 | Portfolio에 유지할 사례 수준 |
| PF07 | AI-Assisted Workflow | Portfolio Document + AI Workflow 후보 | `07_Portfolio_Documents` | AI 활용 개발 흐름의 외부 설명 | 운영 세부는 `Docs/08_AI_Workflow`와 연결 | 중복된 운영 설명은 archive 후보 | Portfolio용 요약과 내부 Workflow 분리 정도 |

---

## 6. 1차 사용자 판단 필요 항목

1. `System Design Records`를 `05_System_Architecture` 하위에 둘지, 별도 `06_System_Design_Records`로 둘지 결정한다.
2. `Engine Technique Document`를 새 `Docs/06_Engine_Technique`로 만들지, 기존 번호 체계를 조정할지 결정한다.
3. `S26`, `S28`처럼 Unreal notify / montage authoring 성격이 섞인 문서를 System Record와 Engine Technique 중 어디에 둘지 결정한다.
4. `PF02~PF05`의 시스템 구조 설명을 새 System Architecture 문서에 어느 정도까지 흡수할지 결정한다.
5. `PF06 Troubleshooting`을 Portfolio 문서로 유지할지, Troubleshooting Summary 또는 Issue Summary로 분리할지 결정한다.
6. `History` 문서 체계를 이번 재구성에 포함할지, Record / Archive 조합으로 대체할지 결정한다.

---

## 7. 다음 작업 제안

1. 본 분류표를 사용자와 검토해 System / Engine / Portfolio 문서 역할을 확정한다.
2. 확정된 역할에 맞춰 파일 이동 / 본문 역할 보정 대상 목록을 만든다.
3. 이동 전 현재 생성된 `Docs/00_Documentation_Index.md`와 문서군별 Index의 갱신 범위를 확인한다.
4. 이후 파일 이동 / 본문 역할 보정 / index 갱신 / prompt 보완을 순차 진행한다.
