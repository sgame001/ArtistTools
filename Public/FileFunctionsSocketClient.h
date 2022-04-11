// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketClient.h"
#include "FileFunctionsSocketClient.generated.h"


UENUM(BlueprintType)
enum class EFileFunctionsSocketClientDirectoryType : uint8
{
	E_gd	UMETA(DisplayName = "Game directory"),
	E_ad 	UMETA(DisplayName = "Absolute directory")
};


UCLASS(Blueprintable, BlueprintType)
class UFileFunctionsSocketClient : public UObject
{
	GENERATED_UCLASS_BODY()

public:


	//APEND bytes in eine Datei muss vorhenden sein fuer webcom plugin. vielleicht auch eine node zum aufteilen einer datei

	static FString getCleanDirectory(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);

	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void writeBytesToFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void addBytesToFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success);
	//UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
	//	static void splittFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, int32 parts, bool& success);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static TArray<uint8> readBytesFromFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool &success);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void readStringFromFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool& success, FString& data);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void writeStringToFile(EFileFunctionsSocketClientDirectoryType directoryType, FString data, FString filePath, bool& success);

	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void getMD5FromFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool& success, FString& MD5);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void bytesToBase64String(TArray<uint8> bytes, FString& base64String);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static TArray<uint8> base64StringToBytes(EFileFunctionsSocketClientDirectoryType directoryType, FString base64String, bool& success);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void fileToBase64String(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool& success, FString& base64String, FString& fileName);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool fileExists(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool directoryExists(EFileFunctionsSocketClientDirectoryType directoryType, FString path);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static int64 fileSize(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool deleteFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	/** Delete a directory and return true if the directory was deleted or otherwise does not exist. **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool deleteDirectory(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	/** Return true if the file is read only. **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool isReadOnly(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	/** Attempt to move a file. Return true if successful. Will not overwrite existing files. **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool moveFile(EFileFunctionsSocketClientDirectoryType directoryTypeTo, FString filePathTo, EFileFunctionsSocketClientDirectoryType directoryTypeFrom, FString filePathFrom);
	/** Attempt to change the read only status of a file. Return true if successful. **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool setReadOnly(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool bNewReadOnlyValue);
	/** Return the modification time of a file. Returns FDateTime::MinValue() on failure **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static FDateTime getTimeStamp(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	/** Sets the modification time of a file **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static void	setTimeStamp(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, FDateTime DateTime);
	/** Return the last access time of a file. Returns FDateTime::MinValue() on failure **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static FDateTime getAccessTimeStamp(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	/** For case insensitive filesystems, returns the full path of the file with the same case as in the filesystem */
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static FString getFilenameOnDisk(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath);
	/** Create a directory and return true if the directory was created or already existed. **/
	UFUNCTION(BlueprintCallable, Category = "SocketClient|SpecialFunctions|File")
		static bool createDirectory(EFileFunctionsSocketClientDirectoryType directoryType, FString path);

private:


};

