#include "Audit/CExecutionMontageAuditCommandlet.h"
#include "Audit/MontageSectionPath.h"

#include "Algo/Unique.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Action/CAction.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Reaction/CReaction.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"
#include "UObject/UnrealType.h"

namespace ExecutionMontageAudit
{
	enum class EExecutionDomain : uint8
	{
		Action,
		Reaction,
	};

	struct FMontageNotifySummary
	{
		int32 CorrectCompleteCount = 0;
		int32 TriggerMismatchCount = 0;
		int32 OppositeCompleteCount = 0;
		float LastCorrectCompleteTime = -1.f;
		float LastNotifyStateEndTime = -1.f;
		TArray<FString> NotifyStateClasses;
	};

	struct FAuditRow
	{
		FString RecordType;
		FString BlueprintPath;
		FString ComponentClass;
		FString Domain;
		FString DataKey;
		FString ExecutorClass;
		FString MontagePath;
		FString CorrectCompleteCount;
		FString OppositeCompleteCount;
		FString LastCompleteTime;
		FString LastNotifyStateEndTime;
		FString NotifyStateClasses;
		FString Severity;
		FString Details;
	};

	struct FMontageUsage
	{
		TSet<FString> Domains;
		TArray<FString> DataReferences;
	};

	const TCHAR* CsvHeader =
		TEXT("RecordType,BlueprintPath,ComponentClass,Domain,DataKey,ExecutorClass,MontagePath,CorrectCompleteCount,OppositeCompleteCount,LastCompleteTime,LastNotifyStateEndTime,NotifyStateClasses,Severity,Details");

	void AddRow(TArray<FString>& InOutCsvLines, const FAuditRow& Row);

	FString CsvEscape(FString Value)
	{
		Value.ReplaceInline(TEXT("\r"), TEXT(" "));
		Value.ReplaceInline(TEXT("\n"), TEXT(" "));
		Value.ReplaceInline(TEXT("\""), TEXT("\"\""));
		return FString::Printf(TEXT("\"%s\""), *Value);
	}

	FString JoinSorted(const TSet<FString>& Values)
	{
		TArray<FString> SortedValues = Values.Array();
		SortedValues.Sort();
		return FString::Join(SortedValues, TEXT("|"));
	}

	FString BuildNotifyConfig(const UObject* NotifyObject)
	{
		if (!IsValid(NotifyObject)) return FString();

		TArray<FString> Settings;
		for (TFieldIterator<FProperty> It(NotifyObject->GetClass(), EFieldIterationFlags::IncludeSuper); It; ++It)
		{
			const FProperty* Property = *It;
			if (!Property->HasAnyPropertyFlags(CPF_Edit)
				|| !Property->GetOwnerStruct()->GetPathName().StartsWith(TEXT("/Script/Portfolio")))
			{
				continue;
			}

			FString Value;
			const void* ValueAddress = Property->ContainerPtrToValuePtr<void>(NotifyObject);
			Property->ExportTextItem_Direct(Value, ValueAddress, nullptr, const_cast<UObject*>(NotifyObject), PPF_None);
			Settings.Add(FString::Printf(TEXT("%s=%s"), *Property->GetName(), *Value));
		}

		Settings.Sort();
		return FString::Join(Settings, TEXT("; "));
	}

