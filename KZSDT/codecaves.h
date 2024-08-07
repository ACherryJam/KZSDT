#pragma once

#include <Windows.h>


void Patch(LPVOID address, const BYTE buffer[]);


void HandleHit();
void CC_HandleHit(void);


void InitializePatches();
void InitializeCodecaves();

