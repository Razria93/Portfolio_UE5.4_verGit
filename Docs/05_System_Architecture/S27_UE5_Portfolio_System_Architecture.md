# 전투 판정(Combat Resolution) 책임 분리 결정

## 1. 목적

본 문서는 `CombatResolution` 계층을 도입할 경우, 기존 `ApplyDamage / TakeDamage / ActionOrchestration / ReactionOrchestration / Feedback` 흐름의 책임이 어떻게 재배치되어야 하는지 정리하기 위한 문서다.

핵심은 `CombatResolution`이 action/reaction을 직접 실행하는 계층이 아니라, hit 이후의 combat outcome을 판정하고 그 결과로 필요한 damage/action/reaction/feedback request를 구성하는 계층이라는 점이다.

---

## 2. 관련 브랜치

- `combat-resolution`

---

## 3. 기존 시스템의 형태

### 현재 Damage Flow

현재 damage 흐름은 수직적인 구조에 가깝다.

```yaml
WeaponActor
-> ApplyDamageComponent::RequestApplyDamage()
-> TargetActor::TakeDamage()
-> TakeDamageComponent::RequestTakeDamage()
-> TakeDamageComponent damage 계산/commit
-> TakeDamageComponent reaction 요청
-> TakeDamageComponent feedback 요청
```

### ApplyDamage / TakeDamage 연결

현재 공격자 쪽 `ApplyDamageComponent`는 `FDefaultDamageEvent`를 구성한 뒤 target actor의 `TakeDamage()`를 호출한다.

```cpp
return InApplyDamageContext.TargetActor->TakeDamage(
	InApplyDamageContext.ApplyDamageAmount.RequestDamage,
	damageEvent,
	InApplyDamageContext.Instigator,
	InApplyDamageContext.DamageCauser
);
```

피격자 쪽 `TakeDamage()`는 현재 `TakeDamageComponent`로 직접 위임한다.

```cpp
if (IsValid(TakeDamageComponent))
{
	finalDamage = TakeDamageComponent->RequestTakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);
}
```

### Damage Commit 이후 Dispatch

그리고 `TakeDamageComponent`는 damage commit 이후 reaction과 feedback까지 직접 dispatch한다.

```cpp
void UCTakeDamageComponent::DispatchTakeDamageCommitted(
	const FTakeDamagePacket& InTakeDamagePacket
) const
{
	if (!InTakeDamagePacket.Result.bAccepted) return;

	if (IsValid(ReactionOrchestratorComp_Cached))
	{
		FDamageReactionRequest damageReactionRequest;
		damageReactionRequest.IntentSource = EReactionIntentSource::TakeDamage;
		damageReactionRequest.TakeDamagePacket = InTakeDamagePacket;

		ReactionOrchestratorComp_Cached->RequestDamageReaction(damageReactionRequest);
	}

	if (IsValid(DamageFeedbackComp_Cached))
	{
		DamageFeedbackComp_Cached->PlayDamageFeedback(InTakeDamagePacket);
	}
}
```

이 구조에서는 `TakeDamageComponent`가 raw damage event 수신, damage 계산, health commit, reaction 요청, feedback 요청을 모두 주도한다.

---

## 4. 기존 시스템의 문제 분석 및 한계

### TakeDamageComponent 책임 과다

`TakeDamageComponent`는 이름상 damage 적용 컴포넌트에 가깝지만, 현재는 피격 이벤트 전체를 처리하는 상위 Flow controller처럼 동작한다.

```yaml
FDamageEvent 수신
-> damage context 구성
-> damage 계산
-> health commit
-> reaction type 간접 결정
-> ReactionOrchestrator 호출
-> DamageFeedback 호출
```

이 구조는 단순 hit/dead reaction만 있을 때는 동작하지만, guard/parry/dodge/counter가 들어오면 책임이 급격히 커진다.