	FString GetNotifyRecommendation(const FString& NotifyClass, const FString& Domains, const bool bIsState)
	{
		if (NotifyClass == TEXT("CAnimNotify_CompleteAction"))
		{
			return TEXT("Action data only: exactly one; place after all required NotifyStates end.");
		}
		if (NotifyClass == TEXT("CAnimNotify_CompleteReaction"))
		{
			return TEXT("Reaction data only: exactly one; place after all required NotifyStates end.");
		}
		if (NotifyClass == TEXT("CAnimNotify_CommitExecution"))
		{
			return TEXT("Source Execution Action only: place on the authoritative outcome/impact frame, before Complete Action.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_EquipSocketTransformTransition"))
		{
			return TEXT("Equip Action only: cover the visual handoff; state end commits the socket transition before Complete Action.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_UnequipSocketTransformTransition"))
		{
			return TEXT("Unequip Action only: cover the visual handoff; state end commits the socket transition before Complete Action.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_WeaponPivotRotationTransition") || NotifyClass == TEXT("CAnimNotifyState_WeaponPivotRotationOverride"))
		{
			return TEXT("Action presentation only: span precisely the authored correction interval; avoid overlapping competing pivot overrides.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_Collision"))
		{
			return TEXT("Action hit window: None enables all weapon collision components; a named CollisionName must resolve. Span only damage-active frames and end before terminal completion.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_HitContext"))
		{
			return TEXT("Action hit window: TriggerActionType/Index must match the owning action; span only the valid hit context.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_ChainWindow"))
		{
			return TEXT("Combo Action only: this state has no WindowKey setting. Open only the intended input-acceptance interval and close before completion.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_ExecutionInterventionWindow"))
		{
			return TEXT("Execution participant window: Action and Reaction are both supported. WindowKey must be non-empty and match an intervention rule; close before terminal completion.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_WeaponSocketTransformTransition"))
		{
			return TEXT("Action presentation only: TargetSocketName must be non-empty; SourceSocketName=None means the current socket. Span the intended handoff interval.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_ActionFeedback") || NotifyClass == TEXT("CAnimNotify_ActionFeedback"))
		{
			return TEXT("Action only: TriggerKey must be non-empty and match a registered feedback consumer.");
		}
		if (NotifyClass == TEXT("CAnimNotifyState_ReactionFeedback") || NotifyClass == TEXT("CAnimNotify_ReactionFeedback"))
		{
			return TEXT("Reaction only: TriggerKey must be non-empty and match a registered feedback consumer.");
		}
		if (NotifyClass == TEXT("CAnimNotify_AllowGuardStart") || NotifyClass == TEXT("CAnimNotify_SwitchToGuard"))
		{
			return TEXT("Guard Action only: place at the intended guard-control handoff, before Complete Action.");
		}
		if (NotifyClass == TEXT("CAnimNotify_AdvanceCombo"))
		{
			return TEXT("Combo Action only: place at the intended combo-consumption decision point.");
		}
		if (NotifyClass == TEXT("CAnimNotify_CombatSignalCue"))
		{
			return TEXT("CueTag must be non-empty and correspond to an expected combat-signal consumer.");
		}
		if (NotifyClass == TEXT("CAnimNotify_SetIncapacitatedPresentation"))
		{
			return TEXT("Use for an intentional presentation transition. TargetPresentation=None is valid only when this reaction deliberately clears the current presentation.");
		}
		if (NotifyClass == TEXT("CAnimNotify_ResetBalanceLifecycle"))
		{
			return TEXT("Place only at the authored balance-lifecycle reset point; validate against the owning reaction outcome.");
		}

		return bIsState
			? TEXT("Review state start/end against the visual or gameplay interval; no universal duration is safe without motion review.")
			: TEXT("Review point timing against the authored gameplay or presentation event; no universal timestamp is safe without motion review.");
	}

	void AddMontageUsage(TMap<FString, FMontageUsage>& InOutUsages, const UAnimMontage* Montage, EExecutionDomain Domain, const FString& Reference)
	{
		if (!IsValid(Montage)) return;

		FMontageUsage& Usage = InOutUsages.FindOrAdd(Montage->GetPathName());
		Usage.Domains.Add(Domain == EExecutionDomain::Action ? TEXT("Action") : TEXT("Reaction"));
		Usage.DataReferences.AddUnique(Reference);
	}

