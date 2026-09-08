# 문서상 p.24 AI Workflow 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.24에 적용한다.

## 1. 페이지 목적

이 페이지는 AI가 구현을 완성했다는 사례가 아니라, AI 제안·초안을 **Work Brief·개발자 수용 판단·검증 기록**으로 통제하는 작업 운영 방식을 설명한다. Git 이력과 변경 추적은 p.25 Traceability에서 다룬다.

## 2. 문서 감사 결과

1. `AI_Work_Pipeline`은 목표·현재 구조 탐색·계획·구조 제안·적용·검증·문서화 단계를 정의하지만, 모든 작업이 항상 전체 단계를 일렬로 통과한다고 규정하지 않는다.
2. Work Brief는 목표·성공 기준·비범위를 먼저 정리하고, 현재 코드·문서·Asset 맥락은 AI 제안 전 조사 입력으로 사용한다.
3. `AI_Workflow_Operation_Guide`는 AI를 탐색·계획·초안 보조자로, 개발자를 수용 판단·변경 범위·최종 검증 책임자로 구분한다.
4. Build·Code Flow·PIE·Editor·Asset 검증은 유형별로 분리하며, 수행하지 못한 항목은 미검증 사유와 다음 조치로 남긴다.
5. 현재 동일 Work ID의 Brief·Review / Decision·Verification Record를 한 화면에서 대조할 캡처는 없다. Prompt 템플릿이나 일반 Pipeline 문서는 실사용 증거로 전용하지 않는다.

## 3. 증거와 표현 범위

| 증거 | 확인 가능한 내용 | 페이지 사용 범위 |
| --- | --- | --- |
| `Docs/08_AI_Workflow` Overview·Work Pipeline·Operation Guide | 역할 분리, 단계·검증·미검증 기록 원칙 | 운영 구조와 사용 범위의 문서 근거 |
| 동일 Work ID 문서 보드 | Work Brief, Review / Decision, Verification Record의 실제 연결 | p.24 중심 실사용 검증 — 신규 캡처 전 `Needs Capture` |

## 4. 금지 표현

- AI가 구현·검증·최종 판단을 대체하거나 완성했다고 표현하지 않는다.
- Prompt 템플릿 또는 일반 절차 문서만으로 특정 작업의 실사용을 증명하지 않는다.
- 수행하지 않은 Build·PIE·Editor·Asset 검증을 완료 또는 성공으로 표현하지 않는다.
- Work Pipeline을 모든 작업에 강제되는 고정 직선 절차로 표현하지 않는다.

## 5. 페이지 구성

1. **요구조건·의도**: AI 제안과 검증된 결과를 분리해야 하는 이유, Work Brief의 목표·비범위·수용 기준 역할을 둔다.
2. **운영 구조**: `User Request → Work Brief → Context Inspection → AI Proposal → Developer Review / Decision → Apply → Verification by Type → Verified / Unverified → Record`의 사용자 제작 다이어그램 슬롯을 둔다. AI Proposal에서 Apply로 직결하지 않는다.
3. **실사용 검증**: 동일 Work ID의 Brief·Review / Decision·Verification Record를 한 문서 보드로 대조하는 슬롯을 둔다.
4. **사용 범위**: AI 역할, 개발자 책임, 완료로 쓰지 않는 미검증 결과를 3칸으로 고정한다.

## 6. 검수 상태

- 문서 감사 및 HTML 구성: 완료 — 요구조건·의도 → 운영 구조 → 실사용 검증 → 사용 범위 형식으로 전환
- 증거 상태: Needs Capture — 동일 Work ID 문서 보드와 중앙 다이어그램 사용자 제작 필요
- PDF 레이아웃 검증: 보완 후 별도 수행 필요
