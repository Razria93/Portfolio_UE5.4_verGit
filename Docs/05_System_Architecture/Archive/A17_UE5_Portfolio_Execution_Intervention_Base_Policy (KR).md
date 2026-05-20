# A17 UE5 Portfolio Execution Intervention Base Policy

## 1. 목적

본 문서는 action / reaction executor가 `WantIntervention()`과 `AllowInterventionBy()`를 통해 기본 전투 개입 정책을 어떻게 표현하는지 정리하기 위한 문서임.

핵심은 incoming execution의 개입 의도와 active execution의 허용 여부를 분리하고, action과 reaction의 기본 정책 차이를 명확히 하는 것임.

## 2. 기존 시스템의 형태

### 2.1 Intervention 판단 흐름

현재 action / reaction orchestration은 active execution이 존재할 때 `FExecutionInterventionQuery`를 구성함.

Intervention 판단은 다음 두 질문으로 나뉨.

```text
incoming execution
-> active execution을 멈추고 들어가고 싶은가
-> WantIntervention()

active execution
-> incoming execution에게 멈춰도 되는가
-> AllowInterventionBy()
```

즉 `WantIntervention()`은 incoming 쪽 규칙이고, `AllowInterventionBy()`는 active 쪽 규칙임.

### 2.2 StopReason 기준

현재 intervention은 `EExecutionStopReason`을 기준으로 cancel과 interrupt를 구분함.

```text
Interrupted
-> 외부 요인 또는 강제 실행에 의해 active execution이 끊기는 경우임

Cancelled
-> 의도적 입력 또는 의식적인 실행 전환에 의해 active execution이 취소되는 경우임
```

예시는 다음과 같음.

```text
HitReaction -> AttackAction interrupt
DeadReaction -> Action/Reaction interrupt
DodgeAction -> HitReaction cancel
CounterAction -> active execution cancel
```

## 3. 기존 시스템의 문제 분석 및 한계

### 3.1 Want와 Allow를 같은 의미로 보면 안 됨

`WantIntervention()`과 `AllowInterventionBy()`는 둘 다 bool을 반환하지만 의미가 다름.

```text
WantIntervention
-> incoming executor가 말하는 개입 의도임

AllowInterventionBy
-> active executor가 말하는 개입 허용 여부임
```

이 둘을 같은 window로 처리하면 “내가 끊고 싶은 상태”와 “내가 끊겨도 되는 상태”가 섞임.

따라서 action / reaction executor는 incoming일 때와 active일 때의 정책을 분리해서 표현해야 함.

### 3.2 Action과 Reaction의 기본 정책은 같을 수 없음

Action은 player / AI intent로 발생하는 능동 실행임.

반면 Reaction은 damage event나 combat interaction result에 의해 발생하는 수동 반응 실행임.

따라서 기본 정책도 다름.

```text
Action
-> 일반 action은 active execution을 멈추고 들어가지 않음
-> Hit/Dead reaction에는 기본적으로 끊길 수 있음

Reaction
-> reaction끼리의 교체/갱신은 window 또는 force policy가 필요함
-> active reaction은 명시적으로 허용된 구간에서만 끊기는 것이 안전함
```

## 4. 리팩터링 방향 및 내용

### 4.1 Action 기본 정책

Action의 incoming policy는 보수적으로 둠.

일반 action은 active execution을 멈추고 들어가려 하지 않음.

```cpp
bool UCAction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	// [NOTE] Base Action Incoming Policy
	// Normal actions do not stop active executions by default.
	// Intentional action-driven intervention requires a specific action override.
	return false;
}
```

즉 dodge, counter, execution action처럼 active execution을 취소하고 들어가야 하는 action은 subclass에서 override해야 함.

### 4.2 Action active policy

Action이 active로 실행 중일 때는 Hit/Dead reaction에 기본적으로 끊기는 정책을 사용함.

```cpp
bool UCAction::AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	switch (InQuery.StopReason)
	{
	case EExecutionStopReason::Interrupted:
	{
		// [NOTE] Base Action Active Policy
		// Hit/Dead reactions interrupt normal actions by default.
		// Other interrupt types require an explicit interrupt window.
		if (InQuery.IncomingPart.IsReactionParticipant())
		{
			const FReactionExecutionContext& incoming = InQuery.IncomingPart.GetReactionContext();

			if (incoming.ReactionDataKey.ReactionType == EReactionType::Dead) return true;
			if (incoming.ReactionDataKey.ReactionType == EReactionType::Hit) return true;
		}

		return IsInterruptibleNow();
	}

	case EExecutionStopReason::Cancelled:
	{
		// [NOTE] Base Action Cancel Policy
		// Intentional cancellation requires an explicit cancel window.
		return IsCancelableNow();
	}

	default:
		return false;
	}
}
```

