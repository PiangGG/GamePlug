// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkSubsystem.h"

UNetworkSubsystem::UNetworkSubsystem()
{
}

void UNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("Network: subsystem initialized"));
}

void UNetworkSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("Network: subsystem Deinitialized"));
}

UNetWorkSystem* UNetworkSubsystem::GetNetWorkSystem()
{
	if (!NetWorkSystem)
	{
		NetWorkSystem=NewObject<UNetWorkSystem>();
		NetWorkSystem->Init();
	}
	return NetWorkSystem;

}
