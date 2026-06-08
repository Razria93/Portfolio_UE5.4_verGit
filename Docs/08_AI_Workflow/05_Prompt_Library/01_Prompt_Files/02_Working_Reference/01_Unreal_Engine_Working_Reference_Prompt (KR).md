# Unreal Engine Working Reference Prompt

## 1. 목적

Unreal Engine C++ 작업에서 책임 경계, 구현 원칙, Blueprint / Asset 경계, 검증 기준을 판단할 때 참고할 기술 기준을 제공한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Unreal Engine 작업의 객체 책임 경계를 검토할 때
-> Actor / Component / Subsystem / Runtime Object 역할을 구분해야 할 때
-> UObject lifetime, Reflection, Delegate, Montage lifecycle 영향을 판단할 때
-> C++ / Blueprint / Asset / Editor 작업과 검증 경로를 분리해야 할 때
```

---

## 3. 사용 방법

`01_Unreal_Engine_Working_Rule_Prompt (KR).md`를 적용한 뒤, 세부 책임 경계나 구현 판단이 필요한 경우 이 Reference Prompt를 함께 참고한다.

이 Prompt는 작업을 직접 지시하기보다, Unreal Engine 작업 판단의 기준으로 사용한다.

---

## 4. 복사용 Prompt

````text
Unreal Engine C++ 작업의 책임 경계와 구현 기준을 검토해줘.

다음 기준으로 현재 구조 또는 제안된 구조를 확인한다.

1. 객체별 책임 경계를 확인한다.
   - Actor는 소유자, Component 구성 / 연결 주체, 외부 진입점 역할에 적합한지 확인한다.
   - ActorComponent는 기능 단위 상태, lifecycle, 실행 / 적용 API에 적합한지 확인한다.
   - Subsystem은 World, GameInstance, Editor 범위의 공통 조율 계층으로 필요한지 확인한다.
   - Runtime Object는 런타임에 생성되는 실행 단위 객체로서, 개별 실행 정책과 runtime 상태를 담당하는지 확인한다.
   - DataAsset / DataTable은 설정, 참조, lookup 데이터의 출처로 사용되는지 확인한다.
   - AnimNotify / Montage는 animation timing event 전달 역할을 넘어서지 않는지 확인한다.
   - Blueprint / Widget은 C++ API 위에서 구성, 연결, 표시를 담당하는지 확인한다.

2. Unreal C++ 구현 원칙을 확인한다.
   - Reflection 노출 범위가 필요한 contract로 제한되어 있는지 확인한다.
   - UObject lifetime, outer, GC, 참조 보관 위치가 명확한지 확인한다.
   - Component cache 초기화 시점과 invalid 상태 처리가 명확한지 확인한다.
   - Delegate binding / unbinding과 Montage lifecycle이 정리되어 있는지 확인한다.
   - 큰 구조 변경은 호출 흐름 안정화, 책임 경계 분리, helper화, 이름 정리 순서로 단계화한다.

3. C++ / Blueprint / Asset / Editor 경계를 분리한다.
   - C++ 변경으로 처리할 항목과 Blueprint / Asset 설정이 필요한 항목을 분리한다.
   - Code Flow로 확인 가능한 항목과 PIE / Editor / Asset 확인이 필요한 항목을 분리한다.
   - 확인하지 못한 Blueprint / Asset 항목은 미검증으로 남긴다.

결과는 다음 형식으로 정리한다.
- 책임 경계 판단
- 적합한 책임
- 피해야 할 책임
- 구현 원칙 위반 가능성
- Blueprint / Asset / Editor 확인 필요 항목
- 미검증 항목
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 검토할 기능 또는 구조
-> 관련 class / component / subsystem / UObject
-> 관련 DataAsset / DataTable / AnimNotify / Montage / Blueprint / Widget
-> 현재 구현 흐름 또는 제안된 변경 흐름
-> 검증 가능한 환경과 미검증 가능성이 있는 항목
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 책임 경계 판단
-> 객체별 적합한 책임
-> 객체별 피해야 할 책임
-> 구현 원칙 위반 가능성
-> C++ / Blueprint / Asset / Editor 경계
-> 검증 필요 항목
-> 미검증 항목
```

