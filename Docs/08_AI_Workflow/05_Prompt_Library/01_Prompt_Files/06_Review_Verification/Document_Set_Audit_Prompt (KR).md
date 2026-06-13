# Document Set Audit Prompt

## 1. 목적

문서군이 각자의 역할을 수행하면서 하나의 운영 체계로 일관되게 작동하는지 정밀 검토한다.

검토 범위는 문서군 전체의 역할, 책임 범위, 용어, 경로, 상태값, 문서 간 참조, 실제 운용 가능성, 에이전트 활용 가능성을 포함한다.

개별 문서의 문장 품질, 공백, 섹션명, Markdown formatting 같은 세부 품질 문제는 직접 해결하지 않고 `Document_Format_Review_Prompt` 검토 항목으로 분리한다.

공통 양식 기준 자체는 `00_Document_Common_Format_Prompt`를 따르고, 개별 문서 품질 검토는 `Document_Format_Review_Prompt`를 따른다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 여러 문서를 반복 수정한 뒤
-> 폴더명 / 파일명 / 공식 용어를 변경한 뒤
-> Index / Checklist / Backlog / Blueprint / Pipeline 간 싱크를 확인할 때
-> 문서군이 실제 운영에 충분한지 확인할 때
-> PR 전 문서 체계 정합성을 점검할 때
-> PR Draft 문서군을 인덱스별로 최종 검증하고 audit 산출물을 갱신할 때
-> 컨텍스트 없는 에이전트 관점의 교차 검토가 필요할 때
```

---

## 3. 사용 방법

검토할 문서 또는 폴더 경로를 입력하고, 최근 변경 목적과 완료 / 검토 중 상태를 함께 제공한다.

검토만 필요한 경우에는 수정하지 않고 Findings와 수정 계획을 요청한다. 수정까지 필요한 경우에는 변경 범위를 명시하고, 적용 후 검증 결과를 요청한다.

PR Draft 최종검증처럼 문서군 안의 여러 인덱스를 순회하는 작업은 `검증 기준 고정 -> 인덱스별 교차검증 -> 최종 audit 산출물 갱신 -> 검증 명령 실행 -> 사용자 검토` 순서로 진행한다.

파일럿 검증이 필요한 경우에는 P01 같은 첫 인덱스로 검증 기준, 출력 형식, audit 반영 위치를 먼저 확정한다. P02~P03에서 같은 절차를 반복해 기준이 흔들리지 않으면 전체 인덱스 순회로 확장한다.

---

## 4. 복사용 Prompt

```text
다음 문서군을 정밀 감사해줘.

너는 이 프로젝트의 사전 컨텍스트를 모르는 외부 시니어 엔지니어이자 기술 문서 리뷰어다.
제공된 문서와 실제 파일 트리만 보고 판단해줘.

검토 대상:
- [문서 / 폴더 / Work List 경로 입력]

최근 변경 목적:
- [최근 변경한 폴더명 / 파일명 / 공식 용어 / 문서 구조 입력]

완료로 판단한 항목:
- [완료 처리한 항목 입력]

아직 검토 중인 항목:
- [미완료 / 검토 필요 / 후속 작업 항목 입력]

검토 목표:
- 각 문서가 맡은 역할을 수행하는지 확인
- 문서군 안에서 문서별 역할, 책임, 참조 흐름이 적절한지 확인
- 문서 간 역할 분리와 참조 관계가 일관적인지 확인
- 현재 파일 경로, 공식 명칭, 상태값, 완료 기준이 서로 맞는지 확인
- 실제 운용하거나 신규 작업자가 읽었을 때 부족한 부분이 있는지 확인
- AI Agent가 이 문서를 컨텍스트로 사용할 때 잘못 추론할 위험이 있는지 확인

검토 기준:
1. 문서 역할 적합성
   - 제목, 목적, 적용 범위, 실제 내용이 일치하는가
   - 문서가 맡은 역할을 수행하기에 충분한가
   - 다른 문서가 담당해야 할 내용을 과하게 포함하고 있지 않은가

