#include "DllInjection.h"

#include <Windows.h>

bool CreateProcessWithDll(LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, const char* dllPath)
{
	dwCreationFlags |= CREATE_SUSPENDED;
	if (!CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation))
		return false;
	if (!InjectDllIntoProcess(lpProcessInformation->hProcess, dllPath))
	{
		TerminateProcess(lpProcessInformation->hProcess, 0);
		CloseHandle(lpProcessInformation->hThread);
		CloseHandle(lpProcessInformation->hProcess);
		return false;
	}
	ResumeThread(lpProcessInformation->hThread);
	return true;
}

bool InjectDllIntoProcess(HANDLE process, const char* dllPath)
{
	auto kernel32 = GetModuleHandle("kernel32");
	auto loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");
	DWORD result;
	return CallRemoteFunction(process, loadLibrary, dllPath, strlen(dllPath) + 1, result) && result != 0;
}

bool CallRemoteFunction(HANDLE process, void* function, const void* arg, size_t argSize, DWORD& returnValueOut)
{
	// Argument must be copied into the target process's address space
	void* remoteArg = nullptr;
	if (arg)
	{
		remoteArg = VirtualAllocEx(process, nullptr, argSize, MEM_COMMIT, PAGE_READWRITE);
		if (!remoteArg)
			return false;
		size_t bytesWritten = 0;
		if (!WriteProcessMemory(process, remoteArg, arg, argSize, &bytesWritten) || bytesWritten < argSize)
		{
			VirtualFreeEx(process, remoteArg, 0, MEM_RELEASE);
			return false;
		}
	}

	// Use CreateRemoteThread to call the function
	auto thread = CreateRemoteThread(process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(function), remoteArg, 0, nullptr);
	if (thread)
	{
		WaitForSingleObject(thread, INFINITE);
		GetExitCodeThread(thread, &returnValueOut); // Exit code is the return value
		CloseHandle(thread);
	}
	if (remoteArg)
		VirtualFreeEx(process, remoteArg, 0, MEM_RELEASE);
	return thread != nullptr;
}