이 정책은 일반 action이 피격에는 기본적으로 끊기고, 특수 action만 override로 막는 구조임.

예시는 다음과 같음.

```text
Normal Attack
-> HitReaction에 의해 interrupt됨

SuperArmor Attack
-> subclass에서 HitReaction interrupt를 막음

DeadReaction
-> 대부분의 action을 interrupt함
```

### 4.3 Reaction incoming policy

Reaction은 incoming일 때도 window 기반으로 개입 의도를 표현함.

```cpp
bool UCReaction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	switch (InQuery.StopReason)
	{
	case EExecutionStopReason::Interrupted:
	{
		// [NOTE] Base Reaction Incoming Policy
		// Reactions request interrupt only while the want-interrupt window is open.
		// Force reactions such as DeadReaction should be handled by orchestration or subclass override.
		return IsWantInterruptNow();
	}

	case EExecutionStopReason::Cancelled:
	{
		// [NOTE] Base Reaction Incoming Cancel Policy
		// Reactions request cancel only while the want-cancel window is open.
		return IsWantCancelNow();
	}

	default:
		return false;
	}
}
```

다만 `HitReaction`처럼 일반 피격 반응이 active action을 끊고 들어가는 경우는 subclass에서 개입 의도를 명시할 수 있음.

`DeadReaction`처럼 강제 개입이 필요한 경우는 orchestration force branch에서 처리하는 것이 적절함.

### 4.4 Reaction active policy

Reaction이 active로 실행 중일 때는 명시적인 allow window가 열려 있어야 개입을 허용함.

```cpp
bool UCReaction::AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	switch (InQuery.StopReason)
	{
	case EExecutionStopReason::Interrupted:
	{
		// [NOTE] Base Reaction Active Policy
		// Active reactions allow interrupts only while the allow-interrupt window is open.
		return IsAllowInterruptNow();
	}

	case EExecutionStopReason::Cancelled:
	{
		// [NOTE] Base Reaction Active Cancel Policy
		// Active reactions allow intentional cancel only while the allow-cancel window is open.
		return IsAllowCancelNow();
	}

	default:
		return false;
	}
}
```

이 정책은 reaction이 이미 외부 자극으로 발생한 실행 상태라는 점을 반영함.

즉 active reaction은 다시 끊기기 위해 명시적인 allow window가 필요함.

## 5. 이후 작업의 방향성

### 5.1 특수 action override

Dodge, counter, execution action은 `UCAction::WantIntervention()`을 override해야 함.

예시는 다음과 같음.

```text
DodgeAction
-> active reaction을 Cancelled reason으로 멈추고 들어가려 함

CounterAction
-> defensive success result 이후 active execution을 cancel하고 들어가려 함
```

### 5.2 특수 reaction override

Hit, Dead, GuardBreak, Knockback 같은 reaction은 기본 정책 위에 subclass rule을 추가해야 함.

예시는 다음과 같음.

```text
HitReaction
-> incoming hit은 active action을 interrupt하려 함

DeadReaction
-> orchestration force branch로 active execution을 강제 interrupt함

GuardBreakReaction
-> guard state를 깨고 stagger 또는 break reaction으로 진입함
```

### 5.3 공통 정책 확장

향후에는 다음 정보가 intervention query 또는 snapshot에 포함될 수 있음.

```text
super armor
guard state
parry state
poise
priority
force intervention
ignore interrupt window
combat interaction result
```

이 정보들은 executor local rule과 orchestration-level arbitration 사이에서 사용할 수 있음.

## 6. 결론

기본 intervention 정책은 incoming execution의 의도와 active execution의 허용 여부를 분리해야 함.

Action은 일반적으로 active execution을 멈추고 들어가지 않으며, 특수 action만 override로 개입 의도를 가짐.

반면 action이 active일 때는 Hit/Dead reaction에 기본적으로 끊기는 것이 자연스러움.

Reaction은 이미 외부 자극으로 발생한 실행 상태이므로, reaction끼리의 교체나 cancel은 명시적인 want/allow window 또는 force policy를 통해 제어하는 것이 안전함.

따라서 현재 권장 기본 정책은 다음과 같음.

```text
UCAction::WantIntervention
-> 기본 false

UCAction::AllowInterventionBy
-> Hit/Dead reaction은 기본 허용
-> 그 외 interrupt/cancel은 window 기반

UCReaction::WantIntervention
-> want window 또는 subclass override 기반

UCReaction::AllowInterventionBy
-> allow window 기반
```