2. 문서별 구조 적합성
   - 문서군 기준에서 각 문서의 내부 흐름이 맡은 역할과 충돌하지 않는가
   - 개별 문장 품질, 공백, 섹션명 같은 세부 수정은 `Document_Format_Review_Prompt` 검토 항목으로 분리할 수 있는가

3. 용어 / 명칭 / 경로 정합성
   - 공식 카테고리명, 파일명, 폴더명, 문서명이 일치하는가
   - 구 명칭, 이전 폴더명, 임시 표현이 남아 있는가
   - EN / KR 표기가 같은 문맥 안에서 흔들리지 않는가
   - 참조 경로가 실제 파일 위치와 맞는가

4. 문서 간 연계성
   - Index / Checklist / Backlog / Blueprint / Pipeline이 같은 구조를 설명하는가
   - 문서 간 책임 범위가 중복되거나 비어 있지 않은가
   - 상위 문서와 하위 문서의 참조 방향이 자연스러운가
   - 한 문서가 다른 문서를 불필요하게 의존하지 않는가

5. 상태값 / 완료 기준
   - 완료 / 진행 중 / 검토 필요 / 보관 / 후속 검토 상태가 일관적인가
   - 파일이 존재한다는 이유만으로 완료처럼 표현되어 있지 않은가
   - Backlog와 Checklist의 상태가 충돌하지 않는가
   - PR / Commit 가능 조건을 판단할 수 있는가
   - 상태 전이 규칙과 다음 문서 / Prompt 진입 조건이 명확한가
   - 차단 상태와 비차단 상태가 같은 완료 기준으로 취급되고 있지 않은가

6. 실제 운용 가능성
   - 사용자가 이 문서군만 보고 다음 작업을 진행할 수 있는가
   - 필요한 입력값, 출력값, 완료 기준, 검증 기준이 충분한가
   - 누락된 본 문서, 기준 문서, Backlog 항목이 있는가
   - 장기 유지보수 시 stale 가능성이 큰 구조가 있는가

7. 에이전트 친화성
   - AI Agent가 문서를 읽고 프로젝트 구조와 작업 기준을 이해할 수 있는가
   - 잘못된 경로, 모호한 용어, 충돌하는 상태값 때문에 오해할 위험이 있는가
   - 에이전트가 탐색해야 할 문서 순서가 명확한가
   - 추가 Index, Overview, Checklist, Backlog가 필요한가

PR Draft 최종검증 기준:
1. 검증 기준 고정
   - 각 인덱스별 Non-Draft 문서, Draft 문서, 해당 branch tip 코드, 중간 audit 문서, 최종 audit 문서를 함께 확인한다.
   - 최종 audit 문서는 실제 파일명 기준으로 `02_Term_Usage_Map.md`, `03_PR_Goal_Flow_Map.md`, `04_Draft_Audit_Findings.md`처럼 역할별로 분리한다.
   - `01_Term_Usage_Classification_Guide.md`는 용어 분류 기준 문서이고, `02_Term_Usage_Map.md`는 실제 용어 사용 목록 문서로 구분한다.
   - 중간 audit 문서는 최종 audit에 누락된 내용이 없는지 확인하기 위한 비교 기준으로 사용한다.

2. 인덱스별 교차검증
   - `P01 -> P18`처럼 인덱스 순서대로 검토한다.
   - Non-Draft 문서는 기존 기준선으로 보고, Draft 문서는 최종 반영 후보로 본다.
   - Draft가 Non-Draft의 핵심 범위를 누락하지 않았는지 확인한다.
   - Draft가 최신 양식으로 재구성되면서 브랜치 시점에 없는 용어를 끌어오지 않았는지 확인한다.

3. branch tip 코드 검증
   - 현재 워크트리 코드가 아니라 해당 문서의 branch tip 기준으로 검증한다.
   - C++ / Config / text 파일은 `git show <branch>:<path>`로 실제 내용을 확인한다.
   - asset / map / Blueprint처럼 바이너리 파일은 `git ls-tree -r --name-only <branch>`로 존재 여부를 확인한다.
   - 브랜치나 파일을 확인할 수 없으면 임의 추정하지 않고 `검증 필요`로 남긴다.

