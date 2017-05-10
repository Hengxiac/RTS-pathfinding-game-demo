#ifndef __PYENGINE_2_0_EVENT_TYPES_H__
#define __PYENGINE_2_0_EVENT_TYPES_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes

#include "../Utils/PEUUID.h"
#include "../Utils/PEClassDecl.h"

#include "../Math/Matrix4x4.h"
#include "../Math/Vector4.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectEnums.h"

#include "PrimeEngine/Utils/Networkable.h"
#include "PrimeEngine/Networking/GhostManager.h"

// Sibling/Children includes
#include "Event.h"

struct lua_State;

namespace PE {

namespace Events {

struct Event_UPDATE : public Event {
	PE_DECLARE_CLASS(Event_UPDATE);
	Event_UPDATE() : m_frameTime(0) {}
	virtual ~Event_UPDATE(){}
	PrimitiveTypes::Float32 m_frameTime;
};

struct Event_SCENE_GRAPH_UPDATE : public Event {
	PE_DECLARE_CLASS(Event_SCENE_GRAPH_UPDATE);

	Event_SCENE_GRAPH_UPDATE() : m_frameTime(0) {}
	virtual ~Event_SCENE_GRAPH_UPDATE(){}
	PrimitiveTypes::Float32 m_frameTime;
};

struct Event_PRE_RENDER_needsRC : public Event {
	PE_DECLARE_CLASS(Event_PRE_RENDER_needsRC);

	Event_PRE_RENDER_needsRC(int &threadOwnershipMask): m_threadOwnershipMask(threadOwnershipMask){}
	virtual ~Event_PRE_RENDER_needsRC(){}
	Event_PRE_RENDER_needsRC &operator=(const Event_PRE_RENDER_needsRC& c){assert(!"not supported. if need one, need to chnage reference to pointer."); return *this;}
	
	Vector4 m_frustumPlane[6];
	int &m_threadOwnershipMask;
};


struct Event_GATHER_DRAWCALLS : public Event {
	PE_DECLARE_CLASS(Event_GATHER_DRAWCALLS);

	Event_GATHER_DRAWCALLS(int &threadOwnershipMask):m_threadOwnershipMask(threadOwnershipMask){}
	virtual ~Event_GATHER_DRAWCALLS(){}

	Event_GATHER_DRAWCALLS &operator=(const Event_GATHER_DRAWCALLS& c){assert(!"not supported. if need one, need to chnage reference to pointer."); return *this;}
	Matrix4x4 m_projectionViewTransform;
	Matrix4x4 m_projectionTransform;
	Matrix4x4 m_parentWorldTransform;
	Matrix4x4 m_viewInvTransform;

	Vector4 m_frustumPlane[6];
	Vector3 m_eyePos;
	Vector3 m_eyeDir;

	EffectDrawOrder::EffectDrawOrder_ m_drawOrder;
	PrimitiveTypes::Float32 m_frameTime;
	PrimitiveTypes::Float32 m_gameTime;
	int &m_threadOwnershipMask;
};

struct Event_GATHER_DRAWCALLS_Z_ONLY : public Event {
	PE_DECLARE_CLASS(Event_GATHER_DRAWCALLS_Z_ONLY);

	Event_GATHER_DRAWCALLS_Z_ONLY() : m_pZOnlyDrawListOverride(NULL) {}
	virtual ~Event_GATHER_DRAWCALLS_Z_ONLY(){}

	Matrix4x4 m_projectionViewTransform;
	Matrix4x4 m_worldTransform;
	Matrix4x4 m_parentWorldTransform;
	Vector3 m_eyePos;
	void *m_pZOnlyDrawListOverride;
};

struct Event_CALCULATE_TRANSFORMATIONS : public Event {
	PE_DECLARE_CLASS(Event_CALCULATE_TRANSFORMATIONS);
	virtual ~Event_CALCULATE_TRANSFORMATIONS(){}
};

struct Event_PRE_GATHER_DRAWCALLS : public Event {
	PE_DECLARE_CLASS(Event_PRE_GATHER_DRAWCALLS);
	virtual ~Event_PRE_GATHER_DRAWCALLS(){}

	Matrix4x4 m_projectionViewTransform;
	Vector3 m_eyePos;
};


struct Event_POP_SHADERS : public Event {
	PE_DECLARE_CLASS(Event_POP_SHADERS);

	virtual ~Event_POP_SHADERS(){}

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);
};

struct Event_CHANGE_TO_DEBUG_SHADER : public Event {
	PE_DECLARE_CLASS(Event_CHANGE_TO_DEBUG_SHADER);
	virtual ~Event_CHANGE_TO_DEBUG_SHADER(){}

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);
};

