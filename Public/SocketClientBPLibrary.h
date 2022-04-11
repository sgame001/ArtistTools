// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketClient.h"
#include "SocketClientBPLibrary.generated.h"

USTRUCT(BlueprintType)
struct FDataBufferStack
{
	
	GENERATED_BODY()
	FDataBufferStack(): addLineBreak(false)
	{
	}

	FDataBufferStack(FString InConnectionID,FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak = false)
		: connectionID(InConnectionID),message(InMessage),byteArray(InByteArray),addLineBreak(InAddLineBreak)
	{}

	FString connectionID;
	FString message;
	TArray<uint8> byteArray;
	bool addLineBreak;
};




UENUM(BlueprintType)
enum class ESocketClientSystem : uint8
{
	Android,
	IOS,
	Windows,
	Linux,
	Mac
};

UENUM(BlueprintType)
enum class ESocketClientDirectoryType : uint8
{
	E_gd	UMETA(DisplayName = "Game directory"),
	E_ad 	UMETA(DisplayName = "Absolute directory")
};

class USocketClientPluginTCPClient;
class USocketClientPluginUDPClient;
class FServerUDPConnectionThread;

UCLASS()
class SOCKETCLIENT_API USocketClientBPLibrary : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

public:

	~USocketClientBPLibrary();



	//Delegates
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTCPSocketClientConnectedCallBack, bool, FString,  FString);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTCPSocketClientReceiveCallBack, FString, const TArray<uint8>&, FString);

	
	// Dynamic Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FsocketClientTCPConnectionEventDelegate, bool, success, FString, message, FString, clientConnectionID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FreceiveTCPMessageEventDelegate, FString, message, const TArray<uint8>&, byteArray, FString, clientConnectionID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FsocketClientUDPConnectionEventDelegate, bool, success, FString, message, FString, clientConnectionID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FreceiveUDPMessageEventDelegate, FString, message, const TArray<uint8>&, byteArray, FString, IP_FromSender, int32, portFromSender, FString, clientConnectionID);

	UFUNCTION()
		void socketClientTCPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|TCP|Events|ConnectionInfo")
		FsocketClientTCPConnectionEventDelegate onsocketClientTCPConnectionEventDelegate;
	UFUNCTION()
		void receiveTCPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|TCP|Events|ReceiveMessage")
		FreceiveTCPMessageEventDelegate onreceiveTCPMessageEventDelegate;
	UFUNCTION()
		void socketClientUDPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|UDP|Events|ConnectionInfo")
		FsocketClientUDPConnectionEventDelegate onsocketClientUDPConnectionEventDelegate;
	UFUNCTION()
		void receiveUDPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const  FString IP, const int32 port,const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|UDP|Events|ReceiveMessage")
		FreceiveUDPMessageEventDelegate onreceiveUDPMessageEventDelegate;

	FTCPSocketClientConnectedCallBack OnTCPSocketClientConnectedCallBack;

	FTCPSocketClientReceiveCallBack OnTCPSocketClientReceiveCallBack;
	
	/**
	*  Get an instance of this library. This allows non-static functions to be called. 
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient")
		static USocketClientBPLibrary* getSocketClientTarget();
	static USocketClientBPLibrary* socketClientBPLibrary;



	/**
	*Connect to a TCP Server
	*@param domainOrIP IP or Domain of your server
	*@param port port to listen
	*@param receiveFilter This allows you to decide which data type you want to receive. If you receive files it makes no sense to convert them into a string.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP")
		void connectSocketClientTCP(FString domainOrIP, int32 port, EReceiveFilterClient receiveFilters, FString &connectionID);



	/**
	* Sends a string or byte array to the server. 
	*@param connectionID The ID to an existing connection.
	*@param message String to send
	*@param byteArray bytes to send
	*@param addLineBreak add a line break at the end
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketClientSendTCP(FString connectionID, FString message, TArray<uint8> byteArray, bool addLineBreak = true);


	/**
	* Terminates an existing connection.
	*@param connectionID The ID to an existing connection.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP")
		void closeSocketClientConnectionTCP(FString connectionID);


	/**
	* Useful if you want to attach events to a certain connection.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP")
		void getTCPConnectionByConnectionID(FString connectionID, bool &found, USocketClientPluginTCPClient* &connection);


	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP")
		bool isTCPConnected(FString connectionID);

	/**
	* Use before a connection has been established and do not use this node if byte arrays are used! It can happen that large messages are split at the receiver. Or that messages sent very quickly are combined into one. With this node the problem can be solved. If message wrapping is activated, a string is inserted before the message and antoher after the message. On receipt, the message must also contain both strings and is split or combined accordingly. 
	*@param header This string is appended before the message.
	*@param footer This string is appended behind the message. If line break is selected, it will be placed after the footer.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP")
		void activateTCPMessageWrappingOnClientPlugin(FString header = "([{UE4-Head}])", FString footer = "([{UE4-Foot}])");

	/**
	* Use before a connection has been established!
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|TCP")
		void deactivateTCPMessageWrappingOnClientPlugin();


	/**
	*Opens a connection on specific ip and port and listen on it.
	*@param DomainOrIP IP or Domain to listen on. 0.0.0.0 means that data can be received on all local IPs.
	*@param port port to listen on
	*@param receiveFilter This allows you to decide which data type you want to receive. If you receive files it makes no sense to convert them into a string.
	*@param maxPacketSize sets the maximum UDP packet size. More than 65507 is not possible.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|UDP")
		void socketClientInitUDPReceiver(FString& connectionID, FString domainOrIP = "0.0.0.0", int32 port = 8888, EReceiveFilterClient receiveFilter = EReceiveFilterClient::E_SAB, int32 maxPacketSize = 65507);

	/**
	* A ConnectionID must be created first with "socketClientInitUDPReceiver". Messages and bytes can be sent to different hosts with the same ConnectionID.
	*@param DomainOrIP target IP or Domain
	*@param port target port
	*@param message String to send
	*@param addLineBreak add a line break at the end
	*@param uniqueID is optional and required when multiple connections to the same server (same ip and port) shall be established. You can use getUniquePlayerID
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|UDP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketClientSendUDP(FString domainOrIP, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak = true, FString connectionID = "");

	/**
	* Terminates an existing connection.
	*@param connectionID The ID to an existing connection.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|UDP")
		void closeSocketClientConnectionUDP(FString connectionID);

	/**
	* Useful if you want to attach events to a certain connection.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|UDP")
		void getUDPInitializationByConnectionID(FString connectionID, bool& found, USocketClientPluginUDPClient*& connection);


	UFUNCTION(BlueprintCallable, Category = "SocketClient|UDP")
		bool isUDPInitialized(FString connectionID);

	/**
	*Trying to determine the local IP. It uses a function in the engine that does not work on all devices. On Windows and Linux it seems to work very well. Very bad on Android. 0.0.0.0 will be returned if it doesn't work.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions")
		static FString getLocalIP();


	/**
	*UE4 uses different socket connections. When Steam is active, Steam Sockets are used for all connections. This leads to problems if you want to use Steam but not Steam Sockets. Therefore you can change the sockets to "System".
	*@param ESocketPlatformServer System = Windows on Windows, Mac = Mac on Mac ect.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions")
		static void changeSocketPlatform(ESocketPlatformClient platform);
	
	/**
	* Returns which system you are currently use. (Windows, OSX, IOS ...)
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions", Meta = (ExpandEnumAsExecs = "system"))
		static void getSystemType(ESocketClientSystem& system);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|SpecialFunctions")
		static int32 getUniquePlayerID(APlayerController* playerController = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|SpecialFunctions")
		static FString getRandomID();

	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|Hex")
		static TArray<uint8> parseHexToBytes(FString hex);

	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|Hex")
		static FString parseHexToString(FString hex);

	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|Hex")
		static FString parseBytesToHex(TArray<uint8> bytes);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|SpecialFunctions|Hex")
		static TArray<uint8> parseHexToBytesPure(FString hex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|SpecialFunctions|Hex")
		static FString parseHexToStringPure(FString hex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketClient|SpecialFunctions|Hex")
		static FString parseBytesToHexPure(TArray<uint8> bytes);


	static ISocketSubsystem* getSocketSubSystem();

	FString resolveDomain(FString domain);
	//ue4 domain resolve does not work with steam. this is my own dns client
	class UDNSClientSocketClient* dnsClient = nullptr;
	TMap<FString, FString> domainCache;

	void getTcpMessageWrapping(FString& header, FString& footer, bool& useWrapping);
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	void PushBuffer(FDataBufferStack InBuffer);
	FDateTime lastUpdateTime;
private:
	ESocketPlatformClient systemSocketPlatform;

	TMap<FString, USocketClientPluginTCPClient*> tcpClients;
	TMap<FString, USocketClientPluginUDPClient*> udpClients;

	FString tcpMessageHeader = "([{UE4-Head}])";
	FString tcpMessageFooter = "([{UE4-Foot}])";
	bool useTCPMessageWrapping = false;

	TArray<FDataBufferStack> BufferStack;;
	
};

