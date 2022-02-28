// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "MYUSTRUCT.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FBlueprintSessionSetting {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Session Management")
		FString key;

	UPROPERTY(BlueprintReadWrite, Category = "Session Management")
		FString value;
};

/* Type for holding our SearchResult and exposing the needed properties to blueprints*/
USTRUCT(BlueprintType)
struct FBlueprintSearchResult {
	GENERATED_USTRUCT_BODY()

	//Our search result. this type is not blueprint accessible
	FOnlineSessionSearchResult result;

	UPROPERTY(BlueprintReadOnly, Category = "Session Management")
		FString ServerName;

	UPROPERTY(BlueprintReadOnly, Category = "Session Management")
		bool bIsInProgress;

	UPROPERTY(BlueprintReadOnly, Category = "Session Management")
		FString MapName;

	UPROPERTY(BlueprintReadOnly, Category = "Session Management")
		int PingInMs;

	UPROPERTY(BlueprintReadOnly, Category = "Session Management")
		int CurrentPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "Session Management")
		int MaxPlayers;

	//Constructor for empty Search Result
	FBlueprintSearchResult() {
		ServerName = FString("No Server Info");
		PingInMs = -1;
		bIsInProgress= false ;
		MapName = FString("No Map Info");
		CurrentPlayers = 0;
		MaxPlayers = 0;
	}

	//Constructor when provided a search result
	FBlueprintSearchResult(FOnlineSessionSearchResult theResult) 
	{
		//keep the result
		result = theResult;

		//retrieve special settings from the search result
		ServerName = GetSpecialSettingString(FString("ServerName"));
		MapName = GetSpecialSettingString(FString("MAPNAME"));
		FString InProgressString = GetSpecialSettingString(FString("InProgress"));

		if (InProgressString == FString("true")) {
			bIsInProgress = true;
		}
		else {
			bIsInProgress = false;
		}

		//get some built in setting data
		MaxPlayers = result.Session.SessionSettings.NumPublicConnections;
		CurrentPlayers = MaxPlayers - result.Session.NumOpenPublicConnections;
		PingInMs = result.PingInMs;
	}

	//function for getting special setting data from our result
	FString GetSpecialSettingString(FString key) {
		//retrieve the session settings
		FOnlineSessionSettings settings = result.Session.SessionSettings;

		//check to see if the key exists
		if (settings.Settings.Contains(FName(*key))) {
			FString value;

			//retrieve the value from the settings
			settings.Settings[FName(*key)].Data.GetValue(value);
			return value;
		}

		//if it doesnt contain that setting
		return FString("NO DATA AT THAT KEY");
	}
};
class NETWORKPLUG_API MYUSTRUCT
{
public:
	MYUSTRUCT();
	~MYUSTRUCT();
};
