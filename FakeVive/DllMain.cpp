#include <memory>
#include <string>
#include <Windows.h>
#include "Logger.h"
#include "OpenVRHook.h"
#include "DDrawProxy.h"

namespace
{
	const char* Title = "FakeVive";
	const char* DebugFlag = "-FakeViveDebug"; // Debug mode will be enabled if this is found in the command line

	void FatalError(const std::string& str)
	{
		if (Logger::Get())
			Logger::Get()->error(str);
		MessageBox(nullptr, str.c_str(), Title, MB_OK | MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), 0);
	}

	bool CheckDebugModeEnabled()
	{
#if _DEBUG
		return true;
#else
		auto commandLine = GetCommandLine();
		return strstr(commandLine, DebugFlag) != nullptr;
#endif
	}

	void InitializeLogging()
	{
		auto debugMode = CheckDebugModeEnabled();
		Logger::Initialize(Title, debugMode);

		char processFileName[32767] = { 0 };
		GetModuleFileName(nullptr, processFileName, sizeof(processFileName));
		Logger::Get()->info("{} loaded into {}", Title, processFileName);
		Logger::Get()->info("Command line: {}", GetCommandLine());
	}

	void InitializeDDrawProxy()
	{
		if (!DDrawProxy::Initialize())
			FatalError("Failed to initialize the ddraw.dll proxy");
	}

	void InitializeHooks()
	{
		Logger::Get()->info("Installing OpenVR hooks");
		if (OpenVRHook::Install())
			Logger::Get()->info("OpenVR hooks installed");
		else
			FatalError("Failed to install OpenVR hooks");
	}

	void Initialize()
	{
		InitializeLogging();
		InitializeDDrawProxy();
		InitializeHooks();
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		Initialize();
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void FakeViveStub()
{
}