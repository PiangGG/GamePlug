// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWorkSystem.h"
#include "Runtime/UMG/Public/UMG.h"
#include "OnlineSubsystem.h"
#include "Online.h"
#include "Blueprint/UserWidget.h"

UNetWorkSystem::UNetWorkSystem(/*const FObjectInitializer& ObjectInitializer*/)
{
	//initial state is None 
	CurrentState = EGameState::ENone;
	//current widget is nothing
	CurrentWidget = nullptr;
        
	/* BIND FUNCTIONS FOR SESSION MANAGEMENT */

	//Create
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNetWorkSystem::OnCreateSessionComplete);

	//Start
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UNetWorkSystem::OnStartOnlineGameComplete);

	//Find
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNetWorkSystem::OnFindSessionsComplete);

	//Join
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNetWorkSystem::OnJoinSessionComplete);

	//Update
	OnUpdateSessionCompleteDelegate = FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UNetWorkSystem::OnUpdateSessionComplete);

	//Destroy
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNetWorkSystem::OnDestroySessionComplete);

	cMainMenu=LoadClass<UUserWidget>(this,TEXT("WidgetBlueprint'/NetWorkPlug/Widgets/MainMenu/W_MainMenu.W_MainMenu_C'"));
	cMPHome=LoadClass<UUserWidget>(this,TEXT("WidgetBlueprint'/NetWorkPlug/Widgets/MainMenu/W_MultiplayerHome.W_MultiplayerHome_C'"));
	cMPJoin=LoadClass<UUserWidget>(this,TEXT("WidgetBlueprint'/NetWorkPlug/Widgets/MainMenu/JoinGameScreen/W_MultiplayerJoinGameMenu.W_MultiplayerJoinGameMenu_C'"));
	cMPHost=LoadClass<UUserWidget>(this,TEXT("WidgetBlueprint'/NetWorkPlug/Widgets/MainMenu/W_HostGameMenu.W_HostGameMenu_C'"));
	cLoadingScreen=LoadClass<UUserWidget>(this,TEXT("WidgetBlueprint'/NetWorkPlug/Widgets/MainMenu/W_LoadingScreen.W_LoadingScreen_C'"));
}

void UNetWorkSystem::Init()
{
	if (GEngine) {
		GEngine->OnNetworkFailure().AddUObject(this, &UNetWorkSystem::HandleNetworkError);
	}
}

void UNetWorkSystem::ChangeState(EGameState newState)
{
	//改变模式所改变模式不为当前模式时修改
	if (newState != CurrentState) {
		LeaveState();
		EnterState(newState);
	}
}

EGameState UNetWorkSystem::GetCurrentGameState()
{
	return  CurrentState;
}

void UNetWorkSystem::SetInputMode(EInputMode newInputMode, bool bShowMouseCursor)
{
	if (!GWorld->GetFirstPlayerController())return;
   
	switch (newInputMode) {
	case EInputMode::EUIOnly: {
			GWorld->GetFirstPlayerController()->SetInputMode(FInputModeUIOnly());
			break;
	}
	case EInputMode::EUIAndGame: {
			GWorld->GetFirstPlayerController()->SetInputMode(FInputModeGameAndUI());
			break;
	}
	case EInputMode::EGameOnly: {
			GWorld->GetFirstPlayerController()->SetInputMode(FInputModeGameOnly());
			break;
	}
	}

	//show or hide mouse cursor
	GWorld->GetFirstPlayerController()->bShowMouseCursor = bShowMouseCursor;
	//GetFirstLocalPlayerController()->bShowMouseCursor=bShowMouseCursor;
	//retain the values for further use
	CurrentInputMode = newInputMode;
 
	bIsShowingMouseCursor = bShowMouseCursor;
}

