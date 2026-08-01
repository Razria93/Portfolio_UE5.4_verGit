using UnrealBuildTool;

public class AssetReferenceInspector : ModuleRules
{
	public AssetReferenceInspector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",
			"ContentBrowser",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Projects",
			"Slate",
			"SlateCore",
			"ToolMenus"
		});
	}
}
