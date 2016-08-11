// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class MagicMirror : ModuleRules
{
    private string ModulePath
    {
        get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }
 

	public MagicMirror(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "K4WLib", "KinectV2" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        PublicIncludePaths.AddRange(new string[] { "KinectV2/Public", "KinectV2/Classes" });
        UEBuildConfiguration.bForceEnableExceptions = true;

        LoadPython(Target);
        LoadMagick(Target);
	}

    public bool LoadPython(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        string AnacondaPlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "_64" : "";
        string AnacondaPath = "E:/Anaconda27" + AnacondaPlatformString;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            

            PublicAdditionalLibraries.Add(Path.Combine(AnacondaPath, "libs", "python27.lib"));
        }

        if (isLibrarySupported)
        {
            // Include path

            PublicIncludePaths.Add(Path.Combine(AnacondaPath));
        }

        Definitions.Add(string.Format("WITH_BOBS_MAGIC_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }

    public bool LoadMagick(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";

            PublicLibraryPaths.Add("C:/Program Files/ImageMagick-7.0.2-Q16/lib/");

            PublicAdditionalLibraries.AddRange(new string[] { "CORE_RL_Magick++_.lib", "CORE_RL_MagickCore_.lib", "CORE_RL_MagickWand_.lib" });
        }

        if (isLibrarySupported)
        {
            // Include path

            PublicIncludePaths.Add("C:/Program Files/ImageMagick-7.0.2-Q16/include/");
        }

        Definitions.Add(string.Format("WITH_BOBS_MAGIC_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }
}
