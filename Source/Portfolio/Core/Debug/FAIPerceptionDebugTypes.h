#pragma once

#include "CoreMinimal.h"

class AActor;

struct FPerceptionCandidateAuditState
{
	bool bEnabled = false;

	float RuntimeStartTime = 0.f;
	uint64 RuntimeStartFrame = 0;

	float FirstRawPerceptionTime = -1.f;
	uint64 FirstRawPerceptionFrame = 0;

	float FirstValidTargetTime = -1.f;
	uint64 FirstValidTargetFrame = 0;

	int32 RawPerceptionEventCount = 0;
	int32 MaxTargetDataMapSize = 0;

	TSet<TWeakObjectPtr<AActor>> RawPerceptionActors;
	TSet<TWeakObjectPtr<AActor>> ValidTargetProviderActors;
	TSet<TWeakObjectPtr<AActor>> InvalidTargetProviderActors;

	void Reset()
	{
		bEnabled = false;

		RuntimeStartTime = 0.f;
		RuntimeStartFrame = 0;

		FirstRawPerceptionTime = -1.f;
		FirstRawPerceptionFrame = 0;

		FirstValidTargetTime = -1.f;
		FirstValidTargetFrame = 0;

		RawPerceptionEventCount = 0;
		MaxTargetDataMapSize = 0;

		RawPerceptionActors.Reset();
		ValidTargetProviderActors.Reset();
		InvalidTargetProviderActors.Reset();
	}
};

struct FBlackboardEngageLatencyAuditState
{
	bool bEnabled = false;

	float RuntimeStartTime = 0.f;
	uint64 RuntimeStartFrame = 0;

	float FirstPerceptionContextTime = -1.f;
	uint64 FirstPerceptionContextFrame = 0;

	float FirstBlackboardTargetTime = -1.f;
	uint64 FirstBlackboardTargetFrame = 0;

	float FirstEngageRequestTime = -1.f;
	uint64 FirstEngageRequestFrame = 0;

	float FirstEngageAssignmentTime = -1.f;
	uint64 FirstEngageAssignmentFrame = 0;

	TWeakObjectPtr<AActor> FirstPerceptionTargetActor;
	TWeakObjectPtr<AActor> FirstBlackboardTargetActor;
	TWeakObjectPtr<AActor> FirstEngageRequestTargetActor;
	TWeakObjectPtr<AActor> FirstEngageAssignmentTargetActor;

	void Reset()
	{
		bEnabled = false;

		RuntimeStartTime = 0.f;
		RuntimeStartFrame = 0;

		FirstPerceptionContextTime = -1.f;
		FirstPerceptionContextFrame = 0;

		FirstBlackboardTargetTime = -1.f;
		FirstBlackboardTargetFrame = 0;

		FirstEngageRequestTime = -1.f;
		FirstEngageRequestFrame = 0;

		FirstEngageAssignmentTime = -1.f;
		FirstEngageAssignmentFrame = 0;

		FirstPerceptionTargetActor.Reset();
		FirstBlackboardTargetActor.Reset();
		FirstEngageRequestTargetActor.Reset();
		FirstEngageAssignmentTargetActor.Reset();
	}
};
