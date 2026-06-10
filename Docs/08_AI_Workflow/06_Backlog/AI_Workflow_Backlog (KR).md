# AI Workflow Backlog

## 1. 목적

본 문서는 AI Workflow 문서군과 Prompt Library의 후속 작업, 검토 후보, 보류 항목을 관리한다.

Index는 위치와 상태만 보여주고, Prompt Blueprint는 제작 / 유지보수 기준을 설명한다. 변동성이 큰 작업 항목과 아직 확정되지 않은 판단은 이 Backlog에서 관리한다.

---

## 2. 작성 기준

Backlog 항목은 다음 기준으로 기록한다.

```yaml
분류
-> Prompt Library / Prompt Files / Work List / Operation / Pipeline / Project Stella / History / Document Category / Portfolio Technical Document

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

### Prompt Files 내용 품질 정밀 검토

- 분류: Prompt Files
- 내용: 생성된 Prompt Files의 복사용 Prompt 내용, 출력 품질, Custom 적용 기준, 실제 사용성을 폴더별로 정밀 검토해야 함
- 상태: 검토 필요
- 우선순위: 우선
- 근거 문서: AI Workflow Index / W01 Work List / Prompt Blueprint
- 다음 조치: Working Rules / Working Reference / Work Planning / Document Writing / Feature Work / Review Verification / Git Operation Prompt를 폴더별로 검토하고, 내용 품질 검토 완료 여부를 Index와 W01에 반영

### AI Workflow Refactor 요구사항 정리

- 분류: Pipeline
- 내용: AI Workflow v1 구축 중 발견한 계층 구조, Prompt Flow, Work Brief / Planning / Work List, 문서 카테고리, History, 검증, Commit / PR 보완 후보를 다음 Refactor 범위에서 검토해야 함
- 상태: 대기
- 우선순위: 우선
- 근거 문서: `AI_Workflow_Refactor_Notes (KR).md` / W02 Work Brief / W02 Feature Work Planning / W02 Work List Draft
- 다음 조치: W02 실제 구현 Branch에서 Workflow 사용성을 확인한 뒤, Refactor Notes 기준으로 우선순위를 재정렬하고 필요한 항목만 반영

### Working Rules / Reference Prompt 실사용 검토

- 분류: Prompt Files
- 내용: Working Rule과 Working Reference를 실제 작업 세션에서 사용해 실행 규칙과 Reference 기준이 충분한지 확인해야 함
- 상태: 검토 필요
- 우선순위: 권장
- 근거 문서: Working Rules / Working Reference Prompt
- 다음 조치: 실제 작업 요청에 적용한 뒤 누락된 실행 규칙, Reference 기준, 검증 표현을 Prompt별로 보완

### Prompt Files 번호 prefix 적용 범위 정리

- 분류: Prompt Files
- 내용: 일부 Prompt 파일에만 번호 prefix가 적용되어 있어, Prompt Files 전체에 번호 prefix를 적용할지 폴더별 핵심 Prompt에만 적용할지 정해야 함
- 상태: 대기
- 우선순위: 권장
- 근거 문서: AI Workflow Index / Prompt Library Maintenance Blueprint
- 다음 조치: Working Rules / Working Reference / Work Planning 검토 후, Document Writing / Review Verification / Git Operation Prompt에도 번호 prefix를 적용할지 결정

### Work Planning Prompt 확장

- 분류: Prompt Library
- 내용: Work Pipeline에 입력할 작업 계획을 만들기 위한 Work Planning Prompt 확장 여부를 검토해야 함
- 상태: 대기
- 우선순위: 권장
- 근거 문서: AI Work Pipeline / W01 Work List
- 다음 조치: `Update_Work_Planning_Prompt`, `Review_Verification_Planning_Prompt`, `Document_Writing_Planning_Prompt`, `Document_Restructure_Work_Planning_Prompt`, `Prompt_Library_Reorganization_Work_Planning_Prompt`, `Pipeline_Implementation_Planning_Prompt` 작성 여부 검토

### Document Set Audit Prompt 검토

- 분류: Prompt Files
- 내용: 우선 생성된 문서군 감사 Prompt를 Prompt Blueprint 기준으로 검토해야 함
- 상태: 검토 필요
- 우선순위: 권장
- 근거 문서: Document Set Audit Prompt
- 다음 조치: Review / Verification Prompt 중 우선 생성된 감사 Prompt로 유지하고, 출력 형식과 실제 감사 범위가 과설계되지 않았는지 검토

### Feature Work Prompt Archive 기준 정리

- 분류: Prompt Library
- 내용: Feature Work Prompt의 유지 / Archive 판단 기준을 정리해야 함
- 상태: 대기
- 우선순위: 권장
- 근거 문서: Prompt Library Maintenance Blueprint
- 다음 조치: Feature Work Prompt 실사용 여부를 확인한 뒤 Archive / 유지 / 삭제 후보 기준 확정

### Legacy Issue Checklist / Work List 명칭 정리

- 분류: Work List
- 내용: D01-D18은 Legacy Issue Checklist로 보관하고, W01 이후 작업 관리 문서는 Work List로 유지하기로 정리함
- 상태: 완료
- 우선순위: 완료
- 근거 문서: `Docs/99_Legacy/Issue_CheckList/` / `Docs/01_Work_List/`
- 다음 조치: 신규 작업 문서와 관련 Prompt에서는 Work List 명칭을 사용

### History 문서 / 운영 방식 구성

- 분류: History
- 내용: AI Workflow에서 History를 별도 문서로 관리할지, 기존 산출물 기록으로 대체할지 정해야 함
- 상태: 대기
- 우선순위: 권장
- 근거 문서: Operation Guide / Work Pipeline
- 다음 조치: History를 별도 문서로 둘지, Work List / Verification Log / PR Document 기록으로 대체할지 결정

### 문서 카테고리 기준 적용

- 분류: Document Category
- 내용: 확정한 문서 카테고리 기준을 Prompt, Index, 기존 문서 폴더, Writing Prompt에 반영해야 함
- 상태: 대기
- 우선순위: 우선
- 근거 문서: 문서 카테고리 기준 / Prompt Custom Blueprint / Document Writing Prompt
- 다음 조치: System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records / Portfolio Technical Document 기준으로 Writing Prompt와 기존 문서 재분류 계획 수립

### Document Writing Prompt 재구성

- 분류: Prompt Files
- 내용: Document Writing Prompt를 확정 문서 카테고리별 작성 / 보완 기준에 맞춰 재구성해야 함
- 상태: 대기
- 우선순위: 우선
- 근거 문서: Document Writing Prompt / Prompt Custom Blueprint
- 다음 조치: System Architecture는 현재 구조 설명 전용으로 축소하고, Architecture Decision Record / Architecture Issue Report / Engine Technique Document / Engine Decision Record / Engine Issue Report / Portfolio Technical Document 작성 Prompt를 추가 또는 변경

### Prompt Library 실사용 검증

- 분류: Prompt Library
- 내용: Prompt Library를 실제 작업에 적용하고 보완 필요성을 확인해야 함
- 상태: 대기
- 우선순위: 보류
- 근거 문서: W01 Work List
- 다음 조치: Prompt Files 검토가 끝난 뒤 실제 작업에 적용하고, 출력 품질 / 누락 입력 / 미검증 처리 방식을 점검

### Prompt 문장 품질 최종 검수

- 분류: Prompt Library
- 내용: Prompt Library v1 구조 확정 후 문장 품질을 최종 검수해야 함
- 상태: 대기
- 우선순위: 보류
- 근거 문서: W01 Work List / Prompt Blueprint
- 다음 조치: v1 구조와 Prompt Files 검토가 끝난 뒤 반박형 메타 문장, 불명확한 용어, 과도한 설명을 최종 검수

---

## 4. 완료된 Backlog 항목

### Prompt Files 상태 체계 정리

- 분류: Prompt Library
- 완료 기준: Index와 W01에서 개별 Prompt 파일 생성 상태와 Prompt Files 분류 기준을 분리

### Prompt Files Format 구조 1차 검토

- 분류: Prompt Files
- 완료 기준: 생성된 Prompt Files가 Prompt Format Blueprint의 기본 섹션 구조를 갖추도록 보완하고, 복사용 Prompt 내부 markdown 헤더 혼동 요소를 정리

### Project Stella 본 개요 문서 작성

- 분류: Project Stella
- 완료 기준: `02_Project_Context/Project_Stella_Overview (KR).md` 작성 및 AI Workflow Index 연결

### Unreal / Project Working Prompt 작성

- 분류: Prompt Library
- 완료 기준: Unreal Engine / Project Stella Working Rule / Reference Prompt 작성 및 AI Workflow Index 연결

### Working Reference 분리

- 분류: Prompt Files
- 완료 기준: 기존 분석 분류를 `02_Working_Reference`로 전환하고, Unreal Reference Prompt와 Project Stella Reference Prompt를 번호 prefix 기준으로 정리

### Project Stella Working Rule / Reference 역할 분리

- 분류: Prompt Files
- 완료 기준: `02_Project_Stella_Working_Rule_Prompt (KR).md`는 작업 세션 실행 규칙 중심으로 축약하고, `02_Project_Stella_Working_Reference_Prompt (KR).md`는 Project Stella 구조 / 용어 / 책임 경계 판단 기준으로 보강

### 문서 카테고리 기준 확정

- 분류: Document Category
- 완료 기준: Refactor Report를 공식 카테고리에서 제외하고 Work List / Bug Report / System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records / Verification Log / PR Document / Portfolio Technical Document 기준으로 확정

### 36ce73b Prompt 추가 반영

- 분류: Prompt Library
- 완료 기준: Working / Document Writing / Review Verification / Git Operation / Feature Work Prompt 후보를 현재 Prompt Files 구조에 재작성하고 AI Workflow Index 연결

### Work Brief / Feature Work Planning Prompt 작성

- 분류: Prompt Files
- 완료 기준: `01_Work_Brief_Intake_Prompt (KR).md`와 `02_Feature_Work_Planning_Prompt (KR).md`를 작성하고, `01_Work_List_Writing_Prompt (KR).md`를 Work Brief / Work Planning 결과 기반의 최종 Work List 작성 역할로 정리
