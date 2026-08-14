#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CEnemyTargetSelectionTypes.h"
#include "CEnemyTargetSelectionComponent.generated.h"

class UCCombatTargetComponent;
struct FCharacterComponentReferences;
enum class ECombatTargetChangeReason : uint8;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCEnemyTargetSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void InitializeReferences(const FCharacterComponentReferences& InReferences);
	FEnemyTargetSelectionResult RequestSelectCombatTarget(AActor* InCandidate, ECombatTargetChangeReason InReason);
	FEnemyTargetSelectionResult RequestClearCombatTarget(ECombatTargetChangeReason InReason);

private:
	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;
};
