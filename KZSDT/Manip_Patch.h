#pragma once

#include <Windows.h>

namespace Manipulation
{
	void Patch(LPVOID address, const BYTE buffer[], size_t size);
	void Codecave(DWORD destAddress, VOID(*func)(VOID), BYTE nopCount);
}