#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DamageEvents.h"
#include "DamageEventId.h"
#include "CWeaponStructure.generated.h"

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	Unarmed = 0,
	Sword,
	Max,
};

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	None = 0,
	Default,
	Max,
};

UENUM(BlueprintType)
enum class EActionType : uint8
{
	Idle = 0,
	LightAttack,
	ComboAttack,
	Max,
};

USTRUCT(BlueprintType)
struct FEquipmentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bCanMove = false;

public:
	FEquipmentData() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bCanMove = false;

public:
	FActionData() = default;

public:
	bool IsValidMinimal() const;

public:
	void BeginPlayMontage(class ACharacter* InOwnerCharacter);
	void EndPlayMontage(class ACharacter* InOwnerCharacter);
};

USTRUCT(BlueprintType)
struct FAttachmentContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EAttachmentType CurrentAttachmentType = EAttachmentType::Max;

public:
	FAttachmentContext() = default;
};

USTRUCT(BlueprintType)
struct FEquipmentContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EEquipmentType CurrentEquipmentType = EEquipmentType::Max;

public:
	FEquipmentContext() = default;
};

USTRUCT(BlueprintType)
struct FActionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EActionType CurrentActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FActionContext() = default;
};

USTRUCT(BlueprintType)
struct FOverlapContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class AActor* OwnerActor = nullptr;							// AttackActor

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;						// Attachment

	UPROPERTY(Transient)
	class UPrimitiveComponent* OverlappedComponent = nullptr;	// Hit Collision of Attachment

	UPROPERTY(Transient)
	class UShapeComponent* OverlapShape = nullptr;			// Cast result: UShapeComponent (nullptr if cast fails)

	UPROPERTY(Transient)
	class AActor* OtherActor = nullptr;							// DamagedActor

	UPROPERTY(Transient)
	class UPrimitiveComponent* OtherComponent = nullptr;		// DamagedComponent

	UPROPERTY(Transient)
	int32 OtherBodyIndex = INDEX_NONE;

	UPROPERTY(Transient)
	bool bFromSweep = false;

	UPROPERTY(Transient)
	FHitResult SweepResult;

public:
	FOverlapContext() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FHitContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FOverlapContext OverlapContext = FOverlapContext();

	UPROPERTY(Transient)
	FAttachmentContext AttachmentContext = FAttachmentContext();

	UPROPERTY(Transient)
	FEquipmentContext  EquipmentContext = FEquipmentContext();

	UPROPERTY(Transient)
	FActionContext ActionContext = FActionContext();

public:
	FHitContext() = default;
};

USTRUCT(BlueprintType)
struct FDamageSpecKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EAttachmentType AttachmentType = EAttachmentType::Max;

	UPROPERTY(EditAnywhere)
	EEquipmentType EquipmentType = EEquipmentType::Max;

	UPROPERTY(EditAnywhere)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FDamageSpecKey() = default;

public:
	bool operator==(const FDamageSpecKey& Other) const
	{
		return EquipmentType == Other.EquipmentType
			&& AttachmentType == Other.AttachmentType
			&& ActionType == Other.ActionType
			&& ActionIndex == Other.ActionIndex;
	}

public:
	friend FORCEINLINE uint32 GetTypeHash(const FDamageSpecKey& Key)
	{
		uint32 H = 0;
		H = HashCombine(H, ::GetTypeHash(static_cast<uint8>(Key.AttachmentType)));
		H = HashCombine(H, ::GetTypeHash(static_cast<uint8>(Key.EquipmentType)));
		H = HashCombine(H, ::GetTypeHash(static_cast<uint8>(Key.ActionType)));
		H = HashCombine(H, ::GetTypeHash(Key.ActionIndex));
		return H;
	}
};

USTRUCT(BlueprintType)
struct FDamageSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float BaseDamage = 0.f;

public:
	FDamageSpec() = default;
};

USTRUCT(BlueprintType)
struct FDamageResult
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float FinalDamage = 0.f;

	// TODO:
	// FVector ImpactPoint;
	// FVector HitNormal;

public:
	FDamageResult() = default;
};

USTRUCT(BlueprintType)
struct FDefaultDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

public:
	UPROPERTY() 
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY()
	FDamageSpec DamageSpec = FDamageSpec();

	UPROPERTY()
	FDamageResult DamageResult = FDamageResult();

public:
	static const int32 ClassID = (int32)EDamageEventTypeId::DefaultDamage;

public:
	FDefaultDamageEvent() = default;

public:
	virtual int32 GetTypeID() const override { return ClassID; }
	virtual bool IsOfType(int32 InID) const override { return InID == ClassID || FDamageEvent::IsOfType(InID); }
};

USTRUCT(BlueprintType)
struct FTakeDamagePayload
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	class AController* EventInstigator = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(VisibleAnywhere)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(VisibleAnywhere)
	FDamageSpec DamageSpec = FDamageSpec();

	UPROPERTY(VisibleAnywhere)
	FDamageResult DamageResult = FDamageResult();

public:
	FTakeDamagePayload() = default;
};

USTRUCT(BlueprintType)
struct FTakeDamageContext
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	class AActor* DamagedActor = nullptr;

	// === Payload ===
	UPROPERTY(VisibleAnywhere)
	class AController* EventInstigator = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(VisibleAnywhere)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(VisibleAnywhere)
	FDamageSpec DamageSpec = FDamageSpec();

	UPROPERTY(VisibleAnywhere)
	FDamageResult DamageResult = FDamageResult();

	// === Damage values ===
	UPROPERTY(VisibleAnywhere)
	float TakenDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	float FinalDamage = 0.f;

	// === TakeDamage-side Flags ===
	UPROPERTY(VisibleAnywhere)
	bool bCanApplyDamage = true;

	UPROPERTY(VisibleAnywhere)
	bool bIsDead = false;

public:
	FTakeDamageContext() = default;
};

UCLASS()
class PORTFOLIO_API UCWeaponStructure : public UObject
{
	GENERATED_BODY()

};
