#include "Component/CTakeDamageComponent.h"
#include "ProjectGlobal.h"

#include "Type/CWeaponStructure.h"

UCTakeDamageComponent::UCTakeDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

	takeDamageContext.TakenDamage = takeDamagePayload.DamageResult.FinalDamage;
	takeDamageContext.FinalDamage = FMath::Max(0.f, takeDamagePayload.DamageResult.FinalDamage); // Set init FinalDamage

	// TODO:
	// takeDamageContext.bCanApplyDamage = (...);
	// takeDamageContext.bIsDead = (...);

	// TODO: 
	// Calculate-damage function

	// TODO: 
	// HP reduction / state transitions / hit reactions (montage, VFX/SFX) / knockback / hit stop (time dilation) / etc.

	PrintTakeDamageSummaryInfo(takeDamageContext);
	// PrintTakeDamageContextInfo(takeDamageContext);

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

void UCTakeDamageComponent::PrintTakeDamageSummaryInfo(const FTakeDamageContext& InTakeDamageContext) const
{
	FLog::Log(TEXT("====== Take Damage Summary ======"));
	FLog::Log(TEXT("[@ TAKE DAMAGE]"));

	FLog::Log(FString::Printf(TEXT("DamagedActor = %s | Instigator = %s | DamageCauser = %s"),
		*GetNameSafe(InTakeDamageContext.DamagedActor),
		*GetNameSafe(InTakeDamageContext.EventInstigator),
		*GetNameSafe(InTakeDamageContext.DamageCauser)
	));

	FLog::Log(FString::Printf(TEXT("TakenDamage = %.3f | FinalDamage = %.3f"),
		InTakeDamageContext.TakenDamage,
		InTakeDamageContext.FinalDamage
	));
	FLog::Log(TEXT("================================="));
}

void UCTakeDamageComponent::PrintTakeDamageContextInfo(const FTakeDamageContext& InTakeDamageContext) const
{
	FLog::Log(TEXT("/////- Take Damage Context -/////"));
	PrintTakeDamageObjectInfo(InTakeDamageContext);
	PrintTakeDamageSpecKeyInfo(InTakeDamageContext);
	PrintTakeDamageSpecInfo(InTakeDamageContext);
	PrintTakeDamageResultInfo(InTakeDamageContext);
	PrintTakeDamageAmountInfo(InTakeDamageContext);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCTakeDamageComponent::PrintTakeDamageObjectInfo(const FTakeDamageContext& InTakeDamageContext) const
{
	FLog::Log(TEXT("---------- Object Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DamagedActor"), *GetNameSafe(InTakeDamageContext.DamagedActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Instigator"), *GetNameSafe(InTakeDamageContext.EventInstigator)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DamageCauser"), *GetNameSafe(InTakeDamageContext.DamageCauser)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCTakeDamageComponent::PrintTakeDamageSpecKeyInfo(const FTakeDamageContext& InTakeDamageContext) const
{
	FLog::Log(TEXT("----------- Key Info ------------"));
	const FDamageSpecKey& damageSpecKey = InTakeDamageContext.DamageSpecKey;
	const FString actionIndexText = (damageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(damageSpecKey.ActionIndex);

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(damageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(damageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(damageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("---------------------------------"));
}

void UCTakeDamageComponent::PrintTakeDamageSpecInfo(const FTakeDamageContext& InTakeDamageContext) const
{
	FLog::Log(TEXT("---------- Damage Spec ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InTakeDamageContext.DamageSpec.BaseDamage));
	FLog::Log(TEXT("---------------------------------"));
}

void UCTakeDamageComponent::PrintTakeDamageResultInfo(const FTakeDamageContext& InTakeDamageContext) const
{
	FLog::Log(TEXT("--------- Damage Result ---------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("FinalDamage"), InTakeDamageContext.DamageResult.FinalDamage));
	FLog::Log(TEXT("---------------------------------"));
}

void UCTakeDamageComponent::PrintTakeDamageAmountInfo(const FTakeDamageContext& InTakeDamageContext) const
{
	FLog::Log(TEXT("---------- Amount Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("TakenDamage"), InTakeDamageContext.TakenDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("FinalDamage"), InTakeDamageContext.FinalDamage));
	FLog::Log(TEXT("---------------------------------"));
}

