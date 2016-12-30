#pragma once

#include <Windows.h>

bool CreateProcessWithDll(LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, const char* dllPath);
bool InjectDllIntoProcess(HANDLE process, const char* dllPath);
bool CallRemoteFunction(HANDLE process, void* function, const void* arg, size_t argSize, DWORD& returnValueOut);