예를 들어 parry를 `TakeDamageComponent`에 넣으면 `TakeDamageComponent`가 defender action 상태를 조회하고, attacker action/reaction까지 제어해야 한다. 이는 damage 적용 컴포넌트가 사실상 작은 combat resolution 계층이 되는 구조다.

### ReactionOrchestrator의 Combat Outcome 해석 책임

현재 `ReactionOrchestrator`는 `TakeDamagePacket`의 결과를 보고 `Hit / Dead / None` reaction type을 결정한다.

```cpp
EReactionType UCReactionOrchestratorComponent::ResolveDamageReactionType(
	const FDamageReactionRequest& InIncomingRequest
) const
{
	const FTakeDamageResult& damageResult = InIncomingRequest.TakeDamagePacket.Result;

	if (!damageResult.bAccepted) return EReactionType::None;

	if (damageResult.DeadState_Before == EDeadState::Alive
		&& damageResult.DeadState_After != EDeadState::Alive)
	{
		return EReactionType::Dead;
	}

	if (damageResult.CommittedDamage > KINDA_SMALL_NUMBER
		&& damageResult.DeadState_After == EDeadState::Alive)
	{
		return EReactionType::Hit;
	}

	return EReactionType::None;
}
```

그러나 이 판단은 execution arbitration이 아니라 combat outcome 해석에 가깝다.

`ReactionOrchestrator`가 장기적으로 담당해야 할 일은 `Hit`인지 `Dead`인지 결정하는 것이 아니라, 이미 요청된 reaction execution을 현재 active execution과 어떻게 조율할지 판단하는 것이다.

### 복합 전투 판정 도입 시 수직 Flow 한계

Parry 예시는 특히 문제가 명확하다.

```yaml
Hit 발생
-> defender가 parry window 중
-> defender damage 없음
-> defender hit reaction 없음
-> attacker reaction 필요
```

이 결과는 damage, defender reaction, attacker reaction, feedback을 동시에 건드린다.

이를 `TakeDamageComponent`에서 처리하면 `TakeDamageComponent`가 action/reaction orchestration을 역방향으로 호출하는 구조가 된다. 이는 현재 수직 흐름 안에서 역류하는 형태이며, 장기적으로 유지하기 어렵다.

---

## 5. 리팩터링 방안 제안

### CombatResolution의 위치

권장 구조는 `Actor::TakeDamage()` 이후 가장 먼저 `CombatResolutionComponent`가 raw damage event를 수신하는 형태다.

```yaml
WeaponActor
-> ApplyDamageComponent
-> FDefaultDamageEvent
-> TargetActor::TakeDamage
-> CombatResolutionComponent::ResolveDamageEvent
   -> combat outcome 판정
   -> damage/action/reaction/feedback request 구성
   -> 각 컴포넌트에 dispatch
```

`Actor::TakeDamage()`는 다음과 같은 형태로 바뀔 수 있다.

```cpp
float ACPlayer::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (DamageAmount <= 0.f) return 0.f;

	float finalDamage = 0.f;

	if (IsValid(CombatResolutionComponent))
	{
		finalDamage = CombatResolutionComponent->ResolveDamageEvent(
			DamageAmount,
			DamageEvent,
			EventInstigator,
			DamageCauser
		);
	}
	else if (IsValid(TakeDamageComponent))
	{
		finalDamage = TakeDamageComponent->RequestTakeDamage(
			DamageAmount,
			DamageEvent,
			EventInstigator,
			DamageCauser
		);
	}

	Super::TakeDamage(finalDamage, DamageEvent, EventInstigator, DamageCauser);

	return finalDamage;
}
```

과도기에는 기존 `TakeDamageComponent` fallback을 유지할 수 있지만, 최종 구조에서는 `CombatResolutionComponent`가 damage event 수신의 중심이 된다.

### CombatResolution의 책임

`CombatResolution`은 action/reaction을 직접 실행하지 않는다.

