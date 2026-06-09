# System Architecture Writing Prompt

## 1. 목적

현재 기준 시스템 구조, 책임 경계, 실행 흐름, 데이터 계약을 System Architecture 문서로 정리한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 현재 시스템 구조를 설명해야 할 때
-> 책임 경계 / 실행 흐름 / 데이터 계약을 정리해야 할 때
-> 설계 기록과 분리된 최신 구조 기준 문서가 필요할 때
```

---

## 3. 사용 방법

관련 Branch, Work Checklist, PR Document, 코드 구조를 제공하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
아래 내용을 System Architecture 문서로 작성 또는 보완해줘.

대상 구조:
- [시스템 / 기능 / 책임 분리 주제]

관련 문서:
- Work Checklist:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Technical Document:

작성 목표:
- 현재 기준 시스템 구조를 설명해줘.
- 설계 결정, 구조 문제, 엔진 기능 사용 방식, 엔진 이슈, 리팩터링 과정은 본문에 길게 섞지 말고 관련 문서나 Record로 연결해줘.
- 객체별 책임과 flow / pipeline을 분리해서 설명해줘.
- 코드 타입명 / API / enum / struct는 실제 이름을 사용해줘.
- 후속 구현 후보와 이미 구현된 구조를 섞지 말아줘.
- Blueprint / BehaviorTree / AnimNotify / Asset 작업은 코드 근거와 문서 근거를 구분해줘.

권장 구조:
1. 제목
2. 목적
3. 적용 범위
4. 구성 요소
5. 책임 경계
6. 실행 흐름
7. 데이터 계약
8. C++ / Blueprint / Asset / Editor 연결
9. 검증 기준
10. 관련 Record / 문서
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 구조 변경 주제
-> 관련 코드 / 문서
-> 현재 구조
-> 관련 Record
-> 검증 결과 또는 미검증 항목
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 현재 구조
-> 구성 요소
-> 책임 경계
-> 실행 흐름
-> 데이터 계약
-> 관련 Record 연결
```

---

## 7. 범위 / 비범위

```yaml
범위
-> System Architecture 작성 / 보완

비범위
-> Architecture Decision Record 작성
-> Architecture Issue Report 작성
-> Engine Technique Document 작성
-> Engine Decision Record / Engine Issue Report 작성
-> PR Document 작성
-> Portfolio Technical Document 압축
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 현재 구현과 후속 설계를 섞지 않음
-> 설계 결정 / 구조 문제 / 엔진 기능 사용 방식 / 엔진 이슈 기록을 현재 구조 설명 본문에 길게 섞지 않음
-> 문서 본문에 현재 workspace 검토 의견을 섞지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 현재 구현 / 과거 기준 / 후속 설계 여부를 분리
-> 결정 이유는 Architecture Decision Record 후보로 분리
-> 구조 문제는 Architecture Issue Report 후보로 분리
-> 엔진 기능 / API / 시스템 사용 방식 설명은 Engine Technique Document 후보로 분리
-> 엔진 사용 결정 / 엔진 동작 문제는 Engine Decision Record / Engine Issue Report 후보로 분리
-> 직접 연결 문서가 없는 항목은 참고 후보로만 분리
```

---

## 10. 검증 기준

```yaml
검증 기준
-> 책임 경계가 명확한가
-> 실행 흐름이 입력 / 판단 / 적용 / 결과로 설명되는가
-> 현재 구조와 Record / 후속 후보가 분리되는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 현재 구조와 책임 경계가 문서화됨

실패
-> 기준 시점 또는 구조 근거 확인 불가

미검증
-> Asset / Blueprint / Runtime 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
System Architecture Writing Prompt
-> 현재 시스템 구조 / 책임 경계 / 실행 흐름 문서 작성

Architecture Decision Record Writing Prompt
-> 시스템 구조 설계 결정과 선택 이유 기록

Architecture Issue Report Writing Prompt
-> 시스템 구조 / 책임 경계 / 설계 흐름에서 발생한 문제와 위험 기록

Engine Technique Document Writing Prompt
-> Unreal Engine 기능 / API / 시스템 사용 방식 설명

Engine Decision Record Writing Prompt
-> Unreal Engine 기능 사용 방식에 대한 결정과 선택 이유 기록

Engine Issue Report Writing Prompt
-> Unreal Engine 동작, 설정, API 사용 중 발생한 기술 이슈 분석

Portfolio Technical Document Writing Prompt
-> 여러 작업 기록을 포트폴리오용 기술 주제로 압축
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> System Architecture 문서 ID / 파일명 규칙
-> Architecture Decision Record / Architecture Issue Report Prompt 작성
-> Engine Technique Document / Engine Decision Record / Engine Issue Report Prompt 작성
```

