#include "Component/CHealthComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

UCHealthComponent::UCHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = Cast<AActor>(GetOwner());
	check(OwnerActor_Cached);

	InitializeHealth(InitMaxHP, InitCurrentHP, bFillToInitMaxHP);
}

void UCHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCHealthComponent::InitializeHealth(float InInitMaxHP, float InInitCurrentHP, bool bFillToMaxHP)
{
	SetMaxHP(InInitMaxHP, bFillToMaxHP);

	if (!bFillToMaxHP)
	{
		SetCurrentHP(InInitCurrentHP);
	}
}

void UCHealthComponent::SetMaxHP(float InNewMaxHP, bool bFillToMaxHP)
{
	// Validate InNewMaxHP
	if (InNewMaxHP <= 0.f) return;

	MaxHP = InNewMaxHP;

	// TODO: `FOnMaxHealthChanged` Delegate BroadCast

	// TODO : Implement `EMaxHealthUpdatePolicy`
	float newCurrentHP = bFillToMaxHP ? MaxHP : FMath::Clamp(CurrentHP, 0.f, MaxHP);
	SetCurrentHP(newCurrentHP);

	// TODO: `FOnHealthChanged` Delegate BroadCast

	UpdateDeadState();
}

void UCHealthComponent::SetCurrentHP(float InNewCurrentHP)
{
	if (InNewCurrentHP < 0.f) return;

	CurrentHP = FMath::Clamp(InNewCurrentHP, 0.f, MaxHP);

	// TODO: `FOnHealthChanged` Delegate BroadCast

	UpdateDeadState();
}

void UCHealthComponent::SetKill()
{
	if (bIsDead) return;

	CurrentHP = 0.f;

	// TODO: `FOnHealthChanged` Delegate BroadCast

	UpdateDeadState();
}

float UCHealthComponent::TakeDamage(float InTakeDamageAmount)
{
	if (bIsDead) return 0.f;
	if (MaxHP <= 0.f) return 0.f;
	if (InTakeDamageAmount <= 0.f) return 0.f;

	float clampedDamage = FMath::Max(0.f, InTakeDamageAmount);

	float oldHP = CurrentHP;
	float newHP = FMath::Clamp(CurrentHP - clampedDamage, 0.f, MaxHP);

	float takenDamage = oldHP - newHP;

	if (takenDamage <= 0.f) return 0.f;

	PreviousHP = oldHP;
	CurrentHP = newHP;

	// TODO: `FOnHealthChanged` Delegate BroadCast

	UpdateDeadState();

	PrintTakeDamageContextInfo();

	return takenDamage;
}

float UCHealthComponent::TakeHeal(float InTakeHealAmount)
{
	if (bIsDead) return 0.f;
	if (MaxHP <= 0.f) return 0.f;
	if (InTakeHealAmount <= 0.f) return 0.f;

	float clampedHeal = FMath::Max(0.f, InTakeHealAmount);

	float oldHP = CurrentHP;
	float newHP = FMath::Clamp(CurrentHP + clampedHeal, 0.f, MaxHP);

	float takenHeal = newHP - oldHP;

	if (takenHeal <= 0.f) return 0.f;

	PreviousHP = oldHP;
	CurrentHP = newHP;

	// TODO: `FOnHealthChanged` Delegate BroadCast

	UpdateDeadState();

	PrintTakeHealContextInfo();

	return takenHeal;
}

void UCHealthComponent::UpdateDeadState()
{
	bool bWasDead = bIsDead;
	bIsDead = (CurrentHP <= 0.f);	// UpdateDeadState

	if (!bWasDead && bIsDead)
	{
		// Alive -> Dead
		// TODO: `FOnDead` Delegate BroadCast
	}
	else if (bWasDead && !bIsDead)
	{
		// Dead -> Alive
		// TODO: `FOnRevived` Delegate BroadCast
	}
}

void UCHealthComponent::PrintTakeDamageContextInfo()
{
	FLog::Log(TEXT("========= Damage Event =========="));
	PrintHealthContextInfo("TakeDamage");
	PrintDeadContextInfo("TakeDamage");
	FLog::Log(TEXT("================================="));
}

void UCHealthComponent::PrintTakeHealContextInfo()
{
	FLog::Log(TEXT("========== Heal Event ==========="));
	PrintHealthContextInfo("TakeHeal");
	PrintDeadContextInfo("TakeHeal");
	FLog::Log(TEXT("================================="));
}

void UCHealthComponent::PrintHealthContextInfo(const FString& InLabel) const
{
	FLog::Log(TEXT("-------- Health Context ---------"));
	if (!InLabel.IsEmpty())
	{
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Label"), *InLabel));
	}

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(OwnerActor_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("MaxHP"), MaxHP));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("PreviousHP"), PreviousHP));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CurrentHP"), CurrentHP));
	
	float hpDelta = PreviousHP - CurrentHP;
	float hpPercent = 0.f;

	if (MaxHP > 0.f)
	{
		hpPercent = CurrentHP / MaxHP;
	}

	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("HPDelta"), hpDelta));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("HPPercent"), hpPercent));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsDead"), bIsDead ? TEXT("true") : TEXT("false")));
	FLog::Log(TEXT("---------------------------------"));
}

void UCHealthComponent::PrintDeadContextInfo(const FString& InLabel) const
{
	FLog::Log(TEXT("--------- Dead Context ----------"));
	if (!InLabel.IsEmpty())
	{
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Label"), *InLabel));
	}

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(OwnerActor_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsDead"), bIsDead ? TEXT("true") : TEXT("false")));
	FLog::Log(TEXT("---------------------------------"));
}