	void AddInventoryRow(
		TArray<FString>& InOutCsvLines,
		const UAnimMontage* Montage,
		const FAnimNotifyEvent& Event,
		const FString& Domains,
		const FString& DataReferences)
	{
		const UObject* NotifyObject = IsValid(Event.Notify) ? static_cast<UObject*>(Event.Notify) : static_cast<UObject*>(Event.NotifyStateClass);
		const bool bIsState = IsValid(Event.NotifyStateClass);
		const FString NotifyClass = GetNameSafe(NotifyObject ? NotifyObject->GetClass() : nullptr);
		const FString TrackName = Montage->AnimNotifyTracks.IsValidIndex(Event.TrackIndex)
			? Montage->AnimNotifyTracks[Event.TrackIndex].TrackName.ToString()
			: FString::Printf(TEXT("Track%d"), Event.TrackIndex);

		FAuditRow Row;
		Row.RecordType = TEXT("MontageNotify");
		Row.BlueprintPath = GetPathNameSafe(Montage);
		Row.ComponentClass = TrackName;
		Row.Domain = Domains.IsEmpty() ? TEXT("NoScannedComponentDataReference") : Domains;
		Row.DataKey = DataReferences;
		Row.ExecutorClass = bIsState ? TEXT("NotifyState") : TEXT("Notify");
		Row.MontagePath = FString::SanitizeFloat(Montage->GetPlayLength());
		Row.CorrectCompleteCount = NotifyClass;
		Row.OppositeCompleteCount = Event.GetNotifyEventName().ToString();
		Row.LastCompleteTime = FString::SanitizeFloat(Event.GetTriggerTime());
		Row.LastNotifyStateEndTime = bIsState ? FString::SanitizeFloat(Event.GetEndTriggerTime()) : FString();
		Row.NotifyStateClasses = bIsState ? FString::SanitizeFloat(Event.GetDuration()) : FString();
		Row.Severity = TEXT("Info");
		Row.Details = FString::Printf(TEXT("Config: %s | Recommendation: %s"), *BuildNotifyConfig(NotifyObject), *GetNotifyRecommendation(NotifyClass, Domains, bIsState));
		AddRow(InOutCsvLines, Row);
	}

	void AddRow(TArray<FString>& InOutCsvLines, const FAuditRow& Row)
	{
		const FString Fields[] =
		{
			Row.RecordType,
			Row.BlueprintPath,
			Row.ComponentClass,
			Row.Domain,
			Row.DataKey,
			Row.ExecutorClass,
			Row.MontagePath,
			Row.CorrectCompleteCount,
			Row.OppositeCompleteCount,
			Row.LastCompleteTime,
			Row.LastNotifyStateEndTime,
			Row.NotifyStateClasses,
			Row.Severity,
			Row.Details,
		};

		TArray<FString> EscapedFields;
		EscapedFields.Reserve(UE_ARRAY_COUNT(Fields));
		for (const FString& Field : Fields)
		{
			EscapedFields.Add(CsvEscape(Field));
		}
		InOutCsvLines.Add(FString::Join(EscapedFields, TEXT(",")));
	}

	bool IsPathUnderRoot(const FString& PackageFilename, const TArray<FString>& RootDirectories)
	{
		for (const FString& RootDirectory : RootDirectories)
		{
			if (PackageFilename.StartsWith(RootDirectory, ESearchCase::IgnoreCase)) return true;
		}
		return false;
	}

	FString GetDomainName(EExecutionDomain Domain)
	{
		return Domain == EExecutionDomain::Action ? TEXT("Action") : TEXT("Reaction");
	}

	FString GetExpectedNotifyClassName(EExecutionDomain Domain)
	{
		return Domain == EExecutionDomain::Action
			? TEXT("CAnimNotify_CompleteAction")
			: TEXT("CAnimNotify_CompleteReaction");
	}

	FString GetOppositeNotifyClassName(EExecutionDomain Domain)
	{
		return Domain == EExecutionDomain::Action
			? TEXT("CAnimNotify_CompleteReaction")
			: TEXT("CAnimNotify_CompleteAction");
	}

	bool ReadIntegerProperty(const UObject* Object, const TCHAR* Name, int64& OutValue)
	{
		const FProperty* Property = Object->GetClass()->FindPropertyByName(Name);
		if (!Property) return false;
		const void* Address = Property->ContainerPtrToValuePtr<void>(Object);
		const FNumericProperty* Numeric = CastField<FNumericProperty>(Property);
		if (const FEnumProperty* Enum = CastField<FEnumProperty>(Property)) Numeric = Enum->GetUnderlyingProperty();
		if (!Numeric || !Numeric->IsInteger()) return false;
		OutValue = Numeric->GetSignedIntPropertyValue(Address);
		return true;
	}

