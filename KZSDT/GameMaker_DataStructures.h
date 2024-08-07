#pragma once
#include <unordered_map>


class YYObjectBase;


namespace GameMaker
{
	enum eDeleteType : int
	{
		eDelete_placementdelete = 3,
		eDelete_delete = 1,
		eDelete_free = 2,
		eDelete_none = 0
	};

	struct SLink;

	struct SLinkListEx {
		SLink *head;
		SLink *tail;
		int offset;
	};

	struct SLink {
		SLink *next;
		SLink *prev;
		SLinkListEx *list;
	};

	template<typename T>
	class HashNode {
	public:
		HashNode<T> *m_pPrev;
		HashNode<T> *m_pNext;
		unsigned int m_ID;
		T *m_pObj;
	};

	template<typename T>
	class HashLink {
	public:
		T *m_pFirst;
		T *m_pLast;
	};

	template<typename T>
	class CHash {
	public:
		HashLink<HashNode<T>> *m_pHashingTable;
		int m_HashingMask;
		int m_Count;
	};

	typedef unsigned int Hash;

	template<class K, class V, int I>
	class Element {
	public:
		V v; // Value
		K k; // Key
		Hash hash; // Hash
	};

	template<class K, class V, int I = 3>
	class CHashMap {
	public:
		int m_curSize;
		int m_numUsed; // actual size of the hashmap!
		int m_curMask;
		int m_growThreshold;
		Element<K, V, I> *m_elements;

		const int GetIdealPosition(Hash _h) const
		{
			return static_cast<int>(this->m_curMask & _h & INT32_MAX);
		}

		const Hash GetHashAt(int _pos) const
		{
			return this->m_elements[_pos].hash;
		}

		const Hash CalculateHash(int _k) const
		{
			return static_cast<Hash>(_k * -1640531535 + 1);
		}

		const Hash CalculateHash(unsigned long long _k) const
		{
			return reinterpret_cast<Hash>(static_cast<Hash>(_k * 11400714819323198549uLL >> 0x20uLL) + 1u);
		}

		const Hash CalculateHash(YYObjectBase *_k) const
		{
			return static_cast<Hash>(((reinterpret_cast<unsigned long long>(_k)) >> 8) + 1);
		}

		const bool CompareKeys(const char *l, const char *r) const
		{
			return strcmp(l, r) == 0;
		}

		const bool CompareKeys(unsigned long long l, unsigned long long r) const
		{
			return l == r;
		}

		const bool CompareKeys(YYObjectBase *l, YYObjectBase *r) const
		{
			// that makes no sense???
			return l == r;
		}
	};

	template<class T>
	struct SLinkedListNode {
		SLinkedListNode *m_pNext;
		SLinkedListNode *m_pPrev;
		T *m_pObj;
	};

	template<class T>
	struct SLinkedList {
		SLinkedListNode<T> *m_pFirst;
		SLinkedListNode<T> *m_pLast;
		int m_Count;
		//eDeleteType m_DeleteType;
	};

	template<class T>
	class cARRAY_STRUCTURE {
		int Length;
		T *Array;
	};

	class cARRAY_OF_POINTERS {
		int Length;
		int m_slotsUsed;
		int m_reserveSize;
		void **Array;
	};

	template<class T>
	class OLinkedList {
	public:
		T *m_pFirst;
		T *m_pLast;
		int m_Count;
		eDeleteType m_DeleteType;
	};

	template<class T>
	class LinkedList {
		T *m_pFirst;
		T *m_pLast;
		int m_Count;
		eDeleteType m_DeleteType;
	};
}