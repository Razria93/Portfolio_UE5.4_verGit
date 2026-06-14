# Prompt Pattern Candidates

## 1. 목적

본 문서는 작업 중 발견한 최근 문서 패턴과 Prompt 반영 후보를 임시 저장하고 관리하는 캐시 문서다.

실제 Prompt 수정은 이 문서에 후보가 기록되었다는 이유만으로 수행하지 않으며, `01_Prompt_Change_Management_Rule (KR).md`의 승인 기준을 따른다.

---

## 2. 상태 기준

```yaml
패턴 수집 항목
-> 작업 중 발견했지만 반복성 / 위험도 / 영향 범위를 아직 판단하지 않은 기준

반영 후보
-> 반복성, 위험도, 실제 산출물 영향이 확인되어 Prompt 반영 검토가 필요한 기준

반영 보류
-> 특정 문서 한정이거나 취향성 표현이라 즉시 Prompt화하지 않는 기준

반영 완료
-> 사용자 승인 후 Prompt에 반영되고 검증까지 끝난 기준

폐기
-> 사용자가 반대했거나 Prompt화할 가치가 낮다고 판단한 기준
```

---

## 3. 기록 형식

```md
### PC-000

- 상태:
- 발견일:
- 발견 산출물:
- 패턴:
- 적용 범위:
- 반복성:
- 위험도:
- 후보 사유:
- 감지 결과:
- 사용자 결정:
- 반영 대상 Prompt:
- 처리 결과:
```

---

## 4. 후보 목록

### PC-001

- 상태: 반영 완료
- 발견일: 2026.06.12
- 발견 산출물: `P07_UE5_Portfolio_Pull_Request_Refactor_Draft.md`
- 패턴: 과거 PR 문서 재작성 시 branch-backed 공식 용어표를 먼저 만들고, 최신 구조 용어가 과거 PR 완료 범위처럼 섞이지 않도록 검증한다.
- 적용 범위: PR Document Writing Prompt
- 위험도: 중간
- 후보 사유:
  - P07 문서 조율 중 `weapon context`, `equipment context`, `attachment context`, `action context`, `damage 기준`, `damage 설정`처럼 유사 용어가 섞이며 문서 전체 정합성 검토 비용이 커졌다.
  - 문제는 특정 단어 하나가 아니라, 해당 branch 시점 코드에 있는 용어와 후속 refactor 이후의 용어가 섞인 데 있었다.
  - 사용자가 "이 용어가 뭐냐"고 물은 경우 단순 설명 요청이 아니라 용어 모델 흔들림 신호일 수 있음이 확인됐다.
- 사용자 결정:
  - PR 프롬프트에 반영 승인.
- 반영 대상 Prompt:
  - `03_PR_Document_Writing_Prompt (KR).md`
- 처리 결과:
  - 작성 프로세스에 branch 시점 고정과 공식 용어표 작성 기준 추가.
  - 용어 처리 기준에 canonical term / 금지 표현 관리 기준 추가.
  - 모호성 처리 기준에 용어 질문 시 공식 용어표 재고정 기준 추가.
  - 검증 기준에 branch 시점 오염, 공식 용어표 일관성, 처리 흐름 용어 검증 추가.

### PC-002

- 상태: 반영 완료
- 발견일: 2026.06.12
- 발견 산출물: `P06_UE5_Portfolio_Pull_Request_Refactor_Draft.md`
- 패턴: PR 문서 보완 중 사용자가 문장 / 용어 / 섹션 역할을 반복 지적하면, 문장 수정 전에 기존 표현의 의도와 근거, 설명 프레임, 섹션 핵심 동사를 먼저 재검토한다.
- 적용 범위: PR Document Writing Prompt
- 위험도: 중간
- 후보 사유:
  - P06 문서 조율 중 초안의 `정리 / 변환 / 경계` 프레임을 과신해 실제 branch 목적이었던 hit collision 발생 / 제어 / 전달 구조가 흐려졌다.
  - `Attachment Overlap Event`는 payload 정리용 구조가 아니라 delegate event 기반 `broadcast / binding / 전달` 흐름이었으므로, 섹션 핵심 동사를 먼저 고정해야 했다.
  - Feature와 Refactoring이 같은 delegate 연결을 반복하면서 동작 단위와 책임 분리 단위가 섞였다.
  - 사용자는 설계 의도와 문서에서 강조할 메시지를 더 정확히 알고 있으나, 코드 사실 / Branch 시점 / API / 데이터 구조는 코드 근거 기준으로 검증해야 한다는 역할 분담 기준이 필요했다.
- 사용자 결정:
  - PR 프롬프트에 반영 승인.
- 반영 대상 Prompt:
  - `03_PR_Document_Writing_Prompt (KR).md`
