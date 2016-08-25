using UnrealBuildTool;

public class KinectV2Editor : ModuleRules
{
    public KinectV2Editor(TargetInfo Target)
    {
        PrivateIncludePaths.Add("KinectV2Editor/Private");

        PublicIncludePaths.Add("KinectV2Editor/Public");

        PublicDependencyModuleNames.AddRange(
            new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"Slate",
				"BlueprintGraph",
                "AnimGraph",
                "AnimGraphRuntime",
                "KinectV2"
			}
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
				"SlateCore",
                "Engine",
				//"UnrealEd",
                //"GraphEditor",
			}
        );

        CircularlyReferencedDependentModules.AddRange(
            new string[] {
                 //"UnrealEd",
                //"GraphEditor",
            }
        ); 

    }
}