void UNetWorkSystem::HostGame(bool bIsLAN, int32 MaxNumPlayers, TArray<FBlueprintSessionSetting> sessionSettings)
{
	//get the online subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	//if OnlineSub is valid
	if (OnlineSub) {
		//get unique player id, we use 0 since we dont allow multiple players per client
		TSharedPtr<const FUniqueNetId> pid = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);

		//create the special settings map
		TMap<FString, FOnlineSessionSetting> SpecialSettings = TMap<FString, FOnlineSessionSetting>();

		//loop through any provided settings and add them to special settings map
		for (auto &setting : sessionSettings) {
			//create a new setting
			FOnlineSessionSetting newSetting;

			//set its data to the provided value
			newSetting.Data = setting.value;

			//ensure the setting is advertised over the network
			newSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;

			//add setting to the map with the specified key
			SpecialSettings.Add(setting.key, newSetting);
		}

		//Change the state to loading screen while attempting to host the game
		ChangeState(EGameState::ELoadingScreen);

		//host the session
		HostSession(pid, GameSessionName, bIsLAN, MaxNumPlayers, SpecialSettings);
	}
}

bool UNetWorkSystem::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN,
	int32 MaxNumPlayers, TMap<FString, FOnlineSessionSetting> SettingsMap)
{
	//Get the online subsystem
        IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

        if (OnlineSub) {
                //get the session interface, so we can call CreateSession on it
                IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

                //ensure the sessions interface retrieved is valid
                // and that the provided userid is valid
                if (Sessions.IsValid() && UserId.IsValid()) {
                        //create the SessionSettings shared pointer
                        SessionSettings = MakeShareable(new FOnlineSessionSettings());

                        //set up the session settings
                        SessionSettings->bIsLANMatch = bIsLAN;
                        SessionSettings->bUsesPresence = true; //we will always be using presence; though this can be changed.
                        SessionSettings->NumPublicConnections = MaxNumPlayers;
                        SessionSettings->NumPrivateConnections = 0;
                        SessionSettings->bAllowInvites = true;
                        SessionSettings->bAllowJoinInProgress = true;
                        SessionSettings->bShouldAdvertise = true;
                        SessionSettings->bAllowJoinViaPresence = true;
                        SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

                        //set a special setting for the map name we're going to use.
                        // for now, we're always going to Map_SandBox however
                        // you can make this a selection hosts can choose either from hosting screen 
                        // or once in game
                        SessionSettings->Set(SETTING_MAPNAME, FString("Map_SandBox"), EOnlineDataAdvertisementType::ViaOnlineService);

                        //add the special settings provided
                        for (auto &setting : SettingsMap) {
                                SessionSettings->Settings.Add(FName(*setting.Key), setting.Value);
                        }

                        //register the delegate with SessionInterface and retrieve the handle
                        OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

                        //call CreateSession and return the value. It does not need to return true.
                        return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
                }
        }
        //NO SUBSYSTEM FOUND
        return false;
}

void UNetWorkSystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	//Get online subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get the sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//clear the delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

			//if it was successful, start game
			if (bWasSuccessful) {
				//set the start session delegate handle
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				//we then call StartSession
				Sessions->StartSession(SessionName);
			}
		}
	}
}

void UNetWorkSystem::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	//Get the Online subsystem as we need to clear the delegate handle
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get the sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//clear the delegate
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	//if the start was successful, we open our map as a listen server
	if (bWasSuccessful) {
		UGameplayStatics::OpenLevel(GetWorld(), "Map_SandBox", true, "listen");
		//change state to travelling
		ChangeState(EGameState::ETravelling);
	}
}

void UNetWorkSystem::FindGames(bool bIsLAN)
{
	//Get the Online Subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	//reset search status
	bHasFinishedSearchingForGames = false;
	bSearchingForGames = false;
	searchResults.Empty();

	if (OnlineSub) {
		//Get unique net ID
		TSharedPtr<const FUniqueNetId> pid = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);

		FindSessions(pid, GameSessionName, bIsLAN);
	}
}

