#include "Component/CWeaponTrailComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAsset/CWeaponTrailDataAsset.h"

void UWeaponTrailComponent::ConfigureAttachment(USkeletalMeshComponent* InMesh, const FWeaponTrailDefinition& InDefinition)
{
	WeaponMesh = InMesh;
	bRequiresSocket = InDefinition.bAttachToSocket;
	TrailSocket = bRequiresSocket ? InDefinition.AttachmentSocket : NAME_None;
	SetAbsolute(false, false, false);
	if (!InMesh) return;
	AttachToComponent(InMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TrailSocket);
	SetRelativeLocationAndRotation(InDefinition.RelativeLocation, InDefinition.RelativeRotation);
	SetRelativeScale3D(FVector::OneVector);
	if (InDefinition.bOverrideRibbonWidth && !InDefinition.RibbonWidthParameter.IsNone())
		SetVariableFloat(InDefinition.RibbonWidthParameter, InDefinition.RibbonWidth);
	// Leave Niagara's authored Position Offset untouched.
}

bool UWeaponTrailComponent::IsTrailAttachmentValid() const
{
	const USkeletalMeshComponent* mesh = WeaponMesh.Get();
	return mesh && GetAttachParent() == mesh && GetAttachSocketName() == TrailSocket
		&& (!bRequiresSocket || (!TrailSocket.IsNone() && mesh->DoesSocketExist(TrailSocket)));
}
