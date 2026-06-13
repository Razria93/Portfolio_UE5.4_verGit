# AI Workflow Refactor Notes

## 1. 목적

본 문서는 `AI Workflow v1` 구축 중 발견한 구조적 보완점과 다음 Refactor 범위에서 참고할 요구사항을 정리한다.

현재 Branch는 AI Workflow 문서 구조와 Prompt Library v1 초안을 닫는 것을 목표로 하며, 본 문서의 항목은 다음 AI Workflow Refactor 또는 실제 기능 구현 Branch에서 검증 / 보완한다.

---

## 2. 현재 상태 요약

```yaml
현재 Branch에서 구성된 것
- AI Workflow 문서 구조
- Prompt Blueprint
- Prompt Files 초안
- Work Brief Intake
- Feature Work Planning
- Work List Writing
- W02 Parry 예시 흐름

현재 Branch에서 검증한 것
- 자연어 요청을 Work Brief로 정리
- Work Brief를 Feature Work Planning으로 변환
- Feature Work Planning을 Work List Draft로 변환
- 실제 구현 없이 문서 변환 흐름만 검증

현재 Branch에서 검증하지 않은 것
- 실제 기능 구현 중 Prompt 사용성
- Work List의 장기 갱신성
- Verification Log / PR Document 연계
- System Architecture / Engine Technique 문서 체계
- Prompt Library 전체 품질
```

---

## 3. 상위 구조 청사진

AI Workflow는 다음 계층으로 다시 정리할 필요가 있다.

```text
자연어 요청
-> Work Brief Intake
-> 작업 유형 판정
-> Planning 공정 선택
   -> Prompt Planning
   -> Work Planning
   -> Review / Verification Planning
   -> Document Writing Planning
   -> Git Operation Planning
-> 필요 시 Work List Writing
-> 실행 공정
   -> 구현
   -> Document Writing
   -> Review / Verification
   -> Verification Log
   -> PR Document Writing
   -> Git Operation
-> Work List Update
```

현재 v1은 `Work Brief Intake`, `Feature Work Planning`, `Refactor Work Planning`, `Work List Writing`을 우선 구성했다.

다음 Refactor에서는 위 계층과 실제 폴더 / Prompt 이름 / 문서 역할이 같은 모델을 공유하는지 점검해야 한다.

---

## 4. 카테고리 재편성 후보

현재 Prompt Files는 역할별 폴더를 갖췄지만, 새 계층 구조와 완전히 일치하지 않는다.

```yaml
재검토 대상
- Prompt Planning
- Work Planning
- Execution
- Document Writing
- Review / Verification
- Git Operation
- Working Rule / Reference
```

검토 기준은 다음으로 둔다.

```yaml
분류 기준
- 요청을 정리하는 Prompt인가
- 계획을 만드는 Prompt인가
- 실제 실행을 지시하는 Prompt인가
- 검증 / 리뷰를 수행하는 Prompt인가
- 문서를 작성하는 Prompt인가
- Git / PR 전 점검을 수행하는 Prompt인가
- 다른 Prompt가 참조하는 기준 문서인가
```

우선순위는 `Work Planning`, `Document Writing`, `Review / Verification` 순서가 높다.

---

## 5. Overview 재정의

현재 `AI_Workflow_Overview (KR).md`는 상위 개요 문서이지만, 초기 기획 / Draft 성격이 일부 남아 있다.

다음 Refactor에서는 다음 방향을 검토한다.

```yaml
권장 방향
- 현재 Overview의 상세 기획성 내용은 Draft 또는 History 성격으로 분리
- 새 Overview는 전체 구조 요약 문서로 축약
- 읽는 순서, 주요 문서 역할, 핵심 공정, Prompt 계층만 간결하게 설명
```

Overview는 운영 규칙을 직접 소유하지 않고, `Operation Guide`, `Work Pipeline`, `Prompt Flow and Routing Blueprint`를 연결하는 상위 요약 역할에 집중한다.

---

## 6. Prompt Flow / Routing 보강 후보

현재 `Prompt Flow and Routing Blueprint`는 입력 / 라우팅 / 계획 / Work List / 실행 / 참조 계층을 설명한다.

다음 Refactor에서는 더 정교한 공정 분기를 검토한다.

```yaml
Prompt Planning
- Feature Prompt Planning
- Refactor Prompt Planning
- Update Prompt Planning

Work Planning
- Feature Work Planning
- Refactor Work Planning
- Update Work Planning

실행 전 Planning 후보
- Review / Verification Planning
- Document Writing Planning
- Git Operation Planning
```

단, 모든 Planning Prompt를 한 번에 만들 필요는 없다.

반복 사용 필요성이 확인된 공정부터 분리한다.

---

## 7. Work Brief 운영 보완

W02에서 Work Brief는 사용자 입력 양식보다 채팅 주도 운영 방식이 더 적합하다는 점이 확인됐다.

