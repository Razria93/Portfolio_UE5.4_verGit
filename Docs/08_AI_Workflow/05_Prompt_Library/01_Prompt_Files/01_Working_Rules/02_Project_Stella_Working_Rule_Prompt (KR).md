# Project Stella Working Rule Prompt

## 1. 목적

Project Stella 작업 세션에서 반복 적용할 실행 규칙을 제공한다.

이 Prompt는 Project Stella 작업을 시작할 때 현재 코드 / 문서 기준, 작업 범위, 검증 가능성, 문서화 필요성을 먼저 고정하기 위한 Prompt다. 프로젝트 구조 판단이 필요한 경우 `02_Project_Stella_Working_Reference_Prompt (KR).md`를 함께 사용한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Project Stella 코드 / 문서 작업을 시작할 때
-> 현재 코드와 문서의 기준 시점이 다를 가능성이 있을 때
-> C++ 완료 상태와 Blueprint / Asset / Editor 확인 상태를 분리해야 할 때
-> 구현 전 작업 범위와 검증 방법을 짧게 고정해야 할 때
```

---

## 2.1 코드 주석 / 섹션 규칙

```yaml
섹션 구분 주석
-> 파일 구조 탐색을 돕는 경우에만 둔다.
-> 함수 그룹 / 타입 그룹 / 구현 책임이 3개 이상으로 나뉠 때 우선 사용한다.
-> 짧은 명사구를 사용하고 장식성 separator, [NOTE], [Policy], [Pass] 태그는 사용하지 않는다.
-> 변수 / UPROPERTY 구간은 섹션 주석보다 Category / 변수명 / struct 이름으로 의미를 표현한다.

.h / .cpp 섹션 동기화
-> .h가 API 책임 단위로 섹션을 나누면 .cpp도 같은 책임 그룹 기준으로 섹션을 둔다.
-> .cpp 섹션명과 순서는 가능하면 .h를 따른다.
-> 구현 전용 helper / local namespace / static helper / 세부 pipeline 단계는 .cpp 전용 섹션으로 둘 수 있다.
-> 작은 파일이나 함수 수가 적은 파일은 섹션을 생략할 수 있다.
-> Unreal lifecycle / callback / 구현 흐름이 더 중요한 경우 구현 흐름을 우선한다.

Type 헤더 섹션
-> Type 헤더 섹션명은 W05 Comment Section Cleanup Work Plan의 taxonomy를 따른다.
-> Enum, Key / Identifier, Data / Config, Runtime State, Runtime Context, Request, Candidate, Payload, Resolution, Result, Packet, Runtime Key / Playback Key, Reserved Pipeline Scaffold, Helper API를 우선 사용한다.

단계형 주석
-> fallback 순서, policy gate, priority matching처럼 순서 자체가 의미 있을 때만 사용한다.
-> 번호 깊이는 한 단계까지만 허용한다.
-> 2-3-1 같은 중첩 번호는 의미 있는 문장형 주석으로 바꾼다.
-> Gate / Preferred / Fallback / Final fallback 같은 prefix를 일관되게 사용한다.