	bool MatchesTerminalTrigger(const UObject* Notify, EExecutionDomain Domain, int64 ExpectedType, int32 ExpectedIndex)
	{
		int64 Type = 0;
		const bool bAction = Domain == EExecutionDomain::Action;
		if (!ReadIntegerProperty(Notify, bAction ? TEXT("TriggerActionType") : TEXT("TriggerReactionType"), Type)) return false;
		const int64 All = bAction ? static_cast<int64>(EActionType::All) : static_cast<int64>(EReactionType::All);
		const int64 None = bAction ? static_cast<int64>(EActionType::None) : static_cast<int64>(EReactionType::None);
		const int64 Max = bAction ? static_cast<int64>(EActionType::Max) : static_cast<int64>(EReactionType::Max);
		if (Type == None || Type == Max || (Type != All && Type != ExpectedType)) return false;
		if (!bAction) return true;
		int64 Index = 0;
		return ReadIntegerProperty(Notify, TEXT("TriggerActionIndex"), Index)
			&& (Index == INDEX_NONE || Index == ExpectedIndex);
	}

	FMontageNotifySummary SummarizeMontage(const UAnimMontage* Montage, EExecutionDomain Domain, int64 ExpectedType, int32 ExpectedIndex, const FSectionPath& Path)
	{
		FMontageNotifySummary Summary;
		if (!IsValid(Montage)) return Summary;

		const FString ExpectedClass = GetExpectedNotifyClassName(Domain);
		const FString OppositeClass = GetOppositeNotifyClassName(Domain);

		for (const FAnimNotifyEvent& Event : Montage->Notifies)
		{
			if (!Path.ContainsTime(Montage, Event.GetTriggerTime())) continue;
			if (IsValid(Event.Notify))
			{
				const FString ClassName = Event.Notify->GetClass()->GetName();
				if (ClassName == ExpectedClass)
				{
					++Summary.CorrectCompleteCount;
					if (!MatchesTerminalTrigger(Event.Notify, Domain, ExpectedType, ExpectedIndex)) ++Summary.TriggerMismatchCount;
					Summary.LastCorrectCompleteTime = FMath::Max(Summary.LastCorrectCompleteTime, Event.GetTriggerTime());
				}
				else if (ClassName == OppositeClass)
				{
					++Summary.OppositeCompleteCount;
				}
			}

			if (IsValid(Event.NotifyStateClass))
			{
				Summary.LastNotifyStateEndTime = FMath::Max(Summary.LastNotifyStateEndTime, Event.GetEndTriggerTime());
				Summary.NotifyStateClasses.AddUnique(Event.NotifyStateClass->GetClass()->GetName());
			}
		}

		Summary.NotifyStateClasses.Sort();
		return Summary;
	}

	FString FormatActionDataKey(const FActionDataKey& Key)
	{
		return FString::Printf(TEXT("%s:%d"), *UEnum::GetValueAsString(Key.ActionType), Key.ActionIndex);
	}

	FString FormatReactionDataKey(const FReactionDataKey& Key)
	{
		return FString::Printf(TEXT("%s:%s:%d"),
			*UEnum::GetValueAsString(Key.MatchMode),
			*UEnum::GetValueAsString(Key.ReactionType),
			Key.ReactionIndex);
	}