void UNetWorkSystem::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN)
{
	//Get the Online Subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//Get the sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid() && UserId.IsValid()) {
			//create the shared pointer to the search settings
			SessionSearch = MakeShareable(new FOnlineSessionSearch());

			//fill in the search settings
			SessionSearch->bIsLanQuery = bIsLAN;
			SessionSearch->MaxSearchResults = 100000000;
			SessionSearch->PingBucketSize = 50;

			//our game will be set up using "presence"
			//if you're not using presence, do no include this next line
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

			//we need a shared reference to pass into the session interface's FindSessions function.
			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

			//Register our delegate with the Online Subsystem and retain the handle
			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			//tell the game instance we've started searching for games
			bSearchingForGames = true;

			//call FindSessions on the SessionInterface
			Sessions->FindSessions(*UserId, SearchSettingsRef);
		}
	}
	else {
		//if there's no online subsystem; immediately call the failure of OnFindSessionsComplete
		OnFindSessionsComplete(false);
	}
}

void UNetWorkSystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	//first we need to clear the delegate handle

	//get Online subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//clear the delegate handle
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		}
	}

	//then we deal with any search results

	//if we were successful, parse search results; else just end the search
	if (bWasSuccessful) {
		//loop through search results and add them to our blueprint accessible array
		for (auto &result : SessionSearch->SearchResults) {
			FBlueprintSearchResult newresult = FBlueprintSearchResult(result);
			searchResults.Add(newresult);
		}
	}

	//mark searching as over
	bHasFinishedSearchingForGames = true;
	bSearchingForGames = false;
}

void UNetWorkSystem::JoinGame(FBlueprintSearchResult result)
{
	//Get the online subsystem so we can extract the unique player id
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get unique id
		TSharedPtr<const FUniqueNetId> pid = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);

		//call join session, passing in the result from our struct
		JoinSession(pid, GameSessionName, result.result);
	}
}

bool UNetWorkSystem::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName,
	const FOnlineSessionSearchResult& SearchResult)
{
	//to hold the result
	bool bSuccessful = false;

	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid() && UserId.IsValid()) {
			//register the delegate and retain the handle
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			//call join session
			bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
		}
	}
	return bSuccessful;
}

void UNetWorkSystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//Get Online Subsystem so we can clear the delegate
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//clear the delegate handle
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

			//get the first player controller so we can call client travel
			APlayerController *const PlayerController = GetWorld()->GetFirstPlayerController();//GetFirstLocalPlayerController();

			//we need an FString to use ClientTravel
			//we can let the session interface construct this string for us
			//by giving it the session name and an empty string
			FString TravelURL;

			if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL)) {
				//finally call ClientTravel. If you wanted to you could print the TravelURL
				//to see what it looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
				//change state to travelling
				ChangeState(EGameState::ETravelling);
			}
		}
	}
}

FString UNetWorkSystem::GetSessionSpecialSettingString(FString key)
{
	//Get Online Subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get session interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//get the current session settings
			FOnlineSessionSettings *settings = Sessions->GetSessionSettings(GameSessionName);

			//ensure settings isnt null
			if (settings) {
				//check if the settings contains the desired key already
				if (settings->Settings.Contains(FName(*key))) {
					//get the value and return it
					FString value;
					settings->Settings[FName(*key)].Data.GetValue(value);

					return value;
				}
				else {
					//key doesnt exist
					return FString("INVALID KEY");
				}
			}
		}
		else {
			return FString("NO SESSION!");
		}
	}
	return FString("NO ONLINE SUBSYSTEM");
}

void UNetWorkSystem::SetOrUpdateSessionSpecialSettingString(FBlueprintSessionSetting newSetting)
{
	//get online subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//get current sessions settings
			FOnlineSessionSettings *settings = Sessions->GetSessionSettings(GameSessionName);

			if (settings) {
				//check to see if it already contains the key
				if (settings->Settings.Contains(FName(*newSetting.key))) {
					//update the setting
					settings->Settings[FName(*newSetting.key)].Data = newSetting.value;
				}
				else { //if the key doesnt already exist
					//create the new setting
					FOnlineSessionSetting setting;

					setting.Data = newSetting.value;
					setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
					
					//add the new setting to the array
					settings->Settings.Add(FName(*newSetting.key), setting);
				}

				//register the delegate and retain the handle
				OnUpdateSessionCompleteDelegateHandle = Sessions->AddOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegate);

				//update the session settings
				Sessions->UpdateSession(GameSessionName, *settings, true);
			}
		}
	}
}

