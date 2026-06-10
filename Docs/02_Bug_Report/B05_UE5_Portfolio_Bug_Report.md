# UE5 Portfolio Bug Report

## 제목

**B05: CurrentHitWindowId 증가 타이밍 문제로 인한 첫 타격 Reject 문제**

## 날짜

**2026.04.06**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-core-shared`

---

## 요약

- `ACAttachment::CollisionEnabled()`에서 `CurrentHitWindowId`가 증가되기 전에 `UShapeComponent`가 먼저 `SetCollisionEnabled` 되어 첫 overlap이 먼저 발생할 수 있었다.

- 그 결과 `BuildOverlapContext()`가 `HitWindowId = -1`을 담은 `FHitContext`를 생성했고, `UCApplyDamageComponent::ValidateRequest()`에서 이를 `InvalidRequest`로 즉시 리젝트 처리하는 버그가 발생했다.

- 해당 버그는 `UShapeComponent`가 `SetCollisionEnabled` 되기 이전에 유효한 `HitWindowId`가 먼저 준비되도록 `ACAttachment::CollisionEnabled()` 흐름을 재정리하여 수정했다.

---

## 환경

- 엔진: Unreal Engine 5.4

- 대상: `ACAttachment` / `UCApplyDamageComponent`

- 관련 흐름:
	- Player 또는 Enemy가 공격 애니메이션 notify를 통해 attachment collision을 활성화한다.
	- 활성화 직후 첫 overlap이 발생한다.
	- overlap 시점의 `HitWindowId`가 아직 유효하게 세팅되지 않는다.
	- `ApplyDamage` 단계에서 첫 타가 `InvalidRequest`로 리젝트된다.

---

## 재현 방법

1. Player 또는 Enemy의 근접 무기 attachment에 대해 공격 애니메이션 notify로 `CollisionEnabled()`를 호출한다.

2. collision이 활성화되는 즉시 상대와 overlap이 발생하도록 근접 거리에서 공격을 수행한다.

3. 첫 overlap 시점의 `FOverlapContext.HitWindowId` 값을 로그로 확인한다.

4. `UCApplyDamageComponent::ValidateRequest()`에서 `HitWindowId == INDEX_NONE` 조건으로 인해 요청이 리젝트되는지 확인한다.

5. 로그에서 다음과 같은 패턴이 재현되는지 확인한다.

```text
RejectReason = EApplyDamageRejectReason::InvalidRequest
HitWindowId = -1
```

---

## 기대 결과 vs 실제 결과

**기대 결과**

- attachment collision이 실제로 overlap을 발생시키기 전에 유효한 hit window id가 먼저 준비되어야 한다.
  
- 첫 overlap도 정상적인 `HitWindowId`를 가진 `FHitContext`로 전달되어야 한다.
  
- 첫 타부터 `ApplyDamage -> TakeDamage` 흐름이 정상적으로 진행되어야 한다.

**실제 결과**

- 첫 overlap 시점에 `HitWindowId`가 `INDEX_NONE(-1)` 상태로 전달되었다.

- `UCApplyDamageComponent::ValidateRequest()`가 해당 요청을 `InvalidRequest`로 리젝트했다.

- 결과적으로 첫 타가 적용되지 않고 sender-side에서 조기 종료되었다.

---

## 원인

```cpp
bool bCollisionEnabled = false;

if (!InName.IsNone())
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		if (collision->GetFName() == InName)
		{
			collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			bCollisionEnabled = true;
			break;
		}
	}
}
else
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		bCollisionEnabled = true;
	}
}

if (!bCollisionEnabled) return;

