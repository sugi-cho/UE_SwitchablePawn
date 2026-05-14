using UnrealBuildTool;

public class SwitchablePawn : ModuleRules
{
	public SwitchablePawn(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"HeadMountedDisplay",
			"InputCore",
			"NavigationSystem",
			"XRBase"
		});
	}
}
