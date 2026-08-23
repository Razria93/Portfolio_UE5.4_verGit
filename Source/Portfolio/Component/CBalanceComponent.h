#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CBalanceTypes.h"
#include "Type/CReactionTypes.h"
#include "CBalanceComponent.generated.h"

struct FCharacterComponentReferences;
struct FReactionExecutionLifecycleEvent;
struct FReactionRequestResult;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBalanceLifecycleStateChanged, EBalanceLifecycleState, EBalanceLifecycleState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBalanceCollapseLoopExpired, uint32);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCBalanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCBalanceComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 1))
	int32 BalanceThreshold = 3;

	UPROPERTY(EditAnywhere, Category = "Balance", meta = (ClampMin = 0.0))
	float CollapseLoopDuration = 5.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	int32 CurrentBalanceCount = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	uint32 BalanceLifecycleSerial = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	EBalanceLifecycleState BalanceLifecycleState = EBalanceLifecycleState::Accumulating;

	UPROPERTY(VisibleInstanceOnly, Category = "Balance")
	EBalanceAbortReason LastAbortReason = EBalanceAbortReason::None;

	FTimerHandle CollapseLoopTimerHandle;

public:
	FOnBalanceLifecycleStateChanged OnBalanceLifecycleStateChanged;
	FOnBalanceCollapseLoopExpired OnBalanceCollapseLoopExpired;

public:
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

public:
	FBalanceAdvanceResult AdvanceFromParry(const struct FCombatResultPacket& InPacket);
	void HandleBalanceLifecycleReactionResolved(const FBalanceLifecyclePacket& InBalanceLifecyclePacket, const FReactionRequestResult& InResult);
	bool HandleCollapseReactionStarted(const struct FReactionExecutionContext& InContext);
	void HandleCollapseReactionTerminal(const FReactionExecutionLifecycleEvent& InEvent);
	bool BeginCollapseOutRequest(uint32 InBalanceLifecycleSerial);
	bool CommitCollapseReset(uint32 InBalanceLifecycleSerial);
	void AbortBalanceLifecycle(EBalanceAbortReason InReason);
	void ShutdownBalanceRuntime();

public:
	int32 GetCurrentBalanceCount() const { return CurrentBalanceCount; }
	int32 GetBalanceThreshold() const { return BalanceThreshold; }
	uint32 GetBalanceLifecycleSerial() const { return BalanceLifecycleSerial; }
	EBalanceLifecycleState GetBalanceLifecycleState() const { return BalanceLifecycleState; }
	EBalanceAbortReason GetLastAbortReason() const { return LastAbortReason; }

	bool IsCollapseLoopPoseActive() const;
	bool IsBalanceLifecycleBlocking() const;
	bool ShouldSuppressCombatTargetFacing() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

private:
	bool MatchesLifecycleContext(const struct FReactionExecutionContext& InContext, EReactionType InReactionType) const;
	void SetBalanceLifecycleState(EBalanceLifecycleState InState);
	void StartCollapseLoopTimer();
	void ClearCollapseLoopTimer();
	void HandleCollapseLoopExpired();
	void ResetBalanceRuntime();
	bool IsDuplicateParryPacket(const struct FCombatResultPacket& InPacket) const;
	void RememberAcceptedParryPacket(const struct FCombatResultPacket& InPacket);

private:
	TMap<TWeakObjectPtr<class AActor>, uint64> LastAcceptedParryResultSerialByTarget;
};
