#ifndef _CHARACTER_CONTROL_GAME_SERVER_OBJ_MANAGER_ADDON_
#define _CHARACTER_CONTROL_GAME_SERVER_OBJ_MANAGER_ADDON_

#include "GameObjectMangerAddon.h"
#include "Events/Events.h"

#include "WayPoint.h"

namespace CharacterControl
{
namespace Components
{

// This struct will be added to GameObjectManager as component
// as a result events sent to game object manager will be able to get to this component
// so we can create custom game objects through this class
struct ServerGameObjectManagerAddon : public GameObjectManagerAddon
{
	PE_DECLARE_SINGLETON_CLASS(ServerGameObjectManagerAddon); // creates a static handle and GteInstance*() methods. still need to create construct

	ServerGameObjectManagerAddon(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) : GameObjectManagerAddon(context, arena, hMyself), nextGhostId(0)
	{}

	enum GhostType
	{
		TANK = 0,
		SHELL = 1,
	};
	// sub-component and event registration
	virtual void addDefaultComponents() ;

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_MoveTank);
	virtual void do_MoveTank(PE::Events::Event *pEvt);
	
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_SendGhostToNewClient);
	virtual void do_SendGhostToNewClient(PE::Events::Event *pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Create_Ghost);
	virtual void do_Create_Ghost(PE::Events::Event *pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Destroy_Ghost);
	virtual void do_Destroy_Ghost(PE::Events::Event *pEvt);
	
	void createTank(int index, int &threadOwnershipMask);
	void createShell(int ghostId, int clientIndex, const Matrix4x4 &t);

	int nextGhostId;
	//////////////////////////////////////////////////////////////////////////
	// Game Specific functionality
	//////////////////////////////////////////////////////////////////////////
	//
};


}
}

#endif
