# Unreal Engine Working Rule Prompt

## 1. 목적

Unreal Engine C++ 작업을 진행할 때 반복 사용할 탐색, 계획, 구현, 검증 기준을 제공한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Unreal Engine C++ 기능 구현 또는 리팩터링을 시작할 때
-> C++ / Blueprint / Asset / Editor 작업 경계를 먼저 나눠야 할 때
-> UObject lifetime, Reflection, Delegate, Montage lifecycle 영향이 있는 작업을 검토할 때
-> Build / PIE / Editor / Asset 검증 가능성을 분리해야 할 때
```

---

## 3. 사용 방법

작업 요청과 함께 `복사용 Prompt`를 전달한다.

프로젝트별 규칙이 추가로 필요하면 이 Prompt를 먼저 적용하고, 프로젝트 전용 Prompt를 추가로 적용한다.

---

## 4. 복사용 Prompt

````text
Unreal Engine C++ 작업을 진행해줘.

작업 방식은 아래 기준을 따른다.

1. 먼저 현재 구조를 확인한다.
   - 관련 C++ class, struct, enum, component, subsystem, UObject, DataAsset, AnimNotify, Montage, Blueprint 연결 지점을 먼저 읽는다.
   - 추측으로 바로 구현하지 않는다.
   - 기존 문서나 주석이 있다면 현재 코드와 같은 시점의 설명인지 구분한다.

2. 구현 전 짧은 계획을 공유한다.
   - 어떤 책임을 어느 객체에 둘지 먼저 설명한다.
   - 변경 범위와 비범위를 구분한다.
   - Asset 또는 Blueprint 작업이 필요한 경우 C++ 변경과 분리해서 적는다.

3. Unreal 책임 경계를 우선한다.
   - Actor는 소유자, Component 구성 / 연결 주체, 외부 진입점 역할을 담당한다.
   - ActorComponent는 기능 단위 상태, lifecycle, 실행 / 적용 API를 담당한다.
   - Subsystem은 전역 또는 World 단위 조율을 담당한다.
   - Runtime Object는 런타임에 생성되는 실행 단위 객체로서, 개별 실행 정책과 runtime 상태를 담당한다.
   - DataAsset / DataTable은 설정과 lookup 데이터를 담당한다.
   - AnimNotify / Montage는 animation timing event를 전달한다.
   - Blueprint / Widget은 C++ API 위에서 구성, 연결, 표시를 담당한다.

세부 책임 경계와 구현 판단은 함께 제공된 `01_Unreal_Engine_Working_Reference_Prompt (KR).md` 기준을 따른다.

4. Unreal C++ 관례를 지킨다.
   - UObject lifetime과 GC를 고려한다.
   - UObject 참조는 필요한 경우 UPROPERTY로 보관한다.
   - Blueprint에 노출해야 하는 API만 UFUNCTION / UPROPERTY로 노출한다.
   - BeginPlay, Tick, EndPlay, Montage lifecycle, Delegate binding / unbinding 시점을 명확히 한다.
   - raw pointer 사용 시 nullptr / IsValid 조건을 분명히 한다.
   - cached component는 초기화 시점과 invalid 상태를 함께 고려한다.

5. C++ 변경과 Blueprint / Asset 작업을 분리한다.
   - C++에서 필요한 class / property / function / event를 먼저 정의한다.
   - Blueprint, Montage, Notify, DataAsset, Widget에서 설정해야 할 항목은 별도 작업으로 기록한다.
   - 코드만으로 검증 가능한 항목과 Editor에서 확인해야 하는 항목을 구분한다.

6. 구현 후 가능한 검증을 수행한다.
   - 가능하면 Unreal C++ Build를 수행한다.
   - Build를 수행하지 못하면 이유를 기록한다.
   - PIE, Editor, Asset 검증이 필요한 경우 미검증 항목으로 남긴다.

7. 구조 변경이 있으면 문서화 필요성을 판단한다.
   - 책임 경계, 실행 flow, lifecycle, data contract가 바뀌면 문서 업데이트가 필요한지 판단한다.
   - 단순 구현 변경은 최종 요약에 남긴다.
   - 아직 구현하지 않은 구조를 현재 구조처럼 쓰지 않는다.

결과를 정리할 때는 다음 항목을 구분한다.
- 현재 구조 요약
- 책임 경계 판단
- 변경 계획 또는 변경 결과
- C++ 변경 / Blueprint / Asset 필요 작업
- 수행한 검증
- 미검증 항목
- 후속 작업 범위

주의:
- 사용자 변경을 되돌리지 않는다.
- 관련 없는 파일을 정리하지 않는다.
- 큰 구조 변경은 작은 단계로 나눈다.
- 이름 변경은 Asset / Blueprint / serialized data 영향 범위를 확인한 뒤 수행한다.
- 검증하지 않은 항목은 완료로 표현하지 않는다.
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 작업 목표
-> 관련 파일 / 클래스 / 시스템 이름
-> 변경하려는 기능 범위
-> 참고해야 할 기존 코드 또는 문서
-> 사용 가능한 검증 환경
-> 사용자가 직접 확인해야 하는 Editor / Asset 항목
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 현재 구조 요약
-> 책임 경계 판단
-> 변경 계획 또는 변경 결과
-> C++ 변경과 Blueprint / Asset 필요 작업 분리
-> 수행한 검증
-> 미검증 항목
-> 후속 작업 범위
```

