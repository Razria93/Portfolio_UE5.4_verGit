#include "Type/CCombatHitTypes.h"

bool FOverlapContext::IsValidMinimal() const
{
	return IsValid(OwnerActor) && IsValid(DamageCauser) && IsValid(OtherActor);
}
