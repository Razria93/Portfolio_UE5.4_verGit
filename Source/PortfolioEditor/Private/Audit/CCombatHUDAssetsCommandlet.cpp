#include "Audit/CCombatHUDAssetsCommandlet.h"
#include "Engine/Font.h"
#include "Engine/FontFace.h"
#include "Engine/Texture2D.h"
#include "Factories/TextureFactory.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace
{
	const FString Root = TEXT("/Game/08_UI/CombatHUD/");
	bool Save(UObject* Asset)
	{
		FSavePackageArgs args;
		args.TopLevelFlags = RF_Public | RF_Standalone;
		args.SaveFlags = SAVE_NoError;
		const FString filename = FPackageName::LongPackageNameToFilename(Asset->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
		return UPackage::SavePackage(Asset->GetOutermost(), Asset, *filename, args);
	}
	UFontFace* Face(const TCHAR* Name, const TCHAR* Filename)
	{
		const FString packageName = Root + Name;
		if (FPackageName::DoesPackageExist(packageName))
			return LoadObject<UFontFace>(nullptr, *(packageName + TEXT(".") + Name));
		TArray<uint8> bytes;
		const FString source = FPaths::ProjectDir() / TEXT("Resources/CombatHUD/Fonts") / Filename;
		if (!FFileHelper::LoadFileToArray(bytes, *source)) return nullptr;
		UFontFace* face = NewObject<UFontFace>(CreatePackage(*packageName), Name, RF_Public | RF_Standalone);
		face->InitializeFromBulkData(source, EFontHinting::Default, bytes.GetData(), bytes.Num());
		return Save(face) ? face : nullptr;
	}
	bool Font(const TCHAR* Name, UFontFace* Latin, UFontFace* Korean)
	{
		const FString packageName = Root + Name;
		if (FPackageName::DoesPackageExist(packageName)) return LoadObject<UFont>(nullptr, *(packageName + TEXT(".") + Name)) != nullptr;
		UFont* font = NewObject<UFont>(CreatePackage(*packageName), Name, RF_Public | RF_Standalone);
		font->FontCacheType = EFontCacheType::Runtime;
		FTypefaceEntry entry(TEXT("Regular"));
		entry.Font = FFontData(Latin);
		font->CompositeFont.DefaultTypeface.Fonts.Add(entry);
		entry.Font = FFontData(Korean);
		font->CompositeFont.FallbackTypeface.Typeface.Fonts.Add(entry);
		return Save(font);
	}
}

UCCombatHUDAssetsCommandlet::UCCombatHUDAssetsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UCCombatHUDAssetsCommandlet::Main(const FString& Params)
{
	if (!FParse::Param(*Params, TEXT("CreateMissing")))
	{
		UE_LOG(LogTemp, Error, TEXT("Requires -CreateMissing; existing assets are never overwritten."));
		return 1;
	}
	UFontFace* regular = Face(TEXT("FF_OxaniumRegular"), TEXT("Oxanium-Regular.ttf"));
	UFontFace* medium = Face(TEXT("FF_OxaniumMedium"), TEXT("Oxanium-Medium.ttf"));
	UFontFace* korean = Face(TEXT("FF_NotoSansKR"), TEXT("NotoSansCJKkr-Regular.otf"));
	if (!regular || !medium || !korean || !Font(TEXT("F_HUDLabel"), regular, korean) || !Font(TEXT("F_HUDName"), medium, korean)) return 1;
	const TCHAR* names[] = { TEXT("Vial"),
		TEXT("ActionRush"), TEXT("ActionGuard"), TEXT("ActionDodge"), TEXT("ActionCounter"),
		TEXT("ActionExecution"), TEXT("ActionGuardBreak") };
	for (const TCHAR* name : names)
	{
		const FString assetName = FString(TEXT("T_HUD")) + name;
		const FString packageName = Root + assetName;
		if (FPackageName::DoesPackageExist(packageName)) continue;
		UTextureFactory* factory = NewObject<UTextureFactory>();
		factory->SuppressImportOverwriteDialog();
		bool cancelled = false;
		UTexture2D* texture = Cast<UTexture2D>(factory->ImportObject(UTexture2D::StaticClass(), CreatePackage(*packageName), *assetName,
			RF_Public | RF_Standalone, FPaths::ProjectDir() / TEXT("Resources/CombatHUD/Icons") / (FString(name) + TEXT(".png")), nullptr, cancelled));
		if (!texture || cancelled) return 1;
		texture->CompressionSettings = TC_EditorIcon;
		texture->LODGroup = TEXTUREGROUP_UI;
		texture->MipGenSettings = TMGS_NoMipmaps;
		texture->MaxTextureSize = 256;
		texture->PostEditChange();
		if (!Save(texture)) return 1;
	}
	return 0;
}
