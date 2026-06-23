#pragma once

#include "CoreMinimal.h"
#include "Type/CWeaponStructure.h"
#include "CReactionFeedbackStructure.generated.h"

UENUM(BlueprintType)
enum class EReactionFeedbackTiming : uint8
{
	None = 0,

	Start,

	Complete,
	Interrupt,

	TriggerWindowBegin,
	TriggerWindowEnd,

	TriggerOnce,

	Max,
};

UENUM(BlueprintType)
enum class EReactionVFXPlayType : uint8
{
	Once,
	Loop
};

UENUM(BlueprintType)
enum class EReactionSFXPlayType : uint8
{
	Once,
	Loop
};

USTRUCT(BlueprintType)
struct FReactionFeedbackKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EReactionType ReactionType = EReactionType::Max;

	UPROPERTY(EditAnywhere)
	FDamageSpecKey ApplyDamageSpecKey = FDamageSpecKey();

public:
	FReactionFeedbackKey() = default;

public:
	bool operator==(const FReactionFeedbackKey& InOther) const
	{
		return ReactionType == InOther.ReactionType
			&& ApplyDamageSpecKey == InOther.ApplyDamageSpecKey;
	}
};

USTRUCT(BlueprintType)
struct FReactionFeedbackRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FReactionFeedbackKey ReactionFeedbackKey = FReactionFeedbackKey();

	UPROPERTY(Transient)
	EReactionFeedbackTiming ReactionFeedbackTiming = EReactionFeedbackTiming::None;

	UPROPERTY(Transient)
	FName TriggerKey = NAME_None;

public:
	FReactionFeedbackRequest() = default;
};

USTRUCT(BlueprintType)
struct FReactionVFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FReactionFeedbackKey ReactionFeedbackKey = FReactionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EReactionFeedbackTiming ReactionFeedbackTiming = EReactionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	EReactionVFXPlayType VFXPlayType = EReactionVFXPlayType::Once;

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
	FReactionVFXFeedbackData() = default;
};

USTRUCT(BlueprintType)
struct FReactionSFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FReactionFeedbackKey ReactionFeedbackKey = FReactionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EReactionFeedbackTiming ReactionFeedbackTiming = EReactionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	EReactionSFXPlayType SFXPlayType = EReactionSFXPlayType::Once;

	UPROPERTY(EditAnywhere)
	class USoundBase* SFX = nullptr;

public:
	FReactionSFXFeedbackData() = default;
};

USTRUCT()
struct FReactionVFXExecutionKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionVFXPlayType VFXPlayType = EReactionVFXPlayType::Once;

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
	FReactionVFXExecutionKey() = default;

public:
	bool operator==(const FReactionVFXExecutionKey& InOther) const
	{
		return VFXPlayType == InOther.VFXPlayType
			&& VFX == InOther.VFX
			&& SocketName == InOther.SocketName
			&& RelativeLocation == InOther.RelativeLocation
			&& RelativeRotation == InOther.RelativeRotation
			&& RelativeScale == InOther.RelativeScale;
	}
};

FORCEINLINE uint32 GetTypeHash(const FReactionVFXExecutionKey& InKey)
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
struct FReactionSFXExecutionKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionSFXPlayType SFXPlayType = EReactionSFXPlayType::Once;

	UPROPERTY(Transient)
	TObjectPtr<class USoundBase> SFX = nullptr;

public:
	FReactionSFXExecutionKey() = default;

public:
	bool operator==(const FReactionSFXExecutionKey& InOther) const
	{
		return SFXPlayType == InOther.SFXPlayType
			&& SFX == InOther.SFX;
	}
};

FORCEINLINE uint32 GetTypeHash(const FReactionSFXExecutionKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.SFXPlayType)));
	H = HashCombine(H, GetTypeHash(InKey.SFX));

	return H;
}
