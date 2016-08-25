// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class K4WLib : ModuleRules
{
    public K4WLib(TargetInfo Target)
    {
        Type = ModuleType.External;

        string SDKDIR = Utils.ResolveEnvironmentVariable("%KINECTSDK20_DIR%");

        SDKDIR = SDKDIR.Replace("\\", "/");

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
            string RedistDirectory = Path.Combine(BaseDirectory, "ThirdParty", "Redist");


            string ShortPlatform = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            PublicIncludePaths.Add(SDKDIR + "inc/");

            string PlatformPath = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64/" : "x86/";

            string LibPath = SDKDIR + "Lib/" + PlatformPath;

            PublicLibraryPaths.Add(LibPath);
            PublicAdditionalLibraries.AddRange(new string[] { "Kinect20.lib", "Kinect20.face.lib" });

            PublicDelayLoadDLLs.AddRange(new string[] {"Kinect20.Face.dll", "Kinect20.dll",  "Microsoft.Kinect.Face.dll", /*vgbtechs/AdaBoostTech.dll", "vgbtechs/RFRProgressTech.dll" */});



            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(RedistDirectory, "Face", ShortPlatform, "Kinect20.Face.dll")));
            //RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(RedistDirectory, "Face", ShortPlatform, "NuiDataBase")));
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(RedistDirectory, "Face", ShortPlatform, "Microsoft.Kinect.Face.dll")));
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(RedistDirectory, "Face", ShortPlatform, "Microsoft.Kinect.Face.xml")));

          



            //PublicDelayLoadDLLs.AddRange(new string[] { "Kinect20.dll", "Kinect20.Face.dll","Microsoft.Kinect.Face.dll", /*vgbtechs/AdaBoostTech.dll", "vgbtechs/RFRProgressTech.dll" */});
            

        }
    }
}