담당하는 것은 combat packet이 현재 상태에서 어떤 outcome인지 판정하는 것이다.

```yaml
NormalHit
Dodged
Guarded
GuardBroken
Parried
Countered
Rejected
```

Parry 기준으로 보면 `CombatResolution`은 다음을 판단한다.

```yaml
defender가 parry action 중인가
parry window가 열려 있는가
incoming hit이 parry 가능한 타입/방향인가
이 hit을 Parried로 판정할 것인가
damage를 적용하지 않을 것인가
defender reaction을 생략할 것인가
attacker reaction request를 만들 것인가
```

반대로 다음은 담당하지 않는다.

```yaml
attacker reaction montage를 실제로 재생할 수 있는가
attacker active action을 interrupt할 수 있는가
counter action을 현재 active action 위에서 start/cancel할 수 있는가
component stop/start가 성공했는가
```

이 판단은 action/reaction orchestration과 component/executor lifecycle의 책임이다.

### Orchestration과의 책임 경계

책임 경계는 다음과 같이 나눈다.

```yaml
CombatResolution
-> combat outcome producer

Action / Reaction Orchestration
-> execution request arbitrator

Component
-> directive / request consumer

Executor
-> montage lifecycle performer
```

Parry 예시는 다음과 같이 해석한다.

```yaml
CombatResolution:
"이 hit은 Parried다. defender damage/reaction은 없고 attacker reaction request가 필요하다."

ReactionOrchestrator:
"그 attacker reaction request가 attacker의 현재 active execution을 interrupt하고 들어갈 수 있는가?"

ReactionComponent:
"결정된 directive/result를 소비해서 active action/reaction을 stop하고 reaction을 start한다."
```

따라서 combat outcome은 확정적이고, 후속 execution request는 별도로 실패할 수 있다.

기본 정책은 다음과 같다.

```yaml
Parry 판정 성공
-> defender damage 없음
-> defender hit reaction 없음
-> attacker reaction request 시도
   -> 성공하면 attacker stagger/parried reaction 실행
   -> 실패하면 reaction montage만 누락됨
```

후속 execution 실패가 combat outcome rollback 사유가 되지는 않는다.

### 권장 구조체

최소 구조는 다음과 같이 시작할 수 있다.

```cpp
UENUM(BlueprintType)
enum class ECombatInteractionResult : uint8
{
	None = 0,

	NormalHit,
	Dodged,
	Guarded,
	GuardBroken,
	Parried,
	Countered,

	Rejected,

	Max,
};
```

```cpp
USTRUCT(BlueprintType)
struct FCombatResolutionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	float RequestedDamage = 0.f;

	UPROPERTY(Transient)
	FDefaultDamageEvent DamageEvent = FDefaultDamageEvent();

	UPROPERTY(Transient)
	AController* EventInstigator = nullptr;

	UPROPERTY(Transient)
	AActor* DamageCauser = nullptr;
};
```

```cpp
USTRUCT(BlueprintType)
struct FCombatResolutionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FCombatResolutionRequest Request = FCombatResolutionRequest();

	UPROPERTY(Transient)
	AActor* Attacker = nullptr;

	UPROPERTY(Transient)
	AActor* Defender = nullptr;

	UPROPERTY(Transient)
	UCActionComponent* DefenderActionComp = nullptr;

	UPROPERTY(Transient)
	UCTakeDamageComponent* DefenderTakeDamageComp = nullptr;

	UPROPERTY(Transient)
	UCReactionOrchestratorComponent* DefenderReactionOrchestrator = nullptr;
};
```

후속 execution 요청은 optional/required 정책을 가질 수 있어야 한다.

```cpp
USTRUCT(BlueprintType)
struct FCombatExecutionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bRequested = false;

	UPROPERTY(Transient)
	bool bRequired = false;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	EExecutionDomain TargetDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	FReactionExecutionRequest ReactionRequest = FReactionExecutionRequest();

	UPROPERTY(Transient)
	FCombatActionRequest ActionRequest = FCombatActionRequest();
};
```

