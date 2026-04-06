# UE5 Portfolio Bug Report (KR)

## 제목

**M3-B02: AIState 전이 시 `AttackIndex` 초기화 누락으로 인한 공격 패턴 고착 버그 수정**

### Date

- **Day 12**

- **2026.03.31**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/ai-bt-context`


---

## 요약

- `UCBTService_UpdateAIState::UpdateAIStateTransition()`에서 Engage 계열 Blackboard 값을 정리할 때 `AttackIndex`를 `ClearValue()`로만 비우고 있었음.

- 이로 인해 AI가 Hit 이후 Engage 상태로 복귀하는 과정에서 `AttackIndex`가 `INDEX_NONE(-1)`이 아닌 `0` 기준값으로 해석되었고, 이후 `UCBTTask_SelectAttackIndex`가 다음 인덱스를 `1`로 계산하면서 결과적으로 피격 이후 계속 `1번` 공격이 선택되는 문제가 발생했음.

- `AttackIndex`를 명시적으로 `INDEX_NONE`으로 초기화하도록 수정하여, 상태 전이 이후 공격 선택이 정상적으로 다시 계산되도록 보정함.


---

## 환경

- Engine: Unreal Engine 5.4

- 대상: AI BehaviorTree Service (`UpdateAIState`)

- 관련 Blackboard Key:

  - `CAIKey::Engage::bInEngageRange`

  - `CAIKey::Engage::bCanAttack`

  - `CAIKey::Engage::bIsAttacking`

  - `CAIKey::Engage::AttackIndex`

- 관련 시나리오:

  - Enemy가 공격 수행

  - Player에게 피격 또는 타겟 상황 변화 발생

  - AIState가 일시적으로 Engage 외 상태로 전이

  - 이후 다시 Engage로 복귀하여 다음 공격 선택 수행


---

## 재현 방법

1. Enemy가 Engage 상태에서 공격 인덱스를 선택하고 공격을 수행함.

2. 공격 도중 또는 공격 직후 Player의 반격, 거리 이탈, HitReact 등의 이유로 AIState가 Engage 외 상태로 전이되도록 만듦.

3. 이후 Enemy가 다시 Engage 상태로 복귀하도록 유도함.

4. 다음 공격 선택 시 `AttackIndex`가 `INDEX_NONE`이 아닌 `0` 기준으로 해석되는 것을 확인함.

5. 반복 관찰 시 Enemy가 `0 / 1 / 2` 콤보 중 `1번` 공격만 반복적으로 사용하는 현상이 나타나는 것을 확인함.


---

## 기대 결과 vs 실제 결과

**기대 결과**

- AIState가 Engage 외 상태로 전이될 때 Engage 관련 Blackboard 값이 일관되게 초기화되어야 함.

- 이후 다시 Engage로 복귀하면 `AttackIndex`는 `INDEX_NONE` 상태에서 시작하여 새 컨텍스트 기준으로 다시 선택되어야 함.

- Enemy 공격 패턴은 상황에 따라 `0 / 1 / 2` 인덱스를 정상적으로 순환 또는 재선택해야 함.

**실제 결과**

- `AttackIndex`가 명시적으로 무효값으로 초기화되지 않아, 후속 선택 로직에서 `0` 기준값으로 해석되었음.

- 이후 `UCBTTask_SelectAttackIndex`가 해당 값을 기준으로 다음 인덱스를 `1`로 계산함.

- 결과적으로 Enemy가 피격 이후 `1번` 콤보 공격만 반복 사용하는 패턴 고착 현상이 발생함.


---

## 원인

```cpp
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, false);
InBlackboardComp->ClearValue(CAIKey::Engage::AttackIndex);
```

- `bInEngageRange`, `bCanAttack`, `bIsAttacking`는 명시적으로 false로 초기화되고 있었음.

- 반면 `AttackIndex`만 `ClearValue()`로 처리되고 있었음.

- 이 Blackboard Key는 이후 공격 선택 로직에서 정수 인덱스로 직접 해석되므로, 단순 Clear가 항상 의도한 `INDEX_NONE`과 동일하게 동작한다고 볼 수 없었음.

- `UCBTTask_SelectAttackIndex`는 현재 `AttackIndex`를 기반으로 다음 공격 인덱스를 계산하는 구조였음.

- 실제 흐름에서는 상태 전이 이후 `AttackIndex`가 `0` 기준값으로 해석되었고, 이후 `UCBTTask_SelectAttackIndex`가 이를 기준으로 다음 인덱스를 `1`로 계산하게 되었음.

- 즉, 이 버그의 본질은 **AIState 전이 시 Engage 관련 상태는 정리되었지만, 공격 선택 컨텍스트의 핵심 값인 `AttackIndex`가 `INDEX_NONE`으로 명시적 초기화되지 않아 다음 선택 로직이 잘못된 기준값에서 시작된 것**임.


---

## 수정

다음과 같이 `AttackIndex` 초기화 방식을 변경함.

```cpp
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);
InBlackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, false);
InBlackboardComp->SetValueAsInt(CAIKey::Engage::AttackIndex, INDEX_NONE);
```

수정 의도는 다음과 같음.

1. 공격 인덱스의 무효 상태를 Blackboard 상에서 명시적으로 표현함.

2. 이후 공격 선택 로직이 항상 `INDEX_NONE`을 기준으로 새 선택을 수행하도록 보장함.

3. Engage 이탈 -> 재진입 과정에서 `0`을 기준으로 다음 인덱스가 `1`로 밀리는 편향 현상을 차단함.


---

## 검증

1. Enemy가 Engage 상태에서 공격 수행 후 HitReact 또는 상태 전이를 겪는 시나리오를 반복 테스트함.

2. Blackboard Debug에서 Engage 이탈 시 `AttackIndex`가 `INDEX_NONE`으로 초기화되는지 확인함.

3. 이후 Engage 재진입 시 공격 인덱스가 새로 선택되는지 확인함.

4. 반복 전투 상황에서 피격 이후에도 공격 선택이 `1번`에 고착되지 않고 정상적으로 분기되는지 확인함.

5. 기존에 관찰되던 `1번 콤보 반복 사용` 현상이 재현되지 않음을 확인함.

6. 수정 전에는 피격 이후 첫 재공격이 항상 `1번`으로 시작되었으나, 수정 후에는 특정 인덱스 고착 없이 정상 분기되는 것을 확인함.


---

## Notes

- Blackboard의 정수형 선택 값은 단순 Clear보다 **명시적 sentinel value 초기화**가 더 안전한 경우가 많음.

- 특히 `AttackIndex`처럼 후속 로직에서 직접 분기 기준으로 사용되는 값은 `INDEX_NONE`과 같은 무효 상태를 일관되게 강제하는 편이 바람직함.

- 이번 수정은 단순 초기화 방식 변경이지만, 실제로는 AI 공격 선택 로직의 시작 기준값을 명확히 고정한 버그 수정에 해당함.


---
