#include "Audit/CWeaponPresentationAuditCommandlet.h"

#include "Algo/Unique.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimTypes.h"
#include "Components/SceneComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "ReferenceSkeleton.h"
#include "UObject/UObjectHash.h"
#include "UObject/UnrealType.h"
#include "Weapon/CWeaponActor.h"

namespace WeaponPresentationAudit
{
	struct FAuditRow
	{
		FString RecordType;
		FString AssetPath;
		FString ObjectClass;
		FString Index;
		FString Track;
		FString StartSeconds;
		FString EndSeconds;
		FString DurationSeconds;
		FString Channel;
		FString ClassName;
		FString Name;
		FString ParentName;
		FString Location;
		FString Rotation;
		FString Scale;
		FString TriggerActionType;
		FString TriggerActionIndex;
		FString SourceSocket;
		FString TargetSocket;
		FString RotationMode;
		FString TargetRotation;
		FString RotationOffset;
		FString LocalAxis;
		FString SignedAngleDegrees;
		FString BlendInDuration;
		FString BlendOutDuration;
		FString BlendExponent;
		FString Severity;
		FString Details;
	};

	struct FNotifyInterval
	{
		int32 NotifyIndex = INDEX_NONE;
		float StartSeconds = 0.f;
		float EndSeconds = 0.f;
		FString Channel;
		FString ClassName;
	};

	const TCHAR* CsvHeader =
		TEXT("RecordType,AssetPath,ObjectClass,Index,Track,StartSeconds,EndSeconds,DurationSeconds,Channel,ClassName,Name,ParentName,Location,Rotation,Scale,TriggerActionType,TriggerActionIndex,SourceSocket,TargetSocket,RotationMode,TargetRotation,RotationOffset,LocalAxis,SignedAngleDegrees,BlendInDuration,BlendOutDuration,BlendExponent,Severity,Details");

	FString CsvEscape(FString Value)
	{
		Value.ReplaceInline(TEXT("\r"), TEXT(" "));
		Value.ReplaceInline(TEXT("\n"), TEXT(" "));
		Value.ReplaceInline(TEXT("\""), TEXT("\"\""));
		return FString::Printf(TEXT("\"%s\""), *Value);
	}

	FString ToCsv(const FAuditRow& Row)
	{
		const FString Fields[] =
		{
			Row.RecordType,
			Row.AssetPath,
			Row.ObjectClass,
			Row.Index,
			Row.Track,
			Row.StartSeconds,
			Row.EndSeconds,
			Row.DurationSeconds,
			Row.Channel,
			Row.ClassName,
			Row.Name,
			Row.ParentName,
			Row.Location,
			Row.Rotation,
			Row.Scale,
			Row.TriggerActionType,
			Row.TriggerActionIndex,
			Row.SourceSocket,
			Row.TargetSocket,
			Row.RotationMode,
			Row.TargetRotation,
			Row.RotationOffset,
			Row.LocalAxis,
			Row.SignedAngleDegrees,
			Row.BlendInDuration,
			Row.BlendOutDuration,
			Row.BlendExponent,
			Row.Severity,
			Row.Details,
		};

		TArray<FString> EscapedFields;
		EscapedFields.Reserve(UE_ARRAY_COUNT(Fields));
		for (const FString& Field : Fields)
		{
			EscapedFields.Add(CsvEscape(Field));
		}
		return FString::Join(EscapedFields, TEXT(","));
	}

	void AddRow(TArray<FString>& InOutCsvLines, const FAuditRow& Row)
	{
		InOutCsvLines.Add(ToCsv(Row));
	}

	FString ExportProperty(const UObject* Object, const FName PropertyName)
	{
		if (!IsValid(Object)) return FString();

		const FProperty* Property = Object->GetClass()->FindPropertyByName(PropertyName);
		if (Property == nullptr) return FString();

		const void* ValueAddress = Property->ContainerPtrToValuePtr<void>(Object);
		FString Value;
		Property->ExportTextItem_Direct(Value, ValueAddress, nullptr, const_cast<UObject*>(Object), PPF_None);
		return Value;
	}

