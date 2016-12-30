#include "OpenVRHook.h"

#include <MinHook.h>
#include <openvr.h>
#include "IVRSystemVTable.h"
#include "Logger.h"
#include "ModelNameHook.h"
#include "VTableEditor.h"

namespace
{
	const char* IVRSystemPrefix = "IVRSystem";

	typedef void* (*T_VR_GetGenericInterface)(const char*, vr::EVRInitError*);
	T_VR_GetGenericInterface Actual_VR_GetGenericInterface;

	bool InstallSystemHooks(vr::IVRSystem* system)
	{
		VTableEditor<vr::IVRSystem> vtable(*system, IVRSystemVTable::Count_);

		Logger::Get()->info("Installing model name hooks");
		if (!ModelNameHook::Install(vtable))
		{
			Logger::Get()->error("Failed to install model name hooks!");
			return false;
		}
		return true;
	}

	void* My_VR_GetGenericInterface(const char* pchInterfaceVersion, vr::EVRInitError* peError)
	{
		Logger::Get()->debug("VR_GetGenericInterface \"{}\"", pchInterfaceVersion);

		static auto hooksInstalled = false;
		auto iface = Actual_VR_GetGenericInterface(pchInterfaceVersion, peError);
		if (!hooksInstalled && iface && strncmp(pchInterfaceVersion, IVRSystemPrefix, strlen(IVRSystemPrefix)) == 0)
		{
			Logger::Get()->info("Installing IVRSystem hooks");
			auto system = static_cast<vr::IVRSystem*>(iface);
			if (InstallSystemHooks(system))
			{
				hooksInstalled = true;
				Logger::Get()->info("IVRSystem hooks installed");
			}
			else
			{
				Logger::Get()->error("Failed to install IVRSystem hooks!");
			}
		}
		return iface;
	}
}

namespace OpenVRHook
{
	bool Install()
	{
		if (MH_Initialize() != MH_OK)
		{
			Logger::Get()->error("Failed to initialize MinHook");
			return false;
		}
		if (MH_CreateHook(vr::VR_GetGenericInterface, My_VR_GetGenericInterface, reinterpret_cast<LPVOID*>(&Actual_VR_GetGenericInterface)) != MH_OK)
		{
			Logger::Get()->error("Failed to hook VR_GetGenericInterface");
			return false;
		}
		if (MH_EnableHook(vr::VR_GetGenericInterface) != MH_OK)
		{
			Logger::Get()->error("Failed to enable VR_GetGenericInterface hook");
			return false;
		}
		return true;
	}
}