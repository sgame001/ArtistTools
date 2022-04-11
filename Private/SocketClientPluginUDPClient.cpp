// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#include "SocketClientPluginUDPClient.h"


USocketClientPluginUDPClient::USocketClientPluginUDPClient(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	this->AddToRoot();
}



void USocketClientPluginUDPClient::socketClientUDPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionIDP){}
void USocketClientPluginUDPClient::receiveUDPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString IPP, const int32 portP, const FString clientConnectionIDP) {}

void USocketClientPluginUDPClient::init(USocketClientBPLibrary* socketClientLibP, FString domainOrIPP, int32 portP, EReceiveFilterClient receiveFilterP, FString connectionIDP, 
	int32 maxPacketSizeP) {
	socketClientBPLibrary = socketClientLibP;
	receiveFilter = receiveFilterP;
	connectionID = connectionIDP;
	domainOrIP = domainOrIPP;
	port = portP;
	maxPacketSize = maxPacketSizeP;
	if (maxPacketSize < 1 || maxPacketSize > 65507)
		maxPacketSize = 65507;

	UDPThread = new FServerUDPConnectionThread(this, socketClientLibP, domainOrIPP, portP);
}


void USocketClientPluginUDPClient::sendUDPMessage(FString domainOrIPP, int32 portP, FString message, TArray<uint8> byteArray, FString clientConnectionID){
	if (UDPSendThread != nullptr) {
		UDPSendThread->sendMessage(message, byteArray, domainOrIPP, portP);
	}
}

void USocketClientPluginUDPClient::closeUDPConnection() {

	if (udpSocketReceiver != nullptr) {
		udpSocketReceiver->Stop();
		udpSocketReceiver = nullptr;
	}

	setRun(false);
	UDPThread = nullptr;
	if (UDPSendThread != nullptr) {
		UDPSendThread->pauseThread(false);
	}

	UDPSendThread = nullptr;
}


void USocketClientPluginUDPClient::UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt) {

	if (FSocketClientModule::isShuttingDown)
		return;

	TSharedPtr<FInternetAddr> peerAddr = EndPt.ToInternetAddr();
	FString ipGlobal = peerAddr->ToString(false);
	int32 portGlobal = peerAddr->GetPort();

	FString recvMessage;
	if (receiveFilter == EReceiveFilterClient::E_SAB || receiveFilter == EReceiveFilterClient::E_S) {
		char* Data = (char*)ArrayReaderPtr->GetData();
		Data[ArrayReaderPtr->Num()] = '\0';
		recvMessage = FString(UTF8_TO_TCHAR(Data));
	}

	TArray<uint8> byteArray;
	if (receiveFilter == EReceiveFilterClient::E_SAB || receiveFilter == EReceiveFilterClient::E_B) {
		byteArray.Append(ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());
	}

	//switch to gamethread
	USocketClientBPLibrary* socketClientBPLibraryGlobal = socketClientBPLibrary;
	USocketClientPluginUDPClient* udpClientGlobal = this;
	FString clientConnectionIDGlobal = connectionID;
	AsyncTask(ENamedThreads::GameThread, [udpClientGlobal, recvMessage, byteArray, ipGlobal, portGlobal, socketClientBPLibraryGlobal, clientConnectionIDGlobal]() {
		if (FSocketClientModule::isShuttingDown)
			return;
		socketClientBPLibraryGlobal->onreceiveUDPMessageEventDelegate.Broadcast(recvMessage, byteArray, ipGlobal, portGlobal, clientConnectionIDGlobal);
		udpClientGlobal->onreceiveUDPMessageEventDelegate.Broadcast(recvMessage, byteArray, ipGlobal, portGlobal, clientConnectionIDGlobal);
	});
}

bool USocketClientPluginUDPClient::isRun(){
	return run;
}

void USocketClientPluginUDPClient::setRun(bool runP){
	run = runP;
}

FSocket* USocketClientPluginUDPClient::getSocket(){
	return socket;
}

void USocketClientPluginUDPClient::setSocket(FSocket* socketP){
	socket = socketP;
}

void USocketClientPluginUDPClient::setUDPSocketReceiver(FUdpSocketReceiver* udpSocketReceiverP){
	udpSocketReceiver = udpSocketReceiverP;
}

FString USocketClientPluginUDPClient::getIP(){
	return domainOrIP;
}

void USocketClientPluginUDPClient::setIP(FString ipP){
	domainOrIP = ipP;
}

int32 USocketClientPluginUDPClient::getPort(){
	return port;
}

FString USocketClientPluginUDPClient::getDomainOrIP(){
	return domainOrIP;
}

FString USocketClientPluginUDPClient::getConnectionID(){
	return connectionID;
}

