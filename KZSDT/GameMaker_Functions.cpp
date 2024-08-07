#include "GameMaker_Functions.h"

#define RC(type, value) reinterpret_cast<type>(value)


GameMaker::FFunc GameMaker::InstanceFind = reinterpret_cast<GameMaker::FFunc>(0x14e0e20u);
GameMaker::FFunc GameMaker::VariableInstanceGet = reinterpret_cast<GameMaker::FFunc>(0x14e8ab0u);
GameMaker::FFunc GameMaker::VariableInstanceSet = reinterpret_cast<GameMaker::FFunc>(0x14e8e30u);
GameMaker::FFunc GameMaker::SpriteAdd = reinterpret_cast<GameMaker::FFunc>(0x1637670u);
GameMaker::FFunc GameMaker::InstanceDestroy = reinterpret_cast<GameMaker::FFunc>(0x14e0d90u);

char **GameMaker::temp_directory = reinterpret_cast<char**>(0x1d094fcu);
char **GameMaker::working_directory = reinterpret_cast<char**>(0x1cf894cu);


void GameMaker::SetupFunctions(uintptr_t exe_base)
{
	InstanceFind = reinterpret_cast<FFunc>(reinterpret_cast<uintptr_t>(InstanceFind) + exe_base);
	VariableInstanceGet = reinterpret_cast<FFunc>(reinterpret_cast<uintptr_t>(VariableInstanceGet) + exe_base);
	VariableInstanceSet = reinterpret_cast<FFunc>(reinterpret_cast<uintptr_t>(VariableInstanceSet) + exe_base);
	SpriteAdd = reinterpret_cast<FFunc>(reinterpret_cast<uintptr_t>(SpriteAdd) + exe_base);
	InstanceDestroy = reinterpret_cast<FFunc>(reinterpret_cast<uintptr_t>(InstanceDestroy) + exe_base);
	
	temp_directory = reinterpret_cast<char**>(reinterpret_cast<uintptr_t>(temp_directory) + exe_base);
	working_directory = reinterpret_cast<char**>(reinterpret_cast<uintptr_t>(working_directory) + exe_base);

	YYStackTrace::global_trace = reinterpret_cast<YYStackTrace **>(exe_base + 0x1ad8b90u);
}

RValue GameMaker::InstanceFindWrapper(CInstance *self, CInstance *other, int object_id, int object_number)
{
	RValue *result = new RValue;
	RValue args[2]{};

	args[0].m_kind = GameMaker::VALUE_REAL;
	args[0].u.v = object_id;
	args[1].m_kind = GameMaker::VALUE_REAL;
	args[1].u.v = object_number;

	GameMaker::InstanceFind(result, self, other, 2, args);

	RValue return_value = *result;
	delete result;

	return return_value;
}

RValue GameMaker::VariableInstanceGetWrapper(CInstance *self, CInstance *other, int inst_id, char *name)
{
	RValue *result = new RValue;

	RValue *args = new RValue[2]{};
	args[0].m_flags = 0;
	args[0].m_kind = VALUE_REAL;
	args[0].u.v = inst_id;

	args[1].m_flags = 0;
	args[1].m_kind = VALUE_STRING;

	GMStringRef *str = new GMStringRef;
	str->m_refCount = 0;
	str->m_size = strlen(name);
	str->m_pString = name;
	args[1].u.s = str;

	VariableInstanceGet(result, self, other, 2, args);
	RValue return_value = *result;

	delete[] args;
	delete str;
	delete result;

	return return_value;
}

RValue GameMaker::SpriteAddWrapper(CInstance *self, CInstance *other, std::string fname, int imgnum, bool removeback, bool smooth, int xorig, int yorig)
{
	RValue result {};

	RValue *args = new RValue[6] {};
	args[0].m_flags = 0;
	args[1].m_flags = 0;
	args[2].m_flags = 0;
	args[3].m_flags = 0;
	args[4].m_flags = 0;
	args[5].m_flags = 0;

	args[0].m_kind = VALUE_STRING;
	args[1].m_kind = VALUE_REAL;
	args[2].m_kind = VALUE_BOOL;
	args[3].m_kind = VALUE_BOOL;
	args[4].m_kind = VALUE_REAL;
	args[5].m_kind = VALUE_REAL;

	GMStringRef *str = new GMStringRef;
	str->m_refCount = 0;
	str->m_size = fname.size();
	str->m_pString = (char*)fname.c_str();
	args[0].u.s = str;
	args[1].u.v = imgnum;
	args[2].u.v = removeback;
	args[3].u.v = smooth;
	args[4].u.v = xorig;
	args[5].u.v = yorig;

	SpriteAdd(&result, self, other, 6, args);

	delete[] args;
	delete str;

	return result;
}

RValue GameMaker::InstanceDestroyWrapper(CInstance *self, CInstance *other, int inst_id, bool perform_destroy_event)
{
	RValue result{};

	RValue *args = new RValue[2] {};
	args[0].m_flags = 0;
	args[0].m_kind = VALUE_REAL;
	args[0].u.v = inst_id;

	args[1].m_flags = 0;
	args[1].m_kind = VALUE_REAL;
	args[1].u.v = (int)perform_destroy_event;

	GameMaker::InstanceDestroy(&result, self, other, 2, args);
	
	delete[] args;

	return result;
}

