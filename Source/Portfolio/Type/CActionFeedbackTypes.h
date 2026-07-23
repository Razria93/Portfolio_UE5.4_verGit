#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "CActionFeedbackTypes.generated.h"

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

USTRUCT(BlueprintType)
struct FActionFeedbackKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FActionFeedbackKey() = default;

public:
	bool operator==(const FActionFeedbackKey& InOther) const
	{
		return ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

USTRUCT(BlueprintType)
struct FActionFeedbackRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

public:
	FActionFeedbackRequest() = default;
};

USTRUCT(BlueprintType)
struct FTrailFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	bool bTrailActive = false;

public:
	FTrailFeedbackData() = default;
};

USTRUCT(BlueprintType)
struct FActionVFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

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
	FVector RelativeScale = FVector(1.f, 1.f, 1.f);

public:
	FActionVFXFeedbackData() = default;
};

USTRUCT(BlueprintType)
struct FActionSFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

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

USTRUCT()
struct FActionVFXExecutionKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(Transient)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(Transient)
	FName TriggerKey = NAME_None;

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
	FVector RelativeScale = FVector(1.f, 1.f, 1.f);

public:
	FActionVFXExecutionKey() = default;

public:
	bool operator==(const FActionVFXExecutionKey& InOther) const
	{
		return ActionFeedbackKey == InOther.ActionFeedbackKey
			&& ActionFeedbackTiming == InOther.ActionFeedbackTiming
			&& TriggerKey == InOther.TriggerKey
			&& VFXPlayType == InOther.VFXPlayType
			&& VFX == InOther.VFX
			&& SocketName == InOther.SocketName
			&& RelativeLocation == InOther.RelativeLocation
			&& RelativeRotation == InOther.RelativeRotation
			&& RelativeScale == InOther.RelativeScale;
	}
};

FORCEINLINE uint32 GetTypeHash(const FActionVFXExecutionKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackKey.ActionType)));
	H = HashCombine(H, GetTypeHash(InKey.ActionFeedbackKey.ActionIndex));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackTiming)));
	H = HashCombine(H, GetTypeHash(InKey.TriggerKey));
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
struct FActionSFXExecutionKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(Transient)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(Transient)
	FName TriggerKey = NAME_None;

	UPROPERTY(Transient)
	EActionSFXPlayType SFXPlayType = EActionSFXPlayType::Once;

	UPROPERTY(Transient)
	TObjectPtr<class USoundBase> SFX = nullptr;

public:
	FActionSFXExecutionKey() = default;

public:
	bool operator==(const FActionSFXExecutionKey& InOther) const
	{
		return ActionFeedbackKey == InOther.ActionFeedbackKey
			&& ActionFeedbackTiming == InOther.ActionFeedbackTiming
			&& TriggerKey == InOther.TriggerKey
			&& SFXPlayType == InOther.SFXPlayType
			&& SFX == InOther.SFX;
	}
};

FORCEINLINE uint32 GetTypeHash(const FActionSFXExecutionKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackKey.ActionType)));
	H = HashCombine(H, GetTypeHash(InKey.ActionFeedbackKey.ActionIndex));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackTiming)));
	H = HashCombine(H, GetTypeHash(InKey.TriggerKey));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.SFXPlayType)));
	H = HashCombine(H, GetTypeHash(InKey.SFX));

	return H;
}
