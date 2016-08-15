//------------------------------------------------------------------------------
// 
//     The Kinect for Windows APIs used here are preliminary and subject to change
// 
//------------------------------------------------------------------------------
using System.IO;

namespace UnrealBuildTool.Rules
{
    using System.IO;

    public class KinectV2 : ModuleRules
    {
        public KinectV2(TargetInfo Target)
        {
           
            PublicIncludePaths.AddRange(
                new string[] {
                    "KinectV2/Public"
					// ... add public include paths required here ...
				}
                );

            PrivateIncludePaths.AddRange(
                new string[] {
					"KinectV2/Private",
                  
					// ... add other private include paths required here ...
				}
                );

            PublicDependencyModuleNames.AddRange(
                new string[]
				{
                   	
                    "CoreUObject",				
                    "Core",
                    "Engine",
                    "InputDevice",
                    //"BlueprintGraph",
                    //"MSSpeech",
                    "InputCore",
                     "K4WLib",
                     //"AnimGraphRuntime"
                   // "Slate"
					// ... add other public dependencies that you statically link with here ...
				}
                );

           





            //AddThirdPartyPrivateStaticDependencies(Target, "K4WLib");

            if (UEBuildConfiguration.bBuildEditor == true)
            {
                //@TODO: Needed for the triangulation code used for sprites (but only in editor mode)
                //@TOOD: Try to move the code dependent on the triangulation code to the editor-only module
              //  PrivateDependencyModuleNames.AddRange(new string [] {"UnrealEd","ContentBrowser"});
                PrivateDependencyModuleNames.AddRange(
               new string[] 
               {
               // ... add private dependencies that you statically link with here ... 
               "Core", "CoreUObject", "InputCore", "Slate", "Engine", "AssetTools"// "UnrealEd", 
               // for FAssetEditorManager
               //"BlueprintGraph", "UnrealEd", "GraphEditor"
               //"KismetWidgets", "KismetCompiler", "BlueprintGraph", "GraphEditor", "Kismet",
               // for FWorkflowCentricApplication
               //"PropertyEditor", "EditorStyle", "Slate", "SlateCore", "Sequencer", "DetailCustomizations", "Settings", "RenderCore",
               }
               );
    
            }


        }
    }
}