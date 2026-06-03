#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
// #include "Type/CActionFeedbackStructure.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CAction.generated.h"

UCLASS(Abstract) // Base Action Class
class PORTFOLIO_API UCAction : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	TSet<FName> ActiveInterventionWindowKeys;

protected:
	uint32 Serial_CurrentPlay = 0;		// Serial of Current Play Action
	uint32 CachedSerial_ActivePlay = 0;	// Cached Serial of Active Play Action

protected:
	UPROPERTY(Transient)
	FActionDataKey ActiveDataKey_Cached = FActionDataKey();

	UPROPERTY(Transient)
	FActionData ActiveData_Cached = FActionData();

	UPROPERTY(Transient)
	UAnimMontage* ActiveMontage_Cached = nullptr;

	UPROPERTY(Transient)
	EActionStopReason LastStopReason_Cached = EActionStopReason::None;

protected:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* OwnerActionComp_Injected = nullptr;

protected:
	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionFeedbackComponent* ActionFeedbackComp_Cached = nullptr;

public:
	// Initialize / Tick
	virtual void InitializeAction(ACharacter* InOwnerCharacter, class UCActionComponent* InOwnerActionComp);
	virtual void Tick(float InDeltaTime) {}

public:
	// State Query
	bool IsActive() const { return bIsActive; }

public:
	const FActionDataKey& GetActiveDataKey() const { return ActiveDataKey_Cached; }
	const FActionData& GetActiveData() const { return ActiveData_Cached; }

public:
	// Decision
	virtual FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const;

protected:
	bool IsIncomingActionType(const FExecutionDecisionQuery& InQuery, EActionType InType) const;
	bool IsIncomingActionType(const FExecutionInterventionQuery& InQuery, EActionType InType) const;

protected:
	bool CanResolveIndependentRelationship(const FExecutionDecisionQuery& InQuery) const;
	bool CanResolveExclusiveRelationship(const FExecutionDecisionQuery& InQuery) const;
	bool TryResolveIndependentOrExclusiveRelationship(const FExecutionDecisionQuery& InQuery, EExecutionRelationship& OutRelationship) const;

public:
	// Lifecycle
	virtual bool Start(const FActionData& InData);
	virtual void Stop(EActionStopReason InStopReason);
	virtual void Complete();

public:
	virtual bool ReserveChain(const FActionData& InData);
	virtual void ConsumeChain();

protected:
	virtual void ClearRuntime();

protected:
	// Montage Lifecycle
	virtual bool PlayMontage(const FActionData& InData);
	virtual void StopMontage(float InBlendOutTime = 0.1f);
	virtual bool BindMontageEndDelegate();

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial);
	bool CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const;

public:
	// Notify
	void HandleNotifyCommand(EActionNotifyCommand InCommand);

protected:
	virtual void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand);

public:
	// Feedback
	virtual void HandleNotifyFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None);

protected:
	void RequestFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;
	virtual FActionFeedbackRequest BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

protected:
	// Action-only hit context
	void PushHitContext();
	void ClearHitContext();
	virtual FActionContext BuildActionContext() const;

public:
	// Intervention Window
	void OpenInterventionWindow(FName InWindowKey);
	void CloseInterventionWindow(FName InWindowKey);

public:
	// Intervention Match
	virtual bool WantIntervention(const FExecutionInterventionQuery& InQuery) const; 	// Incoming API
	virtual bool AllowIntervention(const FExecutionInterventionQuery& InQuery) const;	// Acitve	API

private:
	bool MatchesWantInterventionRules(const TArray<FExecutionInterventionWantRule>& InRules, const FExecutionParticipant& InParticipant) const;
	bool MatchesAllowInterventionRules(const TArray<FExecutionInterventionAllowRule>& InRules, const FExecutionParticipant& InParticipant) const;
	bool IsAllowInterventionRuleTimingSatisfied(const FExecutionInterventionAllowRule& InRule) const;
	bool MatchesAnyInterventionFilter(const TArray<FExecutionInterventionParticipantFilter>& InFilters, const FExecutionParticipant& InParticipant) const;

protected:
	// Event
	void EmitActionEvent(EActionEventType InEventType, int32 InActionIndex = INDEX_NONE) const;

public:
	// [Legacy delegate]
	UFUNCTION()
	virtual void OnWeaponActorCollisionEnabled() {};

	UFUNCTION()
	virtual void OnWeaponActorCollisionDisabled() {};

	UFUNCTION()
	virtual void OnWeaponActorBeginOverlap(AActor* InAttackerActor, AActor* InDamageCauser, UShapeComponent* InAttackCollision, AActor* InTargetActor, UPrimitiveComponent* InHitComponent, int32 InOtherBodyIndex, bool InbFromSweep, const FHitResult& InSweepResult) {};

	UFUNCTION()
	virtual void OnWeaponActorEndOverlap(AActor* InAttackerActor, AActor* InTargetActor) {};
};
