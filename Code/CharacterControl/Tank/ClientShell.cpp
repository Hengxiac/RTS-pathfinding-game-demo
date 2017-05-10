#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Inter-Engine includes
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Networking/EventManager.h"
#include "PrimeEngine/Networking/Client/ClientNetworkManager.h"
#include "CharacterControl/Events/Events.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"
#include "PrimeEngine/Events/StandardGameEvents.h"
#include "PrimeEngine/Events/EventQueueManager.h"
#include "PrimeEngine/Events/StandardControllerEvents.h"
#include "CharacterControl/CharacterControlContext.h"
#include "PrimeEngine/PrimeEngineIncludes.h"

#include "ClientShell.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl {
namespace Components {

PE_IMPLEMENT_CLASS1(ClientShell, Component);
    
ClientShell::ClientShell(PE::GameContext &context, PE::MemoryArena arena,
	PE::Handle myHandle, Vector3 speed, bool isActive, int damage, float networkPingInterval)
: Component(context, arena, myHandle)
, Networkable(context, this)
, m_speed(speed)
, m_active(isActive)
, m_damage(damage)
, m_time(0)
, m_networkPingTimer(networkPingInterval)
, m_networkPingInterval(networkPingInterval)
, m_ghostID(-1)
, ghostCreated(false)
, toRemoveMask(false)
, toRemovePhys(false)
{
}
    
void ClientShell::addDefaultComponents()
{
    Component::addDefaultComponents();
        
    PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, ClientShell::do_UPDATE);
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_Update_Ghost, ClientShell::do_Update_Ghost);
	//PE_REGISTER_EVENT_HANDLER(CharacterControl::Events::Event_Restart_Game, ServerTank::do_Restart_Game);
	// note: these event handlers will be registered only when one tank is activated as client tank (i.e. driven by client input on this machine)
// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Throttle, TankController::do_Tank_Throttle);
// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Turn, TankController::do_Tank_Turn);

}


void ClientShell::do_UPDATE(PE::Events::Event *pEvt)
{
	if (m_active)
	{
		PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);

		moveShell(pRealEvt->m_frameTime);

		m_time += pRealEvt->m_frameTime;
		m_networkPingTimer += pRealEvt->m_frameTime;

		if (m_networkPingTimer > m_networkPingInterval)
		{
			// send client authoritative position event
			/*CharacterControl::Events::Event_MoveTank_C_to_S evt(*m_pContext);
			evt.m_transform = pFirstSN->m_base;

			ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
			pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);*/

			ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
			int ghostId = pNetworkManager->getNetworkContext().getGhostManager()->findGhostID(m_hMyself);
			if (ghostId == -1 && !ghostCreated)
			{
				pNetworkManager->getNetworkContext().getGhostManager()->addToLibrary(m_hMyself, m_ghostID);
				ghostCreated = true;
				pNetworkManager->getNetworkContext().getGhostManager()->scheduleGhost(this, true, m_ghostID, true, 1);
			}

			if (m_ghostID >= 0 && toRemoveMask)
			{
				pNetworkManager->getNetworkContext().getGhostManager()->scheduleGhost(this, true, m_ghostID, true, 1);			
				//pNetworkManager->getNetworkContext().getGhostManager()->removeFromLibrary(m_ghostID);
				toRemoveMask = false;
				m_active = false;
			}
			
			if (ghostId >= 0)
			{
				pNetworkManager->getNetworkContext().getGhostManager()->scheduleGhost(this, true, ghostId);
			}
			m_networkPingTimer = 0.0f;
		}
	}
	else
	{
		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		if (ghostCreated == false)
		{
			
			int ghostId = pNetworkManager->getNetworkContext().getGhostManager()->findGhostID(m_hMyself);
			if (ghostId == -1)
			{
				pNetworkManager->getNetworkContext().getGhostManager()->addToLibrary(m_hMyself, m_ghostID);
				ghostCreated = true;
			}
		}
		if (m_ghostID >= 0 && toRemoveMask)
		{
			//pNetworkManager->getNetworkContext().getGhostManager()->removeFromLibrary(m_ghostID);
			toRemoveMask = false;
		}
	}

	if (toRemovePhys)
	{
		int pCCount = countComponents<PhysicsComponent>();

		for (int i = 0; i < pCCount; i++)
		{
			PE::Handle hPC = getFirstComponentHandle<PhysicsComponent>();
			removeComponent(hPC);
			m_pContext->getPhysicsManager()->removePhys(hPC);
		}
		toRemovePhys = false;
	}
}


void ClientShell::do_Update_Ghost(PE::Events::Event *pEvt)
{
	if (m_active)
		return;
	
	PE::Events::Event_Update_Ghost *pRealEvt = (PE::Events::Event_Update_Ghost *)(pEvt);

	if (pRealEvt->stateMask[POSITION])
	{
		overrideTransform(pRealEvt->m_transform);
	}

	/*for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
	{
		int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

		m_clientProcessing = i;
		pNetworkManager->m_clientConnections[i].getGhostManager()->scheduleGhost(this, true, ghostId);
	}*/
}

void ClientShell::overrideTransform(Matrix4x4 &t)
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

	PE::Handle hFisrtPC = getFirstComponentHandle<PhysicsComponent>();
	if (!hFisrtPC.isValid())
	{
		assert(!"wrong setup. must have physics component referenced");
		return;
	}

	PhysicsComponent *pFirstPC = hFisrtPC.getObject<PhysicsComponent>();

	pFirstPC->setTransform(t);
}

void ClientShell::moveShell(float time)
{
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	if (!hFisrtSN.isValid())
	{
		assert(!"wrong setup. must have scene node referenced");
		return;
	}

	SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

	if (pFirstSN->m_base.getPos().m_y > -0.1f)
	{
		float distance = 20.0f * time;

		pFirstSN->m_base.moveForward(distance);

		pFirstSN->m_base.turnDown(0.07f * time);

		PE::Handle hFisrtPC = getFirstComponentHandle<PhysicsComponent>();
		if (!hFisrtPC.isValid())
		{
			assert(!"wrong setup. must have physics component referenced");
			return;
		}

		PhysicsComponent *pFirstPC = hFisrtPC.getObject<PhysicsComponent>();

		pFirstPC->setTransform(pFirstSN->m_base);

		if (ghostCreated)
		{
			ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
			int ghostId = pNetworkManager->getNetworkContext().getGhostManager()->findGhostID(m_hMyself);

			pNetworkManager->getNetworkContext().getGhostManager()->updateStateMask(ghostId, POSITION);
		}	
	}
	else
	{
		PE::Handle h("Event_Destroy_Ghost", sizeof(Event_Destroy_Ghost));
		Event_Destroy_Ghost *pEvt = new(h) Event_Destroy_Ghost();

		pEvt->m_ghostId = m_ghostID;

		m_pContext->getGameObjectManager()->handleEvent(pEvt);
		h.release();
	}
	
}

int ClientShell::packStateData(char *pDataStream)
{
	int size = 0;
	
	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
	int ghostId = pNetworkManager->getNetworkContext().getGhostManager()->findGhostID(m_hMyself);

	Array<bool> stateMask = pNetworkManager->getNetworkContext().getGhostManager()->findStateMask(ghostId);
	
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

	return size;
}

}
}
