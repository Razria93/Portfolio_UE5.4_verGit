#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "CWeaponTrailComponent.generated.h"

// Scene attachment follows the weapon even when Niagara component ticking is disabled.
UCLASS()
class PORTFOLIO_API UWeaponTrailComponent : public UNiagaraComponent
{
	GENERATED_BODY()
public:
	void ConfigureAttachment(class USkeletalMeshComponent* InMesh, const struct FWeaponTrailDefinition& InDefinition);
	bool IsTrailAttachmentValid() const;
private:
	TWeakObjectPtr<class USkeletalMeshComponent> WeaponMesh;
	FName TrailSocket;
	bool bRequiresSocket = false;
};