- 처리 결과:
  - 복사용 Prompt에 사용자 질문 대응 시 기존 의도와 근거를 먼저 설명하는 기준 추가.
  - 작성 프로세스에 초안 과신 방지, 섹션 핵심 동사 확인, Feature / Refactoring 중복 분리 기준 추가.
  - 작성 기준에 `broadcast`, `binding`, `전달`, `검증`, `제어`, `분리` 같은 실제 구현 동사 우선 기준 추가.
  - 모호성 처리 기준에 코드 사실 / 설계 의도 / 표현 선호 분류와 협업 역할 기준 추가.
  - 검증 기준에 섹션 핵심 동사, Feature / Refactoring 역할 분리, 사용자 지적 후 재판단 설명 여부 추가.

### PC-003

- 상태: 반영 완료
- 발견일: 2026.06.13
- 발견 산출물:
  - `Docs/04_Pull_Request/draft/audit/Term_Usage_Map.md`
  - `Docs/04_Pull_Request/draft/audit/PR_Goal_Flow_Map.md`
  - `Docs/04_Pull_Request/draft/audit/Draft_Audit_Findings.md`
  - `P01_UE5_Portfolio_Pull_Request_Refactor_Draft.md` 파일럿 검증
- 패턴: PR Draft 최종검증은 인덱스별로 Non-Draft / Draft / branch tip code / 중간 audit / 최종 audit을 교차검증하고, 최종 audit 3종에 역할별로 반영한다.
- 적용 범위:
  - Document Set Audit Prompt
  - PR Draft 최종검증 작업
  - Audit 산출물 유지관리
- 위험도: 중간
- 후보 사유:
  - P01 파일럿 검증에서 현재 워크트리 코드는 후속 변경이 섞여 있어, 과거 PR 검증 기준으로 부적절할 수 있음이 확인됐다.
  - 따라서 C++ / Config / text 파일은 `git show <branch>:<path>`로, asset / map / Blueprint는 `git ls-tree -r --name-only <branch>`로 branch tip 기준 확인이 필요하다.
  - Non-Draft는 기존 기준선, Draft는 최종 반영 후보, 중간 audit은 누락 확인 기준, 최종 audit은 현재 운영 기준으로 역할을 분리해야 한다.
  - `Term_Usage_Map`은 통합 기준과 문서별 용어 사용을 분리해야 이후 PR이 추가되어도 관리가 쉽다.
- 사용자 결정:
  - P01 파일럿 기준을 후보로 기록.
  - P02~P03 추가 검증에서 같은 절차가 반복 확인되어 정식 Prompt 기준으로 반영.
- 반영 대상 Prompt:
  - `Document_Set_Audit_Prompt (KR).md`
  - 필요 시 후속 별도 Prompt: `PR_Draft_Final_Audit_Prompt`
- 처리 결과:
  - `Document_Set_Audit_Prompt (KR).md`에 PR Draft 최종검증 기준, 출력 형식, 실행 예시, 검증 기준을 반영했다.
  - 별도 Prompt 생성 여부는 P01~P18 전체 검증 방식 확정 이후 재판단한다.

### PC-004

- 상태: 반영 완료
- 발견일: 2026.06.13
- 발견 산출물:
  - `Docs/04_Pull_Request/draft/audit/Term_Usage_Map.md`
  - P01 용어 사용 파일럿 검토
- 패턴: `Term_Usage_Map`의 문서별 용어 사용은 `Only EN / 병기 / Only KR / 검토 결과`로 나누고, 같은 Pxx 안에서 병기와 Only KR이 같은 개념을 가리키면 병기를 우선하고 Only KR 중복 항목은 제거한다.
- 적용 범위:
  - Term Usage Map
  - PR Draft 최종검증
  - 문서군 용어 audit
- 위험도: 낮음
- 후보 사유:
  - P01 파일럿에서 `TestRoom(테스트 레벨)`과 `테스트 레벨`이 병기 / Only KR 양쪽에 들어가며 중복 여부가 논의됐다.
  - 같은 Pxx 개별 섹션에서는 병기 항목이 이미 쉬운 KR 설명을 포함하므로, 같은 개념의 Only KR 항목을 별도로 두면 용어표가 중복되어 보인다.
  - 다만 통합 섹션에서는 여러 문서에서 반복되는 일반 KR 표현을 별도로 유지할 수 있으므로, 통합 기준과 문서별 기준을 분리해야 한다.
- 사용자 결정:
  - P01 개별 섹션에서는 `TestRoom(테스트 레벨)`을 병기로 유지하고, Only KR의 `테스트 레벨`은 제거하는 방향으로 정리.
  - P02~P03에서도 문서별 용어 사용 구조가 반복 적용되어 정식 Prompt 기준으로 반영.
