#ifndef _CHARACTER_CONTROL_EVENTS_
#define _CHARACTER_CONTROL_EVENTS_

#include "PrimeEngine/Events/StandardEvents.h"

namespace CharacterControl
{
namespace Events
{
struct Event_CreateSoldierNPC : public PE::Events::Event_CREATE_MESH
{
	PE_DECLARE_CLASS(Event_CreateSoldierNPC);

	Event_CreateSoldierNPC(int &threadOwnershipMask): PE::Events::Event_CREATE_MESH(threadOwnershipMask){}
	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	int m_npcType;
	char m_gunMeshName[64];
	char m_gunMeshPackage[64];
	char m_patrolWayPoint[32];
};
struct Event_CreateTarget : public PE::Events::Event_CREATE_MESH
{
	PE_DECLARE_CLASS(Event_CreateTarget);

	Event_CreateTarget(int &threadOwnershipMask) : PE::Events::Event_CREATE_MESH(threadOwnershipMask) {}
	// override SetLuaFunctions() since we are adding custom Lua interface
	static void SetLuaFunctions(PE::Components::LuaEnvironment *pLuaEnv, lua_State *luaVM);

	// Lua interface prefixed with l_
	static int l_Construct(lua_State* luaVM);

	Matrix4x4 m_base;
	char m_name[32];
	char m_movementWayPoint[32];
};
struct Event_MoveTank_C_to_S : public PE::Events::Event, public PE::Networkable
{
	PE_DECLARE_CLASS(Event_MoveTank_C_to_S);
	PE_DECLARE_NETWORKABLE_CLASS

	Event_MoveTank_C_to_S(PE::GameContext &context);
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);


	Matrix4x4 m_transform;
};


struct Event_MoveTank_S_to_C : public Event_MoveTank_C_to_S
{
	PE_DECLARE_CLASS(Event_MoveTank_S_to_C);
	PE_DECLARE_NETWORKABLE_CLASS

	Event_MoveTank_S_to_C(PE::GameContext &context);
	// Netoworkable:
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);

	int m_clientTankId;
};


// tank input controls

struct Event_Tank_Throttle : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Throttle);

	Event_Tank_Throttle(){}
	virtual ~Event_Tank_Throttle(){}

	Vector3 m_relativeMove;
};

struct Event_Tank_Turn : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Turn);

	Event_Tank_Turn(){}
	virtual ~Event_Tank_Turn(){}

	Vector3 m_relativeRotate;
};

struct Event_Tank_Aim : public Event_Tank_Turn {
	PE_DECLARE_CLASS(Event_Tank_Aim);

	Event_Tank_Aim() {}
	virtual ~Event_Tank_Aim() {}
};

struct Event_Tank_Fire : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Fire);

	Event_Tank_Fire() {}
	virtual ~Event_Tank_Fire() {}
};

struct Event_Tank_Reload : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Tank_Reload);

	Event_Tank_Reload() {}
	virtual ~Event_Tank_Reload() {}

	Vector3 m_relativeRotate;
};

struct Event_Shell_Splash : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Shell_Splash);

	Event_Shell_Splash() {}
	virtual ~Event_Shell_Splash() {}

	Vector3 m_exploadePos;
	int damage;
	float damageRange;
};

struct Event_Create_Shell : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Create_Shell);

	Event_Create_Shell() {}
	virtual ~Event_Create_Shell() {}

	Matrix4x4 m_transform;
	bool m_active;
};

struct Event_Get_Hit : public PE::Events::Event {
	PE_DECLARE_CLASS(Event_Get_Hit);

	Event_Get_Hit(int damage) { m_damage = damage; }
	virtual ~Event_Get_Hit() {}

	int m_damage;
};

struct Event_Restart_Game : public PE::Events::Event, public PE::Networkable 
{
	PE_DECLARE_CLASS(Event_Restart_Game);
	PE_DECLARE_NETWORKABLE_CLASS
	Event_Restart_Game(PE::GameContext &context);
	virtual ~Event_Restart_Game() {}
	virtual int packCreationData(char *pDataStream);
	virtual int constructFromStream(char *pDataStream);


	// Factory function used by network
	static void *FactoryConstruct(PE::GameContext&, PE::MemoryArena);

};

struct Event_Client_Restart : public PE::Events::Event
{
	PE_DECLARE_CLASS(Event_Client_Restart);
	
	Event_Client_Restart() {}
	virtual ~Event_Client_Restart() {}
};

}; // namespace Events
}; // namespace CharacterControl

#endif
