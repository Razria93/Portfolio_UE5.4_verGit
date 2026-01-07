#include "Component/CTakeDamageComponent.h"
#include "ProjectGlobal.h"

#include "Type/CWeaponStructure.h"

UCTakeDamageComponent::UCTakeDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCTakeDamageComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCTakeDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UCTakeDamageComponent::RequestTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return ProcessTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

float UCTakeDamageComponent::ProcessTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FDefaultDamageEvent::ClassID))
	{
		const FDefaultDamageEvent& damageEvent = static_cast<const FDefaultDamageEvent&>(DamageEvent);
		return HandleDefaultDamage(damageEvent, EventInstigator, DamageCauser);
	}

	return DamageAmount;
}

float UCTakeDamageComponent::HandleDefaultDamage(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	if (!IsValid(InDamageCauser)) return 0.f;

	// Payload: Save Raw InputData (BeforeData)
	FTakeDamagePayload takeDamagePayload = FTakeDamagePayload();

	takeDamagePayload.EventInstigator = InDamageInstigator;
	takeDamagePayload.DamageCauser = InDamageCauser;
	takeDamagePayload.DamageSpecKey = InDefaultDamageEvent.DamageSpecKey;
	takeDamagePayload.DamageSpec = InDefaultDamageEvent.DamageSpec;
	takeDamagePayload.DamageResult = InDefaultDamageEvent.DamageResult;

	// Context: Save HandleData (AfterData)
	FTakeDamageContext takeDamageContext = FTakeDamageContext();

	takeDamageContext.DamagedActor = GetOwner();
	takeDamageContext.EventInstigator = ResolveInstigatorController(takeDamagePayload.EventInstigator, takeDamagePayload.DamageCauser);
	takeDamageContext.DamageCauser = takeDamagePayload.DamageCauser;
	takeDamageContext.DamageSpecKey = takeDamagePayload.DamageSpecKey;
	takeDamageContext.DamageSpec = takeDamagePayload.DamageSpec;
	takeDamageContext.DamageResult = takeDamagePayload.DamageResult;

	takeDamageContext.TakedDamage = takeDamagePayload.DamageResult.FinalDamage;
	takeDamageContext.FinalDamage = FMath::Max(0.f, takeDamagePayload.DamageResult.FinalDamage); // Set init FinalDamage

	// TODO:
	// takeDamageContext.bCanApplyDamage = (...);
	// takeDamageContext.bIsDead = (...);

	// TODO: 
	// Calculate-damage function

	// TODO: 
	// HP reduction / state transitions / hit reactions (montage, VFX/SFX) / knockback / hit stop (time dilation) / etc.

	PrintDefaultDamageEvent
	(
		takeDamageContext.DamagedActor,
		takeDamageContext.EventInstigator,
		takeDamageContext.DamageCauser,
		takeDamageContext.DamageSpecKey,
		takeDamageContext.DamageSpec,
		takeDamageContext.DamageResult,
		takeDamageContext.TakedDamage,
		takeDamageContext.FinalDamage
	);

	return takeDamageContext.FinalDamage;
}

AController* UCTakeDamageComponent::ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser)
{
	// 1) Best case: engine provided instigator
	if (IsValid(EventInstigator))
		return EventInstigator;

	// 2) Fallback needs a valid causer
	if (!IsValid(DamageCauser))
		return nullptr;

	/* === Fallback Process (DamageCauser-based) === */

	// 2-1) Case 01: Explicit instigator set on the causer (ex. projectile / attachment / trap)
	if (AController* causerInstigator = DamageCauser->GetInstigatorController())
		return causerInstigator;

	// 2-2) Case 02: Direct hit (the causer itself is a Pawn/Character)
	if (APawn* causerPawn = Cast<APawn>(DamageCauser))
	{
		if (AController* causerController = causerPawn->GetController())
			return causerController;
	}

	// 2-3) Case 03: Proxy case (projectile / trap / attachment owned by another actor)
	if (AActor* causerOwner = DamageCauser->GetOwner())
	{
		// 2-3-1) Case 03-01: Owner is the carrier and holds the correct instigator (ex. projectile / attachment / trap)
		if (AController* ownerInstigator = causerOwner->GetInstigatorController())
			return ownerInstigator;

		// 2-3-1) Case 03-02: Fallback (Owner is Pawn/Character)
		if (APawn* ownerPawn = Cast<APawn>(causerOwner))
		{
			if (AController* ownerController = ownerPawn->GetController())
				return ownerController;
		}
	}

	return nullptr;
}

void UCTakeDamageComponent::PrintDefaultDamageEvent(AActor* InDamagedActor, AController* InEventInstigator, AActor* InDamageCauser, const FDamageSpecKey& InDamageSpecKey, const FDamageSpec& InDamageSpec, const FDamageResult& InDamageResult, float InTakedDamage, float InFinalDamage)
{
	const FDamageSpecKey& damageSpecKey = InDamageSpecKey;
	const FDamageSpec& damageSpec = InDamageSpec;
	const FDamageResult& damageResult = InDamageResult;

	// Print ObjectInfo
	FLog::Log(TEXT("[@ TAKE DAMAGE]"));
	FLog::Log(FString::Printf(TEXT("DamagedActor = %s | Instigator = %s | DamageCauser = %s"),
		*GetNameSafe(InDamagedActor),
		*GetNameSafe(InEventInstigator),
		*GetNameSafe(InDamageCauser)
	));

	// Print KeyInfo
	FLog::Log(FString::Printf(TEXT("AttachmentType = %s | EquipmentType = %s | ActionType = %s | ActionIndex = %d"),
		*UEnum::GetValueAsString(damageSpecKey.AttachmentType),
		*UEnum::GetValueAsString(damageSpecKey.EquipmentType),
		*UEnum::GetValueAsString(damageSpecKey.ActionType),
		damageSpecKey.ActionIndex
	));

	// Print DamageInfo
	FLog::Log(FString::Printf(TEXT("Taked Damage = %.3f | Final Damage = %.3f"),
		InTakedDamage,
		InFinalDamage
	));
}