4. 최종 audit 갱신
   - `Term_Usage_Map`: 통합 용어 기준과 문서별 용어 사용을 분리한다.
   - `PR_Goal_Flow_Map`: 각 PR의 목표, 이어받은 것, 새로 만든 흐름, 후속으로 넘긴 범위, branch 검증 결과를 기록한다.
   - `Draft_Audit_Findings`: 코드 정합성 문제, 표현 품질 문제, 사용자 판단 필요 항목을 분리한다.

5. Term Usage Map 작성 기준
   - 통합 섹션에는 문서군 전체에 적용할 기본 용어 기준을 둔다.
   - 문서별 섹션은 각 인덱스별로 `Only EN / 병기 / Only KR / 검토 결과`를 둔다.
   - 병기 형식은 `EN(KR 설명)`을 기본으로 한다.
   - 병기 항목과 Only KR 항목이 같은 개념을 가리키면, 같은 인덱스 안에서는 병기를 우선하고 Only KR 중복 항목은 제거한다.
   - 단, 통합 섹션에서는 여러 문서에서 반복되는 일반 KR 표현을 별도로 유지할 수 있다.
   - 코드 식별자, 함수명, enum, 파일명, 브랜치명은 inline code로 감싼다.
   - 일반 도메인 용어와 핵심 개념명은 inline code를 쓰지 않는다.

6. 최종 검증 명령 실행
   - Draft 본문과 장기 유지 audit 문서는 분리해서 검색한다.
   - Draft 본문 검색에서 audit 폴더를 제외할 때는 Windows 경로에서도 제외되도록 `-g "!**/audit/**"`를 사용한다.
   - KR 문서 일부를 PowerShell로 발췌할 때는 한글 깨짐을 피하기 위해 `Get-Content -Encoding utf8`을 사용한다.
   - 검색 결과가 나왔다는 이유만으로 즉시 수정하지 않는다.
   - 각 검색 결과를 `보완 필요 / 유지 가능 / 의도된 잔여 / audit 기준 문장`으로 판정한다.
   - 같은 표현이라도 문맥에 따라 다르게 판정한다.
     - 예: P06의 `현재 실행 중인 action`은 후속 구조 소급처럼 읽히면 보완 필요.
     - 예: P07의 `현재 action type / action index`는 `FActionContext` 설명이면 유지 가능.
     - 예: P17의 `현재 실행 중인 action 또는 reaction`은 `active execution` 설명이면 유지 가능.
   - `git diff --check`는 최종 단계에서 실행하고, LF/CRLF warning은 공백 오류와 분리해서 보고한다.

7. audit 문서 정리 상태 확인
   - 장기 유지 audit 문서는 `확정 기준 / 문서별 finding / 사용자 판단 필요 / 정리 제안`처럼 현재 상태가 드러나게 쓴다.
   - 이미 Draft 본문에 반영된 항목은 `정리 완료`, `확정 기준`, `유지`처럼 완료 상태로 바꾼다.
   - `권장`, `후보`, `재검토` 같은 표현은 실제로 아직 결정이 필요한 항목에만 사용한다.
   - 사용자 판단이 필요한 항목이 없으면 `없음`으로 명시한다.
   - 중간 산출물 삭제 / archive / 유지는 사용자 승인 전까지 `정리 제안`으로 남긴다.

8. 작업 모드 분리
   - 사용자가 `audit만 수정` 또는 `audit에 반영`을 요청하면 Draft 본문은 수정하지 않는다.
   - 사용자가 `Draft 본문 보완` 또는 `본문 반영`을 요청하면 최종 audit 기준과 충돌하는지 먼저 확인한 뒤 Draft를 수정한다.
   - Draft 본문과 audit 문서를 모두 수정한 경우, 어떤 변경이 본문 반영이고 어떤 변경이 기준 문서 정리인지 결과 보고에서 분리한다.

출력 형식:
1. 핵심 결론
2. 심각도별 Findings
3. 문서 간 싱크 문제
4. 실제 운용 리스크
5. 즉시 수정 항목
6. 후속 검토 후보
7. Document Format Review로 넘길 항목
8. 권장 수정 순서

