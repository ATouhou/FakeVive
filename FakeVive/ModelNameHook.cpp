#include "ModelNameHook.h"

#include "IVRSystemVTable.h"
#include "Logger.h"

namespace
{
	const char* OverrideManufacturer = "HTC";
	const char* OverrideModelNumber = "Vive";

	typedef uint32_t(*T_GetStringTrackedDeviceProperty)(vr::IVRSystem*, vr::TrackedDeviceIndex_t, vr::ETrackedDeviceProperty, char*, uint32_t, vr::ETrackedPropertyError*);
	T_GetStringTrackedDeviceProperty Actual_GetStringTrackedDeviceProperty;

	uint32_t My_GetStringTrackedDeviceProperty(vr::IVRSystem* system, vr::TrackedDeviceIndex_t unDeviceIndex, vr::ETrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError *pError)
	{
		Logger::Get()->debug("IVRSystem::GetStringTrackedDeviceProperty {}", prop);
		auto size = Actual_GetStringTrackedDeviceProperty(system, unDeviceIndex, prop, pchValue, unBufferSize, pError);
		Logger::Get()->debug("IVRSystem::GetStringTrackedDeviceProperty returned {}, error = {}, value = \"{}\"", size, *pError, *pError == vr::TrackedProp_Success ? pchValue : "");

		const char* overrideStr;
		switch (prop)
		{
		case vr::ETrackedDeviceProperty::Prop_ManufacturerName_String:
			overrideStr = OverrideManufacturer;
			break;
		case vr::ETrackedDeviceProperty::Prop_ModelNumber_String:
			overrideStr = OverrideModelNumber;
			break;
		default:
			Logger::Get()->debug("Ignoring tracked device property {}", prop);
			return size;
		}
		Logger::Get()->debug("Forcing tracked device property {} to \"{}\"", prop, overrideStr);

		size = static_cast<uint32_t>(strlen(overrideStr)) + 1;
		strncpy_s(pchValue, unBufferSize, overrideStr, size - 1);
		*pError = vr::TrackedProp_Success;
		return size;
	}
}

namespace ModelNameHook
{
	bool Install(VTableEditor<vr::IVRSystem>& vtable)
	{
		return vtable.Replace(IVRSystemVTable::GetStringTrackedDeviceProperty, My_GetStringTrackedDeviceProperty, &Actual_GetStringTrackedDeviceProperty);
	}
}