```cpp
USTRUCT(BlueprintType)
struct FCombatResolutionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ECombatInteractionResult InteractionResult = ECombatInteractionResult::None;

	UPROPERTY(Transient)
	bool bShouldApplyDamage = false;

	UPROPERTY(Transient)
	FTakeDamagePayload TakeDamagePayload = FTakeDamagePayload();

	UPROPERTY(Transient)
	TArray<FCombatExecutionRequest> ExecutionRequests;

	UPROPERTY(Transient)
	bool bShouldPlayFeedback = false;

	UPROPERTY(Transient)
	FTakeDamagePacket DamageFeedbackPacket = FTakeDamagePacket();

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;
};
```

`FReactionExecutionRequest`는 기존 `FDamageReactionRequest`를 장기적으로 대체하는 request가 된다.

```cpp
USTRUCT(BlueprintType)
struct FReactionExecutionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionIntentSource IntentSource = EReactionIntentSource::None;

	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY(Transient)
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;
};
```

### CombatResolutionComponent API 예시

```cpp
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatResolutionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	float ResolveDamageEvent(
		float InDamageAmount,
		const FDamageEvent& InDamageEvent,
		AController* InEventInstigator,
		AActor* InDamageCauser
	);

private:
	bool BuildRequest(
		float InDamageAmount,
		const FDamageEvent& InDamageEvent,
		AController* InEventInstigator,
		AActor* InDamageCauser,
		FCombatResolutionRequest& OutRequest
	) const;

	bool BuildContext(
		const FCombatResolutionRequest& InRequest,
		FCombatResolutionContext& OutContext
	) const;

	bool ResolveParry(
		const FCombatResolutionContext& InContext,
		FCombatResolutionResult& OutResult
	) const;

	void ResolveNormalHit(
		const FCombatResolutionContext& InContext,
		FCombatResolutionResult& OutResult
	) const;

	void DispatchCombatResolutionResult(FCombatResolutionResult& InOutResult);
};
```

핵심 흐름은 다음과 같다.

```cpp
float UCCombatResolutionComponent::ResolveDamageEvent(
	float InDamageAmount,
	const FDamageEvent& InDamageEvent,
	AController* InEventInstigator,
	AActor* InDamageCauser
)
{
	FCombatResolutionRequest request;
	if (!BuildRequest(InDamageAmount, InDamageEvent, InEventInstigator, InDamageCauser, request))
		return 0.f;

	FCombatResolutionContext context;
	if (!BuildContext(request, context))
		return 0.f;

	FCombatResolutionResult result;

	if (ResolveParry(context, result))
	{
		DispatchCombatResolutionResult(result);
		return result.CommittedDamage;
	}

	ResolveNormalHit(context, result);
	DispatchCombatResolutionResult(result);

	return result.CommittedDamage;
}
```

### Parry 판정 예시

Parry 판정은 `CombatResolution`이 주도하되, 이미 실행 중인 parry action의 local combat rule을 조회한다.