다음 기준은 유지한다.

```yaml
유지할 기준
- 사용자는 자연어로 요청
- Codex가 Work Brief 문서 갱신
- 사용자 원문과 Codex 해석을 중복 보존하지 않음
- 현재 합의된 작업 개요만 남김
- 계획차단 / 비차단, 검토필요 / 선택필요를 섹션으로 분리
```

보완 후보는 다음과 같다.

```yaml
보완 후보
- Work Brief를 더 짧게 유지하는 기준
- Feature Work Planning으로 넘길 최소 필드 계약
- 비차단 / 검토필요 항목의 처리 위치
- 진행 가능 / 준비 완료 / 구현 착수 가능 상태 전이 규칙
```

---

## 8. Work Planning 보완

`Feature Work Planning`은 W02에서 유효했지만, 실제 구현 브랜치에서 사용성이 검증되지는 않았다.

다음 Refactor에서는 실제 구현 결과를 바탕으로 다음 항목을 점검한다.

```yaml
점검 항목
- 선행 확인 단위가 과도하게 길지 않은가
- 구현 단위가 실제 Commit 단위와 맞는가
- 검증 기준이 Work List로 잘 변환되는가
- 사용자 결정 필요 항목이 적절한 시점에 드러나는가
- 문서화 필요 여부가 과도하게 넓지 않은가
```

구현 공정은 다음 단위로 나눌 수 있는지 검토한다.

```yaml
구현 공정 후보
- API 구성
- 흐름 연결
- 정책 결정
- 구체화
- 안정성 검토
```

이 분류는 아직 확정 기준이 아니며, W02 실제 구현에서 적합성을 확인한다.

---

## 9. Work List 위치와 갱신 규칙

Work List는 실행 공정 자체가 아니라 계획을 시각화 / 관리 / 기록하는 상태 문서다.

권장 흐름은 다음이다.

```text
Work Brief
-> Work Planning
-> Work List 필요 여부 판단
-> 필요 시 실행 전 Work List 작성
-> 각 실행 공정 후 Work List Update
-> PR 전 Work List 기준으로 완료 / 검증 상태 확인
```

다음 Refactor에서 정리할 항목은 다음이다.

```yaml
정리 필요
- Work List를 언제 생략할 수 있는가
- Work List Draft와 공식 Work List 승격 기준
- 실행 후 Work List Update 형식
- Verification Log / PR Document와 중복되는 항목
```

---

## 10. 공정 시각화와 추적성

AI 작업은 코드 호출 흐름처럼 추적이 어렵기 때문에, 작업별 추적 항목을 정리할 필요가 있다.

후속으로 다음 항목을 기록하는 방식을 검토한다.

```yaml
작업 추적 항목
- 사용한 Prompt
- 참고한 Prompt
- 참고한 문서
- 참고한 코드 / Asset
- 변경한 파일
- 생성한 문서
- 수행한 검증
- 미검증 항목
- 후속 작업
```

이 정보는 `History`, `Work List`, `Verification Log`, `PR Document` 중 어디에서 관리할지 결정해야 한다.

권장 방향은 다음과 같다.

```yaml
권장 방향
- Work List: 진행 상태와 변경 관리
- Verification Log: 검증 / 미검증 기록
- PR Document: 최종 변경 요약
- History: 장기 작업 맥락과 의사결정 흐름
```

---

## 11. 문서 체계 보완

System Architecture와 Engine Technique 계층은 다음 Refactor에서 정리해야 한다.

현재 확정한 문서 카테고리는 다음 기준이다.

```yaml
작업 / 상태
- Work List
- Verification Log
- PR Document

문제 기록
- Bug Report

시스템 구조
- System Architecture
- System Design Records
- Architecture Decision Record
- Architecture Issue Report

엔진 기술 사용
- Engine Technique Document
- Engine Implementation Records
- Engine Decision Record
- Engine Issue Report

포트폴리오 설명
- Portfolio Technical Document
```

검토할 항목은 다음이다.

```yaml
검토 항목
- System Architecture는 현재 구조 설명만 담당하는가
- Architecture Decision Record는 선택지와 선택 이유만 담당하는가
- Architecture Issue Report는 구조 문제 / 책임 경계 이슈를 담당하는가
- Engine Technique Document는 Unreal 기능 / API 사용 방식을 설명하는가
- Engine Decision Record와 Engine Issue Report가 System 계열과 같은 위계로 다뤄지는가
- Technical Document라는 넓은 이름이 필요한가
```

---

## 12. History 체계 후보

History 문서 체계는 아직 구성되지 않았다.

다음 Refactor에서 History를 별도 문서로 둘지, 기존 산출물 조합으로 대체할지 결정한다.

