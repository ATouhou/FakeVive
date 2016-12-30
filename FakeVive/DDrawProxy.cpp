#include "DDrawProxy.h"

#ifdef DDRAW_PROXY

#include <string>
#include <Windows.h>
#include "Logger.h"

namespace
{
	typedef HRESULT(WINAPI* TDirectDrawCreate)(GUID*, void*, void*);
	TDirectDrawCreate Actual_DirectDrawCreate;
}

namespace DDrawProxy
{
	bool Initialize()
	{
		char systemRoot[32767];
		GetSystemDirectory(systemRoot, sizeof(systemRoot));
		auto ddrawPath = std::string(systemRoot) + "\\ddraw.dll";
		Logger::Get()->info("Using system ddraw.dll at {}", ddrawPath);
		auto ddraw = LoadLibrary(ddrawPath.c_str());
		if (!ddraw)
		{
			Logger::Get()->error("Failed to open the system ddraw.dll!");
			return false;
		}
		Actual_DirectDrawCreate = reinterpret_cast<TDirectDrawCreate>(GetProcAddress(ddraw, "DirectDrawCreate"));
		if (!Actual_DirectDrawCreate)
		{
			Logger::Get()->error("Failed to locate DirectDrawCreate!");
			return false;
		}
		return true;
	}
}

// This should be the only function that needs to be implemented for opengl32.dll apps
extern "C" __declspec(dllexport) HRESULT WINAPI DirectDrawCreate(GUID *lpGUID, void* lplpDD, void* pUnkOuter)
{
	return Actual_DirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
}

#else // DDRAW_PROXY

namespace DDrawProxy
{
	bool Initialize()
	{
		return true;
	}
}

#endif // DDRAW_PROXY