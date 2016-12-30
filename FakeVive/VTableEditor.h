#pragma once

#include <Windows.h>

template<class T>
class VTableEditor
{
public:
	VTableEditor(T& obj, size_t virtualMethodCount)
	{
		vtable_ = *reinterpret_cast<void***>(&obj);
		virtualMethodCount_ = virtualMethodCount;
		VirtualProtect(vtable_, virtualMethodCount_ * sizeof(void*), PAGE_READWRITE, &oldProtect_);
	}

	~VTableEditor()
	{
		VirtualProtect(vtable_, virtualMethodCount_ * sizeof(void*), oldProtect_, &oldProtect_);
	}

	template<class TFunc>
	bool Replace(size_t virtualMethodIndex, TFunc newFunc, TFunc* oldFuncOut)
	{
		if (!newFunc || virtualMethodIndex >= virtualMethodCount_ || !vtable_[virtualMethodIndex])
			return false;
		if (oldFuncOut)
			*oldFuncOut = static_cast<TFunc>(vtable_[virtualMethodIndex]);
		vtable_[virtualMethodIndex] = newFunc;
		return true;
	}

private:
	void** vtable_;
	size_t virtualMethodCount_;
	DWORD oldProtect_;
};