	bool HasProperty(const UObject* Object, const FName PropertyName)
	{
		return IsValid(Object) && Object->GetClass()->FindPropertyByName(PropertyName) != nullptr;
	}

	bool IsWeaponNotifyClass(const FString& ClassName)
	{
		return ClassName.Contains(TEXT("WeaponSocketTransform"))
			|| ClassName.Contains(TEXT("SocketTransformTransition"))
			|| ClassName.Contains(TEXT("WeaponPivotRotation"))
			|| ClassName == TEXT("CAnimNotify_Equip")
			|| ClassName == TEXT("CAnimNotify_Unequip")
			|| ClassName.Contains(TEXT("WeaponPresentationOverride"));
	}

	FString ResolveChannel(const FString& ClassName)
	{
		if (ClassName.Contains(TEXT("SocketTransform"))) return TEXT("AttachmentTransform");
		if (ClassName.Contains(TEXT("PivotRotationTransition"))) return TEXT("PivotTransition");
		if (ClassName.Contains(TEXT("PivotRotationOverride"))) return TEXT("PivotOverride");
		return TEXT("InstantAttachment");
	}

	bool IsLegacyNotifyClass(const FString& ClassName)
	{
		return ClassName == TEXT("CAnimNotify_Equip")
			|| ClassName == TEXT("CAnimNotify_Unequip")
			|| ClassName.Contains(TEXT("WeaponPresentationOverride"));
	}

	FTransform BuildBoneComponentTransform(const FReferenceSkeleton& ReferenceSkeleton, int32 BoneIndex)
	{
		if (BoneIndex == INDEX_NONE) return FTransform::Identity;

		TArray<int32> BoneChain;
		while (BoneIndex != INDEX_NONE)
		{
			BoneChain.Add(BoneIndex);
			BoneIndex = ReferenceSkeleton.GetParentIndex(BoneIndex);
		}

		FTransform ComponentTransform = FTransform::Identity;
		for (int32 ChainIndex = BoneChain.Num() - 1; ChainIndex >= 0; --ChainIndex)
		{
			ComponentTransform = ReferenceSkeleton.GetRefBonePose()[BoneChain[ChainIndex]] * ComponentTransform;
		}
		return ComponentTransform;
	}

	void AuditSocket(
		TArray<FString>& InOutCsvLines,
		const FString& AssetPath,
		const FString& ObjectClass,
		const USkeletalMeshSocket* Socket,
		const FReferenceSkeleton& ReferenceSkeleton)
	{
		if (!IsValid(Socket)) return;

		const FTransform SocketLocal(Socket->RelativeRotation, Socket->RelativeLocation, Socket->RelativeScale);
		const int32 BoneIndex = ReferenceSkeleton.FindBoneIndex(Socket->BoneName);
		const FTransform SocketComponent = SocketLocal * BuildBoneComponentTransform(ReferenceSkeleton, BoneIndex);

		FAuditRow Row;
		Row.RecordType = TEXT("Socket");
		Row.AssetPath = AssetPath;
		Row.ObjectClass = ObjectClass;
		Row.Name = Socket->SocketName.ToString();
		Row.ParentName = Socket->BoneName.ToString();
		Row.Location = Socket->RelativeLocation.ToString();
		Row.Rotation = Socket->RelativeRotation.ToString();
		Row.Scale = Socket->RelativeScale.ToString();
		Row.Details = FString::Printf(
			TEXT("BoneIndex=%d;ComponentTransform=%s"),
			BoneIndex,
			*SocketComponent.ToString());
		AddRow(InOutCsvLines, Row);

		if (Socket->SocketName == TEXT("HandGrip")
			&& !SocketComponent.GetScale3D().Equals(FVector::OneVector, KINDA_SMALL_NUMBER))
		{
			FAuditRow Issue = Row;
			Issue.RecordType = TEXT("Issue");
			Issue.Severity = TEXT("Error");
			Issue.Details = FString::Printf(
				TEXT("HandGrip effective component-space scale is not unit: %s"),
				*SocketComponent.GetScale3D().ToString());
			AddRow(InOutCsvLines, Issue);
		}
	}

