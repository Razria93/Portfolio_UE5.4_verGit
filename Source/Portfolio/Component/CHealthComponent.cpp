#include "Component/CHealthComponent.h"
#include "ProjectGlobal.h"

#include "Type/CHealthStructure.h"

UCHealthComponent::UCHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = Cast<AActor>(GetOwner());
	check(OwnerActor_Cached);

	InitializeHealth(InitMaxHP, InitCurrentHP, MaxHPUpdatePolicy);
}

void UCHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCHealthComponent::InitializeHealth(float InInitMaxHP, float InInitCurrentHP, EMaxHPUpdatePolicy InUpdatePolicy)
{
	if (InInitMaxHP <= 0.f)
	{
		MaxHP = 0.f;
		PreviousHP = 0.f;
		CurrentHP = 0.f;
		DeadState = EDeadState::Dead;

		return;
	}

	MaxHP = InInitMaxHP;

	if (InUpdatePolicy == EMaxHPUpdatePolicy::FillToMax)
	{
		PreviousHP = MaxHP;
		CurrentHP = MaxHP;
	}
	else
	{
		float clampedHP = FMath::Clamp(InInitCurrentHP, 0.f, MaxHP);

		PreviousHP = clampedHP;
		CurrentHP = clampedHP;
	}
	
	DeadState = (CurrentHP > 0.f) ? EDeadState::Alive : EDeadState::Dead;
}

bool UCHealthComponent::TryKill()
{
	if (!CanKill()) return false;

	PreviousHP = CurrentHP;
	CurrentHP = 0.f;
	DeadState = EDeadState::Dying;

	// TODO: `FOnKill` Delegate Broadcast

	return true;
}

bool UCHealthComponent::TryRevive(float InReviveHP)
{
	if (!CanRevive()) return false;
	if (MaxHP <= 0.f) return false;

	const float reviveHP = FMath::Clamp(InReviveHP, 1.f, MaxHP);

	PreviousHP = CurrentHP;
	CurrentHP = reviveHP;
	DeadState = EDeadState::Reviving;

	// TODO: `FOnRevive` Delegate Broadcast

	return true;
}

bool UCHealthComponent::TryCancelRevive()
{
	if (DeadState != EDeadState::Reviving) return false;

	PreviousHP = CurrentHP;
	CurrentHP = 0.f;
	DeadState = EDeadState::Dead;

	// TODO: `FOnCancelRevive` Delegate Broadcast

	return true;
}

bool UCHealthComponent::TryUpdateMaxHP(float InNewMaxHP, EMaxHPUpdatePolicy InUpdatePolicy)
{
	if (DeadState != EDeadState::Alive) return false;
	if (InNewMaxHP <= 0.f) return false;

	MaxHP = InNewMaxHP;
	PreviousHP = CurrentHP;

	if (InUpdatePolicy == EMaxHPUpdatePolicy::FillToMax)
	{
		CurrentHP = MaxHP;
	}
	else
	{
		CurrentHP = FMath::Clamp(CurrentHP, 1.f, MaxHP);
	}

	// TODO: `FOnMaxHealthChanged` Delegate BroadCast

	return true;
}

float UCHealthComponent::TakeDamage(float InTakeDamageAmount)
{
	if (DeadState != EDeadState::Alive) return 0.f;
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

	// PrintTakeDamageContextInfo();

	return takenDamage;
}

float UCHealthComponent::TakeHeal(float InTakeHealAmount)
{
	if (DeadState != EDeadState::Alive) return 0.f;
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

	// PrintTakeHealContextInfo();

	return takenHeal;
}

bool UCHealthComponent::CanKill() const
{
	return DeadState == EDeadState::Alive;
}

bool UCHealthComponent::CanRevive() const
{
	return DeadState == EDeadState::Dead;
}

void UCHealthComponent::EnterDeadState()
{
	DeadState = EDeadState::Dead;
}

void UCHealthComponent::EnterAliveState()
{
	DeadState = EDeadState::Alive;
}

void UCHealthComponent::UpdateDeadState()
{
	bool bDeadFlag = (CurrentHP <= 0.f);

	// Revive is an explicit gameplay transition handled by SetRevive().
	if (DeadState == EDeadState::Alive && bDeadFlag)
	{
		DeadState = EDeadState::Dying;

		// TODO: `FOnDeadState` Delegate Broadcast
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
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DeadState"), *UEnum::GetValueAsString(DeadState)));
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
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DeadState"), *UEnum::GetValueAsString(DeadState)));
	FLog::Log(TEXT("---------------------------------"));
}

