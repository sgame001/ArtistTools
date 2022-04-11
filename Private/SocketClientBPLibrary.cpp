// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#include "SocketClientBPLibrary.h"

#include "DNSClientSocketClient.h"
#include "SocketClient.h"

USocketClientBPLibrary* USocketClientBPLibrary::socketClientBPLibrary;

USocketClientBPLibrary::USocketClientBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {

	socketClientBPLibrary = this;
	/*if (USocketClientHandler::socketClientHandler->getSocketClientTarget() == nullptr) {
		USocketClientHandler::socketClientHandler->addTCPClientToMap("0.0.0.0",0,this);
	 }*/

	if (!FSocketClientModule::isShuttingDown) {
		//Delegates
		onsocketClientTCPConnectionEventDelegate.AddDynamic(this, &USocketClientBPLibrary::socketClientTCPConnectionEventDelegate);
		onreceiveTCPMessageEventDelegate.AddDynamic(this, &USocketClientBPLibrary::receiveTCPMessageEventDelegate);
		onsocketClientUDPConnectionEventDelegate.AddDynamic(this, &USocketClientBPLibrary::socketClientUDPConnectionEventDelegate);
		onreceiveUDPMessageEventDelegate.AddDynamic(this, &USocketClientBPLibrary::receiveUDPMessageEventDelegate);
	}
}

/*Delegate functions*/
void USocketClientBPLibrary::socketClientTCPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionIDP) {}
void USocketClientBPLibrary::receiveTCPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const FString clientConnectionIDP) {}
void USocketClientBPLibrary::socketClientUDPConnectionEventDelegate(const bool success, const FString message, const FString clientConnectionID) {}
void USocketClientBPLibrary::receiveUDPMessageEventDelegate(const FString message, const TArray<uint8>& byteArray, const  FString IP, const int32 port,const FString clientConnectionID) {}


FString USocketClientBPLibrary::getLocalIP() {
	bool canBind = false;
	TSharedRef<FInternetAddr> localIp = USocketClientBPLibrary::getSocketClientTarget()->getSocketSubSystem()->GetLocalHostAddr(*GLog, canBind);

	if (localIp->IsValid()) {
		FString localIP = localIp->ToString(false);
		if (localIP.Equals("127.0.0.1")) {
			UE_LOG(LogTemp, Error, TEXT("Could not detect the local IP."));
			return "0.0.0.0";
		}
		return localIp->ToString(false);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Could not detect the local IP."));
	}
	return "0.0.0.0";
}


USocketClientBPLibrary::~USocketClientBPLibrary() {
	
}

USocketClientBPLibrary * USocketClientBPLibrary::getSocketClientTarget(){
	return socketClientBPLibrary;
}

void USocketClientBPLibrary::connectSocketClientTCP(FString domain, int32 port, EReceiveFilterClient receiveFilter, FString& connectionID) {

	USocketClientPluginTCPClient* tcpClient = NewObject<USocketClientPluginTCPClient>(USocketClientPluginTCPClient::StaticClass());
	connectionID = FGuid::NewGuid().ToString();
	tcpClients.Add(connectionID, tcpClient);
	tcpClient->connect(this, domain, port, receiveFilter, connectionID);
	lastUpdateTime = FDateTime::Now();
}

void USocketClientBPLibrary::socketClientSendTCP(FString connectionID,FString message, TArray<uint8> byteArray, bool addLineBreak) {

	if (connectionID.IsEmpty() || tcpClients.Find(connectionID) == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Connection not found (socketClientSendTCPMessage). %s"), *connectionID);
		return;
	}

	if (message.Len() > 0) {
		if (useTCPMessageWrapping) {
			message = tcpMessageHeader + message + tcpMessageFooter;
		}
		if (addLineBreak) {
			message.Append("\r\n");
		}
	}

	USocketClientPluginTCPClient* tcpClient = *tcpClients.Find(connectionID);
	tcpClient->sendMessage(message, byteArray);

}


