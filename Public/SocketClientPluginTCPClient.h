// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketClient.h"
#include "SocketClientPluginTCPClient.generated.h"


class USocketClientBPLibrary;
class FServerConnectionThread;
class FSendDataToServerThread;

UCLASS(Blueprintable, BlueprintType)
class USocketClientPluginTCPClient : public UObject
{
	GENERATED_UCLASS_BODY()

public:


	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTCPSocketClientConnectedCallBack, bool, FString,  FString);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTCPSocketClientReceiveCallBack, FString, const TArray<uint8>&, FString);
	//Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FsocketClientTCPConnectionEventDelegate, bool, success, FString, message, FString, clientConnectionID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FreceiveTCPMessageEventDelegate, FString, message, const TArray<uint8>&, byteArray, FString, clientConnectionID);

	UFUNCTION()
		void socketClientTCPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|TCP|Events|ConnectionInfo")
		FsocketClientTCPConnectionEventDelegate onsocketClientTCPConnectionEventDelegate;
	UFUNCTION()
		void receiveTCPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|TCP|Events|ReceiveMessage")
		FreceiveTCPMessageEventDelegate onreceiveTCPMessageEventDelegate;

	FTCPSocketClientConnectedCallBack OnTCPSocketClientConnectedCallBack;

	FTCPSocketClientReceiveCallBack OnTCPSocketClientReceiveCallBack;
	
	void connect(USocketClientBPLibrary* mainLib, FString domainOrIP, int32 port, EReceiveFilterClient receiveFilter, FString connectionID);
	void sendMessage(FString message, TArray<uint8> byteArray);
	void closeConnection();

	bool isRun();
	void setRun(bool runP);
	FString getConnectionID();

	void setSocket(FSocket* socket);
	FSocket* getSocket();

	void createSendThread();
	

private:
	bool run = false;
	FString connectionID;
	FString domainOrIP;
	int32 port;
	FSocket* socket = nullptr;

	FServerConnectionThread* tcpConnectionThread = nullptr;
	FSendDataToServerThread* tcpSendThread = nullptr;
	USocketClientBPLibrary* mainLib = nullptr;

};

/* asynchronous Thread*/
class FServerConnectionThread : public FRunnable {

public:

	FServerConnectionThread(USocketClientBPLibrary* socketClientP, FString clientConnectionIDP, EReceiveFilterClient receiveFilterP, FString ipOrDomainP, int32 portP, USocketClientPluginTCPClient* tcpClientP) :
		socketClient(socketClientP),
		clientConnectionID(clientConnectionIDP),
		receiveFilter(receiveFilterP),
		ipOrDomain(ipOrDomainP),
		port(portP),
		tcpClient(tcpClientP){
		FString threadName = "FServerConnectionThread" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override;


protected:
	USocketClientBPLibrary* socketClient = nullptr;
	//USocketClientBPLibrary*		oldClient;
	FString						clientConnectionID;
	FString						originalIP;
	EReceiveFilterClient		receiveFilter;
	FString ipOrDomain;
	int32 port;
	USocketClientPluginTCPClient* tcpClient = nullptr;
	FRunnableThread* thread = nullptr;
};




/* asynchronous Thread*/
class FSendDataToServerThread : public FRunnable {

public:

	FSendDataToServerThread(USocketClientBPLibrary* socketClientLibP, USocketClientPluginTCPClient* tcpClientP, FString clientConnectionIDP) :
		socketClientLib(socketClientLibP),
		tcpClient(tcpClientP),
		clientConnectionID(clientConnectionIDP) {
		FString threadName = "FSendDataToServerThread" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override ;

	void sendMessage(FString messageP, TArray<uint8> byteArrayP);

	void pauseThread(bool pause);


protected:
	TQueue<FString> messageQueue;
	TQueue<TArray<uint8>> byteArrayQueue;
	USocketClientBPLibrary* socketClientLib;
	USocketClientPluginTCPClient* tcpClient = nullptr;
	FString clientConnectionID;
	FRunnableThread* thread = nullptr;
	bool					run;
	bool					paused;
};