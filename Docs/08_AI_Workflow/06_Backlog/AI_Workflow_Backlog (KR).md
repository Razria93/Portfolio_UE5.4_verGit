# AI Workflow Backlog

## 1. 목적

본 문서는 AI Workflow 문서와 Prompt Library의 후속 작업, 검토 후보, 보류 항목을 관리한다.

Index는 위치와 상태만 보여주고, Prompt Blueprint는 제작 / 유지보수 기준을 설명한다. 변동성이 큰 항목은 이 Backlog에서 관리한다.

---

## 2. 작성 기준

Backlog 항목은 다음 형식으로 기록한다.

```yaml
분류
-> Prompt Library / Work Checklist / Operation / Pipeline / Draft / Project Stella / History / Technical Document

내용
-> 처리해야 할 작업 또는 검토해야 할 항목

상태
-> 대기 / 검토 필요 / 검토 중 / 보류 / 완료 / 제외

우선순위
-> 필수 / 우선 / 권장 / 선택 / 보류

근거 문서
-> 항목이 발생한 문서 또는 논의 기준

다음 조치
-> 다음에 요청하거나 수행할 작업
```

---

## 3. 현재 Backlog

### Prompt Files 작성 / 검토

- 분류: Prompt Library
- 내용: Prompt Files 역할별 분류 기준에 맞춰 개별 Prompt를 작성하고 검토해야 함
- 상태: 대기
- 우선순위: 우선
- 근거 문서: D19 Work Checklist
- 다음 조치: Prompt Files 역할별 분류 기준에 따라 개별 Prompt를 작성하고 Prompt Blueprint 기준으로 순차 검토

### Document Set Audit Prompt 검토

- 분류: Prompt Library
- 내용: 우선 생성된 문서군 감사 Prompt를 Prompt Blueprint 기준으로 검토해야 함
- 상태: 검토 필요
- 우선순위: 권장
- 근거 문서: Document Set Audit Prompt
- 다음 조치: Review / Verification Prompt 중 우선 생성된 감사 Prompt로 유지하고 Prompt Blueprint 기준으로 검토

### Feature Work Prompt Archive 기준 정리

- 분류: Prompt Library
- 내용: Feature Work Prompt의 유지 / Archive 판단 기준을 정리해야 함
- 상태: 대기
- 우선순위: 권장
- 근거 문서: Prompt Library Maintenance Blueprint
- 다음 조치: Feature Work Prompt 작성 이후 Archive / 유지 기준 확정

### Work Checklist 명칭 전환 기준

- 분류: Work Checklist
- 내용: 기존 Issue Checklist 명칭과 경로를 Work Checklist 기준으로 전환할 범위와 시점을 정해야 함
- 상태: 대기
- 우선순위: 보류
- 근거 문서: Work Checklist 전환 기준
- 다음 조치: `Docs/01_Issue_CheckList` 폴더명과 D01-D18 파일명 전환은 Work_Checklist_Writing_Prompt 정리 단계에서 별도 계획 수립

### History 문서 / 운영 방식 구성

- 분류: History
- 내용: AI Workflow에서 History를 별도 문서로 관리할지, 기존 산출물 기록으로 대체할지 정해야 함
- 상태: 대기
- 우선순위: 권장
- 근거 문서: Operation Guide / Work Pipeline
- 다음 조치: History를 별도 문서로 둘지, Work Checklist / PR Document 기록으로 대체할지 결정

### System Architecture / Verification Log / PR Document 기준 정리

- 분류: Technical Document
- 내용: 주요 산출 문서의 위치, 최신 기준, 명명 규칙을 정리해야 함
- 상태: 대기
- 우선순위: 권장
- 근거 문서: Operation Guide / Work Pipeline
- 다음 조치: 실제 산출 문서의 폴더, 최신 문서 기준, 명명 규칙을 Index 또는 별도 기준으로 정리

### Prompt Library 실사용 검증

- 분류: Prompt Library
- 내용: Prompt Library를 실제 작업에 적용하고 보완 필요성을 확인해야 함
- 상태: 대기
- 우선순위: 보류
- 근거 문서: D19 Work Checklist
- 다음 조치: 후속 작업 범위에서 실제 작업에 적용 후 보완

### Prompt 문장 품질 최종 검수

- 분류: Prompt Library
- 내용: Prompt Library v1 구조 확정 후 문장 품질을 최종 검수해야 함
- 상태: 대기
- 우선순위: 보류
- 근거 문서: D19 Work Checklist
- 다음 조치: v1 구조 확정 후 문장 품질 검토

---

## 4. 완료된 Backlog 항목

### Prompt Files 상태 체계 정리

- 분류: Prompt Library
- 완료 기준: Index와 D19에서 개별 Prompt 파일 생성 상태와 Prompt Files 분류 기준을 분리

### Project Stella 본 개요 문서 작성

- 분류: Project Stella
- 완료 기준: `02_Project_Context/Project_Stella_Overview (KR).md` 작성 및 AI Workflow Index 연결