```yaml
History가 필요한 경우
- AI와 설계하며 고민한 흐름을 남겨야 함
- 의사결정의 맥락이 ADR만으로 부족함
- 여러 Branch에 걸친 설계 변화가 있음
- 포트폴리오 설명에서 사고 과정을 보여줄 가치가 있음

History를 생략할 수 있는 경우
- Work List / Verification Log / PR Document로 충분히 추적 가능함
- 의사결정은 ADR / Records로 충분히 설명됨
```

권장 기본값은 `보류`다.

W02 실제 구현 후 기록 추적이 부족하다고 느껴질 때 별도 체계로 분리한다.

---

## 13. 검증 / 재검토 Prompt 보강

`Document_Set_Audit_Prompt`는 문서군 정합성 감사에 유효했지만, 다음 축을 더 보강할 수 있다.

```yaml
보강 후보
- 상태 전이 규칙 검토
- Prompt 계층 충돌 검토
- Work Brief / Planning / Work List 필드 계약 검토
- 실행 공정 후 문서 갱신 누락 검토
- Prompt Flow와 실제 사용 흐름 불일치 검토
```

사용자가 자주 보는 검토 관점도 Prompt에 반영할 수 있다.

```yaml
사용자 검토 관점
- 구조 / 구성 / 문맥 / 흐름
- 문서 역할 적합성
- 용어 일관성
- 문서 간 연계성
- 실제 운영 가능성
- 과설계 여부
- 다음 Branch에서 다시 읽어도 이해 가능한가
```

---

## 14. Commit / PR Prompt 보강

Git / PR 계층은 존재하지만, 실제 커밋 분리와 staging 판단에 대한 Prompt는 더 보강할 수 있다.

검토할 항목은 다음이다.

```yaml
검토 항목
- 현재 Branch 목표와 Commit 대상 일치 여부
- Draft / Request / Backlog 문서 포함 여부
- 실제 구현 파일과 문서 파일 분리 기준
- staged / unstaged / untracked 분리
- 커밋 메시지 추천 기준
- PR 문서 작성 전 필수 검증 상태
```

실제 Commit / PR 직전 사용성을 기준으로 보완한다.

---

## 15. 양식과 기능 분리

현재 일부 Prompt는 복사용 Prompt와 결과 양식을 같은 파일에 포함한다.

다음 Refactor에서 다음 기준을 검토한다.

```yaml
분리 후보
- Work Brief Format
- Work List Format
- Verification Log Format
- PR Document Format
- Document Set Audit Output Format
```

분리 기준은 다음으로 둔다.

```yaml
분리하는 경우
- 여러 Prompt가 같은 양식을 반복 사용함
- Prompt 파일이 너무 길어짐
- 양식만 따로 갱신할 필요가 있음

분리하지 않는 경우
- 아직 실사용 사례가 적음
- 양식이 특정 Prompt에만 종속됨
- 분리로 탐색 비용이 늘어남
```

권장 기본값은 `반복 사용이 확인된 뒤 분리`다.

---

## 16. W02 예시의 한계

W02는 좋은 실험이었지만, 실제 운용 기준으로는 과하게 자세한 부분이 있다.

다음 구현 Branch에서 확인할 항목은 다음이다.

```yaml
W02에서 확인할 것
- Work Brief가 충분히 짧은가
- Feature Work Planning이 실제 구현에 도움이 되는가
- Work List Draft가 실행 관리에 유용한가
- 문서 작성 시간이 구현 시간을 과도하게 침식하지 않는가
- Planning 결과가 구현 중 얼마나 자주 바뀌는가
```

W02는 표준 완성형이라기보다 v1 실험 사례로 본다.

실제 구현 후 더 짧고 반복 가능한 양식으로 축약한다.

---

## 17. 우선순위 제안

다음 AI Workflow Refactor에서 우선순위는 다음 순서를 권장한다.

```yaml
1순위
- Work Brief / Feature Work Planning / Work List 실사용 결과 반영
- Prompt Flow / Routing 계층과 폴더 구조 정합성 점검
- Work List 위치 / 갱신 규칙 정리

2순위
- Document Writing Prompt와 문서 카테고리 체계 재정리
- System Architecture / Engine Technique 문서 역할 분리
- Verification Log / PR Document 연계 정리

3순위
- Prompt Planning 계열 추가 여부 검토
- History 문서 체계 신설 여부 검토
- 양식 / 기능 분리
- Commit / PR Prompt 고도화
```

---

## 18. 다음 작업 제안

현재 Branch 이후 권장 흐름은 다음이다.

```text
1. 현재 AI Workflow Branch 마감
2. W02 실제 Parry 구현 Branch 진행
3. 구현 중 Work Brief / Planning / Work List 사용성 기록
4. 구현 완료 후 Verification Log / PR Document 작성
5. 실제 사용 결과를 바탕으로 AI Workflow Refactor 진행
```

지금 시점에서 더 많은 Prompt를 만들기보다, 실제 구현에서 깨지는 지점을 확인한 뒤 보완하는 것이 비용 대비 효율이 높다.
