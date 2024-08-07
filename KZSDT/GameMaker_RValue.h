#pragma once

namespace GameMaker
{
	enum RValueType : int
	{
		VALUE_REAL = 0,
		VALUE_STRING = 1,
		VALUE_ARRAY = 2,
		VALUE_PTR = 3,
		VALUE_VEC3 = 4,
		VALUE_UNDEFINED = 5,
		VALUE_OBJECT = 6,
		VALUE_INT32 = 7,
		VALUE_VEC4 = 8,
		VALUE_MATRIX = 9,
		VALUE_INT64 = 10,
		VALUE_ACCESSOR = 11,
		VALUE_JSNULL = 12,
		VALUE_BOOL = 13,
		VALUE_ITERATOR = 14,
		VALUE_REF = 15,
		VALUE_UNSET = 0xffffff
	};

#pragma pack(push, 4)

	struct GMStringRef
	{
		char *m_pString;
		int m_refCount;
		int m_size;
	};

	struct RValue;

	struct GMRValueArray
	{
		int m_length;
		RValue *m_pArray;
	};


	struct GMArrayRef
	{
		int m_refCount;
		GMRValueArray *m_pDynArray;
		RValue *m_pOwner;
		int m_visited;
		int m_length;
	};


	union RValueUnion
	{
		double v;
		GMStringRef *s;
		long long v64;
		int v32;
		GMArrayRef *arr;
		void *ptr;
	};


	struct RValue
	{
		RValueUnion u;
		int m_flags;
		RValueType m_kind;
	};

#pragma pack(pop)
}