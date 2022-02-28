// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkSubsystem.h"
#include "Modules/ModuleManager.h"

class FNetWorkPlugModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	UNetworkSubsystem* NetworkSubsystem;
};
