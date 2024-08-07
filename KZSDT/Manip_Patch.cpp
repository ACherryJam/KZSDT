#include "Manip_Patch.h"

#include <iostream>
#include "Debug.h"

void Manipulation::Patch(LPVOID address, const BYTE buffer[], size_t size)
{
	SIZE_T bytes_written;

	BOOL result = WriteProcessMemory(GetCurrentProcess(), address, buffer, size, &bytes_written);

	if (result == FALSE || bytes_written != size) {
		abort();
	}

	dcout << "[debug] Patched address " << std::hex << address << std::dec << ", bytes written: " << bytes_written << '\n';
}

void Manipulation::Codecave(DWORD destAddress, VOID(*func)(VOID), BYTE noop_count)
{
	DWORD offset = (PtrToUlong(func) - destAddress) - 5;

	BYTE patch[5] = { 0xE8, 0x00, 0x00, 0x00, 0x00 };

	memcpy(patch + 1, &offset, sizeof(DWORD));
	Patch(reinterpret_cast<LPVOID>(destAddress), patch, sizeof(patch));

	if (noop_count == 0) {
		return;
	}

	BYTE *noop_patch = new BYTE[noop_count] {};

	memset(noop_patch, 0x90, noop_count);
	Patch(reinterpret_cast<LPVOID>(destAddress+5), noop_patch, noop_count);

	dcout << "[debug] Created codecave at " << std::hex << destAddress << std::dec << ", noop count: " << (int)noop_count << '\n';
}
