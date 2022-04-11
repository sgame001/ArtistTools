// Copyright 2017-2019 David Romanski (Socke). All Rights Reserved.
#include "SocketClient.h"

#define LOCTEXT_NAMESPACE "FSocketClientModule"

bool FSocketClientModule::isShuttingDown;

void FSocketClientModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

}

void FSocketClientModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	isShuttingDown = true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSocketClientModule, SocketClient)