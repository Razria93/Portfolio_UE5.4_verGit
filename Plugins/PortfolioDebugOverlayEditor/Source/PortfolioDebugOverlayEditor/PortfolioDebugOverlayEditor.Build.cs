using UnrealBuildTool;

public class PortfolioDebugOverlayEditor : ModuleRules
{
	public PortfolioDebugOverlayEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
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