---

## 7. 범위 / 비범위

```yaml
범위
-> Unreal Engine C++ 책임 경계 판단
-> Unreal C++ 구현 원칙 검토
-> Blueprint / Asset / Editor 경계 기록 기준
-> 작업 전 / 구현 전 / 구현 후 체크리스트

비범위
-> 특정 프로젝트의 고유 구조 최종 판단
-> 코드 직접 수정
-> 실제 Editor / Asset 조작
-> 프로젝트별 문서 작성 양식
```

---

## 8. Unreal C++ 설계 원칙

### 1) Actor

Actor는 외부 진입점과 소유자 역할을 담당한다.

```yaml
적합한 책임
-> Component 구성 / 연결
-> Engine event 수신
-> 외부 Actor와의 high-level 연결

피해야 할 책임
-> 세부 기능 상태를 모두 직접 보관
-> 하위 Component의 정책을 대신 판단
-> 복잡한 gameplay / UI / AI / animation flow를 직접 처리
```

### 2) ActorComponent

ActorComponent는 기능 단위 상태와 lifecycle 적용을 담당한다.

```yaml
적합한 책임
-> 상태 보관
-> owner component cache
-> start / stop / apply / clear
-> delegate broadcast
-> domain-specific API 제공

피해야 할 책임
-> 모든 시스템의 상위 조율자 역할
-> unrelated component의 내부 정책 직접 판단
-> Asset authoring 실수를 runtime logic으로 숨김
```

### 3) Subsystem

Subsystem은 World, GameInstance, Editor 범위의 공통 조율 계층으로 사용한다.

```yaml
적합한 책임
-> 전역 registry
-> World-level query
-> 여러 Actor 사이의 coordination
-> project-wide service

피해야 할 책임
-> 개별 Actor의 세부 lifecycle 직접 소유
-> 단일 ActorComponent로 충분한 기능을 전역화
```

### 4) Runtime Object

Runtime Object는 런타임에 생성되는 실행 단위 객체로서, 개별 실행 정책과 runtime 상태를 담당한다.

```yaml
적합한 책임
-> action / reaction / ability 실행 정책
-> Montage play / stop
-> Notify command 처리
-> runtime window 또는 temporary state 보관

피해야 할 책임
-> owner Actor 전체 상태를 임의로 변경
-> 다른 domain Component를 직접 제어
-> data lookup과 orchestration을 동시에 수행
```

### 5) DataAsset / DataTable

DataAsset과 DataTable은 설정, 참조, lookup 데이터의 출처로 사용한다.

```yaml
적합한 책임
-> tuning value
-> Montage / sound / effect reference
-> type-keyed data mapping
-> designer-editable configuration

피해야 할 책임
-> runtime state 보관
-> 실행 판단 flow를 숨기는 과도한 flag 조합
```

### 6) AnimNotify / Montage

AnimNotify와 Montage는 animation timing event를 전달한다.

```yaml
적합한 책임
-> 특정 timing에 C++ API 호출
-> window begin / end 전달
-> Montage section / Notify command 전달

피해야 할 책임
-> gameplay policy 자체를 Notify 배치로만 표현
-> source / target 관계를 Asset에서 불명확하게 작성
```

### 7) Blueprint / Widget

Blueprint와 Widget은 C++ API 위의 구성 / 연결 / 표시 계층으로 다룬다.

```yaml
적합한 책임
-> designer-facing setup
-> UI binding
-> Asset reference 연결
-> simple event graph composition

피해야 할 책임
-> C++에서 추적해야 할 핵심 상태를 숨김
-> complex branching policy를 graph 안에만 보관
-> C++ 검증이 불가능한 gameplay core logic 작성
```

---

## 9. Unreal C++ 구현 원칙

### 1) Unreal Reflection 범위를 명확히 한다

`UCLASS`, `USTRUCT`, `UENUM`, `UPROPERTY`, `UFUNCTION`은 Engine과 Blueprint에 노출되는 contract다.

