#include "ServerGameObjectManagerAddon.h"

#include "PrimeEngine/Lua/Server/ServerLuaEnvironment.h"
#include "PrimeEngine/Networking/Server/ServerNetworkManager.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"

#include "Characters/SoldierNPC.h"
#include "WayPoint.h"
#include "Characters/Target.h"
#include "Tank/ServerTank.h"
#include "Tank/ServerShell.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;
using namespace CharacterControl::Components;

namespace CharacterControl{
namespace Components
{
PE_IMPLEMENT_CLASS1(ServerGameObjectManagerAddon, GameObjectManagerAddon); // creates a static handle and GteInstance*() methods. still need to create construct

void ServerGameObjectManagerAddon::addDefaultComponents()
{
	GameObjectManagerAddon::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Event_MoveTank_C_to_S, ServerGameObjectManagerAddon::do_MoveTank);
	PE_REGISTER_EVENT_HANDLER(Event_SEND_GHOST_TO_NEW_CLIENT, ServerGameObjectManagerAddon::do_SendGhostToNewClient);
	PE_REGISTER_EVENT_HANDLER(Event_Create_Ghost, ServerGameObjectManagerAddon::do_Create_Ghost);
	PE_REGISTER_EVENT_HANDLER(Event_Destroy_Ghost, ServerGameObjectManagerAddon::do_Destroy_Ghost);
}

void ServerGameObjectManagerAddon::do_MoveTank(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_MoveTank_C_to_S>());

	Event_MoveTank_C_to_S *pTrueEvent = (Event_MoveTank_C_to_S*)(pEvt);

	// need to send this event to all clients except the client it came from

	Event_MoveTank_S_to_C fwdEvent(*m_pContext);
	fwdEvent.m_transform = pTrueEvent->m_transform;
	fwdEvent.m_clientTankId = pTrueEvent->m_networkClientId; // need to tell cleints which tank to move

	ServerNetworkManager *pNM = (ServerNetworkManager *)(m_pContext->getNetworkManager());
	pNM->scheduleEventToAllExcept(&fwdEvent, m_pContext->getGameObjectManager(), pTrueEvent->m_networkClientId);

}

void ServerGameObjectManagerAddon::do_SendGhostToNewClient(PE::Events::Event *pEvt)
{
	Event_SEND_GHOST_TO_NEW_CLIENT *pTrueEvent = (Event_SEND_GHOST_TO_NEW_CLIENT*)(pEvt);

	int netIndex = pTrueEvent->m_clientId;

	ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());

	PE::Handle *pHC = m_components.getFirstPtr();

	
	{
		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (netIndex == 0)
			{
				if (pC->isInstanceOf<ServerTank>())
				{
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->addToLibrary((*pHC), nextGhostId++);
					ServerTank *pTK = (ServerTank *)(pC);

					pTK->m_clientProcessing = netIndex;
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->scheduleGhost(pTK, true, nextGhostId-1, true, TANK);
				}

				else if (pC->isInstanceOf<ServerShell>())
				{
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->addToLibrary((*pHC), nextGhostId++);
					ServerShell *pTK = (ServerShell *)(pC);

					pTK->m_clientProcessing = netIndex;
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->scheduleGhost(pTK, true, nextGhostId - 1, true, SHELL);
				}
				
			}

			else
			{
				if (pC->isInstanceOf<ServerTank>())
				{
					int ghostId = pNetworkManager->m_clientConnections[0].getGhostManager()->findGhostID((*pHC));
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->addToLibrary((*pHC), ghostId);

					ServerTank *pTK = (ServerTank *)(pC);
					
					pTK->m_clientProcessing = netIndex;
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->scheduleGhost(pTK, true, ghostId, true, TANK);
				}

				else if (pC->isInstanceOf<ServerShell>())
				{
					int ghostId = pNetworkManager->m_clientConnections[0].getGhostManager()->findGhostID((*pHC));
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->addToLibrary((*pHC), ghostId);
					ServerShell *pTK = (ServerShell *)(pC);

					pTK->m_clientProcessing = netIndex;
					pNetworkManager->m_clientConnections[netIndex].getGhostManager()->scheduleGhost(pTK, true, ghostId, true, SHELL);
				}
			}
			
		}
	}
}

void ServerGameObjectManagerAddon::do_Create_Ghost(PE::Events::Event *pEvt)
{
	//assert(pEvt->isInstanceOf<Event_Create_Ghost>());

	Event_Create_Ghost *pTrueEvent = (Event_Create_Ghost*)(pEvt);

	if (pTrueEvent->m_ghostType == SHELL)
	{
		createShell(pTrueEvent->m_ghostId, pTrueEvent->m_networkClientId, pTrueEvent->m_transform);
	}

}

