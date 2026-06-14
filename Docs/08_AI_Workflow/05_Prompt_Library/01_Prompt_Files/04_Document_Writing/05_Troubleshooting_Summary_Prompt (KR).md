# Troubleshooting Summary Prompt

## 1. 목적

여러 Bug Report, Architecture Issue Report, Engine Issue Report를 문제 유형, 공통 원인, 수정 패턴, 재발 방지 기준 중심의 Portfolio Document 하위 Troubleshooting 유형으로 압축한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 여러 Bug Report / Architecture Issue Report / Engine Issue Report를 제출용 Troubleshooting 기술 문서로 묶을 때
-> 개별 이슈보다 원인 유형과 해결 패턴을 설명해야 할 때
-> Portfolio Document 안에서 문제 해결 패턴을 별도 주제로 압축해야 할 때
```

---

## 3. 사용 방법

관련 Bug Report / Architecture Issue Report / Engine Issue Report 묶음과 참고 문서를 제공하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
아래 Bug Report / Architecture Issue Report / Engine Issue Report 묶음을 기반으로 Portfolio Document 하위 Troubleshooting 유형 문서를 작성 또는 보완해줘.

관련 이슈 문서:
- [Bxx 문서 / issue]

관련 문서:
- Work List:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Document:

작성 목표:
- 개별 이슈 문서를 시간순으로 나열하지 말고 문제 유형별로 묶어줘.
- 문제 유형 -> 관련 이슈 -> 공통 원인 -> 해결 패턴 -> 검증 기준 -> 정리 흐름으로 작성해줘.
- Unreal Engine 특성, AI state, Damage pipeline, Action / Reaction execution flow 같은 기술 축으로 분류해줘.
- 확인되지 않은 원인이나 검증은 단정하지 말고 확인 필요로 분리해줘.

권장 구조:
1. 문서 목적
2. 문제 분석 방식
3. 문제 유형
4. 문제 유형
5. 문제 유형
6. 정리
````

---

## 5. 입력 기준

```yaml
입력 기준
-> Bug Report / Architecture Issue Report / Engine Issue Report 목록
-> 관련 Work List / System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records / Verification Log / PR Document
-> 문제 유형 후보
-> 검증 결과
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 문제 유형별 묶음
-> 관련 이슈 문서
-> 공통 원인
-> 해결 패턴
-> 검증 기준
-> 재발 방지 기준
```

---

## 7. 범위 / 비범위

```yaml
범위
-> 여러 Bug Report / Architecture Issue Report / Engine Issue Report 압축 / 재구성
-> Portfolio Document 하위 Troubleshooting 유형 작성

비범위
-> 개별 Bug Report 작성
-> 개별 Architecture Issue Report 작성
-> 개별 Engine Issue Report 작성
-> 버그 수정 구현
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 버그를 시간순으로만 나열하지 않음
-> 확인되지 않은 원인을 단정하지 않음
-> 최신 구조와 버그 발생 당시 구조를 섞지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 문제 유형 후보로 분리
-> 원인 확인 필요 항목으로 표시
```

---

## 10. 검증 기준

```yaml
검증 기준
-> 문제 유형과 공통 원인이 연결되는가
-> 해결 패턴과 검증 기준이 연결되는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> Bug Report / Architecture Issue Report / Engine Issue Report 묶음이 Portfolio Document 하위 Troubleshooting 유형으로 재구성됨

실패
-> Bug Report / Architecture Issue Report / Engine Issue Report 근거 부족

미검증
-> 원인 또는 검증 결과 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Bug Report Writing Prompt
-> 개별 버그 분석 문서 작성

Architecture Issue Report Writing Prompt
-> 시스템 구조 / 책임 경계 / 설계 흐름에서 발생한 문제와 위험 기록

Engine Issue Report Writing Prompt
-> Unreal Engine 동작, 설정, API 사용 중 발생한 기술 이슈 분석

Troubleshooting Summary Prompt
-> 여러 Bug Report / Architecture Issue Report / Engine Issue Report를 Portfolio Document 하위 Troubleshooting 유형으로 압축
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> Troubleshooting 유형 분류 기준
-> Portfolio Document 내부 유형으로 유지할지 별도 Prompt로 유지할지 실사용 후 재검토
```