- 반영 대상 Prompt:
  - `Document_Set_Audit_Prompt (KR).md`
  - 필요 시 `00_Document_Common_Format_Prompt (KR).md`의 용어 audit 보조 기준
- 처리 결과:
  - `Document_Set_Audit_Prompt (KR).md`의 Term Usage Map 작성 기준과 PR Draft 최종검증 실행 예시에 반영했다.
  - `00_Document_Common_Format_Prompt (KR).md` 반영은 보류한다.

### PC-005

- 상태: 반영 완료
- 발견일: 2026.06.14
- 발견 산출물:
  - `Docs/04_Pull_Request/draft/audit/02_Term_Usage_Map.md`
  - `Docs/04_Pull_Request/draft/audit/03_PR_Goal_Flow_Map.md`
  - `Docs/04_Pull_Request/draft/audit/04_Draft_Audit_Findings.md`
  - P01~P18 Draft 최종 검증 명령 실행 결과
- 패턴: PR Draft 최종 검증은 Draft 본문과 장기 유지 audit 문서를 분리해 검색하고, 검색 결과를 `보완 필요 / 유지 가능 / 의도된 잔여 / audit 기준 문장`으로 판정한 뒤 `git diff --check`로 마무리한다.
- 적용 범위:
  - Document Set Audit Prompt
  - PR Draft 최종검증
  - Audit 산출물 유지관리
- 위험도: 낮음
- 후보 사유:
  - 동일한 검색어라도 P06의 `현재 실행 중인 action`은 후속 구조 소급 위험으로 보완 대상이었지만, P07의 `현재 action type / action index`와 P17의 `현재 실행 중인 action 또는 reaction`은 문맥상 유지 대상이었다.
  - Audit 문서에서 `권장`, `후보`, `재검토` 표현이 남으면 이미 반영된 finding과 남은 판단 항목이 섞여 보일 수 있었다.
  - LF/CRLF warning은 `git diff --check` 공백 오류와 분리해 보고해야 한다.
- 사용자 결정:
  - 최종 검증 명령 세트와 검색 결과 판정 방식을 Prompt에 반영.
  - Audit 장기 유지 문서는 `확정 기준 / 문서별 finding / 사용자 판단 필요 / 정리 제안` 상태가 드러나게 관리.
- 반영 대상 Prompt:
  - `Document_Set_Audit_Prompt (KR).md`
- 처리 결과:
  - `Document_Set_Audit_Prompt (KR).md`에 최종 검증 명령 실행 기준, 검색 결과 판정 기준, audit 문서 정리 상태 기준, LF/CRLF warning 보고 기준을 반영했다.
  - P01 파일럿 통과 조건, Windows 환경의 audit 제외 검색 패턴, UTF-8 문서 발췌 기준, `audit만 수정` / `Draft 본문 수정` 작업 모드 분리 기준을 추가 보강했다.

### PC-006

- 상태: 패턴 수집 항목
- 발견일: 2026.06.14
- 발견 산출물:
  - `P19_UE5_Portfolio_Pull_Request.md`
  - `W02_UE5_Portfolio_Work_List.md`
- 패턴: 구형 명칭 제거를 설명할 때 단순 문자열 삭제처럼 쓰지 않고, `구형 EN 문서`, `구형 문서명`, `구형 prefix`, `구형 경로` 중 무엇을 제거했는지 목적 중심으로 표현한다.
- 적용 범위:
  - PR Document Writing Prompt
  - Index Writing Prompt
  - 문서 운영 체계 정리 PR
- 반복성: 미확정
- 위험도: 낮음
- 후보 사유:
  - W02에서는 `구형 EN 문서와 구형 문서명 흔적 제거`가 목표 수준에 포함됐지만, P19에서는 처음에 `Technical Document` / `Txx` 제거 중심으로만 표현되어 범위가 좁게 읽혔다.
  - 문서 체계 정리 작업에서 구형 명칭 제거는 단순 문자열 정리가 아니라, 최신 문서 유형 / ID / 탐색 기준과 맞지 않는 흔적을 제거하는 작업이다.
  - 다만 현재는 P19 문서 맥락에서 확인된 기준이므로, 공통 Prompt에 즉시 반영하기보다 반복 적용 여부를 더 확인한다.
- 감지 결과:
  - P19에는 `구형 EN 문서와 구형 문서명` 표현을 반영했다.
- 사용자 결정:
  - 후보 기록.
- 반영 대상 Prompt:
  - 반복 확인 시 `03_PR_Document_Writing_Prompt (KR).md`
  - 반복 확인 시 `00_Index_Writing_Prompt (KR).md`
- 처리 결과:
  - 현재는 후보 기록만 수행.