void USocketClientPluginUDPClient::setUDPSendThread(FServerUDPSendMessageThread* udpSendThreadP){
	UDPSendThread = udpSendThreadP;
}

int32 USocketClientPluginUDPClient::getMaxPacketSize()
{
	return maxPacketSize;
}

uint32 FServerUDPSendMessageThread::Run()
{
	FSocket* socket = udpClient->getSocket();
		FString connectionID = udpClient->getConnectionID();
		maxPacketSize = udpClient->getMaxPacketSize();
		
		while (udpClient->isRun() && socket != nullptr) {


			if (udpClient->isRun() && (messageQueue.IsEmpty() == false || byteArrayQueue.IsEmpty() == false)) {

				TSharedRef<FInternetAddr> addr = USocketClientBPLibrary::getSocketSubSystem()->CreateInternetAddr();
				bool bIsValid;
				addr->SetIp(*sendToip, bIsValid);
				addr->SetPort(sendToport);
				int32 sent = 0;
				TArray<uint8> byteArray;
				if (bIsValid) {
					while (messageQueue.IsEmpty() == false) {
						FString m;
						messageQueue.Dequeue(m);
						FTCHARToUTF8 Convert(*m);
						byteArray.Append((uint8*)Convert.Get(), Convert.Length());
						sendBytes(socket, byteArray, sent, addr);
					}

					while (byteArrayQueue.IsEmpty() == false) {
						byteArrayQueue.Dequeue(byteArray);
						sendBytes(socket, byteArray, sent, addr);
					}

				}
				else {
					UE_LOG(LogTemp, Error, TEXT("Can't send to %s:%i"), *sendToip, sendToport);
				}

			}

			if (udpClient->isRun()) {
				pauseThread(true);
				//workaround. suspend do not work on all platforms. lets sleep
				while (paused && udpClient->isRun()) {
					FPlatformProcess::Sleep(0.01);
				}
			}
		}

		if (socket != nullptr) {
			socket->Close();
			socket = nullptr;
			udpClient->setSocket(nullptr);
		}


		USocketClientBPLibrary* socketClientTMP = socketClient;
		USocketClientPluginUDPClient* udpClientGlobal = udpClient;
		FString ipGlobal = mySocketip;
		int32 portGlobal = mySocketport;
		AsyncTask(ENamedThreads::GameThread, [socketClientTMP, udpClientGlobal, ipGlobal, portGlobal, connectionID]() {
			if (socketClientTMP != nullptr)
				socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "UDP connection closed. " + ipGlobal + ":" + FString::FromInt(portGlobal), connectionID);
			if (udpClientGlobal != nullptr)
				udpClientGlobal->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "UDP connection closed. " + ipGlobal + ":" + FString::FromInt(portGlobal), connectionID);
		});


		thread = nullptr;
		return 0;
}

void FServerUDPSendMessageThread::sendMessage(FString messageP, TArray<uint8> byteArrayP, FString ip, int32 port)
{
	sendToip = socketClient->resolveDomain(ip);
	sendToport = port;
	if (messageP.Len() > 0)
		messageQueue.Enqueue(messageP);
	if (byteArrayP.Num() > 0)
		byteArrayQueue.Enqueue(byteArrayP);
	pauseThread(false);
}

void FServerUDPSendMessageThread::sendBytes(FSocket*& socketP, TArray<uint8>& byteArray, int32& sent,
                                            TSharedRef<FInternetAddr>& addr)
{
	if (byteArray.Num() > maxPacketSize) {
		TArray<uint8> byteArrayTemp;
		for (int32 i = 0; i < byteArray.Num(); i++) {
			byteArrayTemp.Add(byteArray[i]);
			if (byteArrayTemp.Num() == maxPacketSize) {
				sent = 0;
				socketP->SendTo(byteArrayTemp.GetData(), byteArrayTemp.Num(), sent, *addr);
				byteArrayTemp.Empty();
			}
		}
		if (byteArrayTemp.Num() > 0) {
			sent = 0;
			socketP->SendTo(byteArrayTemp.GetData(), byteArrayTemp.Num(), sent, *addr);
			byteArrayTemp.Empty();
		}
	}
	else {
		sent = 0;
		socketP->SendTo(byteArray.GetData(), byteArray.Num(), sent, *addr);
	}

	byteArray.Empty();
}