	void AddMontageAuditRows(
		TArray<FString>& InOutCsvLines,
		int32& InOutErrorCount,
		int32& InOutWarningCount,
		const UBlueprint* Blueprint,
		const UActorComponent* Component,
		EExecutionDomain Domain,
		const FString& DataKey,
		const UClass* ExecutorClass,
		const UAnimMontage* Montage,
		int64 ExpectedType,
		int32 ExpectedIndex,
		FName StartSection,
		float PlayRate)
	{
		FAuditRow Row;
		Row.RecordType = TEXT("ExecutionMontage");
		Row.BlueprintPath = GetPathNameSafe(Blueprint);
		Row.ComponentClass = GetNameSafe(Component ? Component->GetClass() : nullptr);
		Row.Domain = GetDomainName(Domain);
		Row.DataKey = DataKey;
		Row.ExecutorClass = GetPathNameSafe(ExecutorClass);
		Row.MontagePath = GetPathNameSafe(Montage);

		if (!IsValid(Montage))
		{
			Row.Severity = TEXT("Error");
			Row.Details = TEXT("Configured data entry has no valid Montage.");
			AddRow(InOutCsvLines, Row);
			++InOutErrorCount;
			return;
		}

		const FSectionPath Path = BuildSectionPath(Montage, StartSection, PlayRate);
		if (!Path.Error.IsEmpty())
		{
			Row.Severity = TEXT("Error");
			Row.Details = Path.Error;
			AddRow(InOutCsvLines, Row);
			++InOutErrorCount;
			return;
		}
		const FMontageNotifySummary Summary = SummarizeMontage(Montage, Domain, ExpectedType, ExpectedIndex, Path);
		Row.CorrectCompleteCount = FString::FromInt(Summary.CorrectCompleteCount);
		Row.OppositeCompleteCount = FString::FromInt(Summary.OppositeCompleteCount);
		Row.LastCompleteTime = Summary.LastCorrectCompleteTime >= 0.f
			? FString::SanitizeFloat(Summary.LastCorrectCompleteTime)
			: FString();
		Row.LastNotifyStateEndTime = Summary.LastNotifyStateEndTime >= 0.f
			? FString::SanitizeFloat(Summary.LastNotifyStateEndTime)
			: FString();
		Row.NotifyStateClasses = FString::Join(Summary.NotifyStateClasses, TEXT("|"));
		Row.Severity = TEXT("Info");
		Row.Details = TEXT("Direct terminal class and trigger checks passed on the authored section path; runtime overrides and delivery are not verified.");

		if (Summary.CorrectCompleteCount == 0)
		{
			Row.Severity = TEXT("Error");
			Row.Details = FString::Printf(TEXT("Missing required %s."), *GetExpectedNotifyClassName(Domain));
			++InOutErrorCount;
		}
		else if (Summary.OppositeCompleteCount > 0)
		{
			Row.Severity = TEXT("Error");
			Row.Details = FString::Printf(TEXT("Contains %s in %s data."), *GetOppositeNotifyClassName(Domain), *GetDomainName(Domain));
			++InOutErrorCount;
		}
		else if (Summary.TriggerMismatchCount > 0)
		{
			Row.Severity = TEXT("Error");
			Row.Details = TEXT("Complete Notify trigger does not match this data entry, or trigger properties are unreadable.");
			++InOutErrorCount;
		}
		else if (Summary.CorrectCompleteCount > 1)
		{
			Row.Severity = TEXT("Warning");
			Row.Details = TEXT("Multiple matching terminal notifies require Editor review.");
			++InOutWarningCount;
		}
		else if (Path.bLoops || Path.Sections.Num() > 1)
		{
			Row.Severity = TEXT("Warning");
			Row.Details = TEXT("Reachable Complete found; multi-section/loop NotifyState ordering requires Editor review.");
			++InOutWarningCount;
		}
		else if (Summary.LastNotifyStateEndTime >= 0.f && Summary.LastCorrectCompleteTime < Summary.LastNotifyStateEndTime)
		{
			Row.Severity = TEXT("Warning");
			Row.Details = TEXT("Complete Notify occurs before the final NotifyState ends; confirm this ordering in the Editor.");
			++InOutWarningCount;
		}

		AddRow(InOutCsvLines, Row);
	}