struct Event_CLOSED_WINDOW : public Event {
	PE_DECLARE_CLASS(Event_CLOSED_WINDOW);
	virtual ~Event_CLOSED_WINDOW(){}
};

struct Event_PLAY_ANIMATION : public Event {
	PE_DECLARE_CLASS(Event_PLAY_ANIMATION);

	Event_PLAY_ANIMATION() : m_animSetIndex(-1), m_animIndex((PrimitiveTypes::UInt32)(-1)) {}
	virtual ~Event_PLAY_ANIMATION(){}

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);
	PrimitiveTypes::UInt32 m_animSetIndex;
	PrimitiveTypes::UInt32 m_animIndex;
};


struct Event_ANIMATION_ENDED : public Event {
	PE_DECLARE_CLASS(Event_ANIMATION_ENDED);

	Event_ANIMATION_ENDED() : m_animIndex((PrimitiveTypes::UInt32)(-1)) {}
	virtual ~Event_ANIMATION_ENDED(){}

	Handle m_hModel;
	PrimitiveTypes::UInt32 m_animIndex;
};

struct Event_IK_REPORT : public Event {
	PE_DECLARE_CLASS(Event_IK_REPORT);

	Event_IK_REPORT() : m_overflow(false) {}
	virtual ~Event_IK_REPORT(){}

	Handle m_hSkin;
	PrimitiveTypes::Bool m_overflow;
	PrimitiveTypes::Float32 m_angleTurned;
	PrimitiveTypes::Float32 m_angleNeeded;
	PrimitiveTypes::Bool m_turnRight;
};

struct Event_CREATE_LIGHT : public Event {
	PE_DECLARE_CLASS(Event_CREATE_LIGHT);

	Event_CREATE_LIGHT()
	{}
	virtual ~Event_CREATE_LIGHT(){}

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	Vector3 m_pos, m_u, m_v, m_n; // orientation
	Vector4	m_ambient;
	Vector4	m_diffuse;
	Vector4	m_spec;
	Vector3	m_att;
	float	m_spotPower;
	float	m_range;
	bool m_isShadowCaster;
	float	m_type; //0 = point, 1 = directional, 2 = spot
	
	PEUUID m_peuuid;
};

struct Event_CREATE_MESH : public Event {
	PE_DECLARE_CLASS(Event_CREATE_MESH);

	Event_CREATE_MESH(int &threadOwnershipMask) : hasCustomOrientation(false), m_threadOwnershipMask(threadOwnershipMask)
	{
		StringOps::writeToString("", m_meshFilename, 255);
		StringOps::writeToString("Default", m_package, 255);
	}
	virtual ~Event_CREATE_MESH(){}

	Event_CREATE_MESH &operator=(const Event_CREATE_MESH& c){assert(!"not supported. if need one, need to chnage reference to pointer."); return *this;}
	

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	char m_meshFilename[255];
	char m_package[255];
	Vector3 m_pos, m_u, m_v, m_n; 
	bool hasCustomOrientation;
	PEUUID m_peuuid;
	int &m_threadOwnershipMask;
};
struct Event_CREATE_NAVMESH : public Event {
	PE_DECLARE_CLASS(Event_CREATE_NAVMESH);

	Event_CREATE_NAVMESH(int &threadOwnershipMask) : hasCustomOrientation(false), m_threadOwnershipMask(threadOwnershipMask)
	{
		StringOps::writeToString("", m_meshFilename, 255);
		StringOps::writeToString("Default", m_package, 255);
	}
	virtual ~Event_CREATE_NAVMESH() {}

	Event_CREATE_NAVMESH &operator=(const Event_CREATE_NAVMESH& c) { assert(!"not supported. if need one, need to chnage reference to pointer."); return *this; }


	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	char m_meshFilename[255];
	char m_package[255];
	Vector3 m_pos, m_u, m_v, m_n;
	bool hasCustomOrientation;
	PEUUID m_peuuid;
	int &m_threadOwnershipMask;
};
struct Event_CREATE_SKELETON : public Event {
	PE_DECLARE_CLASS(Event_CREATE_SKELETON);

	Event_CREATE_SKELETON(int &threadOwnershipMask) : hasCustomOrientation(false), m_threadOwnershipMask(threadOwnershipMask)
	{
		StringOps::writeToString("", m_skelFilename, 255);
		StringOps::writeToString("Default", m_package, 255);
	}
	virtual ~Event_CREATE_SKELETON(){}

	Event_CREATE_SKELETON &operator=(const Event_CREATE_SKELETON& c){assert(!"not supported. if need one, need to change reference to pointer."); return *this;}
	

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	char m_skelFilename[255];
	char m_package[255];
	Vector3 m_pos, m_u, m_v, m_n; 