```yaml
노출 필요
-> Blueprint에서 설정해야 하는 값
-> Editor에서 조정해야 하는 값
-> serialization / GC 추적이 필요한 참조
-> delegate 또는 event binding이 필요한 API

노출 불필요
-> 내부 helper
-> transient 계산 값
-> C++ 내부에서만 쓰는 local policy
```

### 2) UObject lifetime을 먼저 고려한다

UObject 기반 객체를 만들거나 보관할 때는 outer와 보관 위치를 먼저 정한다.

```yaml
확인 항목
-> 누가 생성하는가
-> outer는 무엇인가
-> 누가 참조를 보관하는가
-> GC 대상인가
-> level transition / owner destroy 시 어떻게 정리되는가
```

### 3) Component cache는 초기화 시점을 명확히 한다

Component cache는 편리하지만 초기화 시점과 invalid 상태를 함께 관리해야 한다.

```yaml
권장 기준
-> BeginPlay 또는 Initialize 단계에서 cache
-> 필수 Component는 check 또는 명확한 reject 처리
-> 선택 Component는 IsValid 조건으로 분기
-> runtime에 Component가 사라질 수 있는 구조라면 재조회 또는 invalid 처리 고려
```

### 4) Delegate와 Montage lifecycle을 정리한다

Delegate binding은 중복 호출과 stale callback을 만들기 쉽다.

```yaml
확인 항목
-> binding 시점
-> unbinding 시점
-> owner destroy 시 안전성
-> Montage interrupted / completed 구분
-> serial 또는 token으로 이전 callback 무시 필요 여부
```

### 5) 큰 구조 변경은 단계로 나눈다

큰 구조 변경은 호출 흐름 안정화와 책임 경계 분리를 먼저 확인한 뒤 단계적으로 진행한다.

```yaml
우선순위
-> 현재 호출 흐름 안정화
-> 책임 경계 분리
-> 공통 알고리즘 helper화
-> 이름 정리
-> base class 또는 subsystem 통합
```

---

## 10. Blueprint / Asset 경계 기록 기준

C++ 변경과 Editor 작업은 같은 기능에 속하더라도 다른 검증 경로를 가진다.

```yaml
C++ 변경
-> class / struct / enum / component / function 추가 또는 수정

Blueprint 작업
-> graph 연결
-> event binding
-> exposed property 설정

Asset 작업
-> Montage notify 배치
-> DataAsset 값 설정
-> animation / sound / Niagara / material reference 연결

검증 작업
-> C++ Build
-> PIE runtime
-> Editor visual check
-> Asset reference check
```

Asset 작업이 필요하지만 수행하지 못했다면 다음처럼 남긴다.

```yaml
미검증
-> Montage notify timing은 Editor에서 확인 필요
-> Blueprint exposed property 연결은 확인 필요
-> DataAsset reference 설정은 확인 필요
```

---

## 11. 작업 체크리스트

### 시작 전

```yaml
관련 파일 검색
관련 타입 / enum / struct 확인
관련 Component / Subsystem 확인
Blueprint / Asset 연결 여부 확인
현재 호출 flow 확인
변경 범위 / 비범위 정리
```

### 구현 전

```yaml
책임 경계 결정
public API 변경 여부 확인
Reflection 노출 필요 여부 확인
UObject lifetime / GC 영향 확인
Delegate / Montage lifecycle 영향 확인
Blueprint / Asset 수정 필요 여부 기록
```

### 구현 후

```yaml
Build 또는 정적 검증 수행
PIE / Editor 검증 필요 여부 기록
문서 업데이트 필요 여부 판단
미검증 항목 명시
사용자 변경을 되돌리지 않았는지 확인
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Unreal Engine Working Rule Prompt
-> Unreal Engine 작업에 공통 적용할 실행 규칙 제공

Unreal Engine Working Reference Prompt
-> Unreal Engine 작업의 책임 경계, 구현 원칙, Blueprint / Asset 경계 기준 제공

Project-specific Working Reference Prompt
-> 프로젝트 구조가 Unreal 공통 규칙과 맞거나 다른 지점을 판단하는 Reference 기준 제공
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
