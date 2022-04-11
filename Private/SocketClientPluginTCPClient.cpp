// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#include "SocketClientPluginTCPClient.h"


USocketClientPluginTCPClient::USocketClientPluginTCPClient(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	//This prevents the garbage collector from killingand deleting the class from RAM.
	this->AddToRoot();
}


void USocketClientPluginTCPClient::socketClientTCPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionID){}
void USocketClientPluginTCPClient::receiveTCPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString clientConnectionID){}

void USocketClientPluginTCPClient::connect(USocketClientBPLibrary* mainLibP, FString domainOrIPP, int32 portP, EReceiveFilterClient receiveFilter, FString connectionIDP){
	mainLib = mainLibP;
	connectionID = connectionIDP;
	domainOrIP = domainOrIPP;
	port = portP;
	tcpConnectionThread = new FServerConnectionThread(mainLib, connectionID, receiveFilter, domainOrIP, port,this);
}

void USocketClientPluginTCPClient::sendMessage(FString message, TArray<uint8> byteArray){
	if (run && tcpSendThread != nullptr) {
		tcpSendThread->sendMessage(message, byteArray);
	}
}

void USocketClientPluginTCPClient::closeConnection(){
	setRun(false);
	tcpConnectionThread = nullptr;
	tcpSendThread = nullptr;
	mainLib = nullptr;
}

bool USocketClientPluginTCPClient::isRun(){
	return run;
}

void USocketClientPluginTCPClient::setRun(bool runP) {
	run = runP;
}

FString USocketClientPluginTCPClient::getConnectionID(){
	return connectionID;
}

void USocketClientPluginTCPClient::setSocket(FSocket* socketP){
	socket = socketP;
}

FSocket* USocketClientPluginTCPClient::getSocket(){
	return socket;
}

void USocketClientPluginTCPClient::createSendThread(){
	tcpSendThread = new FSendDataToServerThread(mainLib, this, connectionID);
}

