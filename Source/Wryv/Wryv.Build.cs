// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Wryv : ModuleRules
{
	public Wryv(TargetInfo Target)
	{
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
    //PublicDependencyModuleNames.Add( "UMG" );
    //PublicDependencyModuleNames.Add("Landscape");
    //PublicDependencyModuleNames.Add("Networking");
		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		//PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
    PrivateDependencyModuleNames.AddRange(new string[] { "Sockets", "Networking" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
		// if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64))
		// {
		//		if (UEBuildConfiguration.bCompileSteamOSS == true)
		//		{
		//			DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		//		}
		// }
	}
}