```cpp
bool UCCombatResolutionComponent::ResolveParry(
	const FCombatResolutionContext& InContext,
	FCombatResolutionResult& OutResult
) const
{
	if (!IsValid(InContext.DefenderActionComp)) return false;

	UCAction* activeAction = InContext.DefenderActionComp->GetActiveActionExecutor();
	UCAction_Parrying* parryAction = Cast<UCAction_Parrying>(activeAction);

	if (!IsValid(parryAction)) return false;

	if (!parryAction->CanResolveParry(InContext))
		return false;

	OutResult = FCombatResolutionResult();
	OutResult.InteractionResult = ECombatInteractionResult::Parried;

	OutResult.bShouldApplyDamage = false;
	OutResult.bShouldPlayFeedback = true;

	FCombatExecutionRequest attackerReaction;
	attackerReaction.bRequested = true;
	attackerReaction.bRequired = false;
	attackerReaction.TargetActor = InContext.Attacker;
	attackerReaction.TargetDomain = EExecutionDomain::Reaction;
	attackerReaction.ReactionRequest.IntentSource = EReactionIntentSource::CombatResolution;
	attackerReaction.ReactionRequest.ReactionType = EReactionType::Hit;
	attackerReaction.ReactionRequest.ApplyDamageSpecKey = InContext.Request.DamageEvent.ApplyDamageSpecKey;
	attackerReaction.ReactionRequest.DamageImpactInfo = InContext.Request.DamageEvent.DamageImpactInfo;
	attackerReaction.ReactionRequest.SourceActor = InContext.Defender;
	attackerReaction.ReactionRequest.TargetActor = InContext.Attacker;

	OutResult.ExecutionRequests.Add(attackerReaction);
	OutResult.CommittedDamage = 0.f;

	return true;
}
```

`UCAction_Parrying`의 API는 execution decision이 아니라 combat outcome evaluation API여야 한다.

```cpp
bool UCAction_Parrying::CanResolveParry(
	const FCombatResolutionContext& InContext
) const
{
	if (!bIsActive) return false;
	if (!bParryWindowOpened) return false;

	// Later:
	// - attack direction
	// - weapon type
	// - attack property
	// - distance / angle

	return true;
}
```

이는 `ResolveExecutionDecision()`과 다르다.

```yaml
ResolveExecutionDecision
-> 이 action을 현재 실행할 수 있는가

CanResolveParry
-> 이미 실행 중인 parry action이 이 hit을 Parried outcome으로 바꿀 수 있는가
```

### Dispatch 예시

```cpp
void UCCombatResolutionComponent::DispatchCombatResolutionResult(
	FCombatResolutionResult& InOutResult
)
{
	if (InOutResult.bShouldApplyDamage && IsValid(TakeDamageComp_Cached))
	{
		const FTakeDamagePacket damagePacket =
			TakeDamageComp_Cached->ApplyResolvedDamage(InOutResult.TakeDamagePayload);

		InOutResult.CommittedDamage = damagePacket.Result.CommittedDamage;
		InOutResult.DamageFeedbackPacket = damagePacket;

		if (damagePacket.Result.bAccepted)
		{
			BuildDefenderReactionRequestFromDamagePacket(damagePacket, InOutResult);
			InOutResult.bShouldPlayFeedback = true;
		}
	}

	for (const FCombatExecutionRequest& executionRequest : InOutResult.ExecutionRequests)
	{
		const bool bApplied = DispatchExecutionRequest(executionRequest);

		if (!bApplied && executionRequest.bRequired)
		{
			FLog::Log(TEXT("[CombatResolution] Required execution request failed."));
		}
	}

	if (InOutResult.bShouldPlayFeedback && IsValid(DamageFeedbackComp_Cached))
	{
		DamageFeedbackComp_Cached->PlayDamageFeedback(InOutResult.DamageFeedbackPacket);
	}
}
```

후속 execution dispatch는 actor의 orchestrator를 찾아 요청한다.

```cpp
bool UCCombatResolutionComponent::DispatchExecutionRequest(
	const FCombatExecutionRequest& InRequest
) const
{
	if (!InRequest.bRequested) return true;
	if (!IsValid(InRequest.TargetActor)) return false;

	switch (InRequest.TargetDomain)
	{
	case EExecutionDomain::Action:
	{
		UCActionOrchestratorComponent* actionOrchestrator =
			InRequest.TargetActor->FindComponentByClass<UCActionOrchestratorComponent>();

		return IsValid(actionOrchestrator)
			&& actionOrchestrator->RequestCombatAction(InRequest.ActionRequest).IsAccepted();
	}

	case EExecutionDomain::Reaction:
	{
		UCReactionOrchestratorComponent* reactionOrchestrator =
			InRequest.TargetActor->FindComponentByClass<UCReactionOrchestratorComponent>();

		return IsValid(reactionOrchestrator)
			&& reactionOrchestrator->RequestReaction(InRequest.ReactionRequest).IsAccepted();
	}

	default:
		return false;
	}
}
```