uint32 FServerConnectionThread::Run()
{
	//UE_LOG(LogTemp, Display, TEXT("DoWork:%s"),*(FDateTime::Now()).ToString());
		FString ip = socketClient->resolveDomain(ipOrDomain);
		int32 portGlobal = port;
		FString clientConnectionIDGlobal = clientConnectionID;
		USocketClientBPLibrary* socketClientGlobal = socketClient;
		USocketClientPluginTCPClient* tcpClientGlobal = tcpClient;

		//message wrapping
		FString tcpMessageHeader;
		FString tcpMessageFooter;
		bool useTCPMessageWrapping;

		socketClient->getTcpMessageWrapping(tcpMessageHeader, tcpMessageFooter, useTCPMessageWrapping);

		FString tcpMessageFooterLineBreak = tcpMessageFooter+"\r\n";
		FString tcpMessageFooterLineBreak2 = tcpMessageFooter + "\r";;


		//UE_LOG(LogTemp, Warning, TEXT("Tread:%s:%i"),*ip, port);
		ISocketSubsystem* sSS = USocketClientBPLibrary::getSocketSubSystem();
		if (sSS == nullptr) {
			const TCHAR* socketErr = sSS->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [ip, portGlobal, clientConnectionIDGlobal, socketClientGlobal, tcpClientGlobal, socketErr]() {
				if (socketClientGlobal != nullptr)
				{
					socketClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection failed(1). SocketSubSystem does not exist:" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					socketClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection failed(1). SocketSubSystem does not exist:" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
				}
					
				if (tcpClientGlobal != nullptr)
				{
					tcpClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection failed(1). SocketSubSystem does not exist:" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					tcpClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection failed(1). SocketSubSystem does not exist:" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
				}
					
				});
			return 0;
		}
		TSharedRef<FInternetAddr> addr = sSS->CreateInternetAddr();
		bool bIsValid;
		addr->SetIp(*ip, bIsValid);
		addr->SetPort(port);

		if (bIsValid) {
			// create the socket
			FSocket* socket = sSS->CreateSocket(NAME_Stream, TEXT("socketClient"), addr->GetProtocolType());
			tcpClient->setSocket(socket);
;

			// try to connect to the server
			if (socket == nullptr || socket->Connect(*addr) == false) {
				const TCHAR* socketErr = sSS->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [ip, portGlobal, clientConnectionIDGlobal, socketClientGlobal, tcpClientGlobal, socketErr]() {
					if (socketClientGlobal != nullptr)
					{
						socketClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection failed(2):" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						socketClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection failed(2):" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}
						
					if (tcpClientGlobal != nullptr)
					{
						tcpClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection failed(2):" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						tcpClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection failed(2):" + FString(socketErr) + "|" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}
						
					});
			}
			else {
				AsyncTask(ENamedThreads::GameThread, [ip, portGlobal, clientConnectionIDGlobal, tcpClientGlobal, socketClientGlobal]() {
					if (socketClientGlobal != nullptr)
					{
						socketClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(true, "Connection successful:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						socketClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(true, "Connection successful:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}
						
					if (tcpClientGlobal != nullptr)
					{
						tcpClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(true, "Connection successful:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						tcpClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(true, "Connection successful:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}
						
					});
				tcpClient->setRun(true);
				tcpClient->createSendThread();
				int64 ticks1;
				int64 ticks2;
				TArray<uint8> byteDataArray;
				FString mainMessage;
				bool inCollectMessageStatus = false;

				uint32 DataSize;
				while (socket != nullptr && tcpClient->isRun()) {

					//ESocketConnectionState::SCS_Connected does not work https://issues.unrealengine.com/issue/UE-27542
					//Compare ticks is a workaround to get a disconnect. clientSocket->Wait() stop working after disconnect. (Another bug?)
					//If it doesn't wait any longer, ticks1 and ticks2 should be the same == disconnect.
					ticks1 = FDateTime::Now().GetTicks();
					socket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan::FromSeconds(0.1));
					ticks2 = FDateTime::Now().GetTicks();

					bool hasData = socket->HasPendingData(DataSize);
					if (!hasData && ticks1 == ticks2) {
						UE_LOG(LogTemp, Display, TEXT("TCP connection broken. End Loop"));
						break;
					}

					if (hasData) {
						FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
						Datagram->SetNumUninitialized(DataSize);
						int32 BytesRead = 0;
						if (socket->Recv(Datagram->GetData(), Datagram->Num(), BytesRead)) {

							if (useTCPMessageWrapping) {
								if (receiveFilter == EReceiveFilterClient::E_SAB || receiveFilter == EReceiveFilterClient::E_S) {
									char* Data = (char*)Datagram->GetData();
									Data[BytesRead] = '\0';

									FString recvMessage = FString(UTF8_TO_TCHAR(Data));
									if (recvMessage.StartsWith(tcpMessageHeader)) {
										inCollectMessageStatus = true;
										recvMessage.RemoveFromStart(tcpMessageHeader);
									}
									if (recvMessage.EndsWith(tcpMessageFooter) || recvMessage.EndsWith(tcpMessageFooterLineBreak) || recvMessage.EndsWith(tcpMessageFooterLineBreak2)) {
										inCollectMessageStatus = false;
										if (!recvMessage.RemoveFromEnd(tcpMessageFooter)) {
											if (!recvMessage.RemoveFromEnd(tcpMessageFooterLineBreak)) {
												if (recvMessage.RemoveFromEnd(tcpMessageFooterLineBreak2)) {
													recvMessage.Append("\r");
												}
											}
											else {
												recvMessage.Append("\r\n");
											}
										}

										//splitt merged messages
										if (recvMessage.Contains(tcpMessageHeader)) {
											TArray<FString> lines;
											int32 lineCount = recvMessage.ParseIntoArray(lines, *tcpMessageHeader, true);
											for (int32 i = 0; i < lineCount; i++) {
												mainMessage = lines[i];
												if (mainMessage.EndsWith(tcpMessageFooter) || mainMessage.EndsWith(tcpMessageFooterLineBreak) || mainMessage.EndsWith(tcpMessageFooterLineBreak2)) {
													if (!mainMessage.RemoveFromEnd(tcpMessageFooter)) {
														if (!mainMessage.RemoveFromEnd(tcpMessageFooterLineBreak)) {
															if (mainMessage.RemoveFromEnd(tcpMessageFooterLineBreak2)) {
																mainMessage.Append("\r");
															}
														}
														else {
															mainMessage.Append("\r\n");
														}
													}
												}

												//switch to gamethread
												AsyncTask(ENamedThreads::GameThread, [mainMessage, byteDataArray, clientConnectionIDGlobal, tcpClientGlobal, socketClientGlobal]() {
													if (socketClientGlobal != nullptr)
													{
														socketClientGlobal->onreceiveTCPMessageEventDelegate.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
														socketClientGlobal->OnTCPSocketClientReceiveCallBack.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
													}
													
													if (tcpClientGlobal != nullptr)
													{
														tcpClientGlobal->onreceiveTCPMessageEventDelegate.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
														tcpClientGlobal->OnTCPSocketClientReceiveCallBack.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
													}
													});

												Datagram->Empty();
												mainMessage.Empty();
											}
											continue;
										}
										else {
											mainMessage.Append(recvMessage);
										}


									}
									if (inCollectMessageStatus) {
										mainMessage.Append(recvMessage);
										continue;
									}
									if (mainMessage.IsEmpty()) {
										continue;
									}

								}

							}
							else {

								if (receiveFilter == EReceiveFilterClient::E_SAB || receiveFilter == EReceiveFilterClient::E_S) {
									char* Data = (char*)Datagram->GetData();
									Data[BytesRead] = '\0';
									mainMessage = FString(UTF8_TO_TCHAR(Data));
								}
								
								if (receiveFilter == EReceiveFilterClient::E_SAB || receiveFilter == EReceiveFilterClient::E_B) {
									byteDataArray.Append(Datagram->GetData(), Datagram->Num());
								}
							}



							

							//switch to gamethread
							AsyncTask(ENamedThreads::GameThread, [mainMessage, byteDataArray, clientConnectionIDGlobal, tcpClientGlobal, socketClientGlobal]() {
								if (socketClientGlobal != nullptr)
								{
									socketClientGlobal->onreceiveTCPMessageEventDelegate.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
									socketClientGlobal->OnTCPSocketClientReceiveCallBack.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
								}
								if (tcpClientGlobal != nullptr)
								{
									tcpClientGlobal->onreceiveTCPMessageEventDelegate.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
									tcpClientGlobal->OnTCPSocketClientReceiveCallBack.Broadcast(mainMessage, byteDataArray, clientConnectionIDGlobal);
								}
								});
						}
						mainMessage.Empty();
						byteDataArray.Empty();
						Datagram->Empty();
					}
				}


				AsyncTask(ENamedThreads::GameThread, [ip, portGlobal, clientConnectionIDGlobal, tcpClientGlobal, socketClientGlobal]() {
					if (socketClientGlobal != nullptr)
					{
						socketClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection close:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						socketClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection close:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}
					if (tcpClientGlobal != nullptr)
					{
						tcpClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection close:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
						tcpClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection close:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					}

				});
			}

			//wait for game thread
			//FPlatformProcess::Sleep(1);

			if (tcpClient->isRun()) {
				USocketClientBPLibrary::getSocketClientTarget()->closeSocketClientConnectionTCP(clientConnectionID);
			}

			
			tcpClient->setRun(false);
			if (socket != nullptr) {
				socket->Close();
				socket = nullptr;
				sSS->DestroySocket(socket);
			}
			thread = nullptr;
		}
		else {
			AsyncTask(ENamedThreads::GameThread, [ip, portGlobal, clientConnectionIDGlobal, tcpClientGlobal, socketClientGlobal]() {
				if (socketClientGlobal != nullptr)
				{
					socketClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection failed(3). IP not valid:" + ip+ ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					socketClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection failed(3). IP not valid:" + ip+ ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
				}
				if (tcpClientGlobal != nullptr)
				{
					tcpClientGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection failed(3). IP not valid:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
					tcpClientGlobal->OnTCPSocketClientConnectedCallBack.Broadcast(false, "Connection failed(3). IP not valid:" + ip + ":" + FString::FromInt(portGlobal) + "|" + clientConnectionIDGlobal, clientConnectionIDGlobal);
				}
				});
		}

		return 0;
}

