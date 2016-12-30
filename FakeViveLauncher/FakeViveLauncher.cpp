#include <string>
#include <Windows.h>

#include "DllInjection.h"

namespace
{
	const char* Title = "FakeVive Launcher";
	const char* DllName = "FakeVive.dll";

	void FatalError(const std::string& str)
	{
		MessageBox(nullptr, str.c_str(), Title, MB_OK | MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), 0);
	}

	bool GetCurrentProcessFileName(std::string& fileNameOut)
	{
		char fileNameBuf[32767];
		if (!GetModuleFileName(nullptr, fileNameBuf, sizeof(fileNameBuf)))
			return false;
		fileNameOut = fileNameBuf;
		fileNameOut.erase(0, fileNameOut.find_last_of("\\/") + 1);
		return true;
	}

	bool FindAppExe(std::string& fileNameOut)
	{
		// Look for the largest EXE in the current directory which isn't this one
		// A bit hacky, but should be pretty reliable most of the time
		std::string currentExe;
		if (!GetCurrentProcessFileName(currentExe))
			return false;
		WIN32_FIND_DATA file;
		auto findHandle = FindFirstFile("*.exe", &file);
		if (findHandle == INVALID_HANDLE_VALUE)
			return false;
		size_t largestSize = 0;
		auto found = false;
		do
		{
			auto fileSize = (static_cast<size_t>(file.nFileSizeHigh) << 32) | file.nFileSizeLow;
			if (fileSize >= largestSize && file.cFileName != currentExe)
			{
				largestSize = fileSize;
				fileNameOut = file.cFileName;
				found = true;
			}
		} while (FindNextFile(findHandle, &file));
		FindClose(findHandle);
		return found;
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	std::string commandLine = lpCmdLine;
	if (commandLine.empty())
	{
		if (!FindAppExe(commandLine))
			FatalError("Failed to locate the application .exe! Please set it as a launch option.");
	}
	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	PROCESS_INFORMATION processInfo;
	if (!CreateProcessWithDll(nullptr, &commandLine[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo, DllName))
		FatalError("Failed to launch \"" + commandLine + "\"!");
	return 0;
}