PR Draft 최종검증 출력 형식:
1. 인덱스 상태
2. 확인한 기준 문서 / branch
3. Non-Draft vs Draft 차이
4. branch tip 코드 검증 결과
5. 최종 audit 반영 내용
6. Draft 자체검토 결과
7. 남은 보완 후보
8. 사용자 판단 필요 항목
9. 다음 인덱스 진행 가능 여부
10. 최종 검증 명령 결과
11. 검색 결과 판정
12. Draft 본문 수정 여부
13. Audit 문서 갱신 여부

제약 조건:
- 단순 취향보다 실제 혼동, 누락, 충돌, 운용 위험을 우선한다.
- 문서의 역할을 임의로 바꾸지 않는다.
- 현재 파일 트리와 실제 경로를 기준으로 판단한다.
- 완료되지 않은 항목을 완료로 표현하지 않는다.
- 즉시 수정할 항목과 후속 검토 후보를 분리한다.
- 개별 문서의 문장 품질 / 공백 / 섹션명 문제는 `Document_Format_Review_Prompt` 검토 항목으로 분리한다.
- 검토만 요청받은 경우 직접 수정하지 않고 수정 계획을 제안한다.
- 수정까지 요청받은 경우 변경 범위를 좁혀 적용하고 검증 결과를 요약한다.
- PR Draft 최종검증에서는 Draft 본문 수정과 audit 산출물 갱신을 구분한다.
- 사용자가 audit 갱신만 요청한 경우 Draft 본문은 수정하지 않는다.
- 사용자가 Draft 본문 수정만 요청한 경우 audit 문서 갱신 필요 여부를 별도 보고한다.
- 중간 audit 산출물은 삭제하지 않고, 최종 병합문서 검토 후 사용자 승인에 따라 정리한다.
```

---

## 5. 입력 기준

```yaml
입력 기준
-> 검토할 문서 경로
-> 함께 봐야 할 상위 / 하위 문서
-> 현재 변경 목적
-> 최근 변경된 폴더명 / 파일명 / 공식 용어
-> 완료로 판단한 항목
-> 아직 검토 중인 항목
-> 인덱스 순회 범위
-> Non-Draft / Draft 문서 경로
-> branch 이름 또는 branch를 확인할 수 있는 문서 위치
-> 중간 audit 문서 경로
-> 최종 audit 문서 경로
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 핵심 결론
-> 심각도별 Findings
-> 문서 간 싱크 문제
-> 실제 운용 리스크
-> 즉시 수정 항목
-> 후속 검토 후보
-> Document Format Review로 넘길 항목
-> 권장 수정 순서
-> 인덱스별 검증 결과
-> 최종 audit 반영 내역
-> branch tip 코드 검증 결과
-> 다음 인덱스 진행 여부
```

---

## 7. 범위 / 비범위

```yaml
범위
-> 문서군의 역할, 구조, 용어, 경로, 상태값 감사
-> 문서 간 참조와 책임 범위 정합성 검토
-> 실제 운용 가능성과 에이전트 활용 가능성 검토
-> 즉시 수정 항목과 후속 검토 후보 분리
-> 개별 문서 품질 문제를 Document Format Review 항목으로 분리
-> PR Draft 문서군의 Non-Draft / Draft / branch tip code / audit 문서 교차검증
-> 최종 audit 산출물의 누락 / 중복 / 구버전 표현 확인

