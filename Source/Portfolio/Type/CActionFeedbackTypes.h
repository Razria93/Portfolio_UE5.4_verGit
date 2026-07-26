#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "CActionFeedbackTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EActionFeedbackTiming : uint8
{
	None,

	Start,

	Complete,
	Interrupt,

	Chain,

	TriggerOnce,
	TriggerWindowBegin,
	TriggerWindowEnd
};

enum class EActionFeedbackMatchTier : uint8
{
	None = 0,

	AnyActionAnyIndex,
	ExactActionAnyIndex,
	ExactActionExactIndex,
};

UENUM(BlueprintType)
enum class EActionVFXPlayType : uint8
{
	Once,
	Loop
};

UENUM(BlueprintType)
enum class EActionSFXPlayType : uint8
{
	Once,
	Loop
};

// Key / Identifier

USTRUCT(BlueprintType)
struct FActionFeedbackMatchKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FActionFeedbackMatchKey() = default;

public:
	bool operator==(const FActionFeedbackMatchKey& InOther) const
	{
		return ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

// Request

USTRUCT(BlueprintType)
struct FActionFeedbackRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackMatchKey ActionFeedbackMatchKey = FActionFeedbackMatchKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

public:
	FActionFeedbackRequest() = default;
};

// Data / Config

USTRUCT(BlueprintType)
struct FActionTrailFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackMatchKey ActionFeedbackMatchKey = FActionFeedbackMatchKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	bool bTrailActive = false;

public:
	FActionTrailFeedbackData() = default;
};

USTRUCT(BlueprintType)
struct FActionVFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackMatchKey ActionFeedbackMatchKey = FActionFeedbackMatchKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	EActionVFXPlayType VFXPlayType = EActionVFXPlayType::Once;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* VFX = nullptr;

	UPROPERTY(EditAnywhere)
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere)
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	FVector RelativeScale = FVector::OneVector;

public:
	FActionVFXFeedbackData() = default;
};

USTRUCT(BlueprintType)
struct FActionSFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackMatchKey ActionFeedbackMatchKey = FActionFeedbackMatchKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	EActionSFXPlayType SFXPlayType = EActionSFXPlayType::Once;

	UPROPERTY(EditAnywhere)
	class USoundBase* SFX = nullptr;

public:
	FActionSFXFeedbackData() = default;
};

// Runtime Key / Playback Key

USTRUCT()
struct FActionVFXPlaybackKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionVFXPlayType VFXPlayType = EActionVFXPlayType::Once;

	UPROPERTY(Transient)
	TObjectPtr<class UNiagaraSystem> VFX = nullptr;

	UPROPERTY(Transient)
	FName SocketName = NAME_None;

	UPROPERTY(Transient)
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FVector RelativeScale = FVector::OneVector;

public:
	FActionVFXPlaybackKey() = default;

public:
	bool operator==(const FActionVFXPlaybackKey& InOther) const
	{
		return VFXPlayType == InOther.VFXPlayType
			&& VFX == InOther.VFX
			&& SocketName == InOther.SocketName
			&& RelativeLocation == InOther.RelativeLocation
			&& RelativeRotation == InOther.RelativeRotation
			&& RelativeScale == InOther.RelativeScale;
	}
};

FORCEINLINE uint32 GetTypeHash(const FActionVFXPlaybackKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.VFXPlayType)));
	H = HashCombine(H, GetTypeHash(InKey.VFX));
	H = HashCombine(H, GetTypeHash(InKey.SocketName));

	H = HashCombine(H, GetTypeHash(InKey.RelativeLocation.X));
	H = HashCombine(H, GetTypeHash(InKey.RelativeLocation.Y));
	H = HashCombine(H, GetTypeHash(InKey.RelativeLocation.Z));

	H = HashCombine(H, GetTypeHash(InKey.RelativeRotation.Pitch));
	H = HashCombine(H, GetTypeHash(InKey.RelativeRotation.Yaw));
	H = HashCombine(H, GetTypeHash(InKey.RelativeRotation.Roll));

	H = HashCombine(H, GetTypeHash(InKey.RelativeScale.X));
	H = HashCombine(H, GetTypeHash(InKey.RelativeScale.Y));
	H = HashCombine(H, GetTypeHash(InKey.RelativeScale.Z));

	return H;
}

USTRUCT()
struct FActionSFXPlaybackKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionSFXPlayType SFXPlayType = EActionSFXPlayType::Once;

	UPROPERTY(Transient)
	TObjectPtr<class USoundBase> SFX = nullptr;

public:
	FActionSFXPlaybackKey() = default;

public:
	bool operator==(const FActionSFXPlaybackKey& InOther) const
	{
		return SFXPlayType == InOther.SFXPlayType
			&& SFX == InOther.SFX;
	}
};

FORCEINLINE uint32 GetTypeHash(const FActionSFXPlaybackKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.SFXPlayType)));
	H = HashCombine(H, GetTypeHash(InKey.SFX));

	return H;
}
