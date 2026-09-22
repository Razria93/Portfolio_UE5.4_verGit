using UnrealBuildTool;

public class PortfolioEditor : ModuleRules
{
	public PortfolioEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Portfolio",
			"Niagara",
			"AnimationModifiers",
			"AnimationBlueprintLibrary",
			"UMG",
			"SlateCore",
			"UnrealEd",
			"RenderCore",
			"RHI",
		});
	}
}