이 흐름에서 `CombatResolution`은 후속 execution을 직접 실행하지 않는다. request를 만들고 orchestrator에게 전달한다.

---

## 6. 시행착오 과정

처음에는 parry 같은 기능을 `TakeDamageComponent` 안에서 간이 구현하는 방안도 고려할 수 있었다.

그러나 이 방식은 `TakeDamageComponent`가 defender action 상태를 조회하고, attacker reaction/action까지 요청하는 구조가 된다. 이는 기존 수직 Flow 안에서 역류하는 형태이며, 장기적으로 `CombatResolution`을 도입할 때 다시 제거해야 할 가능성이 높다.

또한 `CombatResolution`이 action/reaction 실행 가능 여부까지 판단하면 orchestration의 의미가 사라진다.

따라서 다음 기준을 둔다.

```yaml
CombatResolution
-> combat packet이 현재 전투 상태에서 어떤 outcome인지 판단한다.

Orchestration
-> outcome으로 생성된 execution request가 현재 active execution과 어떤 관계인지 판단한다.

Component / Executor
-> 결정된 result/directive를 실제 lifecycle로 소비한다.
```

후속 execution이 실패하더라도 combat outcome은 기본적으로 rollback하지 않는다.

```yaml
Parried
-> defender damage 없음
-> defender reaction 없음
-> attacker reaction request 실패 가능
-> 실패해도 Parried 판정 자체는 유지
```

단, 특정 게임 정책상 attacker reaction이 반드시 성공해야 parry가 성립한다면, 이는 execution dispatch 결과를 rollback하는 방식이 아니라 combat resolution 단계의 선행 조건으로 모델링해야 한다.

예시는 다음과 같다.

```yaml
attacker가 parryable state인가
attacker가 forced reaction immune인가
attack property가 parryable인가
```

이 조건이 실패하면 처음부터 `Parried` outcome을 만들지 않는다.

---

## 7. 결론

CombatResolution은 기존 `TakeDamageComponent`와 action/reaction orchestrator를 수평화하기 위한 계층이다.

최종 책임 구조는 다음과 같다.

```yaml
ApplyDamageComponent
-> 공격자 damage attempt 생성
-> FDamageEvent transport 생성

CombatResolutionComponent
-> FDamageEvent 수신
-> attacker/defender 상태 수집
-> combat outcome 판정
-> damage/action/reaction/feedback request 생성
-> dispatch

TakeDamageComponent
-> resolved damage payload 소비
-> health commit

ActionOrchestrator / ReactionOrchestrator
-> execution request arbitration
-> relationship / intervention / apply mode 판단

ActionComponent / ReactionComponent
-> directive/result 소비
-> active execution stop/start/reserve

Executor
-> montage lifecycle / notify window / local combat rule 제공
```

따라서 CombatResolution 도입 시 가장 크게 바뀌는 부분은 다음이다.

```yaml
1. Player/Enemy::TakeDamage 진입 위임 대상
2. TakeDamageComponent의 raw FDamageEvent 수신 책임 축소
3. TakeDamageComponent의 reaction/feedback dispatch 제거
4. ReactionOrchestrator의 ResolveDamageReactionType 제거 또는 축소
5. FDamageReactionRequest를 FReactionExecutionRequest로 전환
6. CombatResolutionResult 기반 damage/action/reaction/feedback dispatch 추가
```

이 구조를 따르면 `CombatResolution`은 combat outcome producer로 남고, action/reaction orchestration은 execution request arbitrator로 남는다.










