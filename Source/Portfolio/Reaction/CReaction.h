#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "Type/CReactionFeedbackStructure.h"
#include "Type/CReactionOrchestrationStructure.h"
#include "CReaction.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCReaction : public UObject
{
	GENERATED_BODY()

protected:
	/* === Runtime State === */
	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	TArray<FExecutionInterventionParticipantFilter> WantCancelFilters;

	UPROPERTY(Transient)
	TArray<FExecutionInterventionParticipantFilter> WantInterruptFilters;

	UPROPERTY(Transient)
	TArray<FExecutionInterventionParticipantFilter> AllowCancelFilters;

	UPROPERTY(Transient)
	TArray<FExecutionInterventionParticipantFilter> AllowInterruptFilters;

protected:
	uint32 Serial_CurrentPlay = 0;		// Serial of Current Play Reaction
	uint32 CachedSerial_ActivePlay = 0;	// Cached Serial of Active Play Reaction

protected:
	UPROPERTY(Transient)
	FReactionDataKey ActiveDataKey_Cached = FReactionDataKey();

	UPROPERTY(Transient)
	FReactionData ActiveData_Cached = FReactionData();

	UPROPERTY(Transient)
	class UAnimMontage* ActiveMontage_Cached = nullptr;

	UPROPERTY(Transient)
	EReactionStopReason LastStopReason_Cached = EReactionStopReason::None;

protected:
	/* === Injection Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* OwnerReactionComp_Injected = nullptr;

protected:
	UPROPERTY(Transient)
	class UCReactionFeedbackComponent* ReactionFeedbackComp_Cached = nullptr;

public:
	// Initialize / Tick
	virtual void Initialize(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComp);
	virtual void Tick(float InDeltaTime) {};

public:
	// State Query
	bool IsActive() const { return bIsActive; }

public:
	const FReactionDataKey& GetActiveDataKey() const { return ActiveDataKey_Cached; }
	const FReactionData& GetActiveData() const { return ActiveData_Cached; }

public:
	// Decision
	virtual FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const;

protected:
	bool IsIncomingReactionType(const FExecutionDecisionQuery& InQuery, EReactionType InType) const;
	bool IsIncomingReactionType(const FExecutionInterventionQuery& InQuery, EReactionType InType) const;

protected:
	bool CanResolveIndependentRelationship(const FExecutionDecisionQuery& InQuery) const;
	bool TryResolveIndependentOrExclusiveRelationship(const FExecutionDecisionQuery& InQuery, EExecutionRelationship& OutRelationship) const;

public:
	// Lifecycle
	virtual bool Start(const FReactionData& InData);
	virtual void Stop(EReactionStopReason InStopReason);
	virtual void Complete();

protected:
	virtual void ClearRuntime();

protected:
	// Montage Lifecycle
	virtual bool PlayMontage(const FReactionData& InData);
	virtual void StopMontage(float InBlendOutTime = 0.1f);
	virtual bool BindMontageEndDelegate();

protected:
	UFUNCTION()
	void OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial);
	bool CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const;

public:
	// Notify
	void HandleNotifyCommand(EReactionNotifyCommand InCommand);

protected:
	virtual void HandleSpecificNotifyCommand(EReactionNotifyCommand InCommand);

public:
	// Feedback
	void HandleNotifyFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None);

protected:
	void RequestFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;
	virtual FReactionFeedbackRequest BuildFeedbackRequest(EReactionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;

public:
	// Intervention Window
	void OpenInterventionWindow(
		const FExecutionInterventionParticipantFilter& InOwnerFilter, EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole, const TArray<FExecutionInterventionParticipantFilter>& InCounterpartFilters);
	void CloseInterventionWindow(
		const FExecutionInterventionParticipantFilter& InOwnerFilter, EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole, const TArray<FExecutionInterventionParticipantFilter>& InCounterpartFilters);

public:
	// Intervention Match
	virtual bool MatchesWantIntervention(const FExecutionInterventionQuery& InQuery) const;
	virtual bool MatchesAllowIntervention(const FExecutionInterventionQuery& InQuery) const;

private:
	bool MatchesInterventionOwner(const FExecutionInterventionParticipantFilter& InOwnerFilter) const;
	bool MatchesAnyInterventionFilter(const TArray<FExecutionInterventionParticipantFilter>& InFilters, const FExecutionParticipant& InParticipant) const;

private:
	TArray<FExecutionInterventionParticipantFilter>* GetInterventionFilterContainer(EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole);
	const TArray<FExecutionInterventionParticipantFilter>* GetInterventionFilterContainer(EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole) const;

public:
	// Debug
	void PrintReactionExecutorRuntimeInfo_Public() const;

private:
	void PrintReactionExecutorRuntimeInfo() const;

	void PrintStopReasonInfo(EReactionStopReason InStopReason) const;
	void PrintIgnoredStopReasonInfo() const;
};