	void AuditActionComponent(TArray<FString>& InOutCsvLines, TMap<FString, FMontageUsage>& InOutMontageUsages, int32& InOutErrorCount, int32& InOutWarningCount, const UBlueprint* Blueprint, const UCActionComponent* Component)
	{
		if (!IsValid(Component)) return;

		const FArrayProperty* ActionDatasProperty = FindFProperty<FArrayProperty>(Component->GetClass(), TEXT("ActionDatas"));
		if (ActionDatasProperty == nullptr) return;

		const void* ArrayAddress = ActionDatasProperty->ContainerPtrToValuePtr<void>(Component);
		FScriptArrayHelper ArrayHelper(ActionDatasProperty, ArrayAddress);
		for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
		{
			const FActionData* Data = reinterpret_cast<const FActionData*>(ArrayHelper.GetRawPtr(Index));
			if (Data == nullptr) continue;
			AddMontageUsage(InOutMontageUsages, Data->Montage, EExecutionDomain::Action, FString::Printf(TEXT("%s:%s"), *GetPathNameSafe(Blueprint), *FormatActionDataKey(Data->ActionDataKey)));

			AddMontageAuditRows(
				InOutCsvLines,
				InOutErrorCount,
				InOutWarningCount,
				Blueprint,
				Component,
				EExecutionDomain::Action,
				FormatActionDataKey(Data->ActionDataKey),
				Data->ActionExecutorKey.Get(),
				Data->Montage,
				static_cast<int64>(Data->ActionDataKey.ActionType),
				Data->ActionDataKey.ActionIndex,
				Data->StartSectionName,
				Data->PlayRate);
		}
	}

	void AuditReactionComponent(TArray<FString>& InOutCsvLines, TMap<FString, FMontageUsage>& InOutMontageUsages, int32& InOutErrorCount, int32& InOutWarningCount, const UBlueprint* Blueprint, const UCReactionComponent* Component)
	{
		if (!IsValid(Component)) return;

		const FArrayProperty* ReactionDatasProperty = FindFProperty<FArrayProperty>(Component->GetClass(), TEXT("ReactionDatas"));
		if (ReactionDatasProperty == nullptr) return;

		const void* ArrayAddress = ReactionDatasProperty->ContainerPtrToValuePtr<void>(Component);
		FScriptArrayHelper ArrayHelper(ReactionDatasProperty, ArrayAddress);
		for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
		{
			const FReactionData* Data = reinterpret_cast<const FReactionData*>(ArrayHelper.GetRawPtr(Index));
			if (Data == nullptr) continue;
			AddMontageUsage(InOutMontageUsages, Data->Montage, EExecutionDomain::Reaction, FString::Printf(TEXT("%s:%s"), *GetPathNameSafe(Blueprint), *FormatReactionDataKey(Data->ReactionDataKey)));

			AddMontageAuditRows(
				InOutCsvLines,
				InOutErrorCount,
				InOutWarningCount,
				Blueprint,
				Component,
				EExecutionDomain::Reaction,
				FormatReactionDataKey(Data->ReactionDataKey),
				Data->ReactionExecutorKey.Get(),
				Data->Montage,
				static_cast<int64>(Data->ReactionDataKey.ReactionType),
				Data->ReactionDataKey.ReactionIndex,
				Data->StartSectionName,
				Data->PlayRate);
		}
	}

	void AuditBlueprint(TArray<FString>& InOutCsvLines, TMap<FString, FMontageUsage>& InOutMontageUsages, int32& InOutErrorCount, int32& InOutWarningCount, const UBlueprint* Blueprint)
	{
		if (!IsValid(Blueprint) || !IsValid(Blueprint->GeneratedClass)) return;

		AActor* ClassDefaultObject = Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject(false));
		if (!IsValid(ClassDefaultObject)) return;

		TInlineComponentArray<UActorComponent*> Components(ClassDefaultObject);
		for (const UActorComponent* Component : Components)
		{
			if (const UCActionComponent* ActionComponent = Cast<UCActionComponent>(Component))
			{
				AuditActionComponent(InOutCsvLines, InOutMontageUsages, InOutErrorCount, InOutWarningCount, Blueprint, ActionComponent);
			}
			else if (const UCReactionComponent* ReactionComponent = Cast<UCReactionComponent>(Component))
			{
				AuditReactionComponent(InOutCsvLines, InOutMontageUsages, InOutErrorCount, InOutWarningCount, Blueprint, ReactionComponent);
			}
		}
	}
}

