#include <iostream>
#include <algorithm>
#include <cstring>

#include <Windows.h>

#include "minhook/include/MinHook.h"

#include "Debug.h"
#include "Global.h"
#include "GameMaker.h"
#include "KZSDT_Enemy.h"
#include "KatanaZero.h"
#include "Manipulation.h"

#include "resource.h"


using namespace GameMaker;
using namespace Manipulation;

std::wstring CreateShieldFile(HMODULE module)
{
	HANDLE hTempFile = INVALID_HANDLE_VALUE;

	TCHAR szTempFileName[MAX_PATH];
	TCHAR lpTempPathBuffer[MAX_PATH];

	DWORD dwRetVal = GetCurrentDirectory(MAX_PATH, lpTempPathBuffer);
	if (dwRetVal > MAX_PATH || dwRetVal == 0)
	{
		dcout << "GetTempPath failed";
		throw;
	}

	std::string a(*GameMaker::working_directory, strlen(*GameMaker::working_directory));
	std::wstring _path(a.begin(), a.end());
	dcout << *GameMaker::temp_directory << '\n';
	UINT uRetVal = GetTempFileName(_path.c_str(), // directory for tmp files
								   TEXT("KZSDT"),    // temp file name prefix 
								   0,                // create unique name 
								   szTempFileName);  // buffer for name
	if (uRetVal == 0)
	{
		dcout << "GetTempFileName failed";
		throw;
	}

	std::wstring path = std::wstring(szTempFileName);
	path.replace(path.find(L".tmp"), 4, L".png");

	if (!MoveFile(szTempFileName, path.c_str())) {
		dcout << "MoveFile failed.";
		throw;
	}

	std::wcout << path << '\n';

	HRSRC res_info = FindResource(module, MAKEINTRESOURCE(PNG_SHIELD), L"PNG");
	dcout << "res_info = " << res_info << '\n';
	if (res_info == NULL) {
		dcout << "Shield resource not found, error = " << GetLastError() << '\n';
		throw;
	}

	HGLOBAL res = LoadResource(module, res_info);
	dcout << "res = " << res << '\n';
	if (res == NULL) {
		dcout << "Shield resource failed to load.";
		throw;
	}

	LPVOID res_memory = LockResource(res);
	DWORD res_size = SizeofResource(module, res_info);
	dcout << "res_memory = " << res_memory << ", res_size = " << res_size << '\n';

	hTempFile = CreateFile(path.c_str(), // file name 
						   GENERIC_WRITE,        // open for write 
						   0,                    // do not share 
						   NULL,                 // default security 
						   CREATE_ALWAYS,        // overwrite existing
						   FILE_ATTRIBUTE_NORMAL,// normal file 
						   NULL);                // no template 
	if (hTempFile == INVALID_HANDLE_VALUE)
	{
		dcout << "Failed to open temp file.";
		throw;
	}

	DWORD bytes_written;
	if (!WriteFile(hTempFile, res_memory, res_size, &bytes_written, NULL)) {
		dcout << "Failed to write the file.";
		throw;
	}
	dcout << bytes_written << '\n';

	if (!CloseHandle(hTempFile)) {
		dcout << "Failed to close the file.";
		throw;
	}

	return path;
}

int sprite_id = -1;

typedef void(_cdecl *YYGML_draw_sprite_ext_t)(GameMaker::CInstance*, int, int, float, float, float, float, float, int, float);
YYGML_draw_sprite_ext_t trampoline_YYGML_draw_sprite_ext;

typedef RValue*(*draw_enemy_icons_t)(GameMaker::CInstance*, GameMaker::CInstance*, RValue*);
draw_enemy_icons_t trampoline_draw_enemy_icons;

bool drawing_enemy_icons = false;
bool other_icon_in_place = false; 

void __cdecl Hook_YYGML_draw_sprite_ext(GameMaker::CInstance *inst, int sprite_index, int animation_index, float x, float y, float xscale, float yscale, float angle, int color, float alpha)
{
	RValue player = InstanceFindWrapper(nullptr, nullptr, (int)GameObjects::obj_bikeplayer, 0);
	int player_id = (int)player.u.v;
		
	if (player_id != -4) {	
		RValue r_sprite_index = VariableInstanceGetWrapper(nullptr, nullptr, player_id, (char *)"sprite_index");
	}

	if (drawing_enemy_icons) {
		other_icon_in_place = true;
		trampoline_YYGML_draw_sprite_ext(inst, sprite_index, animation_index, x-15, y, xscale, yscale, angle, color, alpha);
	}
	else {
		trampoline_YYGML_draw_sprite_ext(inst, sprite_index, animation_index, x, y, xscale, yscale, angle, color, alpha);
	}
}

