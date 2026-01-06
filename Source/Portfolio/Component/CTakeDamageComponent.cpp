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

	const FDamageSpecKey& damageSpecKey = InDefaultDamageEvent.DamageSpecKey;
	const FDamageResult& damageResult = InDefaultDamageEvent.DamageResult;

	// Calculate Damage (Minimal)
	const float takedDamage = FMath::Max(0.f, damageResult.FinalDamage);

	// TODO: HP reduction / state transitions / hit reactions (montage, VFX/SFX) / knockback / hit stop (time dilation) / etc.

	PrintDefaultDamageEvent(takedDamage, InDefaultDamageEvent, InDamageInstigator, InDamageCauser);

	return takedDamage;
}

void UCTakeDamageComponent::PrintDefaultDamageEvent(float InTakeDamage, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	const FDamageSpecKey& damageSpecKey = InDefaultDamageEvent.DamageSpecKey;
	const FDamageSpec& damageSpec = InDefaultDamageEvent.DamageSpec;
	const FDamageResult& damageResult = InDefaultDamageEvent.DamageResult;

	// Print ObjectInfo
	FLog::Log(TEXT("[@ TAKE DAMAGE]"));
	FLog::Log(FString::Printf(TEXT("Victim = %s | Instigator = %s | Causer = %s"),
		*GetNameSafe(IsValid(GetOwner()) ? GetOwner() : nullptr),
		*GetNameSafe(InDamageInstigator),
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
	FLog::Log(FString::Printf(TEXT("Base Damage = %.3f | Applied Damage = %.3f | Taked Damage = %.3f"),
		damageSpec.BaseDamage,
		damageResult.FinalDamage,
		InTakeDamage
	));
}