UCExecutionMontageAuditCommandlet::UCExecutionMontageAuditCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UCExecutionMontageAuditCommandlet::Main(const FString& Params)
{
	using namespace ExecutionMontageAudit;

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
			UE_LOG(LogTemp, Error, TEXT("[ExecutionMontageAudit] Invalid root: %s"), *LongPackageRoot);
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
	TMap<FString, FMontageUsage> MontageUsages;
	TArray<TWeakObjectPtr<UAnimMontage>> LoadedMontages;

	int32 LoadedPackageCount = 0;
	int32 FailedPackageCount = 0;
	int32 ErrorCount = 0;
	int32 WarningCount = 0;
	for (const FString& PackageFilename : PackageFiles)
	{
		if (!IsPathUnderRoot(PackageFilename, RootDirectories)) continue;

		FString LongPackageName;
		if (!FPackageName::TryConvertFilenameToLongPackageName(PackageFilename, LongPackageName))
		{
			++FailedPackageCount;
			UE_LOG(LogTemp, Warning, TEXT("[ExecutionMontageAudit] Cannot resolve package name: %s"), *PackageFilename);
			continue;
		}

		UPackage* Package = LoadPackage(nullptr, *LongPackageName, LOAD_NoWarn | LOAD_Quiet);
		if (!IsValid(Package))
		{
			++FailedPackageCount;
			UE_LOG(LogTemp, Warning, TEXT("[ExecutionMontageAudit] Cannot load package: %s"), *LongPackageName);
			continue;
		}

		++LoadedPackageCount;
		ForEachObjectWithPackage(Package, [&CsvLines, &MontageUsages, &LoadedMontages, &ErrorCount, &WarningCount](UObject* Object)
		{
			if (const UBlueprint* Blueprint = Cast<UBlueprint>(Object))
			{
				AuditBlueprint(CsvLines, MontageUsages, ErrorCount, WarningCount, Blueprint);
			}
			else if (UAnimMontage* Montage = Cast<UAnimMontage>(Object))
			{
				LoadedMontages.AddUnique(Montage);
			}
			return true;
		});
	}

	TArray<FString> InventoryCsvLines;
	int32 NotifyEventCount = 0;
	InventoryCsvLines.Add(TEXT("RecordType,MontagePath,Track,OwnershipDomain,DataReferences,EventKind,MontageLength,NotifyClass,NotifyDisplayName,StartTime,EndTime,Duration,Severity,ConfigAndRecommendation"));
	LoadedMontages.Sort([](const TWeakObjectPtr<UAnimMontage>& Left, const TWeakObjectPtr<UAnimMontage>& Right)
	{
		return GetPathNameSafe(Left.Get()) < GetPathNameSafe(Right.Get());
	});

	for (const TWeakObjectPtr<UAnimMontage>& WeakMontage : LoadedMontages)
	{
		const UAnimMontage* Montage = WeakMontage.Get();
		if (!IsValid(Montage)) continue;

		const FMontageUsage* Usage = MontageUsages.Find(Montage->GetPathName());
		const FString Domains = Usage ? JoinSorted(Usage->Domains) : FString();
		FString DataReferences;
		if (Usage)
		{
			TArray<FString> SortedReferences = Usage->DataReferences;
			SortedReferences.Sort();
			DataReferences = FString::Join(SortedReferences, TEXT(" | "));
		}

		if (Montage->Notifies.IsEmpty())
		{
			FAuditRow EmptyRow;
			EmptyRow.RecordType = TEXT("MontageWithoutDirectNotifies");
			EmptyRow.BlueprintPath = GetPathNameSafe(Montage);
			EmptyRow.Domain = Domains.IsEmpty() ? TEXT("NoScannedComponentDataReference") : Domains;
			EmptyRow.DataKey = DataReferences;
			EmptyRow.Severity = TEXT("Info");
			EmptyRow.Details = TEXT("No AnimNotify or AnimNotifyState events.");
			AddRow(InventoryCsvLines, EmptyRow);
			continue;
		}

		for (const FAnimNotifyEvent& Event : Montage->Notifies)
		{
			AddInventoryRow(InventoryCsvLines, Montage, Event, Domains, DataReferences);
			++NotifyEventCount;
		}
	}

	FAuditRow Summary;
	Summary.RecordType = TEXT("Summary");
	Summary.Severity = ErrorCount > 0 || FailedPackageCount > 0 ? TEXT("Error") : (WarningCount > 0 ? TEXT("Warning") : TEXT("Info"));
	Summary.Details = FString::Printf(
		TEXT("Roots=%s;DiscoveredPackages=%d;LoadedPackages=%d;FailedPackages=%d;Errors=%d;Warnings=%d;DataRows=%d"),
		*RootsParameter,
		PackageFiles.Num(),
		LoadedPackageCount,
		FailedPackageCount,
		ErrorCount,
		WarningCount,
		FMath::Max(0, CsvLines.Num() - 1));
	AddRow(CsvLines, Summary);

	FString OutputFilename;
	FParse::Value(*Params, TEXT("Output="), OutputFilename);
	if (OutputFilename.IsEmpty())
	{
		OutputFilename = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Audit/ExecutionMontageAudit.csv"));
	}
	else if (FPaths::IsRelative(OutputFilename))
	{
		OutputFilename = FPaths::Combine(FPaths::ProjectSavedDir(), OutputFilename);
	}
	OutputFilename = FPaths::ConvertRelativePathToFull(OutputFilename);

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputFilename), true);
	const FString CsvText = FString::Join(CsvLines, LINE_TERMINATOR) + LINE_TERMINATOR;
	if (!FFileHelper::SaveStringToFile(CsvText, *OutputFilename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogTemp, Error, TEXT("[ExecutionMontageAudit] Cannot write output: %s"), *OutputFilename);
		return 3;
	}

	UE_LOG(LogTemp, Display, TEXT("[ExecutionMontageAudit] Wrote report: %s"), *OutputFilename);
	UE_LOG(LogTemp, Display, TEXT("[ExecutionMontageAudit] %s"), *Summary.Details);

	FString InventoryOutputFilename;
	FParse::Value(*Params, TEXT("InventoryOutput="), InventoryOutputFilename);
	if (InventoryOutputFilename.IsEmpty())
	{
		InventoryOutputFilename = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Audit/MontageNotifyInventory.csv"));
	}
	else if (FPaths::IsRelative(InventoryOutputFilename))
	{
		InventoryOutputFilename = FPaths::Combine(FPaths::ProjectSavedDir(), InventoryOutputFilename);
	}
	InventoryOutputFilename = FPaths::ConvertRelativePathToFull(InventoryOutputFilename);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(InventoryOutputFilename), true);
	const FString InventoryCsvText = FString::Join(InventoryCsvLines, LINE_TERMINATOR) + LINE_TERMINATOR;
	if (!FFileHelper::SaveStringToFile(InventoryCsvText, *InventoryOutputFilename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogTemp, Error, TEXT("[ExecutionMontageAudit] Cannot write inventory: %s"), *InventoryOutputFilename);
		return 4;
	}
	UE_LOG(LogTemp, Display, TEXT("[ExecutionMontageAudit] Wrote notify inventory: %s | Events=%d | Montages=%d"), *InventoryOutputFilename, NotifyEventCount, LoadedMontages.Num());
	UE_LOG(LogTemp, Display, TEXT("[ExecutionMontageAudit] Scope: direct Montage notifies and Blueprint CDO component data only. Sequence notifies, level overrides, dynamic references and runtime notify delivery are not verified. NoScannedComponentDataReference does not mean unused."));

	bool bFailOnIssues = false;
	FParse::Bool(*Params, TEXT("FailOnIssues="), bFailOnIssues);
	return bFailOnIssues && (ErrorCount > 0 || FailedPackageCount > 0) ? 1 : 0;
}