RValue *Hook_draw_enemy_icons(GameMaker::CInstance *self, GameMaker::CInstance *other, RValue *result)
{
	if (KZSDT::enemies.at(self->i_id).GetHP() == 2)
		drawing_enemy_icons = true;

	RValue *ret = trampoline_draw_enemy_icons(self, other, result);

	if (drawing_enemy_icons) {
		if (other_icon_in_place) { 
			trampoline_YYGML_draw_sprite_ext(self, sprite_id, -1, self->i_x + 15, self->i_y-100, 2, 2, 0, 16777215, 1);
		}
		else {
			trampoline_YYGML_draw_sprite_ext(self, sprite_id, -1, self->i_x, self->i_y - 100, 2, 2, 0, 16777215, 1);
		}
	}

	drawing_enemy_icons = false;
	other_icon_in_place = false;

	return ret;
}

typedef void(__cdecl *StartRoom_t)(int, int);
StartRoom_t trampoline_StartRoom = nullptr;

void __cdecl Hook_StartRoom(int param_1, bool param_2)
{
	if (sprite_id == -1) {
		dcout << "Sprited\n";
		std::wstring path = CreateShieldFile(Global::module);

		std::string filepath;
		std::transform(path.begin(), path.end(), std::back_inserter(filepath), [](wchar_t c) {
			return (char)c;
					   });
		sprite_id = GameMaker::SpriteAddWrapper(NULL, NULL, filepath, 1, 0, 1, 6, 7).u.v;
		dcout << "Filepath = " << filepath << "\nSprite id = " << sprite_id << '\n';

		DeleteFile((LPCWSTR)path.c_str());
	}

	trampoline_StartRoom(param_1, param_2);
}

typedef void(*ObjectScript)(CInstance *, CInstance *);

ObjectScript EnemyBikeDraw, EnemyBikeBullyDraw, EnemyBikeMachinegunDraw, EnemyBikeVDraw;
std::vector<std::pair<double, double>> shield_positions {};

#define Hook_DrawScript(OriginalFunction)																			 \
void Hook_##OriginalFunction(CInstance *self, CInstance *other)														 \
{																													 \
	OriginalFunction(self, other); \
\
	KZSDT::Enemy &enemy = KZSDT::enemies.at(self->i_id);															 \
																													 \
	if (enemy.GetHP() == 2) {																						 \
		shield_positions.push_back({self->i_x, self->i_y}); \
	}																				 \
}

//trampoline_YYGML_draw_sprite_ext(self, sprite_id, -1, self->i_x + 15, self->i_y - 100, 2, 2, 0, 16777215, 1); 

Hook_DrawScript(EnemyBikeDraw);
Hook_DrawScript(EnemyBikeBullyDraw);
Hook_DrawScript(EnemyBikeMachinegunDraw);
Hook_DrawScript(EnemyBikeVDraw);

typedef void(*Draw_Room_t)(void);
Draw_Room_t trampoline_Draw_Room;

void Hook_Draw_Room(void)
{
	trampoline_Draw_Room();

	for (const auto &pos : shield_positions) {
		// Note that animation_index is 0, because with animation index of -1 GameMaker tried to find animation index in the instance
		// which is NULL in our case
		trampoline_YYGML_draw_sprite_ext(NULL, sprite_id, 0, pos.first - 20, pos.second - 125, 2, 2, 0, 16777215, 1);
	}

	shield_positions = {};
}


void InitializeShieldDrawing()
{
	MH_STATUS result;
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x150e600u),
						   Hook_YYGML_draw_sprite_ext,
						   reinterpret_cast<LPVOID *>(&trampoline_YYGML_draw_sprite_ext));
	Hooks::MinHook_Assert(result);
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x1d8440u),
						   Hook_draw_enemy_icons,
						   reinterpret_cast<LPVOID *>(&trampoline_draw_enemy_icons));
	Hooks::MinHook_Assert(result);
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x154dca0u),
						   Hook_StartRoom,
						   reinterpret_cast<LPVOID *>(&trampoline_StartRoom));
	Hooks::MinHook_Assert(result);
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x15e42a0u),
						   Hook_Draw_Room,
						   reinterpret_cast<LPVOID *>(&trampoline_Draw_Room));
	Hooks::MinHook_Assert(result);

	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x13a1f10u),
						   Hook_EnemyBikeDraw,
						   reinterpret_cast<LPVOID *>(&EnemyBikeDraw));
	Hooks::MinHook_Assert(result);
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x13b3a80u),
						   Hook_EnemyBikeBullyDraw,
						   reinterpret_cast<LPVOID *>(&EnemyBikeBullyDraw));
	Hooks::MinHook_Assert(result);
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x13a7720u),
						   Hook_EnemyBikeMachinegunDraw,
						   reinterpret_cast<LPVOID *>(&EnemyBikeMachinegunDraw));
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x13aed00u),
						   Hook_EnemyBikeVDraw,
						   reinterpret_cast<LPVOID *>(&EnemyBikeVDraw));
	Hooks::MinHook_Assert(result);
}
