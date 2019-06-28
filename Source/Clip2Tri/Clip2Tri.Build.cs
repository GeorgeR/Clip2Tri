using UnrealBuildTool;

public class Clip2Tri : ModuleRules
{
	public Clip2Tri(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

	    if (Target.Version.MinorVersion <= 19)
	    {
			PublicIncludePaths.AddRange(
				new string[] {
					"Clip2Tri/Public"
				});
			
			PrivateIncludePaths.AddRange(
				new string[] {
					"Clip2Tri/Private"
				});
		}
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			});

	    Definitions.AddRange(
            new string[]
            {
                "ANSI_DECLARATORS",
                "SINGLE",
                "NO_TIMER",
                "TRILIBRARY",
                "CPU86"
            });
	}
}