if (!bHitWindowOpened)
{
	++CurrentHitWindowId; // Error Point

	bHitWindowOpened = true;

	if (IsValid(ApplyDamageComp_Cached))
	{
		ApplyDamageComp_Cached->NotifyHitWindowOpened(this, CurrentHitWindowId);
	}
}
```

- 문제의 핵심은 `ACAttachment::CollisionEnabled()`의 처리 순서였다.

- 기존 구조에서는 attachment collision 활성화와 hit window 초기화 타이밍이 분리되어 있었고, 첫 overlap이 hit window id 준비 이전에 들어올 수 있었다.

- 그 결과 흐름은 다음과 같았다.
	1. 공격 notify가 `CollisionEnabled()`를 호출했다.
	2. attachment collision 활성화 직후 첫 overlap callback이 발생했다.
	3. `BuildOverlapContext()`는 아직 갱신되지 않은 `CurrentHitWindowId`를 읽었다.
	4. `HitWindowId = INDEX_NONE(-1)`가 `FHitContext`에 포함되었다.
	5. `UCApplyDamageComponent::ValidateRequest()`가 이를 invalid request로 판단했다.

- 해당 버그의 본질은 **첫 overlap이 발생하기 전에 hit window id가 유효한 값으로 준비되어야 하는데, overlap 타이밍과 hit window 초기화 타이밍이 어긋나 있었던 것**이다.

---

## 수정

```cpp
TArray<UShapeComponent*> collisionsToEnable;

if (!InName.IsNone())
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		if (collision->GetFName() == InName)
		{
			collisionsToEnable.Add(collision);
			break;
		}
	}
}
else
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		collisionsToEnable.Add(collision);
	}
}

// Early-Return
if (collisionsToEnable.IsEmpty()) return;

if (!bHitWindowOpened)
{
	++CurrentHitWindowId;
	bHitWindowOpened = true;

	if (IsValid(ApplyDamageComp_Cached))
	{
		ApplyDamageComp_Cached->NotifyHitWindowOpened(this, CurrentHitWindowId);
	}
}

for (UShapeComponent* collision : collisionsToEnable)
{
	collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
```

- `ACAttachment::CollisionEnabled()`의 흐름을 다음과 같이 보정했다.
	1. 먼저 실제로 활성화할 `UShapeComponent` 목록을 수집한다.
	2. 활성화할 collision이 하나도 없으면 즉시 종료한다.
	3. 활성화 대상이 있을 때만 hit window를 열고 `CurrentHitWindowId`를 증가시킨다.
	4. 그 이후에 실제 collision enable을 수행한다.

- 수정 의도는 다음과 같다.
	1. 유효한 collision 없이 hit window만 열리는 잘못된 상태를 방지한다.
	2. 첫 overlap 이전에 항상 유효한 `HitWindowId`가 준비되도록 보장한다.
	3. `ApplyDamage`가 첫 타를 `InvalidRequest`로 리젝트하지 않도록 sender-side 입력 조건을 안정화한다.

---

## 검증 결과

1. Player 공격과 Enemy 공격 양쪽에서 공격 시작 직후 첫 overlap 로그를 반복 확인했다.

2. 첫 overlap 시점에 `HitWindowId`가 더 이상 `-1`로 출력되지 않는지 확인했다.

3. `UCApplyDamageComponent::ValidateRequest()`가 첫 타를 `InvalidRequest`로 리젝트하지 않는지 확인했다.

4. 첫 타부터 `ApplyDamage` summary 로그가 정상적으로 출력되는지 확인했다.

5. 활성화할 collision이 없는 경우 hit window만 열리는 잘못된 상태가 발생하지 않는지 확인했다.

---

## 비고

```cpp
===== Apply Damage Summary ======
[@ APPLY DAMAGE]
| DamageCauser = BP_CAttachment_Sword_C_1
| Target = BP_CEnemy_C_1
| HitWindowId = 0
| Base = 10.000
| Request = 10.000
| Committed = 10.000
=================================
```

- `HitWindowId`의 초기값을 `0`으로 바꾸는 방식도 고려했으나 구조적 문제를 숨기는 임시 대응에 가까워 적용하지 않았다.

- `INDEX_NONE(-1)`는 “아직 유효한 hit window가 열리지 않았다”는 의미로 유지하는 편이 구조적으로 더 안전하다.

- 이 수정의 핵심은 **collision enable 가능 여부 확인 -> hit window open -> 실제 collision enable** 순서로 흐름을 명확히 정렬한 것이다.

---
