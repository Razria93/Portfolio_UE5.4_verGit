# W05 Tuning Constants Rules

## 제목

**W05: 상수 / 튜닝 데이터 분류 규칙**

## 날짜

**2026.07.25**

## 상태

- [x] 내부 규칙값 / 튜닝 데이터 / 외부 계약값 분류 기준 정리
- [x] enum / helper / constexpr 적용 기준 정리
- [x] config / UPROPERTY / DataAsset 분리 기준 정리
- [x] serialized UPROPERTY regrouping migration 기준 정리

---

## 1. 목적

이 문서는 코드에 직접 들어간 숫자 / 문자열 / 기본값을 정리할 때 반복 적용할 일반 규칙을 고정한다.

목표는 모든 literal을 기계적으로 제거하는 것이 아니다. 값의 성격을 먼저 분류한 뒤, 내부 규칙값은 코드 의미를 명확히 하고, 튜닝 데이터는 소유권을 정하며, 외부 계약값은 임의 변경하지 않는다.

프로젝트별 파일명, 현재 후보 목록, 처리 / 보류 판정은 Work Plan 문서에 둔다.

---

## 2. 분류 원칙

### 2.1 내부 규칙값

내부 규칙값은 코드 프로토콜, mode, sentinel, index, default policy처럼 값이 바뀌면 의미 계약이 깨지는 값이다.

```text
예시:
-> closed mode 값
-> legacy index mapping
-> first / initial id
-> unset timestamp
-> default policy duration
-> internal multiplier policy
```

규칙:

```text
-> config / DataAsset로 빼지 않는다.
-> 닫힌 선택지이면 enum + helper로 표현한다.
-> 단일 기준값이면 constexpr / static constexpr로 표현한다.
-> 기존 UE sentinel이 있으면 새 상수보다 INDEX_NONE, NAME_None, KINDA_SMALL_NUMBER 등을 우선한다.
-> 외부 API가 int32 / float 값을 요구하면 enum / helper 경계에서만 primitive 값으로 변환한다.
```

### 2.2 닫힌 선택지 / mode 값

닫힌 선택지 / mode 값은 raw number를 직접 비교하지 않는다.

권장:

```text
-> 의미 표현: enum class
-> 외부 int32 contract: CVar / key / legacy field에서만 유지
-> 변환 지점: helper function
```

예시:

```cpp
enum class EExampleMode : int32
{
	Default = 0,
	Reduced = 1,
	Disabled = 2,
};

EExampleMode ResolveExampleMode(int32 InValue);
int32 GetExampleModeValue(EExampleMode InMode);
```

### 2.3 단일 정책값

단일 정책값은 선택지가 아니라 기준값이다.

```text
예시:
-> default blend out time
-> default debug print duration
-> first rebuild id
-> unset timestamp
-> damage multiplier
```

규칙:

```text
-> 값의 소유 도메인 근처에 constexpr / static constexpr로 둔다.
-> 여러 파일이 공유하면 가장 작은 공용 Type / helper 헤더를 사용한다.
-> 튜닝 가능성이 큰 값은 constexpr로 확정하지 말고 튜닝 데이터 후보로 분류한다.
```

### 2.4 튜닝 데이터

튜닝 데이터는 gameplay balance, actor setup, visual / audio feedback, AI perception, interval / range / cap처럼 조정 가능한 값이다.

규칙:

```text
-> named constant만 붙이는 것은 임시 정리일 수 있다.
-> UPROPERTY, config USTRUCT, DataAsset 중 소유권을 정한다.
-> 클래스별로만 조정하면 UPROPERTY(EditDefaultsOnly / EditAnywhere)를 우선한다.
-> 여러 값이 항상 묶여 움직이면 config USTRUCT 후보로 둔다.
-> 여러 actor / Blueprint / archetype이 같은 설정 묶음을 공유하면 DataAsset 후보로 둔다.
-> DataAsset 전환은 asset 생성, editor load, Blueprint compile, PIE smoke 검증이 필요한 구조 변경으로 본다.
```

### 2.4.1 Serialized UPROPERTY regrouping migration

기존 `UPROPERTY`를 config `USTRUCT`로 묶거나 이름 / 소유권을 바꿀 때는 Blueprint / asset serialization 값을 먼저 보호한다.

규칙:

```text
-> 기존 field를 바로 삭제하지 않는다.
-> 기존 field는 DeprecatedProperty로 1 pass 유지한다.
-> deprecated field는 editor 설정 소유자가 아니라 legacy asset 값을 읽기 위한 migration slot이다.
-> deprecated field에는 raw default literal을 다시 두지 않는다.
-> deprecated field의 기본 상태가 필요하면 새 config struct 기본값에서 초기화한다.
-> 새 field / struct가 default이고 legacy field가 non-default이면 PostLoad에서 복사한다.
-> float은 정확 비교 대신 FMath::IsNearlyEqual 기준으로 default 여부를 판단한다.
-> UObject / asset reference는 새 값이 nullptr이고 legacy 값이 valid일 때 복사한다.
-> migration이 필요한 asset을 Editor에서 열고 저장한다.
-> 저장 검증 이후 후속 PR 또는 후속 커밋에서 deprecated field와 migration 코드를 제거한다.
```

권장 커밋 순서:

```text
1. 규칙 추가 + 코드 migration layer 추가
2. Editor에서 대상 asset을 열고 저장
3. 저장 검증 이후 deprecated field / PostLoad migration code 제거
```

주의:

```text
-> migration layer는 영구 구조가 아니다.
-> runtime 로직은 새 config struct만 읽는다.
-> migration layer 추가 커밋과 제거 커밋은 분리한다.
-> asset 저장 커밋은 코드 변경 커밋과 분리한다.
```

### 2.5 외부 계약값

외부 계약값은 이름이나 값이 외부 시스템과 연결된 값이다.

```text
예시:
-> CVar 이름
-> input binding 이름
-> Blackboard key 이름
-> CreateDefaultSubobject 이름
-> serialized id
-> gameplay tag
-> asset path
-> debug / audit taxonomy 문자열
```

규칙:

```text
-> 임의 변경하지 않는다.
-> CVar 숫자 mode 값은 외부 ABI일 수 있으므로 값은 유지한다.
-> CVar 내부 해석은 enum / helper / constexpr로 감쌀 수 있다.
-> 문자열 중앙화는 탐색성과 로그 의미가 좋아지는 경우에만 한다.
```

### 2.6 유지 가능한 literal

다음 값은 기본적으로 상수화하지 않는다.

```text
-> 0 / 1 초기화
-> loop index
-> count reset
-> hash seed
-> pure virtual = 0
-> enum None = 0
-> obvious arithmetic
-> FVector::ZeroVector / FVector::OneVector
-> nullptr / false / true
```

단, 같은 값이 도메인 의미를 가지며 반복되면 내부 규칙값 또는 튜닝 데이터로 다시 분류한다.

---

## 3. 작업 분리 기준

```text
상수 정리형:
-> 내부 규칙값 / sentinel / mode / index / default policy 숫자에 이름을 부여한다.
-> 값 변경 없음.
-> editor 노출 구조 변경 없음.

튜닝 데이터 정리형:
-> 이동속도, 시야, capsule, camera, hit stop 같은 값의 소유권을 결정한다.
-> UPROPERTY / config USTRUCT / DataAsset 중 적절한 형태를 선택한다.
-> Blueprint / Editor / PIE 검증을 포함한다.
```

상수 정리와 튜닝 데이터 구조 변경은 성격이 다르므로 같은 브랜치에서 진행하더라도 커밋과 검증 단위를 분리한다.
