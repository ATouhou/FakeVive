#pragma once

#include <openvr.h>
#include "VTableEditor.h"

namespace ModelNameHook
{
	bool Install(VTableEditor<vr::IVRSystem>& vtable);
}