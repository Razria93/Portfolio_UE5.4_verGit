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

		// Editor regression tests instantiate native AI controllers.
		PrivateDependencyModuleNames.Add("AIModule");
		PrivateDependencyModuleNames.AddRange(new string[] { "MovieSceneCapture", "ImageWrapper", "Slate", "AudioMixer" });
	}
}