void UNetWorkSystem::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	//we need to Clear the delegate
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//clear the delegate
			Sessions->ClearOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegateHandle);
		}
	} 
}

void UNetWorkSystem::LeaveGame()
{
	//get the online subsystem
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get sessions interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//register the delegate
			OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			//destroy the session
			Sessions->DestroySession(GameSessionName);
		}
	}
}

void UNetWorkSystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	//get online subsystem so we can clear the delegate
	IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub) {
		//get session interface
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid()) {
			//clear the delegate
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

		}
	}

	//if it was successful, go to main menu
	if (bWasSuccessful)
		{
			UGameplayStatics::OpenLevel(GWorld,"Map_MainMenu", true);
			ChangeState(EGameState::ETravelling);
		}
}

void UNetWorkSystem::HandleNetworkError(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	//simply leave any game if we run into a network error
	LeaveGame();
}

void UNetWorkSystem::EnterState(EGameState newState)
{
	 //set the current state to newState
	CurrentState = newState;

	 if (!GWorld)return;
	 
	 if (!GWorld->GetFirstPlayerController())return;
	
	switch (CurrentState) {
	case EGameState::ELoadingScreen: {
		
			CurrentWidget = CreateWidget<UUserWidget>(GWorld->GetFirstPlayerController(), cLoadingScreen);

			if (!CurrentWidget)return;

			CurrentWidget->AddToViewport();

			SetInputMode(EInputMode::EUIOnly, true);
			break;
	}
	case EGameState::EMainMenu: {
		
			CurrentWidget = CreateWidget<UUserWidget>(GWorld->GetFirstPlayerController(), cMainMenu);

			if (!CurrentWidget)return;
       
			CurrentWidget->AddToViewport();
           
			SetInputMode(EInputMode::EUIOnly, true);
			break;
	}
	case EGameState::EMultiplayerHome: {
	
			CurrentWidget = CreateWidget<UUserWidget>(GWorld->GetFirstPlayerController(), cMPHome);
			if (!CurrentWidget)return;
			CurrentWidget->AddToViewport();

			SetInputMode(EInputMode::EUIOnly, true);
			break;
	}
	case EGameState::EMultiplayerJoin: {

			CurrentWidget = CreateWidget<UUserWidget>(GWorld->GetFirstPlayerController(), cMPJoin);
			if (!CurrentWidget)return;
			CurrentWidget->AddToViewport();
	        
			SetInputMode(EInputMode::EUIOnly, true);
			break;
	}
	case EGameState::EMultiplayerHost: {
		
			CurrentWidget = CreateWidget<UUserWidget>(GWorld->GetFirstPlayerController(), cMPHost);
			if (!CurrentWidget)return;
			CurrentWidget->AddToViewport();

			SetInputMode(EInputMode::EUIOnly, true);
			break;
	}
	case EGameState::EMultiplayerInGame: {
		
			SetInputMode(EInputMode::EGameOnly, false);
			break;
	}
	case EGameState::ETravelling: {

			SetInputMode(EInputMode::EUIOnly, false);
			break;
	}
	default: {
			break;
	}
	}
}

void UNetWorkSystem::LeaveState()
{
	switch (CurrentState) {
		case EGameState::ELoadingScreen:
			{
			
			}
		case EGameState::EMainMenu:
			{
			
			}
		case EGameState::EMultiplayerHome:
			{
			
			}
		case EGameState::EMultiplayerJoin:
			{
			
			}
		case EGameState::EMultiplayerHost:
			{
				if (CurrentWidget)
					{
						CurrentWidget->RemoveFromViewport();
						CurrentWidget = nullptr;
					}
				break;
			}
		case EGameState::EMultiplayerInGame:
			{
				break;
			}
		case EGameState::ETravelling:
			{
				break;
			}
		case EGameState::ENone:
			{
				break;
			}
		default:
			{
				break;
			}
	}
	EnterState(EGameState::ENone);
}
