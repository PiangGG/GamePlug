// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetWorkSystem.h"
#include "Subsystems/EngineSubsystem.h"
#include "NetworkSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPLUG_API UNetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UNetworkSubsystem();

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetNetWorkSystem"), Category = "NetWork|Subsystem")
	UNetWorkSystem* GetNetWorkSystem();

private:
	UNetWorkSystem* NetWorkSystem;
};