	void AuditMontage(TArray<FString>& InOutCsvLines, UAnimMontage* Montage)
	{
		if (!IsValid(Montage)) return;

		TArray<FNotifyInterval> Intervals;
		const FString AssetPath = Montage->GetPathName();

		for (int32 NotifyIndex = 0; NotifyIndex < Montage->Notifies.Num(); ++NotifyIndex)
		{
			const FAnimNotifyEvent& NotifyEvent = Montage->Notifies[NotifyIndex];
			UObject* NotifyObject = IsValid(NotifyEvent.NotifyStateClass)
				? static_cast<UObject*>(NotifyEvent.NotifyStateClass.Get())
				: static_cast<UObject*>(NotifyEvent.Notify.Get());
			if (!IsValid(NotifyObject)) continue;

			const FString ClassName = NotifyObject->GetClass()->GetName();
			if (!IsWeaponNotifyClass(ClassName)) continue;

			const float StartSeconds = NotifyEvent.GetTriggerTime();
			const float EndSeconds = IsValid(NotifyEvent.NotifyStateClass)
				? NotifyEvent.GetEndTriggerTime()
				: StartSeconds;
			const FString Channel = ResolveChannel(ClassName);

			FAuditRow Row;
			Row.RecordType = TEXT("MontageNotify");
			Row.AssetPath = AssetPath;
			Row.ObjectClass = Montage->GetClass()->GetName();
			Row.Index = FString::FromInt(NotifyIndex);
			Row.Track = FString::FromInt(NotifyEvent.TrackIndex);
			Row.StartSeconds = FString::SanitizeFloat(StartSeconds);
			Row.EndSeconds = FString::SanitizeFloat(EndSeconds);
			Row.DurationSeconds = FString::SanitizeFloat(FMath::Max(0.f, EndSeconds - StartSeconds));
			Row.Channel = Channel;
			Row.ClassName = ClassName;
			Row.Name = NotifyEvent.NotifyName.ToString();
			Row.TriggerActionType = ExportProperty(NotifyObject, TEXT("TriggerActionType"));
			Row.TriggerActionIndex = ExportProperty(NotifyObject, TEXT("TriggerActionIndex"));
			Row.SourceSocket = ExportProperty(NotifyObject, TEXT("SourceSocketName"));
			Row.TargetSocket = ExportProperty(NotifyObject, TEXT("TargetSocketName"));
			Row.RotationMode = ExportProperty(NotifyObject, TEXT("RotationMode"));
			Row.TargetRotation = ExportProperty(NotifyObject, TEXT("TargetRotation"));
			Row.RotationOffset = ExportProperty(NotifyObject, TEXT("RotationOffset"));
			Row.LocalAxis = ExportProperty(NotifyObject, TEXT("LocalAxis"));
			Row.SignedAngleDegrees = ExportProperty(NotifyObject, TEXT("SignedAngleDegrees"));
			Row.BlendInDuration = ExportProperty(NotifyObject, TEXT("BlendInDuration"));
			Row.BlendOutDuration = ExportProperty(NotifyObject, TEXT("BlendOutDuration"));
			Row.BlendExponent = ExportProperty(NotifyObject, TEXT("BlendExponent"));
			AddRow(InOutCsvLines, Row);

			if (IsLegacyNotifyClass(ClassName))
			{
				FAuditRow Issue = Row;
				Issue.RecordType = TEXT("Issue");
				Issue.Severity = TEXT("Warning");
				Issue.Details = TEXT("Legacy weapon attachment/presentation notify remains in content.");
				AddRow(InOutCsvLines, Issue);
			}

			if (ClassName.Contains(TEXT("WeaponPivotRotation"))
				&& !HasProperty(NotifyObject, TEXT("TriggerActionType")))
			{
				FAuditRow Issue = Row;
				Issue.RecordType = TEXT("Issue");
				Issue.Severity = TEXT("Warning");
				Issue.Details = TEXT("Pivot notify has no action trigger guard.");
				AddRow(InOutCsvLines, Issue);
			}

			if (IsValid(NotifyEvent.NotifyStateClass))
			{
				FNotifyInterval& Interval = Intervals.AddDefaulted_GetRef();
				Interval.NotifyIndex = NotifyIndex;
				Interval.StartSeconds = StartSeconds;
				Interval.EndSeconds = EndSeconds;
				Interval.Channel = Channel;
				Interval.ClassName = ClassName;
			}
		}

		for (int32 LeftIndex = 0; LeftIndex < Intervals.Num(); ++LeftIndex)
		{
			for (int32 RightIndex = LeftIndex + 1; RightIndex < Intervals.Num(); ++RightIndex)
			{
				const FNotifyInterval& Left = Intervals[LeftIndex];
				const FNotifyInterval& Right = Intervals[RightIndex];
				if (Left.Channel != Right.Channel) continue;

				const float OverlapStart = FMath::Max(Left.StartSeconds, Right.StartSeconds);
				const float OverlapEnd = FMath::Min(Left.EndSeconds, Right.EndSeconds);
				if (OverlapEnd <= OverlapStart + KINDA_SMALL_NUMBER) continue;

				FAuditRow Issue;
				Issue.RecordType = TEXT("Issue");
				Issue.AssetPath = AssetPath;
				Issue.ObjectClass = Montage->GetClass()->GetName();
				Issue.Channel = Left.Channel;
				Issue.Severity = TEXT("Error");
				Issue.Details = FString::Printf(
					TEXT("Exclusive channel overlap: notify %d (%s) and %d (%s), %.6f-%.6f sec."),
					Left.NotifyIndex,
					*Left.ClassName,
					Right.NotifyIndex,
					*Right.ClassName,
					OverlapStart,
					OverlapEnd);
				AddRow(InOutCsvLines, Issue);
			}
		}
	}