	PEUUID m_peuuid;
	Handle resHandle;
	bool hasCustomOrientation;

	int &m_threadOwnershipMask;
};


struct Event_CREATE_ANIM_SET : public Event {
	PE_DECLARE_CLASS(Event_CREATE_ANIM_SET);

	Event_CREATE_ANIM_SET(int &threadOwnershipMask) :m_threadOwnershipMask(threadOwnershipMask)
	{
		StringOps::writeToString("", animSetFilename, 255);
		StringOps::writeToString("Default", m_package, 255);
	}
	virtual ~Event_CREATE_ANIM_SET(){}

	Event_CREATE_ANIM_SET &operator=(const Event_CREATE_ANIM_SET& c){assert(!"not supported. if need one, need to change reference to pointer."); return *this;}


	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	char animSetFilename[255];
	char m_package[255];

	PEUUID m_peuuid;
	Handle resHandle;
	int &m_threadOwnershipMask;
};


struct Event_CREATE_NAVGRID : public Event {
	PE_DECLARE_CLASS(Event_CREATE_NAVGRID);

	Event_CREATE_NAVGRID() : hasCustomOrientation(false)
	{
		StringOps::writeToString("", gameObjName, 255);
	}
	virtual ~Event_CREATE_NAVGRID(){}

	char gameObjName[255];
	Vector3 pos; 
	char gridMeshName[255];
	char gridFileName[255];
	Vector3 u; 
	Vector3 v; 
	Vector3 n;
	bool hasCustomOrientation;
};

//Physics
struct Event_PHYSICS_END : public Event {
	PE_DECLARE_CLASS(Event_PHYSICS_END);
	virtual ~Event_PHYSICS_END(){}

};

struct Event_PHYSICS_START : public Event {
	PE_DECLARE_CLASS(Event_PHYSICS_START);

	Event_PHYSICS_START() : m_frameTime(0)
	{}
	virtual ~Event_PHYSICS_START(){}

	PrimitiveTypes::Float32 m_frameTime;
};

struct Event_VORTEX_ENDED : public Event {
	PE_DECLARE_CLASS(Event_VORTEX_ENDED);
	virtual ~Event_VORTEX_ENDED(){}

};

struct Event_MOVE : public Event {
	PE_DECLARE_CLASS(Event_MOVE);

	Event_MOVE() : m_delta(0){}
	virtual ~Event_MOVE(){}

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	PrimitiveTypes::Float32 m_delta;
	Vector3 m_dir;
};

struct Event_SET_DEBUG_TARGET_HANDLE : public Event {
	PE_DECLARE_CLASS(Event_SET_DEBUG_TARGET_HANDLE);

	Event_SET_DEBUG_TARGET_HANDLE(){}
	virtual ~Event_SET_DEBUG_TARGET_HANDLE(){}

	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	Handle m_hDebugTarget;
	int m_debugEvent;

};

struct Event_Resolve_Collision : public Event {
	PE_DECLARE_CLASS(Event_Resolve_Collision);

	Event_Resolve_Collision(PE::Handle first, PE::Handle second)
	{
		moving = first;
		collided = second;
	}
	virtual ~Event_Resolve_Collision() {}

	PE::Handle moving;
	PE::Handle collided;
};

struct Event_CONSTRUCT_SOUND : public Event {
	PE_DECLARE_CLASS(Event_CONSTRUCT_SOUND);

	Event_CONSTRUCT_SOUND()
	{
		StringOps::writeToString("", m_waveBankFilename, 255);
	}
	virtual ~Event_CONSTRUCT_SOUND() {}

	char m_waveBankFilename[255];
};

struct Event_ADDED_AS_COMPONENT : public Event {
	PE_DECLARE_CLASS(Event_ADDED_AS_COMPONENT);

	Event_ADDED_AS_COMPONENT():m_pAllowedEventsToPropagateToParent(NULL){}
	virtual ~Event_ADDED_AS_COMPONENT(){}
	
	Handle m_hComponentParent;
	int *m_pAllowedEventsToPropagateToParent;
};

struct Event_CHARACTER_HIT_BY_MELEE : public Event {
	PE_DECLARE_CLASS(Event_CHARACTER_HIT_BY_MELEE);

	Event_CHARACTER_HIT_BY_MELEE(){}
	virtual ~Event_CHARACTER_HIT_BY_MELEE(){}

	PrimitiveTypes::Float32 force;
	Vector3 direction;
	Handle attacker;
};


struct Event_SERVER_CLIENT_CONNECTION_ACK : public Event, public Networkable {
	PE_DECLARE_CLASS(Event_SERVER_CLIENT_CONNECTION_ACK);
	PE_DECLARE_NETWORKABLE_CLASS

