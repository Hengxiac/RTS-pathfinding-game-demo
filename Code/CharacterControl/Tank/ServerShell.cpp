#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Inter-Engine includes
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Networking/EventManager.h"
#include "PrimeEngine/Networking/Server/ServerNetworkManager.h"
#include "CharacterControl/Events/Events.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"
#include "PrimeEngine/Events/StandardGameEvents.h"
#include "PrimeEngine/Events/EventQueueManager.h"
#include "PrimeEngine/Events/StandardControllerEvents.h"
#include "CharacterControl/CharacterControlContext.h"
#include "PrimeEngine/PrimeEngineIncludes.h"

#include "ServerShell.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

// Arkane Control Values
#define Analog_To_Digital_Trigger_Distance 0.5f
static float Debug_Fly_Speed = 8.0f; //Units per second
#define Debug_Rotate_Speed 2.0f //Radians per second
#define Player_Keyboard_Rotate_Speed 20.0f //Radians per second

namespace CharacterControl {
namespace Components {

PE_IMPLEMENT_CLASS1(ServerShell, Component);
    
ServerShell::ServerShell(PE::GameContext &context, PE::MemoryArena arena,
	PE::Handle myHandle,  float networkPingInterval)
: Component(context, arena, myHandle)
, m_time(0)
, m_networkPingTimer(0)
, m_networkPingInterval(networkPingInterval)
, Networkable(context, this)
, m_clientProcessing(-1)

{
}
    
void ServerShell::addDefaultComponents()
{
    Component::addDefaultComponents();
        
    PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, ServerShell::do_UPDATE);
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_Update_Ghost, ServerShell::do_Update_Ghost);
	//PE_REGISTER_EVENT_HANDLER(CharacterControl::Events::Event_Restart_Game, ServerShell::do_Restart_Game);
	// note: these event handlers will be registered only when one tank is activated as client tank (i.e. driven by client input on this machine)
// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Throttle, TankController::do_Tank_Throttle);
// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Turn, TankController::do_Tank_Turn);

}


void ServerShell::do_UPDATE(PE::Events::Event *pEvt)
{
	PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);

	m_time += pRealEvt->m_frameTime;
	m_networkPingTimer += pRealEvt->m_frameTime;
    
	if (m_networkPingTimer > m_networkPingInterval)
	{
		// send client authoritative position event
		/*CharacterControl::Events::Event_MoveTank_C_to_S evt(*m_pContext);
		evt.m_transform = pFirstSN->m_base;

		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);*/

		ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());

		for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
		{
			int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

			m_clientProcessing = i;
			pNetworkManager->m_clientConnections[i].getGhostManager()->scheduleGhost(this, true, ghostId);
		}

		m_networkPingTimer = 0.0f;
	}
}

void ServerShell::do_Update_Ghost(PE::Events::Event *pEvt)
{
	PE::Events::Event_Update_Ghost *pRealEvt = (PE::Events::Event_Update_Ghost *)(pEvt);

	ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());


	if (pRealEvt->stateMask[POSITION])
	{
		overrideTransform(pRealEvt->m_transform);

		for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
		{
			if (pRealEvt->m_networkClientId == i)
				continue;
			int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, POSITION);
		}
	}

	/*for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
	{
		int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

		m_clientProcessing = i;
		pNetworkManager->m_clientConnections[i].getGhostManager()->scheduleGhost(this, true, ghostId);
	}*/
}

void ServerShell::overrideTransform(Matrix4x4 &t)
{
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	if (!hFisrtSN.isValid())
	{
		assert(!"wrong setup. must have scene node referenced");
		return;
	}

	SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

	pFirstSN->m_base.setU(t.getU());
	pFirstSN->m_base.setV(t.getV());
	pFirstSN->m_base.setN(t.getN());
	pFirstSN->m_base.setPos(t.getPos());
}


int ServerShell::packStateData(char *pDataStream)
{
	int size = 0;
	
	if (m_clientProcessing >= 0)
	{
		ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());
		int ghostId = pNetworkManager->m_clientConnections[m_clientProcessing].getGhostManager()->findGhostID(m_hMyself);

		Array<bool> stateMask = pNetworkManager->m_clientConnections[m_clientProcessing].getGhostManager()->findStateMask(ghostId);

		//size += PE::Components::StreamManager::WriteBool(true, pDataStream);
		if (stateMask[HITPOINT] > 0)
		{
			size += PE::Components::StreamManager::WriteInt32(0, &pDataStream[size]);
		}

		if (stateMask[SHELLLEFT] > 0)
		{
			size += PE::Components::StreamManager::WriteInt32(0, &pDataStream[size]);
		}

		if (stateMask[POSITION] > 0)
		{
			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return 0;
			}

			SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

			size += PE::Components::StreamManager::WriteMatrix4x4(pFirstSN->m_base, &pDataStream[size]);
		}

		if (stateMask[TOPPOSITION] > 0)
		{
			Matrix4x4 dummy;
			dummy.loadIdentity();
			size += PE::Components::StreamManager::WriteMatrix4x4(dummy, &pDataStream[size]);
		}
	}

	return size;
}

}
}
