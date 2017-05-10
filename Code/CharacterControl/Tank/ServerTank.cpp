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

#include "ServerTank.h"

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

PE_IMPLEMENT_CLASS1(ServerTank, Component);
    
ServerTank::ServerTank(PE::GameContext &context, PE::MemoryArena arena,
	PE::Handle myHandle, float speed, Matrix4x4 &spawnPos, 
	float networkPingInterval, int hitPoint, int shellVolume)
: Component(context, arena, myHandle)
, m_timeSpeed(speed)
, m_time(0)
, m_counter(0)
, m_networkPingTimer(0)
, m_networkPingInterval(networkPingInterval)
, Networkable(context, this)
, m_clientProcessing(-1)
, m_hitPoint(hitPoint)
, m_shellLeft(shellVolume)
, m_shellMax(shellVolume)
, m_HPMax(hitPoint)
{
	m_spawnPos = spawnPos;
	m_topTransform.loadIdentity();
}
    
void ServerTank::addDefaultComponents()
{
    Component::addDefaultComponents();
        
    PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, ServerTank::do_UPDATE);
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_Update_Ghost, ServerTank::do_Update_Ghost);
	PE_REGISTER_EVENT_HANDLER(CharacterControl::Events::Event_Restart_Game, ServerTank::do_Restart_Game);
	// note: these event handlers will be registered only when one tank is activated as client tank (i.e. driven by client input on this machine)
// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Throttle, TankController::do_Tank_Throttle);
// 	PE_REGISTER_EVENT_HANDLER(Event_Tank_Turn, TankController::do_Tank_Turn);

}


void ServerTank::do_UPDATE(PE::Events::Event *pEvt)
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

void ServerTank::do_Restart_Game(PE::Events::Event *pEvt)
{
	overrideTransform(m_spawnPos);
	m_hitPoint = m_HPMax;
	m_shellLeft = m_shellMax;

	m_topTransform.setU({ 0.0f, 0.0f,0.0f });
	m_topTransform.setV({ 0.0f, 0.0f,0.0f });
	m_topTransform.setN({ 0.0f, 0.0f,0.0f });
	m_topTransform.setPos({ 0.0f, 0.0f,0.0f });
	
	m_topTransform.loadIdentity();
	ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());
	for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
	{
		if (pNetworkManager->m_clientConnections[i].getGhostManager() >= 0)
		{
			int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, HITPOINT);
			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, SHELLLEFT);
			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, POSITION);
			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, TOPPOSITION);
		}
	}

}

void ServerTank::do_Update_Ghost(PE::Events::Event *pEvt)
{
	PE::Events::Event_Update_Ghost *pRealEvt = (PE::Events::Event_Update_Ghost *)(pEvt);

	ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());

	if (pRealEvt->stateMask[HITPOINT])
	{
		m_hitPoint = pRealEvt->m_intVal[0];

		for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
		{
			if (pRealEvt->m_networkClientId == i)
				continue;
			int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, HITPOINT);
		}

	}

	if (pRealEvt->stateMask[SHELLLEFT])
	{
		m_shellLeft = pRealEvt->m_intVal[1];

		for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
		{
			if (pRealEvt->m_networkClientId == i)
				continue;
			int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, SHELLLEFT);
		}

	}

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

	if (pRealEvt->stateMask[TOPPOSITION])
	{
		m_topTransform.setU(pRealEvt->m_transform2.getU());
		m_topTransform.setV(pRealEvt->m_transform2.getV());
		m_topTransform.setN(pRealEvt->m_transform2.getN());
		m_topTransform.setPos(pRealEvt->m_transform2.getPos());

		for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
		{
			if (pRealEvt->m_networkClientId == i)
				continue;
			int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

			pNetworkManager->m_clientConnections[i].getGhostManager()->updateStateMask(ghostId, TOPPOSITION);
		}
	}

	/*for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
	{
		int ghostId = pNetworkManager->m_clientConnections[i].getGhostManager()->findGhostID(m_hMyself);

		m_clientProcessing = i;
		pNetworkManager->m_clientConnections[i].getGhostManager()->scheduleGhost(this, true, ghostId);
	}*/
}

void ServerTank::overrideTransform(Matrix4x4 &t)
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


int ServerTank::packStateData(char *pDataStream)
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
			size += PE::Components::StreamManager::WriteInt32(m_hitPoint, &pDataStream[size]);
		}

		//size += PE::Components::StreamManager::WriteBool(false, pDataStream);

		if (stateMask[SHELLLEFT] > 0)
		{
			size += PE::Components::StreamManager::WriteInt32(m_shellLeft, &pDataStream[size]);
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
			size += PE::Components::StreamManager::WriteMatrix4x4(m_topTransform, &pDataStream[size]);
		}
	}

	return size;
}

}
}
