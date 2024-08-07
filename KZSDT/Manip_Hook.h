#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <Windows.h>

#include "Debug.h"

#include "minhook/include/MinHook.h"

namespace Manipulation::Hooks
{
	class IHook
	{
	public:
		virtual bool Enable() = 0;
		virtual bool Disable() = 0;
		virtual bool Toggle() = 0;
	};

	template<typename FType>
	class Hook : public IHook
	{
		bool isEnabled = false;

		FType _trampoline {};
		LPVOID _hook_function {};
		LPVOID _target_function {};

	public:
		Hook(LPVOID pTargetFunction, LPVOID pHookFunction);
		Hook(LPVOID pTargetFunction, LPVOID pHookFunction, FType &pTrampoline);

		~Hook();

		bool Enable();
		bool Disable();
		bool Toggle();

		const FType trampoline() { return _trampoline; }
	};

	template <typename FType>
	Hook<FType>::Hook(LPVOID pTargetFunction,
					  LPVOID pHookFunction)
	{
		_target_function = pTargetFunction;
		_hook_function = pHookFunction;

		MH_STATUS status = MH_CreateHook(pTargetFunction, pHookFunction, reinterpret_cast<LPVOID *>(&_trampoline));
		if (status != MH_OK) {
			std::cout << "Failed to create the hook. " << status << " " << pTargetFunction << std::endl;
		}
	}

	template <typename FType>
	Hook<FType>::Hook(LPVOID pTargetFunction,
					  LPVOID pHookFunction,
					  FType &pTrampoline)
		: Hook(pTargetFunction, pHookFunction)
	{
		pTrampoline = _trampoline;
	}

	template <typename FType>
	Hook<FType>::~Hook() {
		Disable();
	}

	template <typename FType>
	bool Hook<FType>::Enable() {
		if (isEnabled) {
			return false;
		}

		MH_STATUS status = MH_EnableHook(_target_function);
		if (status != MH_OK) {
			std::cout << "Failed to enable the hook. " << status << std::endl;
		}

		isEnabled = true;
		return true;
	}

	template <typename FType>
	bool Hook<FType>::Disable() {
		if (!isEnabled) {
			return false;
		}

		MH_STATUS status = MH_DisableHook(_target_function);
		if (status != MH_OK) {
			//throw std::exception("Failed to disable the hook.");
			std::cout << "Failed to disable the hook. " << status << std::endl;
		}

		isEnabled = false;
		return true;
	}

	template <typename FType>
	bool Hook<FType>::Toggle() {
		if (!isEnabled) {
			return Enable();
		}
		return Disable();
	}

	extern std::map<std::string, std::unique_ptr<IHook>> hooks;

	template <typename FType>
	Hook<FType> *CreateHook(std::string name, LPVOID target, LPVOID hook);

	template <typename FType>
	Hook<FType> *GetHook(std::string name);

	bool MinHook_Assert(MH_STATUS status);
};
