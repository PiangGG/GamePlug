// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetWorkPlug.h"

#include "Subsystems/SubsystemBlueprintLibrary.h"

#define LOCTEXT_NAMESPACE "FNetWorkPlugModule"

void FNetWorkPlugModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (!NetworkSubsystem)
	{
		
	}
	UE_LOG(LogTemp, Log, TEXT("Network:FNetWorkPlugModule::StartupModule()"));
}

void FNetWorkPlugModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogTemp, Log, TEXT("Network:FNetWorkPlugModule::ShutdownModule()"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNetWorkPlugModule, NetWorkPlug)