void ServerGameObjectManagerAddon::createTank(int index, int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    Tank

	//game object manager
	//  TankController
	//    scene node
	// now add game objects

	Vector3 spawnPos((index % 2 * 2 - 1) * 36.0f - 6.0f * index, 0, (index % 2 * 2 - 1) * 41.0f);
	/*Matrix4x4 base;
	
	base.loadIdentity();
	base.setPos(spawnPos);*/
	
	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	pSN->m_base.loadIdentity();
	pSN->m_base.setPos(spawnPos);

	if ((index % 2))
	{
		pSN->m_base.turnRight(3.1415f);
	}
	//RootSceneNode::Instance()->addComponent(hSN);
	Matrix4x4 spawnMatrix(pSN->m_base);

	int hitPoint = 200;//(index % 2) * 200 + 200;
	int shellMax = 5;//(index % 2) * 2 + 5;
	
	PE::Handle hServerTank("ServerTank", sizeof(ServerTank));
	ServerTank *pServerTank = new(hServerTank) ServerTank(*m_pContext, m_arena, hServerTank, 0.05f, spawnMatrix, 0.00f, hitPoint, shellMax);
	pServerTank->addDefaultComponents();

	addComponent(hServerTank);
	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = { 0 }; // we will pass empty array as allowed events to propagate so that when we add
													// scene node to the square controller, the square controller doesnt try to handle scene node's events
													// because scene node handles events through scene graph, and is child of square controller just for referencing purposes
	pServerTank->addComponent(hSN, &alllowedEventsToPropagate[0]);

	ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());

	for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
	{
		pNetworkManager->m_clientConnections[i].getGhostManager()->addToLibrary(hServerTank, nextGhostId++);

		pServerTank->m_clientProcessing = i;
		pNetworkManager->m_clientConnections[i].getGhostManager()->scheduleGhost(pServerTank, true, nextGhostId - 1, true, 0);
	}
}

void ServerGameObjectManagerAddon::createShell(int ghostId, int clientIndex, const Matrix4x4 &t)
{
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	pSN->m_base.loadIdentity();
	pSN->m_base.setPos(t.getPos());
	pSN->m_base.setU(t.getU());
	pSN->m_base.setV(t.getV());
	pSN->m_base.setN(t.getN());

	PE::Handle hServerShell("ServerShell", sizeof(ServerShell));
	ServerShell *pServerShell = new(hServerShell) ServerShell(*m_pContext, m_arena, hServerShell, 0.00f);
	pServerShell->addDefaultComponents();

	addComponent(hServerShell);

	static int alllowedEventsToPropagate[] = { 0 }; 
	
	pServerShell->addComponent(hSN, &alllowedEventsToPropagate[0]);

	ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());

	for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
	{
		GhostManager* pGhostManager = pNetworkManager->m_clientConnections[i].getGhostManager();
		pGhostManager->addToLibrary(hServerShell, ghostId);

		pServerShell->m_clientProcessing = i;
		
		if (i == clientIndex)
		{
			pGhostManager->resetStateMask(ghostId);
			continue;
		}
		pGhostManager->scheduleGhost(pServerShell, true, ghostId, true, 1);
	}
}

void ServerGameObjectManagerAddon::do_Destroy_Ghost(PE::Events::Event *pEvt)
{
	Event_Destroy_Ghost* pTrueEvent = (Event_Destroy_Ghost*)(pEvt);
	
	ServerNetworkManager *pNetworkManager = (ServerNetworkManager *)(m_pContext->getNetworkManager());
	PE::Handle h = pNetworkManager->m_clientConnections[0].getGhostManager()->getGhostHandle(pTrueEvent->m_ghostId);

	if (pTrueEvent->m_ghostType == SHELL || true)
	{
		ServerShell* pServerShell = h.getObject<ServerShell>();

		for (int i = 0; i < pNetworkManager->m_clientConnections.m_size; i++)
		{
			GhostManager* pGhostManager = pNetworkManager->m_clientConnections[i].getGhostManager();
			
			if (i != pTrueEvent->m_networkClientId)
			{
				pServerShell->m_clientProcessing = i;
				pGhostManager->scheduleGhost(pServerShell, true, pTrueEvent->m_ghostId, true, 1);
			}
			//pGhostManager->removeFromLibrary(pTrueEvent->m_ghostId);
		}
	}

}

}
}
