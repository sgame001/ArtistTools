// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketClient.h"
#include "SocketClientPluginUDPClient.generated.h"


class USocketClientBPLibrary;
class FServerUDPConnectionThread;
class FServerUDPSendMessageThread;


UCLASS(Blueprintable, BlueprintType)
class USocketClientPluginUDPClient : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	//Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FsocketClientUDPConnectionEventDelegate, bool, success, FString, message, FString, clientConnectionID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FreceiveUDPMessageEventDelegate, FString, message, const TArray<uint8>&, byteArray, FString, IP_FromSender, int32, portFromSender, FString, clientConnectionID);

	UFUNCTION()
		void socketClientUDPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|UDP|Events|ConnectionInfo")
		FsocketClientUDPConnectionEventDelegate onsocketClientUDPConnectionEventDelegate;
	UFUNCTION()
		void receiveUDPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const  FString IP, const int32 port, const FString clientConnectionID);
	UPROPERTY(BlueprintAssignable, Category = "SocketClient|UDP|Events|ReceiveMessage")
		FreceiveUDPMessageEventDelegate onreceiveUDPMessageEventDelegate;

	void init(USocketClientBPLibrary* socketClientLibP, FString domain, int32 port, EReceiveFilterClient receiveFilter, FString clientConnectionID, int32 maxPacketSize = 65507);
	void sendUDPMessage(FString domainOrIP, int32 port, FString message, TArray<uint8> byteArray, FString clientConnectionID);
	void closeUDPConnection();
	void UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);


	bool isRun();
	void setRun(bool runP);

	FSocket* getSocket();
	void setSocket(FSocket* socketP);

	void setUDPSocketReceiver(FUdpSocketReceiver* udpSocketReceiver);

	FString getIP();
	void setIP(FString ipP);
	int32 getPort();
	FString getDomainOrIP();

	FString getConnectionID();
	void setUDPSendThread(FServerUDPSendMessageThread* udpSendThreadP);

	int32 getMaxPacketSize();

private:
	bool run = false;
	EReceiveFilterClient receiveFilter;
	FString connectionID;
	FString domainOrIP;
	int32 port;
	int32 maxPacketSize = 65507;

	USocketClientBPLibrary* socketClientBPLibrary = nullptr;

	FUdpSocketReceiver* udpSocketReceiver = nullptr;
	FSocket* socket = nullptr;
	FServerUDPConnectionThread* UDPThread = nullptr;
	FServerUDPSendMessageThread* UDPSendThread = nullptr;
};



/* asynchronous Threads*/
class FServerUDPSendMessageThread : public FRunnable {

public:

	FServerUDPSendMessageThread(USocketClientPluginUDPClient* udpClientP, USocketClientBPLibrary* socketClientP, FString mySocketipP, int32 mySocketportP) :
		udpClient(udpClientP),
		socketClient(socketClientP),
		mySocketip(mySocketipP),
		mySocketport(mySocketportP) {
		FString threadName = "FServerUDPSendMessageThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override ;


	void sendMessage(FString messageP, TArray<uint8> byteArrayP, FString ip, int32 port);

	void pauseThread(bool pause) {
		paused = pause;
		if (thread != nullptr)
			thread->Suspend(pause);
	}

	void sendBytes(FSocket*& socketP, TArray<uint8>& byteArray, int32& sent, TSharedRef<FInternetAddr>& addr);

protected:
	USocketClientPluginUDPClient* udpClient = nullptr;
	USocketClientBPLibrary* socketClient = nullptr;
	FString mySocketip;
	int32 mySocketport;
	FString sendToip;
	int32 sendToport;
	FRunnableThread* thread = nullptr;
	bool					paused;
	TQueue<FString> messageQueue;
	TQueue<TArray<uint8>> byteArrayQueue;
	int32 maxPacketSize = 65507;
};


class FServerUDPConnectionThread : public FRunnable {

public:

	FServerUDPConnectionThread(USocketClientPluginUDPClient* udpClientP, USocketClientBPLibrary* socketClientP, FString ipP, int32 portP) :
		udpClient(udpClientP),
		socketClient(socketClientP),
		ipGlobal(ipP),
		portGlobal(portP) {
		FString threadName = "FServerUDPConnectionThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override;




protected:
	USocketClientPluginUDPClient* udpClient = nullptr;
	USocketClientBPLibrary* socketClient = nullptr;
	FRunnableThread* thread = nullptr;
	FString					ipGlobal;
	int32					portGlobal;
	FSocket* socket = nullptr;
	bool reuseSocket = false;
};