void USocketClientBPLibrary::closeSocketClientConnectionTCP(FString connectionID){
	if (connectionID.IsEmpty() || tcpClients.Find(connectionID) == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Connection not found (closeSocketClientTCPConnection). %s"), *connectionID);
		return;
	}

	USocketClientPluginTCPClient* tcpClient = *tcpClients.Find(connectionID);
	tcpClient->closeConnection();
	tcpClients.Remove(connectionID);
	tcpClient = nullptr;
}

void USocketClientBPLibrary::getTCPConnectionByConnectionID(FString connectionID, bool& found, USocketClientPluginTCPClient* &connection){
	if (connectionID.IsEmpty() || tcpClients.Find(connectionID) == nullptr) {
		found = false;
		connection = nullptr;
		return;
	}

	found = true;
	connection = *tcpClients.Find(connectionID);
}

bool USocketClientBPLibrary::isTCPConnected(FString connectionID){
	if (connectionID.IsEmpty() || tcpClients.Find(connectionID) == nullptr) {
		return false;
	}

	return (*tcpClients.Find(connectionID))->isRun();
}

void USocketClientBPLibrary::activateTCPMessageWrappingOnClientPlugin(FString header, FString footer){
	tcpMessageHeader = header;
	tcpMessageFooter = footer;
	useTCPMessageWrapping = true;
}

void USocketClientBPLibrary::deactivateTCPMessageWrappingOnClientPlugin(){
	useTCPMessageWrapping = true;
}

void USocketClientBPLibrary::getTcpMessageWrapping(FString& header, FString& footer, bool& useWrapping) {
	header = tcpMessageHeader;
	footer = tcpMessageFooter;
	useWrapping = useTCPMessageWrapping;
}

TStatId USocketClientBPLibrary::GetStatId() const
{
	return TStatId();
}

void USocketClientBPLibrary::PushBuffer(FDataBufferStack InBuffer)
{
	BufferStack.Push(InBuffer);
}


void USocketClientBPLibrary::Tick(float DeltaSeconds)
{
	if(BufferStack.Num() > 0)
	{

		//if((FDateTime::Now() - lastUpdateTime).GetSeconds() > 1.0f)
		//{
			lastUpdateTime = FDateTime::Now();
			FDataBufferStack Buffer = BufferStack[0];
			BufferStack.RemoveAt(0);
			socketClientSendTCP(Buffer.connectionID,Buffer.message,Buffer.byteArray,Buffer.addLineBreak);
		//}
	}
}



void USocketClientBPLibrary::socketClientInitUDPReceiver(FString& connectionID, FString domain, int32 port, EReceiveFilterClient receiveFilter, int32 maxPacketSize) {

	FString key = domain + FString::FromInt(port);
	if (udpClients.Find(key) != nullptr) {
		USocketClientPluginUDPClient* udpClient = *udpClients.Find(key);
		connectionID = udpClient->getConnectionID();
		onsocketClientUDPConnectionEventDelegate.Broadcast(true, "Connection already present:"+domain+":"+ FString::FromInt(port), connectionID);
		return;
	}

	USocketClientPluginUDPClient* udpClient = NewObject<USocketClientPluginUDPClient>(USocketClientPluginUDPClient::StaticClass());
	connectionID = FGuid::NewGuid().ToString();
	udpClients.Add(connectionID, udpClient);
	udpClients.Add(key, udpClient);
	udpClient->init(this, domain, port, receiveFilter, connectionID, maxPacketSize);
}


void USocketClientBPLibrary::socketClientSendUDP(FString domain, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak, FString clientConnectionIDP) {

	if (clientConnectionIDP.IsEmpty() || udpClients.Find(clientConnectionIDP) == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Connection not found (socketClientSendUDPMessage). %s"), *clientConnectionIDP);
		return;
	}

	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}
	USocketClientPluginUDPClient* udpClient = *udpClients.Find(clientConnectionIDP);
	udpClient->sendUDPMessage(domain, port, message, byteArray, clientConnectionIDP);
}