uint32 FSendDataToServerThread::Run()
{
	if (tcpClient == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Class is not initialized."));
			return 0;
		}

		/*if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, TEXT("tcp socket 1"));*/

		FString clientConnectionIDGlobal = clientConnectionID;
		//USocketClientPluginTCPClient* tcpClientGlobal = tcpClient;
		USocketClientBPLibrary* socketClientLibGlobal = socketClientLib;

		// get the socket
		FSocket* socket = tcpClient->getSocket();

		while (tcpClient->isRun()) {

			// try to connect to the server
			if (socket == NULL || socket == nullptr) {
				UE_LOG(LogTemp, Error, TEXT("Connection not exist."));
				//AsyncTask(ENamedThreads::GameThread, [clientConnectionIDGlobal, socketClientLibGlobal]() {
				//	if (socketClientLibGlobal != nullptr) {
				//		socketClientLibGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection not exist:" + clientConnectionIDGlobal, clientConnectionIDGlobal);
				//		socketClientLibGlobal->closeSocketClientConnection();
				//	}
				//});
				break;
			}

			if (socket != nullptr && socket->GetConnectionState() == ESocketConnectionState::SCS_Connected) {
				while (messageQueue.IsEmpty() == false) {
					FString m;
					messageQueue.Dequeue(m);
					FTCHARToUTF8 Convert(*m);
					int32 sent = 0;
					socket->Send((uint8*)((ANSICHAR*)Convert.Get()), Convert.Length(), sent);
				}

				while (byteArrayQueue.IsEmpty() == false) {
					TArray<uint8> ba;
					byteArrayQueue.Dequeue(ba);
					int32 sent = 0;
					socket->Send(ba.GetData(), ba.Num(), sent);
					ba.Empty();
				}

			}
			//else {
			//	UE_LOG(LogTemp, Error, TEXT("Connection Lost"));
			//	AsyncTask(ENamedThreads::GameThread, [clientConnectionIDGlobal, socketClientLibGlobal]() {
			//		if (socketClientLibGlobal != nullptr)
			//			socketClientLibGlobal->onsocketClientTCPConnectionEventDelegate.Broadcast(false, "Connection lost:" + clientConnectionIDGlobal, clientConnectionIDGlobal);
			//		});
			//}
			if (tcpClient->isRun()) {
				pauseThread(true);
				//workaround. suspend do not work on all platforms. lets sleep
				while (paused && tcpClient->isRun()) {
					FPlatformProcess::Sleep(0.01);
				}
			}
		}
		thread = nullptr;
		return 0;
}

void FSendDataToServerThread::sendMessage(FString messageP, TArray<uint8> byteArrayP)
{
	if (messageP.Len() > 0)
		messageQueue.Enqueue(messageP);
	if (byteArrayP.Num() > 0)
		byteArrayQueue.Enqueue(byteArrayP);
	pauseThread(false);
}

void FSendDataToServerThread::pauseThread(bool pause)
{
	paused = pause;
	if (thread != nullptr)
		thread->Suspend(pause);
}