비범위
-> 코드 리뷰
-> 문서 내용의 전면 재작성
-> 사용자가 명시하지 않은 문서 역할 변경
-> 실제 파일 수정
-> 개별 문서의 문장 품질 / 공백 / Markdown formatting 직접 검토
-> branch 코드 자체의 수정
-> 사용자 승인 없는 중간 audit 산출물 삭제
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 문법 교정보다 구조적 정합성 우선
-> 공통 양식 검토는 00_Document_Common_Format_Prompt와 Document_Format_Review_Prompt의 기준에 위임함
-> 실제 파일 경로 기준으로 판단
-> 과거 PR 문서는 해당 branch tip 기준으로 판단
-> 현재 워크트리 코드만 보고 과거 PR 문서를 검증하지 않음
-> 상태값 충돌을 반드시 확인
-> 구 명칭 / 구 경로 / 이전 기준 잔여 여부 확인
-> 사용자가 명시하지 않은 문서 역할 변경은 제안으로만 남김
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 실제 파일 트리와 문서 본문 중 충돌하는 부분을 분리
-> 확인 가능한 경로 / 상태와 추정이 필요한 항목을 분리
-> 문서 역할 변경이 필요해 보이면 즉시 수정이 아니라 제안으로 남김
-> 완료 여부를 판단하기 어렵다면 검토 필요 또는 후속 검토 후보로 분리
```

---

## 10. 검증 기준

```yaml
검증 기준
-> 파일 경로와 문서 내 참조 경로가 일치하는지 확인
-> Index / Checklist / Backlog 상태값이 충돌하지 않는지 확인
-> 상태값이 다음 단계로 어떻게 전이되는지 확인
-> 차단 / 비차단 항목이 다음 Prompt 진입 조건과 충돌하지 않는지 확인
-> 구 명칭 / 구 경로 / 이전 기준 잔여 여부를 확인
-> 즉시 수정 항목과 후속 검토 후보가 분리되었는지 확인
-> 개별 문서 품질 문제가 Document Format Review 항목으로 분리되었는지 확인
-> 에이전트가 따라갈 문서 순서와 기준이 명확한지 확인
-> PR Draft 최종검증 시 각 인덱스가 Non-Draft / Draft / branch tip code / 중간 audit / 최종 audit과 교차검증되었는지 확인
-> Term Usage Map이 통합 기준과 문서별 용어 사용으로 분리되었는지 확인
-> 문서별 용어 사용에 Only EN / 병기 / Only KR / 검토 결과가 있는지 확인
-> 병기 항목과 Only KR 항목이 같은 개념으로 중복되지 않는지 확인
-> PR Goal Flow Map에 목표 / 이어받은 것 / 새로 만든 흐름 / 후속 범위 / branch 검증 결과가 있는지 확인
-> Draft Audit Findings에 코드 정합성 문제와 표현 품질 문제가 분리되었는지 확인
-> Draft Audit Findings의 완료 항목과 후보 항목이 섞이지 않는지 확인
-> 검색 결과가 `보완 필요 / 유지 가능 / 의도된 잔여 / audit 기준 문장`으로 판정되었는지 확인
-> `git diff --check`를 수행했는지 확인
-> `git diff --check`의 LF/CRLF warning을 공백 오류와 분리해 보고했는지 확인
```

---

## 11. PR Draft 최종검증 실행 예시

```text
P01~P18 Draft 문서를 인덱스별로 최종검증해줘.

검증 기준:
- 각 Pxx별로 Non-Draft 문서, Draft 문서, 해당 branch tip 코드, 중간 audit 문서, 최종 audit 문서를 교차검증한다.
- 최종 audit 문서는 `02_Term_Usage_Map.md`, `03_PR_Goal_Flow_Map.md`, `04_Draft_Audit_Findings.md`이다.
- 중간 audit 문서는 audit 폴더의 나머지 문서이다.
- 현재 워크트리 코드가 아니라 Draft의 브랜치 값을 기준으로 branch tip 코드를 확인한다.
- C++ / Config / text 파일은 `git show <branch>:<path>`로 확인한다.
- asset / map / Blueprint는 `git ls-tree -r --name-only <branch>`로 존재 여부를 확인한다.
- 확인할 수 없는 항목은 추정하지 않고 `검증 필요`로 남긴다.

진행 방식:
1. P01부터 순서대로 진행한다.
2. 각 인덱스마다 Draft 본문 자체검토와 audit 반영 내용을 보고한다.
3. `Term_Usage_Map.md`는 통합 기준과 문서별 용어 사용으로 나눈다.
4. 문서별 용어 사용은 각 Pxx 아래에 `Only EN / 병기 / Only KR / 검토 결과`를 둔다.
5. 병기와 Only KR이 같은 개념을 가리키면 같은 Pxx 안에서는 병기를 우선하고 Only KR 중복은 제거한다.
6. Draft 본문은 사용자가 요청하지 않으면 수정하지 않는다.
7. 최종 audit 3종만 갱신한다.

