// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.

#include "FileFunctionsSocketClient.h"


UFileFunctionsSocketClient::UFileFunctionsSocketClient(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

}

FString UFileFunctionsSocketClient::getCleanDirectory(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	if (directoryType == EFileFunctionsSocketClientDirectoryType::E_ad) {
		return FPaths::ConvertRelativePathToFull(filePath);
	}
	else {
		FString ProjectDir = FPaths::ProjectDir();
		return FPaths::ConvertRelativePathToFull(ProjectDir + filePath);
	}
}

void UFileFunctionsSocketClient::writeBytesToFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success) {
	success = FFileHelper::SaveArrayToFile(bytes, *getCleanDirectory(directoryType, filePath));
}

void UFileFunctionsSocketClient::addBytesToFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success) {
	FArchive* writer = IFileManager::Get().CreateFileWriter(*getCleanDirectory(directoryType, filePath), EFileWrite::FILEWRITE_Append);
	if (!writer) {
		success = false;
		return;
	}
	writer->Seek(writer->TotalSize());
	writer->Serialize(bytes.GetData(), bytes.Num());
	writer->Close();
	delete writer;

	success = true;
}

//void UFileFunctionsSocketClient::splittFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, int32 parts, bool& success){
//	if (parts <= 0)
//		parts = 1;
//	FArchive* reader = IFileManager::Get().CreateFileReader(*getCleanDirectory(directoryType, filePath));
//	if (!reader) {
//		success = false;
//		return;
//	}
//	
//	int64 splittAfterBytes = reader->TotalSize()/ ((int64)parts);
//	TArray<uint8> bytes;
//
//	for (int32 i = 0; i < parts; i++){
//		bytes.AddUninitialized(splittAfterBytes);
//		reader->Serialize(bytes.GetData(), splittAfterBytes);
//		if (FFileHelper::SaveArrayToFile(bytes, *getCleanDirectory(directoryType, filePath)) == false) {
//			success = false;
//			return;
//		}
//		splittAfterBytes =
//		reader->Seek();
//	}
//
//}

TArray<uint8> UFileFunctionsSocketClient::readBytesFromFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool& success) {
	TArray<uint8> result;
	success = FFileHelper::LoadFileToArray(result, *getCleanDirectory(directoryType, filePath));
	return result;
}

void UFileFunctionsSocketClient::readStringFromFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool& success, FString& data) {
	data.Empty();
	success = FFileHelper::LoadFileToString(data, *getCleanDirectory(directoryType, filePath));
}

void UFileFunctionsSocketClient::writeStringToFile(EFileFunctionsSocketClientDirectoryType directoryType, FString data, FString filePath, bool& success) {
	success = FFileHelper::SaveStringToFile(data, *getCleanDirectory(directoryType, filePath));
}


void UFileFunctionsSocketClient::getMD5FromFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool& success, FString& MD5) {
	MD5.Empty();
	FArchive* reader = IFileManager::Get().CreateFileReader(*getCleanDirectory(directoryType, filePath));
	if (!reader) {
		success = false;
		return;
	}

	TArray<uint8> byteArrayTmp;
	int64 totalSize = reader->TotalSize();
	int64 loadedBytes = 0;
	int64 leftUploadBytes = 1024;


	if (totalSize < leftUploadBytes)
		leftUploadBytes = totalSize;


	uint8 Digest[16];
	FMD5 Md5Gen;

	while ((loadedBytes + leftUploadBytes) <= totalSize) {
		byteArrayTmp.Reset(leftUploadBytes);
		byteArrayTmp.AddUninitialized(leftUploadBytes);
		reader->Serialize(byteArrayTmp.GetData(), byteArrayTmp.Num());
		loadedBytes += leftUploadBytes;
		reader->Seek(loadedBytes);

		Md5Gen.Update(byteArrayTmp.GetData(), byteArrayTmp.Num());
	}

	leftUploadBytes = totalSize - loadedBytes;
	if (leftUploadBytes > 0) {
		byteArrayTmp.Reset(leftUploadBytes);
		byteArrayTmp.AddUninitialized(leftUploadBytes);
		reader->Serialize(byteArrayTmp.GetData(), byteArrayTmp.Num());
		loadedBytes += leftUploadBytes;
		Md5Gen.Update(byteArrayTmp.GetData(), byteArrayTmp.Num());
	}

	if (reader != nullptr) {
		reader->Close();
		delete reader;
	}

	if (totalSize != loadedBytes) {
		success = false;
		return;
	}

	Md5Gen.Final(Digest);
	for (int32 i = 0; i < 16; i++) {
		MD5 += FString::Printf(TEXT("%02x"), Digest[i]);
	}

	success = true;
}


void UFileFunctionsSocketClient::bytesToBase64String(TArray<uint8> bytes, FString& base64String) {
	base64String.Empty();
	base64String = FBase64::Encode(bytes);
}

TArray<uint8> UFileFunctionsSocketClient::base64StringToBytes(EFileFunctionsSocketClientDirectoryType directoryType, FString base64String, bool& success) {
	TArray<uint8> fileData;
	success = FBase64::Decode(*base64String, fileData);
	return fileData;
}

void UFileFunctionsSocketClient::fileToBase64String(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool& success, FString& base64String, FString& fileName) {
	base64String.Empty();
	fileName.Empty();
	TArray<uint8> fileData;
	if (!FFileHelper::LoadFileToArray(fileData, *getCleanDirectory(directoryType, filePath))) {
		success = false;
		return;
	}
	base64String = FBase64::Encode(fileData);
	success = true;
}

bool UFileFunctionsSocketClient::fileExists(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPaths::FileExists(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketClient::directoryExists(EFileFunctionsSocketClientDirectoryType directoryType, FString path) {
	return FPaths::DirectoryExists(*getCleanDirectory(directoryType, path));
}

int64 UFileFunctionsSocketClient::fileSize(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().FileSize(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketClient::deleteFile(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketClient::deleteDirectory(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().DeleteDirectory(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketClient::isReadOnly(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().IsReadOnly(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketClient::moveFile(EFileFunctionsSocketClientDirectoryType directoryTypeTo, FString filePathTo, EFileFunctionsSocketClientDirectoryType directoryTypeFrom, FString filePathFrom) {
	return FPlatformFileManager::Get().GetPlatformFile().MoveFile(*getCleanDirectory(directoryTypeTo, filePathTo), *getCleanDirectory(directoryTypeFrom, filePathFrom));
}

bool UFileFunctionsSocketClient::setReadOnly(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, bool bNewReadOnlyValue) {
	return FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*getCleanDirectory(directoryType, filePath), bNewReadOnlyValue);
}

FDateTime UFileFunctionsSocketClient::getTimeStamp(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetTimeStamp(*getCleanDirectory(directoryType, filePath));
}

void UFileFunctionsSocketClient::setTimeStamp(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath, FDateTime DateTime) {
	FPlatformFileManager::Get().GetPlatformFile().SetTimeStamp(*getCleanDirectory(directoryType, filePath), DateTime);
}

FDateTime UFileFunctionsSocketClient::getAccessTimeStamp(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetAccessTimeStamp(*getCleanDirectory(directoryType, filePath));
}

FString UFileFunctionsSocketClient::getFilenameOnDisk(EFileFunctionsSocketClientDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetFilenameOnDisk(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketClient::createDirectory(EFileFunctionsSocketClientDirectoryType directoryType, FString path) {
	return FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*getCleanDirectory(directoryType, path));
}