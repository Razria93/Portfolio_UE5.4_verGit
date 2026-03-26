#include "Component/CHealthComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

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

	if (bFillToMaxHP)
	{
		PreviousHP = CurrentHP;
		CurrentHP = MaxHP;
	}

	// TODO: `FOnMaxHealthChanged` Delegate BroadCast

	UpdateDeadState();
}

void UCHealthComponent::SetCurrentHP(float InNewCurrentHP)
{
	if (InNewCurrentHP < 0.f) return;

	PreviousHP = CurrentHP;
	CurrentHP = FMath::Clamp(InNewCurrentHP, 0.f, MaxHP);

	// TODO: `FOnHealthChanged` Delegate BroadCast

	UpdateDeadState();
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

bool UCHealthComponent::TryKill()
{
	if (!CanKill()) return false;

	PreviousHP = CurrentHP;
	CurrentHP = 0.f;
	DeadState = EDeadState::Dying;

	// TODO: `FOnDead` Delegate Broadcast

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

	// TODO: `FOnReviveStarted` Delegate Broadcast

	return true;
}

void UCHealthComponent::CancelRevive()
{
	if (DeadState != EDeadState::Reviving) return;

	PreviousHP = CurrentHP;
	CurrentHP = 0.f;
	DeadState = EDeadState::Dead;
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
		// TODO: `FOnDead` Delegate Broadcast
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