uint32 FServerUDPConnectionThread::Run()
{
	USocketClientPluginUDPClient* udpClientGlobal = udpClient;
	FString ip = socketClient->resolveDomain(ipGlobal);
	udpClient->setIP(ip);
	int32 port = portGlobal;
	FString connectionID = udpClient->getConnectionID();
			

	if (socket == nullptr || socket == NULL) {

		USocketClientBPLibrary* socketClientTMP = socketClient;

		FString endpointAdress = ip + ":" + FString::FromInt(port);
		FIPv4Endpoint Endpoint;

		// create the socket
		FString socketName;
		ISocketSubsystem* socketSubsystem = USocketClientBPLibrary::getSocketSubSystem();

		TSharedPtr<class FInternetAddr> addr = socketSubsystem->CreateInternetAddr();
		bool validIP = true;
		addr->SetPort(port);
		addr->SetIp(*ip, validIP);


		if (!validIP) {
			UE_LOG(LogTemp, Error, TEXT("SocketClient UDP. Can't set ip"));
			AsyncTask(ENamedThreads::GameThread, [udpClientGlobal,socketClientTMP, addr, connectionID]() {
				if (socketClientTMP != nullptr)
					socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "SocketClient UDP. Can't set ip", connectionID);
				if (udpClientGlobal != nullptr)
					udpClientGlobal->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "SocketClient UDP. Can't set ip", connectionID);
				});
			thread = nullptr;
			return 0;
		}

		socket = socketSubsystem->CreateSocket(NAME_DGram, *socketName, addr->GetProtocolType());


		if (socket == nullptr || socket == NULL) {
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			UE_LOG(LogTemp, Error, TEXT("UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router. %s:%i. Error: %s"), *ip, port, SocketErr);
			AsyncTask(ENamedThreads::GameThread, [udpClientGlobal,socketClientTMP, addr, SocketErr, connectionID]() {
				if (socketClientTMP != nullptr)
					socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 0) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr, connectionID);
				if (udpClientGlobal != nullptr)
					udpClientGlobal->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 0) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr, connectionID);
			});
			thread = nullptr;
			return 0;
		}

		if (!socket->SetRecvErr()) {
			UE_LOG(LogTemp, Error, TEXT("SocketClient UDP. Can't set recverr"));
		}


		if (socket == nullptr || socket == NULL || !validIP) {
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			UE_LOG(LogTemp, Error, TEXT("UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router. %s:%i. Error: %s"), *ip, port, SocketErr);
			AsyncTask(ENamedThreads::GameThread, [udpClientGlobal,socketClientTMP, addr, SocketErr, connectionID]() {
				if (socketClientTMP != nullptr)
					socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 1) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr, connectionID);
				if (udpClientGlobal != nullptr)
					udpClientGlobal->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 1) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr, connectionID);
				});
			thread = nullptr;
			return 0;
		}
		socket->SetReuseAddr(true);
		socket->SetNonBlocking(true);
		socket->SetBroadcast(true);

		if (!socket->Bind(*addr)) {
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			UE_LOG(LogTemp, Error, TEXT("UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router. %s:%i. Error: %s"), *ip, port, SocketErr);

			AsyncTask(ENamedThreads::GameThread, [udpClientGlobal,socketClientTMP, addr, SocketErr, connectionID]() {
				if (socketClientTMP != nullptr)
					socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 2) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr, connectionID);
				if (udpClientGlobal != nullptr)
					udpClientGlobal->onsocketClientUDPConnectionEventDelegate.Broadcast(false, "(Error 2) UE4 could not init a UDP socket. You can only create listening connections on local IPs. An external IP must be redirected to a local IP in your router." + addr->ToString(true) + " Error:" + SocketErr, connectionID);
				});
			thread = nullptr;
			return 0;
		}



		FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
		FUdpSocketReceiver* udpSocketReceiver = new FUdpSocketReceiver(socket, ThreadWaitTime, TEXT("SocketClientBPLibUDPReceiverThread"));
		udpSocketReceiver->OnDataReceived().BindUObject(udpClient, &USocketClientPluginUDPClient::UDPReceiver);
		udpSocketReceiver->Start();
		udpClient->setUDPSocketReceiver(udpSocketReceiver);

		udpClient->setSocket(socket);
		udpClient->setRun(true);
		udpClient->setUDPSendThread(new FServerUDPSendMessageThread(udpClient, socketClient,ip,port));


		AsyncTask(ENamedThreads::GameThread, [udpClientGlobal,socketClientTMP, addr, connectionID]() {
			if (socketClientTMP != nullptr)
				socketClientTMP->onsocketClientUDPConnectionEventDelegate.Broadcast(true, "Init UDP Connection OK. " + addr->ToString(true), connectionID);
			if (udpClientGlobal != nullptr)
				udpClientGlobal->onsocketClientUDPConnectionEventDelegate.Broadcast(true, "Init UDP Connection OK. " + addr->ToString(true), connectionID);
		});

	}
	thread = nullptr;
	return 0;
}