	void AuditSkeleton(TArray<FString>& InOutCsvLines, USkeleton* Skeleton)
	{
		if (!IsValid(Skeleton)) return;

		for (const TObjectPtr<USkeletalMeshSocket>& Socket : Skeleton->Sockets)
		{
			AuditSocket(
				InOutCsvLines,
				Skeleton->GetPathName(),
				Skeleton->GetClass()->GetName(),
				Socket.Get(),
				Skeleton->GetReferenceSkeleton());
		}
	}

	void AuditSkeletalMesh(TArray<FString>& InOutCsvLines, USkeletalMesh* SkeletalMesh)
	{
		if (!IsValid(SkeletalMesh)) return;

		for (const USkeletalMeshSocket* Socket : SkeletalMesh->GetMeshOnlySocketList())
		{
			AuditSocket(
				InOutCsvLines,
				SkeletalMesh->GetPathName(),
				SkeletalMesh->GetClass()->GetName(),
				Socket,
				SkeletalMesh->GetRefSkeleton());
		}
	}

	void AuditBlueprint(TArray<FString>& InOutCsvLines, UBlueprint* Blueprint)
	{
		if (!IsValid(Blueprint) || !IsValid(Blueprint->GeneratedClass)) return;

		UObject* ClassDefaultObject = Blueprint->GeneratedClass->GetDefaultObject(false);
		if (!IsValid(ClassDefaultObject)) return;

		const FName RelevantProperties[] =
		{
			TEXT("SocketName_Hand"),
			TEXT("SocketName_Holster"),
			TEXT("PivotSocketName"),
		};

		bool bRelevant = false;
		for (const FName PropertyName : RelevantProperties)
		{
			bRelevant |= HasProperty(ClassDefaultObject, PropertyName);
		}
		if (!bRelevant) return;

		FAuditRow Row;
		Row.RecordType = TEXT("WeaponBlueprint");
		Row.AssetPath = Blueprint->GetPathName();
		Row.ObjectClass = Blueprint->GeneratedClass->GetName();
		Row.SourceSocket = ExportProperty(ClassDefaultObject, TEXT("SocketName_Holster"));
		Row.TargetSocket = ExportProperty(ClassDefaultObject, TEXT("SocketName_Hand"));
		Row.Details = FString::Printf(
			TEXT("PivotSocketName=%s"),
			*ExportProperty(ClassDefaultObject, TEXT("PivotSocketName")));
		AddRow(InOutCsvLines, Row);

		if (const ACWeaponActor* WeaponActor = Cast<ACWeaponActor>(ClassDefaultObject))
		{
			if (const USceneComponent* ActorRoot = WeaponActor->GetRootComponent();
				IsValid(ActorRoot) && !ActorRoot->GetRelativeTransform().Equals(FTransform::Identity))
			{
				FAuditRow Issue = Row;
				Issue.RecordType = TEXT("Issue");
				Issue.Severity = TEXT("Error");
				Issue.Details = FString::Printf(
					TEXT("ActorRootComponent relative transform must be Identity. Current=%s"),
					*ActorRoot->GetRelativeTransform().ToHumanReadableString());
				AddRow(InOutCsvLines, Issue);
			}

			if (const USkeletalMeshComponent* WeaponMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>();
				IsValid(WeaponMesh) && WeaponMesh->GetRelativeTransform().ContainsNaN())
			{
				FAuditRow Issue = Row;
				Issue.RecordType = TEXT("Issue");
				Issue.Severity = TEXT("Error");
				Issue.Details = FString::Printf(
					TEXT("WeaponMeshComponent relative transform contains NaN. Current=%s"),
					*WeaponMesh->GetRelativeTransform().ToHumanReadableString());
				AddRow(InOutCsvLines, Issue);
			}
		}

		if (!IsValid(Blueprint->SimpleConstructionScript)) return;

		for (const USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (!IsValid(Node)) continue;

			FAuditRow ComponentRow;
			ComponentRow.RecordType = TEXT("BlueprintComponent");
			ComponentRow.AssetPath = Blueprint->GetPathName();
			ComponentRow.ObjectClass = GetNameSafe(Node->ComponentClass);
			ComponentRow.Name = Node->GetVariableName().ToString();
			ComponentRow.ParentName = Node->ParentComponentOrVariableName.ToString();
			ComponentRow.Details = FString::Printf(TEXT("AttachTo=%s"), *Node->AttachToName.ToString());

			if (const USceneComponent* SceneTemplate = Cast<USceneComponent>(Node->ComponentTemplate))
			{
				ComponentRow.Location = SceneTemplate->GetRelativeLocation().ToString();
				ComponentRow.Rotation = SceneTemplate->GetRelativeRotation().ToString();
				ComponentRow.Scale = SceneTemplate->GetRelativeScale3D().ToString();
			}
			AddRow(InOutCsvLines, ComponentRow);
		}
	}

