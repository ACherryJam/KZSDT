#include "GameMaker_Structs.h"

GameMaker::YYStackTrace** GameMaker::YYStackTrace::global_trace = nullptr;

RValue &GameMaker::CInstanceBase::GetYYVarRef(int index) 
{
	if (yyvars) {
		return yyvars[index];
	}
	return InternalGetYYVarRef(index);
}

RValue &GameMaker::CInstance::GetRVRef(int index)
{
	if (this->yyvars != nullptr)
		return this->yyvars[index];
	else
		return InternalGetYYVarRef(index);
}

