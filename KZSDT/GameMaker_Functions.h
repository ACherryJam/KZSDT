#pragma once

#include <string>

#include "GameMaker_RValue.h"
#include "GameMaker_Structs.h"

namespace GameMaker
{
	extern char **temp_directory;
	extern char **working_directory;

	typedef void(_cdecl *FFunc)(RValue *, CInstance *, CInstance *, int, RValue *);
	extern FFunc InstanceFind;
	extern FFunc VariableInstanceGet;
	extern FFunc VariableInstanceSet;
	extern FFunc SpriteAdd;
	extern FFunc InstanceDestroy;

	void SetupFunctions(uintptr_t exe_base);

	RValue InstanceFindWrapper(CInstance *self, CInstance *other, int object_id, int object_number);
	RValue VariableInstanceGetWrapper(CInstance *self, CInstance *other, int inst_id, char *name);
	RValue SpriteAddWrapper(CInstance *self, CInstance *other, std::string fname, int imgnum, bool removeback, bool smooth, int xorig, int yorig);
	RValue InstanceDestroyWrapper(CInstance *self, CInstance *other, int inst_id, bool perform_destroy_event = true);
}