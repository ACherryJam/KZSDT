// dllmain.cpp : Определяет точку входа для приложения DLL.
#include <Windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <iostream>

#include "minhook/include/MinHook.h"

#include "Debug.h"
#include "Global.h"
#include "KZSDT.h"
#include "GameMaker.h"


VOID CreateConsole()
{
	AllocConsole();
	FILE *p_file;
	freopen_s(&p_file, "CONOUT$", "w", stdout);
}

// Initialize function called by the loader's inject function
void Initialize(HMODULE hModule)
{
	Global::module = hModule;

	HMODULE exe_base = GetModuleHandleW(nullptr);
	Global::exe_base = reinterpret_cast<uintptr_t>(exe_base);

#if _DEBUG
	CreateConsole();
	dcout <<  "Console works yo" << std::endl;
#endif

	if (MH_Initialize() != MH_OK) {
		dcout << "Failed to init MH.";
		abort();
	}

#if _DEBUG
	InitializeTests();
#endif

	GameMaker::SetupFunctions(Global::exe_base);
	InitializeHooks();
	InitializeCodecaves();
	InitializeBikeEnemyHitHandling();
	InitializeShieldDrawing();

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		dcout << "Failed to enable hooks.";
		abort();
	}
}

void Uninitialize()
{
	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
		dcout << "Failed to disable hooks.";
		abort();
	}	
	if (MH_Uninitialize() != MH_OK) {
		dcout << "Failed to uninitialize MH.";
		abort();
	}
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (ulReason)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule);
			Initialize(hModule);
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			Uninitialize();
	}

	return TRUE;
}