void USocketClientBPLibrary::closeSocketClientConnectionUDP(FString connectionID) {
	if (connectionID.IsEmpty() || udpClients.Find(connectionID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Connection not found (closeSocketClientUDPConnection). %s"), *connectionID);
		return;
	}

	USocketClientPluginUDPClient* udpClient = *udpClients.Find(connectionID);
	udpClient->closeUDPConnection();
	udpClients.Remove(connectionID);

	FString key = udpClient->getDomainOrIP() + FString::FromInt(udpClient->getPort());
	udpClients.Remove(key);
	udpClient = nullptr;
}

void USocketClientBPLibrary::getUDPInitializationByConnectionID(FString connectionID, bool& found, USocketClientPluginUDPClient*& connection){
	if (connectionID.IsEmpty() || udpClients.Find(connectionID) == nullptr) {
		found = false;
		connection = nullptr;
		return;
	}

	found = true;
	connection = *udpClients.Find(connectionID);
}

bool USocketClientBPLibrary::isUDPInitialized(FString connectionID){
	if (connectionID.IsEmpty() || udpClients.Find(connectionID) == nullptr) {
		return false;
	}

	return (*udpClients.Find(connectionID))->isRun();
}

void USocketClientBPLibrary::changeSocketPlatform(ESocketPlatformClient platform) {
	USocketClientBPLibrary::getSocketClientTarget()->systemSocketPlatform = platform;
}

FString USocketClientBPLibrary::resolveDomain(FString serverDomainP) {

	FString* cachedDomainPointer = domainCache.Find(serverDomainP);
	if (cachedDomainPointer != nullptr) {
		return *cachedDomainPointer;
	}

	//is IP
	TArray<FString> ipNumbers;
	int32 lineCount = serverDomainP.ParseIntoArray(ipNumbers, TEXT("."), true);
	if (lineCount == 4 && serverDomainP.Len() <= 15 && serverDomainP.Len() >= 7) {
		domainCache.Add(serverDomainP, serverDomainP);
		return serverDomainP;
	}

	//resolve Domain
	ISocketSubsystem* sSS = USocketClientBPLibrary::getSocketSubSystem();

	auto ResolveInfo = sSS->GetHostByName(TCHAR_TO_ANSI(*serverDomainP));
	while (!ResolveInfo->IsComplete());

	int32 errorCode = ResolveInfo->GetErrorCode();
	if (errorCode == 0) {
		const FInternetAddr* Addr = &ResolveInfo->GetResolvedAddress();
		uint32 OutIP = 0;
		FString adr = Addr->ToString(false);
		domainCache.Add(serverDomainP, adr);
		return adr;
	}
	else {
		if (dnsClient == nullptr)
			dnsClient = NewObject<UDNSClientSocketClient>(UDNSClientSocketClient::StaticClass());
		dnsClient->resolveDomain(sSS, serverDomainP);
		int32 timeout = 1000;
		while (dnsClient->isResloving() && timeout > 0) {
			timeout -= 10;
			FPlatformProcess::Sleep(0.01);
		}
		FString adr = dnsClient->getIP();
		domainCache.Add(serverDomainP, adr);
		return adr;
	}

	return serverDomainP;
}


void USocketClientBPLibrary::getSystemType(ESocketClientSystem& system) {
#if PLATFORM_ANDROID
	system = ESocketClientSystem::Android;
	return;
#endif	
#if PLATFORM_IOS
	system = ESocketClientSystem::IOS;
	return;
#endif	
#if PLATFORM_WINDOWS
	system = ESocketClientSystem::Windows;
	return;
#endif	
#if PLATFORM_LINUX
	system = ESocketClientSystem::Linux;
	return;
#endif	
#if PLATFORM_MAC
	system = ESocketClientSystem::Mac;
	return;
#endif	
}

TArray<uint8> USocketClientBPLibrary::parseHexToBytes(FString hex) {
	TArray<uint8> bytes;

	if (hex.Contains(" ")) {
		hex = hex.Replace(TEXT(" "), TEXT(""));
	}

	if (hex.Len() % 2 != 0) {
		UE_LOG(LogTemp, Error, TEXT("This is not a valid hex string: %s"), *hex);
		return bytes;
	}


	TArray<TCHAR> charArray = hex.GetCharArray();
	for (int32 i = 0; i < (charArray.Num() - 1); i++) {
		if (CheckTCharIsHex(charArray[i]) == false) {
			UE_LOG(LogTemp, Error, TEXT("This is not a valid hex string: %s"), *hex);
			return bytes;
		}
	}


	bytes.AddZeroed(hex.Len() / 2);
	HexToBytes(hex, bytes.GetData());

	return bytes;
}

FString USocketClientBPLibrary::parseHexToString(FString hex) {
	TArray<uint8> bytes = parseHexToBytes(hex);
	char* Data = (char*)bytes.GetData();
	Data[bytes.Num()] = '\0';
	return FString(UTF8_TO_TCHAR(Data));
}

FString USocketClientBPLibrary::parseBytesToHex(TArray<uint8> bytes) {
	FString hex;
	hex = BytesToHex(bytes.GetData(), bytes.Num());
	return hex;
}

TArray<uint8> USocketClientBPLibrary::parseHexToBytesPure(FString hex) {
	return parseHexToBytes(hex);
}

FString USocketClientBPLibrary::parseHexToStringPure(FString hex) {
	return parseHexToString(hex);
}

FString USocketClientBPLibrary::parseBytesToHexPure(TArray<uint8> bytes) {
	return parseBytesToHex(bytes);
}


int32 USocketClientBPLibrary::getUniquePlayerID(APlayerController* playerController) {
	if (playerController == nullptr || playerController->GetLocalPlayer() == nullptr)
		return 0;
	return playerController->GetLocalPlayer()->GetUniqueID();
}

FString USocketClientBPLibrary::getRandomID(){
	return FGuid::NewGuid().ToString();
}


ISocketSubsystem* USocketClientBPLibrary::getSocketSubSystem() {
	switch (USocketClientBPLibrary::getSocketClientTarget()->systemSocketPlatform)
	{
	case ESocketPlatformClient::E_SSC_SYSTEM:
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	case ESocketPlatformClient::E_SSC_WINDOWS:
		return ISocketSubsystem::Get(FName(TEXT("WINDOWS")));
	case ESocketPlatformClient::E_SSC_MAC:
		return ISocketSubsystem::Get(FName(TEXT("MAC")));
	case ESocketPlatformClient::E_SSC_IOS:
		return ISocketSubsystem::Get(FName(TEXT("IOS")));
	case ESocketPlatformClient::E_SSC_UNIX:
		return ISocketSubsystem::Get(FName(TEXT("UNIX")));
	case ESocketPlatformClient::E_SSC_ANDROID:
		return ISocketSubsystem::Get(FName(TEXT("ANDROID")));
	case ESocketPlatformClient::E_SSC_PS4:
		return ISocketSubsystem::Get(FName(TEXT("PS4")));
	case ESocketPlatformClient::E_SSC_XBOXONE:
		return ISocketSubsystem::Get(FName(TEXT("XBOXONE")));
	case ESocketPlatformClient::E_SSC_HTML5:
		return ISocketSubsystem::Get(FName(TEXT("HTML5")));
	case ESocketPlatformClient::E_SSC_SWITCH:
		return ISocketSubsystem::Get(FName(TEXT("SWITCH")));
	case ESocketPlatformClient::E_SSC_DEFAULT:
		return ISocketSubsystem::Get();
	default:
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	}
}