	Event_SERVER_CLIENT_CONNECTION_ACK(PE::GameContext &context);
	virtual ~Event_SERVER_CLIENT_CONNECTION_ACK();

	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);

	

	PrimitiveTypes::Int32 m_clientId; // id given to client by server
};

struct Event_Update_Ghost : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Update_Ghost);

	Event_Update_Ghost() {}

	virtual ~Event_Update_Ghost() {}
	virtual int constructFromStream(char *pDataStream);

	bool stateMask[PE::Components::GhostManager::MASK_SIZE];
	Matrix4x4 m_transform;
	Matrix4x4 m_transform2;
	PrimitiveTypes::Int32 m_intVal[2];
};

struct Event_Create_Ghost : public Event_Update_Ghost {
	PE_DECLARE_CLASS(Event_Create_Ghost);

	Event_Create_Ghost() {}

	virtual ~Event_Create_Ghost() {}
	virtual int constructFromStream(char *pDataStream);

	int m_ghostType;
	int m_ghostId;

};

struct Event_Destroy_Ghost : public Event_Create_Ghost {
	PE_DECLARE_CLASS(Event_Destroy_Ghost);

	Event_Destroy_Ghost() {}

	virtual ~Event_Destroy_Ghost() {}
	virtual int constructFromStream(char *pDataStream);

	int m_ghostType;
	int m_ghostId;

};

struct Event_SEND_GHOST_TO_NEW_CLIENT: public Event{

	PE_DECLARE_CLASS(Event_SEND_GHOST_TO_NEW_CLIENT);

	Event_SEND_GHOST_TO_NEW_CLIENT() {}
	virtual ~Event_SEND_GHOST_TO_NEW_CLIENT(){}

	PrimitiveTypes::Int32 m_clientId; // id given to client by server
};

struct Event_PRE_PHYSICS : public Event {
	PE_DECLARE_CLASS(Event_PRE_PHYSICS);

	virtual ~Event_PRE_PHYSICS() {}

	PrimitiveTypes::Float32 m_frameTime;
};

struct Event_PHYSICS_UPDATE : public Event {
	PE_DECLARE_CLASS(Event_PHYSICS_UPDATE);

	Event_PHYSICS_UPDATE() : m_frameTime(0) {}
	virtual ~Event_PHYSICS_UPDATE() {}

	PrimitiveTypes::Float32 m_frameTime;
};

struct Event_POST_PHYSICS : public Event {
	PE_DECLARE_CLASS(Event_POST_PHYSICS);
	virtual ~Event_POST_PHYSICS() {}
};
struct Event_UPDATE_TARGET_POS : public Event {
	PE_DECLARE_CLASS(Event_UPDATE_TARGET_POS);

	Event_UPDATE_TARGET_POS() : m_targetPos{ 0.0f, 0.0f, 0.0f }, deleteOld(true) {}
	virtual ~Event_UPDATE_TARGET_POS() {}

	Vector3 m_targetPos;
	bool deleteOld;
	bool patrolling;
};

struct Event_CHECK_SELECTED : public Event {
	PE_DECLARE_CLASS(Event_CHECK_SELECTED);

	Event_CHECK_SELECTED() : multiSelect(false) {}
	virtual ~Event_CHECK_SELECTED() {}

	Vector3 m_target;
	Vector3 m_origin;
	bool multiSelect;
};

struct Event_UPDATE_GROUP : public Event {
	PE_DECLARE_CLASS(Event_UPDATE_GROUP);

	Event_UPDATE_GROUP() : isStore(false), num(-1) {}
	virtual ~Event_UPDATE_GROUP() {}

	int num;
	bool isStore;
};

struct Event_TOGGLE_DEBUG_RENDER : public Event {
	PE_DECLARE_CLASS(Event_TOGGLE_DEBUG_RENDER);

	Event_TOGGLE_DEBUG_RENDER() {}
	virtual ~Event_TOGGLE_DEBUG_RENDER() {}

};

struct Event_CAMERA_STORAGE : public Event {
	PE_DECLARE_CLASS(Event_CAMERA_STORAGE);

	Event_CAMERA_STORAGE() : isStore(false), num(-1) {}
	virtual ~Event_CAMERA_STORAGE() {}
	int num;
	bool isStore;
};

struct Event_CONTROL_GROUP : public Event {
	PE_DECLARE_CLASS(Event_CONTROL_GROUP);

	Event_CONTROL_GROUP() : isStore(false), num(-1) {}
	virtual ~Event_CONTROL_GROUP() {}
	int num;
	bool isStore;
};
};
};


#endif