검증 명령:
- `rg -n "관련 PR / 문서|기반 마련|현재 실행 중인 action|Hit Context|Reaction Result|Reserved|Equipment Pipeline" Docs/04_Pull_Request/draft -g "P*.md" -g "!**/audit/**"`
- `rg -n "관련 PR / 문서|P17 / P18은 범위에서 제외|P01~P16 기준|중간 통합|통합 초안|Reserved|Hit Context|Reaction Result 단독 표준화|현재 실행 중인 action.*정리한다|현재 action.*재검토|^권장:|확인 후보:|수정 후보|보완 후보|후보로 둔다" Docs/04_Pull_Request/draft/audit/01_Term_Usage_Classification_Guide.md Docs/04_Pull_Request/draft/audit/02_Term_Usage_Map.md Docs/04_Pull_Request/draft/audit/03_PR_Goal_Flow_Map.md Docs/04_Pull_Request/draft/audit/04_Draft_Audit_Findings.md`
- `rg -n "^## 관련" Docs/04_Pull_Request/draft -g "P*.md" -g "!**/audit/**"`
- `rg -n "## 문서별 용어 사용|### P01|#### Only EN|#### EN\\(KR\\)|#### Only KR|#### 검토 결과" Docs/04_Pull_Request/draft/audit/02_Term_Usage_Map.md`
- `rg -n "브랜치 검증|branch 검증|P01|P18" Docs/04_Pull_Request/draft/audit/03_PR_Goal_Flow_Map.md Docs/04_Pull_Request/draft/audit/04_Draft_Audit_Findings.md`
- `git diff --check -- Docs/04_Pull_Request/draft`
- `git diff --check -- Docs/04_Pull_Request/draft/audit`

문서 발췌 명령:
- `Get-Content -Encoding utf8 Docs/04_Pull_Request/draft/audit/04_Draft_Audit_Findings.md | Select-Object -Skip 220 -First 40`
```

---

## 12. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 문서군의 구조, 역할, 용어, 경로, 상태값, 운용 리스크가 분리되어 정리됨
-> PR Draft 최종검증에서는 각 인덱스의 교차검증 결과와 최종 audit 반영 내역이 기록됨

실패
-> 검토 대상 문서 또는 실제 파일 트리를 확인할 수 없음
-> 문서군의 기준 문서와 상태 문서가 서로 크게 충돌해 판단 근거가 부족함
-> branch tip 코드를 확인할 수 없고 대체 검증 기준도 없음

미검증
-> 실제 파일을 열람하지 못한 문서
-> 외부 도구나 사용자 확인이 필요한 상태값
-> branch나 asset 존재 여부를 확인하지 못한 항목
```

---

## 13. 기존 Prompt와 역할 경계

```yaml
Code Review Prompt
-> 코드 변경의 버그, 회귀, 테스트 누락 검토

Verification Log Prompt
-> 수행한 검증과 미검증 항목 기록

00_Document_Common_Format_Prompt
-> 파일명 / 상단 메타 / 문체 / 공백 / Markdown 구조의 공통 기준

Document Format Review Prompt
-> 개별 문서 또는 소수 문서의 내부 품질 검토

Document Set Audit Prompt
-> 문서군의 역할, 구조, 용어, 경로, 상태값, 운용 가능성 감사
-> PR Draft 문서군의 최종검증과 audit 산출물 정리
```

---

## 14. 후속 보완 후보

```yaml
후속 보완 후보
-> 실제 문서 재검토 작업에 사용한 뒤 출력 형식 보완
-> Document Format Review와 Document Set Audit의 위임 기준 보완
-> 에이전트 교차 검토 요청 문구를 더 구체화할지 확인
-> PR Draft 최종검증을 P01~P18 전체에 적용한 뒤 전용 Prompt로 분리할지 확인
```