---

## 7. 범위 / 비범위

```yaml
범위
-> Unreal Engine C++ 작업 규칙
-> C++ / Blueprint / Asset / Editor 경계 판단
-> Unreal 객체 책임과 lifecycle 기준
-> Build / PIE / Editor / Asset 검증 구분

비범위
-> 이 Prompt 단독으로 특정 프로젝트의 고유 구조를 최종 판단하는 것
-> 특정 기능의 구현 세부 설계
-> 실제 Editor / Asset 조작
-> 프로젝트별 문서 작성 양식
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 확인하지 않은 Blueprint / Asset 상태를 완료로 단정하지 않음
-> C++ Build 성공을 PIE / Editor / Asset 검증 완료로 해석하지 않음
-> AnimNotify / Montage를 gameplay policy 자체로 보지 않음
-> Blueprint에 숨겨진 핵심 상태를 C++ 검증 완료처럼 취급하지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 관련 코드 / 문서를 먼저 탐색
-> 확인 가능한 정보와 사용자 확인이 필요한 정보를 분리
-> Blueprint / Asset / Editor 확인이 필요한 항목은 질문 또는 미검증으로 표시
-> 구조 변경 위험이 크면 구현 전 선택지로 분리
```

---

## 10. 검증 기준

```yaml
검증 기준
-> Build: C++ compile / link 확인
-> Code Flow: 호출 흐름, 타입, include, reflection macro 확인
-> PIE: 입력, collision, montage, runtime 동작 확인
-> Editor: Blueprint compile, exposed property, DataAsset, reference 확인
-> Asset: Montage notify, animation, sound, Niagara, material reference 확인
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 요청 범위의 C++ 변경 또는 분석이 끝나고 가능한 검증 결과가 명시됨

실패
-> Build 실패, 구조 확인 실패, 필수 파일 접근 실패, 요구 조건 충돌이 있음

미검증
-> PIE / Editor / Asset / Blueprint 확인이 필요하지만 현재 환경에서 확인하지 못함
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Unreal Engine Working Rule Prompt
-> Unreal Engine 작업에 공통 적용할 실행 규칙 제공

Project-specific Working Reference Prompt
-> 프로젝트 구조가 Unreal 공통 규칙과 맞거나 다른 지점을 판단하는 Reference 기준 제공

Unreal Engine Working Reference Prompt
-> Unreal Engine 작업의 책임 경계, 구현 원칙, Blueprint / Asset 경계 기준 제공

Project-specific Working Rule Prompt
-> 특정 프로젝트의 구조, 명칭, 작업 방식에 맞춘 추가 규칙 제공
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> Network Replication 작업 기준
-> Gameplay Ability System 사용 기준
-> UMG / Slate 작업 기준
-> AI / BehaviorTree 작업 기준
-> Asset rename / redirector 처리 기준
-> Profiling / Optimization 작업 기준
```