	bool IsPathUnderRoot(const FString& PackageFilename, const TArray<FString>& RootDirectories)
	{
		for (const FString& RootDirectory : RootDirectories)
		{
			if (PackageFilename.StartsWith(RootDirectory, ESearchCase::IgnoreCase)) return true;
		}
		return false;
	}
}

UCWeaponPresentationAuditCommandlet::UCWeaponPresentationAuditCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UCWeaponPresentationAuditCommandlet::Main(const FString& Params)
{
	using namespace WeaponPresentationAudit;

	FString RootsParameter;
	FParse::Value(*Params, TEXT("Roots="), RootsParameter);
	if (RootsParameter.IsEmpty())
	{
		RootsParameter = TEXT("/Game");
	}

	TArray<FString> LongPackageRoots;
	RootsParameter.ParseIntoArray(LongPackageRoots, TEXT(","), true);
	if (LongPackageRoots.IsEmpty())
	{
		LongPackageRoots.Add(TEXT("/Game"));
	}

	TArray<FString> RootDirectories;
	for (FString& LongPackageRoot : LongPackageRoots)
	{
		LongPackageRoot.TrimStartAndEndInline();
		if (!FPackageName::IsValidLongPackageName(LongPackageRoot))
		{
			UE_LOG(LogTemp, Error, TEXT("[WeaponPresentationAudit] Invalid root: %s"), *LongPackageRoot);
			return 2;
		}

		FString RootDirectory = FPackageName::LongPackageNameToFilename(LongPackageRoot);
		FPaths::NormalizeDirectoryName(RootDirectory);
		RootDirectories.Add(RootDirectory);
	}

	TArray<FString> PackageFiles;
	for (const FString& RootDirectory : RootDirectories)
	{
		IFileManager::Get().FindFilesRecursive(PackageFiles, *RootDirectory, TEXT("*.uasset"), true, false, false);
	}
	PackageFiles.Sort();
	PackageFiles.SetNum(Algo::Unique(PackageFiles));

	TArray<FString> CsvLines;
	CsvLines.Add(CsvHeader);

	int32 LoadedPackageCount = 0;
	int32 FailedPackageCount = 0;
	for (const FString& PackageFilename : PackageFiles)
	{
		if (!IsPathUnderRoot(PackageFilename, RootDirectories)) continue;

		FString LongPackageName;
		if (!FPackageName::TryConvertFilenameToLongPackageName(PackageFilename, LongPackageName))
		{
			++FailedPackageCount;
			UE_LOG(LogTemp, Warning, TEXT("[WeaponPresentationAudit] Cannot resolve package name: %s"), *PackageFilename);
			continue;
		}

		UPackage* Package = LoadPackage(nullptr, *LongPackageName, LOAD_NoWarn | LOAD_Quiet);
		if (!IsValid(Package))
		{
			++FailedPackageCount;
			UE_LOG(LogTemp, Warning, TEXT("[WeaponPresentationAudit] Cannot load package: %s"), *LongPackageName);
			continue;
		}

		++LoadedPackageCount;
		ForEachObjectWithPackage(Package, [&CsvLines](UObject* Object)
		{
			if (UAnimMontage* Montage = Cast<UAnimMontage>(Object))
			{
				AuditMontage(CsvLines, Montage);
			}
			else if (USkeleton* Skeleton = Cast<USkeleton>(Object))
			{
				AuditSkeleton(CsvLines, Skeleton);
			}
			else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Object))
			{
				AuditSkeletalMesh(CsvLines, SkeletalMesh);
			}
			else if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
			{
				AuditBlueprint(CsvLines, Blueprint);
			}
			return true;
		});
	}

	FAuditRow Summary;
	Summary.RecordType = TEXT("Summary");
	Summary.Severity = FailedPackageCount == 0 ? TEXT("Info") : TEXT("Warning");
	Summary.Details = FString::Printf(
		TEXT("Roots=%s;DiscoveredPackages=%d;LoadedPackages=%d;FailedPackages=%d;DataRows=%d"),
		*RootsParameter,
		PackageFiles.Num(),
		LoadedPackageCount,
		FailedPackageCount,
		FMath::Max(0, CsvLines.Num() - 1));
	AddRow(CsvLines, Summary);

	for (const FString& CsvLine : CsvLines)
	{
		UE_LOG(LogTemp, Display, TEXT("[WeaponPresentationAuditCSV] %s"), *CsvLine);
	}

	FString OutputParameter;
	if (FParse::Value(*Params, TEXT("Output="), OutputParameter) && !OutputParameter.IsEmpty())
	{
		FString OutputFilename = OutputParameter;
		if (FPaths::IsRelative(OutputFilename))
		{
			OutputFilename = FPaths::Combine(FPaths::ProjectSavedDir(), OutputFilename);
		}
		OutputFilename = FPaths::ConvertRelativePathToFull(OutputFilename);

		IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputFilename), true);
		const FString CsvText = FString::Join(CsvLines, LINE_TERMINATOR) + LINE_TERMINATOR;
		if (!FFileHelper::SaveStringToFile(CsvText, *OutputFilename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			UE_LOG(LogTemp, Error, TEXT("[WeaponPresentationAudit] Cannot write output: %s"), *OutputFilename);
			return 3;
		}
		UE_LOG(LogTemp, Display, TEXT("[WeaponPresentationAudit] Wrote report: %s"), *OutputFilename);
	}

	bool bFailOnIssues = false;
	FParse::Bool(*Params, TEXT("FailOnIssues="), bFailOnIssues);
	if (bFailOnIssues)
	{
		for (const FString& CsvLine : CsvLines)
		{
			if (CsvLine.Contains(TEXT("\"Issue\""))) return 1;
		}
	}

	return FailedPackageCount == 0 ? 0 : 1;
}
