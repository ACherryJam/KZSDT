#include "Manip_Hook.h"

using namespace Manipulation;

std::map<std::string, std::unique_ptr<Hooks::IHook>> Hooks::hooks { };

bool Hooks::MinHook_Assert(MH_STATUS status)
{
	if (status != MH_STATUS::MH_OK) {
		std::cerr << "MinHook call failed! Status: " << status << '\n';
		abort();
	}
	dcout << "Minhook call all right\n";

	return true;
}

template <typename FType>
Hooks::Hook<FType> *Hooks::CreateHook(std::string name, LPVOID target_function, LPVOID hook_function)
{
	Hook<FType> *hook = new Hook<FType>(target_function, hook_function);
	hooks[name] = std::unique_ptr<IHook>(hook);

	return hook;
}

template <typename FType>
Hooks::Hook<FType> *Hooks::GetHook(std::string name)
{
	std::unique_ptr<IHook> &ptr = hooks.at(name);
	
	return reinterpret_cast<Hook<FType>*>(ptr.get());
}
