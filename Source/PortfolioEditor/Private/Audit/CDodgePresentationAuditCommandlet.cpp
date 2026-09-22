#include "Audit/CDodgePresentationAuditCommandlet.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Components/MeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialInstance.h"
#include "UObject/UnrealType.h"
#include "GameFramework/Actor.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/TimelineTemplate.h"
#include "NiagaraSystem.h"

UCDodgePresentationAuditCommandlet::UCDodgePresentationAuditCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UCDodgePresentationAuditCommandlet::Main(const FString& Params)
{
	UMaterial* material = LoadObject<UMaterial>(nullptr, TEXT("/Game/CharacterOverlay/Materials/M_Fresnel_D"));
	if (!material) return 1;
	UE_LOG(LogTemp, Display, TEXT("PresentationAudit Material=%s Blend=%d"), *material->GetPathName(), int32(material->BlendMode));
	const auto* outputs = material->GetEditorOnlyData();
	UE_LOG(LogTemp, Display, TEXT("Outputs Emissive=%s Opacity=%s"), *GetNameSafe(outputs->EmissiveColor.Expression), *GetNameSafe(outputs->Opacity.Expression));
	UMaterialInstance* instance = LoadObject<UMaterialInstance>(nullptr, TEXT("/Game/CharacterOverlay/Materials/Material_Instance/MI_Fresnel"));
	if (!instance) return 1;
	for (const auto& scalar : instance->ScalarParameterValues)
		UE_LOG(LogTemp, Display, TEXT("MI Scalar %s=%f"), *scalar.ParameterInfo.Name.ToString(), scalar.ParameterValue);
	for (const auto& vector : instance->VectorParameterValues)
		UE_LOG(LogTemp, Display, TEXT("MI Vector %s=%s"), *vector.ParameterInfo.Name.ToString(), *vector.ParameterValue.ToString());
	for (UMaterialExpression* expression : material->GetExpressionCollection().Expressions)
	{
		if (!expression) continue;
		UE_LOG(LogTemp, Display, TEXT("Expression %s %s"), *expression->GetName(), *expression->GetClass()->GetName());
		for (TFieldIterator<FProperty> it(expression->GetClass()); it; ++it)
		{
			if (it->GetOwnerClass() == UMaterialExpression::StaticClass() || it->GetOwnerClass() == UObject::StaticClass()) continue;
			FString value;
			it->ExportText_InContainer(0, value, expression, expression, expression, PPF_None);
			if (!value.IsEmpty()) UE_LOG(LogTemp, Display, TEXT("  %s=%s"), *it->GetName(), *value);
		}
	}
	for (const TCHAR* path : {
		TEXT("/Game/01_Character/01_Player/BP_CPlayer_Stellar"),
		TEXT("/Game/01_Character/02_Enemy/BP_CEnemy"),
		TEXT("/Game/06_Weapon/BP_CWeaponActor_Sword"),
		TEXT("/Game/06_Weapon/BP_CWeaponActor_SciFi_Sword") })
	{
		UBlueprint* bp = LoadObject<UBlueprint>(nullptr, path);
		if (!bp || !bp->SimpleConstructionScript) return 1;
		UE_LOG(LogTemp, Display, TEXT("Blueprint %s"), path);
		TArray<UEdGraph*> graphs;
		bp->GetAllGraphs(graphs);
		for (const UEdGraph* graph : graphs)
		{
			UE_LOG(LogTemp, Display, TEXT("Graph %s"), *graph->GetName());
			for (const UEdGraphNode* node : graph->Nodes)
			{
				if (!node) continue;
				UE_LOG(LogTemp, Display, TEXT("Node %s Class=%s Title=%s"), *node->GetName(),
					*node->GetClass()->GetName(), *node->GetNodeTitle(ENodeTitleType::ListView).ToString().Replace(TEXT("\n"), TEXT(" ")));
				for (const UEdGraphPin* pin : node->Pins)
				{
					if (!pin) continue;
					UE_LOG(LogTemp, Display, TEXT(" Pin %s Dir=%d Default=%s Object=%s"), *pin->PinName.ToString(), int32(pin->Direction),
						*pin->DefaultValue, *GetPathNameSafe(pin->DefaultObject));
					for (const UEdGraphPin* link : pin->LinkedTo)
						if (link) UE_LOG(LogTemp, Display, TEXT("  Link %s.%s"), *GetNameSafe(link->GetOwningNode()), *link->PinName.ToString());
				}
			}
		}
		for (UTimelineTemplate* timeline : bp->Timelines)
		{
			if (!timeline) continue;
			UE_LOG(LogTemp, Display, TEXT("Timeline %s Length=%f Loop=%d"), *timeline->GetName(), timeline->TimelineLength, timeline->bLoop);
			for (TFieldIterator<FProperty> it(timeline->GetClass()); it; ++it)
			{
				if (!it->GetName().Contains(TEXT("Tracks"))) continue;
				FString value;
				it->ExportText_InContainer(0, value, timeline, timeline, timeline, PPF_None);
				UE_LOG(LogTemp, Display, TEXT(" TimelineTracks %s=%s"), *it->GetName(), *value);
			}
		}
		if (const AActor* cdo = bp->GeneratedClass ? Cast<AActor>(bp->GeneratedClass->GetDefaultObject()) : nullptr)
		{
			TArray<UMeshComponent*> meshes;
			cdo->GetComponents(meshes);
			for (UMeshComponent* mesh : meshes)
			{
				UE_LOG(LogTemp, Display, TEXT("  Native Mesh=%s Visible=%d"), *mesh->GetName(), mesh->IsVisible());
				if (auto* skinned = Cast<USkinnedMeshComponent>(mesh))
					UE_LOG(LogTemp, Display, TEXT("  Asset=%s"), *GetPathNameSafe(skinned->GetSkinnedAsset()));
			}
		}
		for (USCS_Node* node : bp->SimpleConstructionScript->GetAllNodes())
		{
			UMeshComponent* mesh = Cast<UMeshComponent>(node->ComponentTemplate);
			if (!mesh) continue;
			UE_LOG(LogTemp, Display, TEXT("  Mesh=%s Template=%s Visible=%d Tags=%d"), *node->GetVariableName().ToString(), *mesh->GetName(), mesh->IsVisible(), mesh->ComponentTags.Num());
			if (auto* skinned = Cast<USkinnedMeshComponent>(mesh))
				UE_LOG(LogTemp, Display, TEXT("  Asset=%s"), *GetPathNameSafe(skinned->GetSkinnedAsset()));
			for (int32 i = 0; i < mesh->GetNumMaterials(); ++i)
				UE_LOG(LogTemp, Display, TEXT("  Material[%d]=%s"), i, *GetPathNameSafe(mesh->GetMaterial(i)));
		}
	}
	for (const TCHAR* path : {
		TEXT("/Game/07_FX/VFX/SwordTrailVFX/Niagara/NS_Trail_Tech"),
		TEXT("/Game/07_FX/VFX/SwordTrailVFX/Niagara/NS_Trail_Water"),
		TEXT("/Game/07_FX/VFX/CombatVFX/HealPositive/NS/Vfx_SpeedUp") })
	{
		UNiagaraSystem* system = LoadObject<UNiagaraSystem>(nullptr, path);
		if (!system) return 1;
		UE_LOG(LogTemp, Display, TEXT("Niagara %s"), path);
		TArray<FNiagaraVariable> parameters;
		system->GetExposedParameters().GetParameters(parameters);
		for (const FNiagaraVariable& parameter : parameters)
			UE_LOG(LogTemp, Display, TEXT(" UserParameter %s Type=%s"), *parameter.GetName().ToString(), *parameter.GetType().GetName());
	}
	return 0;
}
