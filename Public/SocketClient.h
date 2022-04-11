// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "Runtime/Core/Public/Misc/SecureHash.h"
#include "Modules/ModuleManager.h"


UENUM(BlueprintType)
enum class EReceiveFilterClient : uint8
{
	E_SAB 	UMETA(DisplayName = "Message And Bytes"),
	E_S		UMETA(DisplayName = "Message"),
	E_B		UMETA(DisplayName = "Bytes")

};


UENUM(BlueprintType)
enum class ESocketPlatformClient : uint8
{
	E_SSC_SYSTEM		UMETA(DisplayName = "System"),
	E_SSC_DEFAULT 		UMETA(DisplayName = "Auto"),
	E_SSC_WINDOWS		UMETA(DisplayName = "WINDOWS"),
	E_SSC_MAC			UMETA(DisplayName = "MAC"),
	E_SSC_IOS			UMETA(DisplayName = "IOS"),
	E_SSC_UNIX			UMETA(DisplayName = "UNIX"),
	E_SSC_ANDROID		UMETA(DisplayName = "ANDROID"),
	E_SSC_PS4			UMETA(DisplayName = "PS4"),
	E_SSC_XBOXONE		UMETA(DisplayName = "XBOXONE"),
	E_SSC_HTML5			UMETA(DisplayName = "HTML5"),
	E_SSC_SWITCH		UMETA(DisplayName = "SWITCH")

};

#ifndef __FileFunctionsSocketClient
#define __FileFunctionsSocketClient
#include "FileFunctionsSocketClient.h"
#endif

#ifndef __SocketClientBPLibrary
#define __SocketClientBPLibrary
#include "SocketClientBPLibrary.h"
#endif

#ifndef __SocketClientPluginTCPClient
#define __SocketClientPluginTCPClient
#include "SocketClientPluginTCPClient.h"
#endif

#ifndef __SocketClientPluginUDPClient
#define __SocketClientPluginUDPClient
#include "SocketClientPluginUDPClient.h"
#endif

class FSocketClientModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static bool isShuttingDown;
};