금지
-> 주석 정리 작업에서 타입명 / 필드명 / 함수명 / 동작을 함께 변경하지 않는다.
-> include 변경, DataAsset 전환, serialized field 변경을 주석 정리와 묶지 않는다.
```

---

## 3. 사용 방법

작업 요청과 함께 `복사용 Prompt`를 전달한다.

Unreal Engine 공통 규칙이 필요한 경우 `01_Unreal_Engine_Working_Rule_Prompt (KR).md`를 먼저 적용한다. Project Stella 구조, 용어, 책임 경계 판단이 필요한 경우 `02_Project_Stella_Working_Reference_Prompt (KR).md`를 추가로 참고한다.

---

## 4. 복사용 Prompt

````text
Project Stella 작업을 진행해줘.

기본적으로 Unreal Engine C++ 작업 규칙을 따르되, Project Stella 작업에서는 아래 실행 규칙을 우선 적용해줘.

1. 먼저 현재 기준을 확인해줘.
   - 관련 코드, 문서, Work List의 기준 시점을 구분해줘.
   - 문서와 코드가 다르면 구현 판단은 현재 코드 기준으로 하고, 차이는 별도 정리 항목으로 남겨줘.
   - 현재 구현, 후속 설계, 미검증 항목을 섞어 완료 상태처럼 표현하지 말아줘.

2. 구현 전 짧은 계획을 공유해줘.
   - 현재 흐름
   - 바꿀 책임 경계
   - 직접 수정할 범위
   - 건드리지 않을 범위
   - 검증 방법
   - 사용자 확인이 필요한 항목

3. Project Stella 작업 맥락을 유지해줘.
   - Stella Blade 액션 시스템 분석 / 구현 포트폴리오라는 맥락을 유지해줘.
   - 원작 전체 재현이 아니라, 핵심 전투 구조를 설명 가능한 방식으로 구현하는 프로젝트로 판단해줘.
   - Action / Reaction / Damage / Feedback 책임이 섞일 가능성이 있으면 Reference 기준으로 분리해줘.

4. C++ / Blueprint / Asset / Editor 작업을 분리해줘.
   - C++로 처리할 항목
   - Blueprint 설정이 필요한 항목
   - Montage / DataAsset / VFX / SFX 같은 Asset 확인이 필요한 항목
   - Editor 또는 PIE에서만 확인 가능한 항목

5. 구현 후 가능한 검증을 수행해줘.
   - 가능하면 UE C++ Build를 실행해줘.
   - Build하지 못하면 이유를 남겨줘.
   - PIE / Editor / Asset 검증은 별도 항목으로 구분해줘.
   - 확인하지 못한 항목은 미검증으로 남겨줘.

6. 구조 변경이 있으면 문서화 필요 여부를 판단해줘.
   - 책임 경계 변경
   - execution flow 변경
   - data contract 변경
   - asset authoring 기준 변경
   - bug 원인 / 해결 과정
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 작업 목표
-> 관련 코드 / 문서 경로
-> 관련 Branch 또는 Work List
-> 사용자가 우려하는 책임 경계
-> 검증 가능한 환경
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 현재 기준 요약
-> 작업 계획 또는 변경 결과
-> 직접 수정한 범위와 건드리지 않은 범위
-> C++ / Blueprint / Asset / Editor 검증 구분
-> 미검증 항목
-> 문서화 필요 여부
```

---

## 7. 범위 / 비범위

```yaml
범위
-> Project Stella 작업 세션 실행 규칙
-> 구현 전 계획 공유 기준
-> 검증 상태 분리 기준
-> 문서화 필요 여부 판단 기준

비범위
-> Unreal Engine 일반 규칙 전체 설명
-> Project Stella 구조 / 용어 / 책임 경계의 상세 판단
-> 특정 기능의 세부 구현 계획
-> 실제 Asset / Blueprint 조작
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 현재 구현되지 않은 후속 구조를 현재 구조처럼 설명하지 않음
-> 검증하지 않은 항목을 완료로 표현하지 않음
-> 사용자 변경을 되돌리지 않음
-> 관련 없는 파일을 정리하지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 현재 코드와 문서 시점을 먼저 구분
-> 확인 가능한 정보와 사용자 확인이 필요한 정보를 분리
-> 구조 판단이 필요한 경우 Project Stella Working Reference 기준으로 분리
-> 선택이 필요한 경우 질문 또는 다음 선택지로 제시
```

---

## 10. 검증 기준

```yaml
검증 기준
-> Build
-> Code Flow
-> PIE
-> Editor
-> Asset
-> 미검증
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 요청 범위의 작업 또는 분석이 끝나고 검증 상태가 분리됨

실패
-> 필수 구조 확인 실패, Build 실패, 요구 조건 충돌 발생

미검증
-> PIE / Editor / Asset / Blueprint 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Unreal Engine Working Rule Prompt
-> Unreal Engine 작업 공통 실행 규칙 제공

Project Stella Working Rule Prompt
-> Project Stella 작업 세션에 직접 적용할 실행 규칙 제공

Project Stella Working Reference Prompt
-> Project Stella 구조, 용어, 책임 경계, 위험 후보 판단 기준 제공
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> Project Stella 작업별 검증 로그 연결 기준
-> 작업 단계별 Pipeline Prompt 연결 기준
-> 기능별 Reference 기준 추가 필요 여부
```
