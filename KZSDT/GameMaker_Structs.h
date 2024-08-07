#pragma once

#include <iostream>

#include "Debug.h"

#include "GameMaker_RValue.h"
#include "GameMaker_DataStructures.h"

using GameMaker::RValue;

typedef long long INT64;


namespace GameMaker
{
#pragma pack(push, 4)

	class CInstanceBase
	{
	public:
		RValue *yyvars;

		virtual ~CInstanceBase() = 0;

		virtual RValue &InternalGetYYVarRef(int index) = 0;
		RValue &GetYYVarRef(int index);
	};;

	enum EJSRetValBool
	{
		EJSRVB_TRUE = 1,
		EJSRVB_FALSE = 0,
		EJSRVB_TYPE_ERROR = 2
	};

	typedef void(*GetOwnPropertyFunc)(YYObjectBase *object, RValue *res, char *name);
	typedef void(*DeletePropertyFunc)(YYObjectBase *object, RValue *res, char *name, bool flag);
	typedef EJSRetValBool(*DefineOwnPropertyFunc)(YYObjectBase *object, char *name, RValue *res, bool flag);

	class YYObjectBase : public CInstanceBase
	{
		YYObjectBase *m_pNextObject;
		YYObjectBase *m_pPrevObject;
		YYObjectBase *m_prototype;
		void *m_pcre;
		void *m_pcreExtra;
		char *m_class;
		GetOwnPropertyFunc m_getOwnProperty;
		DeletePropertyFunc m_deleteProperty;
		DefineOwnPropertyFunc m_defineOwnProperty;
	public:
		CHashMap<int, RValue *, 3> *m_yyvarsMap;
		unsigned int m_nvars;
		unsigned int m_flags;
		unsigned int m_capacity;
		unsigned int m_visited;
		unsigned int m_visitedGC;
		int m_GCGen;
		int m_GCcreationframe;
		int m_slot;
		int m_kind;
		int m_rvalueInitType;
		int m_curSlot;
		int m_unused;
	public:
		virtual ~YYObjectBase() = 0;
		virtual RValue &InternalGetYYVarRef(int index) = 0;
	};

	struct YYRECT
	{
		int bbox_left;
		int bbox_top;
		int bbox_right;
		int bbox_bottom;
	};

	class cInstancePathAndTimeline {
	public:
		int i_path_index;
		float i_path_position;
		float i_path_positionprevious;
		float i_path_speed;
		float i_path_scale;
		float i_path_orientation;
		int i_path_end;
		float i_path_xstart;
		float i_path_ystart;
		int i_timeline_index;
		float i_timeline_prevposition;
		float i_timeline_position;
		float i_timeline_speed;
	};

	struct CPhysicsDataGM {
		float *m_physicsVertices;
		bool m_physicsObject;
		bool m_physicsSensor;
		bool m_physicsAwake;
		bool m_physicsKinematic;
		int m_physicsShape;
		int m_physicsGroup;
		float m_physicsDensity;
		float m_physicsRestitution;
		float m_physicsLinearDamping;
		float m_physicsAngularDamping;
		float m_physicsFriction;
		int m_physicsVertexCount;
	};

	enum eGML_TYPE : unsigned int {
		eGMLT_ERROR = 0xffff0000,
		eGMLT_NONE = 0x0,
		eGMLT_DOUBLE = 0x1,
		eGMLT_STRING = 0x2,
		eGMLT_INT32 = 0x4
	};

	struct RToken {
		int kind;
		eGML_TYPE type;
		int ind;
		int ind2;
		RValue value;
		int itemnumb;
		RToken *items;
		int position;
	};

	struct YYGMLFunc
	{
		const char *pName;
		void *ptr;
	};

	class CCode {
		virtual ~CCode() { };
	public:
		CCode *m_pNext;
		int i_kind;
		bool i_compiled;
		char *i_str;
		RToken i_token;
		RValue i_value;
		void *i_pVM;
		void *i_pVMDebugInfo;
		char *i_pCode;
		char *i_pName;
		int i_CodeIndex;
		YYGMLFunc *i_pFunc;
		bool i_watch;
		int i_offset;
		int i_locals;
		int i_args;
		int i_flags;
	};

	class CEvent
	{
	public:
		CCode *e_code;
		int m_OwnerObjectID;
	};

	class CInstance;

	class CObjectGM {
	public:
		char *m_pName;
		CObjectGM *m_pParent;
		CHashMap<int, CObjectGM *, 2> *m_childrenMap;
		CHashMap<unsigned long long, CEvent*, 3> *m_eventsMap;
		CPhysicsDataGM m_physicsData;
		SLinkedList<CInstance> m_Instances;
		SLinkedList<CInstance> m_Instances_Recursive;
		unsigned int m_Flags;
		int m_spriteindex;
		int m_depth;
		int m_parent;
		int m_mask;
		int m_ID;
	};

	class CInstance : public YYObjectBase
	{
	public:
		long long m_CreateCounter;
		CObjectGM *m_pObject;
		struct CPhysicsObject *m_pPhysicsObject;
		struct CSkeletonInstance *m_pSkeletonAnimation;
		unsigned int m_Instflags;
		int i_id;
		int i_object_index;
		int i_sprite_index;
		float i_image_index;
		float i_image_speed;
		float i_image_scalex;
		float i_image_scaley;
		float i_image_angle;
		float i_image_alpha;
		unsigned int i_image_blend;
		float i_x;
		float i_y;
		float i_xstart;
		float i_ystart;
		float i_xprevious;
		float i_yprevious;
		float i_direction;
		float i_speed;
		float i_friction;
		float i_gravity_direction;
		float i_gravity;
		float i_hspeed;
		float i_vspeed;
		YYRECT i_bbox;
		int i_timer[12];
		cInstancePathAndTimeline *m_pPathAndTimeline;
		CCode *i_initcode;
		CCode *i_precreatecode;
		CObjectGM *m_pOldObject;
		int i_layer;
		int i_mask_index;
		short i_nMouseOver;
		CInstance *m_pNext;
		CInstance *m_pPrev;
		SLink m_collisionLink;
		SLink m_dirtyLink;
		SLink m_withLink;
		float i_depth;
		float i_currentdepth;
		float i_lastImageNumber;
		unsigned int i_collisionTestNumber;

		virtual ~CInstance() = 0;
		RValue &GetRVRef(int index);
	};

#pragma pack(pop)

	struct YYStackTrace
	{
		YYStackTrace *m_pNext;
		char *m_pName;
		int m_line;

		static void PrintStackTrace(YYStackTrace *trace) {
			int i = 1;
			while (trace != nullptr) {
				dcout << "[" << i << "] " << trace->m_pName << ", line " << trace->m_line << std::endl;
				trace = trace->m_pNext;
				i++;
			}
		}
		static YYStackTrace **global_trace;
	};
}