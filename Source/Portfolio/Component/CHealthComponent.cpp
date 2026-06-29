#include "Component/CHealthComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Type/CHealthStructure.h"

UCHealthComponent::UCHealthComponent()
{
}

void UCHealthComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	ValidateRequiredComponentReferences();

	InitializeHealth(InitMaxHP, InitCurrentHP, MaxHPUpdatePolicy);
}

bool UCHealthComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	struct FRequiredComponentReference
	{
		const UObject* Object = nullptr;
		const TCHAR* Label = TEXT("");
	};

	const FRequiredComponentReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
	};

	for (const FRequiredComponentReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

void UCHealthComponent::InitializeHealth(float InInitMaxHP, float InInitCurrentHP, EMaxHPUpdatePolicy InUpdatePolicy)
{
	if (InInitMaxHP <= 0.f)
	{
		MaxHP = 0.f;
		PreviousHP = 0.f;
		CurrentHP = 0.f;
		ChangeDeadState(EDeadState::Dead);

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

	if (CurrentHP > 0.f)
	{
		ChangeDeadState(EDeadState::Alive);
	}
	else
	{
		ChangeDeadState(EDeadState::Dead);
	}
}

bool UCHealthComponent::TryKill()
{
	if (!CanKill()) return false;

	PreviousHP = CurrentHP;
	CurrentHP = 0.f;
	ChangeDeadState(EDeadState::Dying);

	return true;
}

bool UCHealthComponent::TryRevive(float InReviveHP)
{
	if (!CanRevive()) return false;
	if (MaxHP <= 0.f) return false;

	const float reviveHP = FMath::Clamp(InReviveHP, 1.f, MaxHP);

	PreviousHP = CurrentHP;
	CurrentHP = reviveHP;
	ChangeDeadState(EDeadState::Reviving);

	return true;
}

bool UCHealthComponent::TryCancelRevive()
{
	if (DeadState != EDeadState::Reviving) return false;

	PreviousHP = CurrentHP;
	CurrentHP = 0.f;
	ChangeDeadState(EDeadState::Dead);

	return true;
}

bool UCHealthComponent::TryUpdateMaxHP(float InNewMaxHP, EMaxHPUpdatePolicy InUpdatePolicy)
{
	if (!IsAlive()) return false;
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
	if (!IsAlive()) return 0.f;
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
	if (!IsAlive()) return 0.f;
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

bool UCHealthComponent::IsAlive() const
{
	return DeadState == EDeadState::Alive;
}

bool UCHealthComponent::IsDead() const
{
	return DeadState == EDeadState::Dead;
}

bool UCHealthComponent::CanKill() const
{
	return IsAlive();
}

bool UCHealthComponent::CanRevive() const
{
	return IsDead();
}

void UCHealthComponent::EnterDeadState()
{
	ChangeDeadState(EDeadState::Dead);
}

void UCHealthComponent::EnterAliveState()
{
	ChangeDeadState(EDeadState::Alive);
}

void UCHealthComponent::UpdateDeadState()
{
	bool bDeadFlag = (CurrentHP <= 0.f);

	// Revive is an explicit gameplay transition handled by SetRevive().
	if (DeadState == EDeadState::Alive && bDeadFlag)
	{
		ChangeDeadState(EDeadState::Dying);
	}
}

void UCHealthComponent::ChangeDeadState(EDeadState InNewDeadState)
{
	if (DeadState == InNewDeadState) return;

	const EDeadState prevDeadState = DeadState;
	DeadState = InNewDeadState;

	if (OnDeadStateChanged.IsBound())
	{
		OnDeadStateChanged.Broadcast(prevDeadState, DeadState);
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

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerCharacter"), *GetNameSafe(OwnerCharacter_Injected)));
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

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerCharacter"), *GetNameSafe(OwnerCharacter_Injected)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DeadState"), *UEnum::GetValueAsString(DeadState)));
	FLog::Log(TEXT("---------------------------------"));
}

