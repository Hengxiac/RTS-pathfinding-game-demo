#include "ClientGameObjectManagerAddon.h"

#include "PrimeEngine/PrimeEngineIncludes.h"

#include "Characters/SoldierNPC.h"
#include "WayPoint.h"
#include "Tank/ClientTank.h"
#include "Tank/ClientShell.h"
#include "CharacterControl/Client/ClientSpaceShip.h"
#include "Characters/Target.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;
using namespace CharacterControl::Components;

namespace CharacterControl{
namespace Components
{
PE_IMPLEMENT_CLASS1(ClientGameObjectManagerAddon, Component); // creates a static handle and GteInstance*() methods. still need to create construct

void ClientGameObjectManagerAddon::addDefaultComponents()
{
	GameObjectManagerAddon::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Event_CreateSoldierNPC, ClientGameObjectManagerAddon::do_CreateSoldierNPC);
	PE_REGISTER_EVENT_HANDLER(Event_CREATE_WAYPOINT, ClientGameObjectManagerAddon::do_CREATE_WAYPOINT);
	PE_REGISTER_EVENT_HANDLER(Event_CreateTarget, ClientGameObjectManagerAddon::do_CreateTarget);
	// note this component (game obj addon) is added to game object manager after network manager, so network manager will process this event first
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_SERVER_CLIENT_CONNECTION_ACK, ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK);

	PE_REGISTER_EVENT_HANDLER(Event_Create_Ghost, ClientGameObjectManagerAddon::do_Create_Ghost);

	PE_REGISTER_EVENT_HANDLER(Event_MoveTank_S_to_C, ClientGameObjectManagerAddon::do_MoveTank);

	PE_REGISTER_EVENT_HANDLER(Event_Client_Restart, ClientGameObjectManagerAddon::do_Restart_Game);

	PE_REGISTER_EVENT_HANDLER(Event_Create_Shell, ClientGameObjectManagerAddon::do_Create_Shell);

	PE_REGISTER_EVENT_HANDLER(Event_Destroy_Ghost, ClientGameObjectManagerAddon::do_Destroy_Ghost);

	PE_REGISTER_EVENT_HANDLER(Event_Resolve_Collision, ClientGameObjectManagerAddon::do_Resolve_Collision);
}

void ClientGameObjectManagerAddon::do_CreateSoldierNPC(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CreateSoldierNPC>());

	Event_CreateSoldierNPC *pTrueEvent = (Event_CreateSoldierNPC*)(pEvt);

	createSoldierNPC(pTrueEvent);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Vector3 pos, int &threadOwnershipMask)
{
	Event_CreateSoldierNPC evt(threadOwnershipMask);
	evt.m_pos = pos;
	evt.m_u = Vector3(1.0f, 0, 0);
	evt.m_v = Vector3(0, 1.0f, 0);
	evt.m_n = Vector3(0, 0, 1.0f);
	
	StringOps::writeToString( "SoldierTransform.mesha", evt.m_meshFilename, 255);
	StringOps::writeToString( "Soldier", evt.m_package, 255);
	StringOps::writeToString( "mg34.x_mg34main_mesh.mesha", evt.m_gunMeshName, 64);
	StringOps::writeToString( "CharacterControl", evt.m_gunMeshPackage, 64);
	StringOps::writeToString( "", evt.m_patrolWayPoint, 32);
	createSoldierNPC(&evt);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Event_CreateSoldierNPC *pTrueEvent)
{
	PEINFO("CharacterControl: GameObjectManagerAddon: Creating CreateSoldierNPC\n");

	PE::Handle hSoldierNPC("SoldierNPC", sizeof(SoldierNPC));
	SoldierNPC *pSoldierNPC = new(hSoldierNPC) SoldierNPC(*m_pContext, m_arena, hSoldierNPC, pTrueEvent);
	pSoldierNPC->addDefaultComponents();

	// add the soldier as component to the ObjecManagerComponentAddon
	// all objects of this demo live in the ObjecManagerComponentAddon
	addComponent(hSoldierNPC);
}
void ClientGameObjectManagerAddon::do_CreateTarget(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CreateTarget>());

	Event_CreateTarget *pTrueEvent = (Event_CreateTarget*)(pEvt);

	createTarget(pTrueEvent);
}

void ClientGameObjectManagerAddon::createTarget(Vector3 pos, int &threadOwnershipMask)
{
	Event_CreateTarget evt(threadOwnershipMask);
	evt.m_pos = pos;
	evt.m_u = Vector3(1.0f, 0, 0);
	evt.m_v = Vector3(0, 1.0f, 0);
	evt.m_n = Vector3(0, 0, 1.0f);

	StringOps::writeToString("imrod.x_imrodmesh_mesh.mesha", evt.m_meshFilename, 255);
	StringOps::writeToString("Default", evt.m_package, 255);
	StringOps::writeToString("", evt.m_movementWayPoint, 32);
	createTarget(&evt);
}

void ClientGameObjectManagerAddon::createTarget(Event_CreateTarget *pTrueEvent)
{
	PEINFO("CharacterControl: GameObjectManagerAddon: Creating CreateTarget\n");

	PE::Handle hTarget("Target", sizeof(Target));
	Target *pTarget = new(hTarget) Target(*m_pContext, m_arena, hTarget, pTrueEvent);
	pTarget->addDefaultComponents();

	// add the soldier as component to the ObjecManagerComponentAddon
	// all objects of this demo live in the ObjecManagerComponentAddon
	addComponent(hTarget);
}
void ClientGameObjectManagerAddon::do_CREATE_WAYPOINT(PE::Events::Event *pEvt)
{
	PEINFO("GameObjectManagerAddon::do_CREATE_WAYPOINT()\n");

	assert(pEvt->isInstanceOf<Event_CREATE_WAYPOINT>());

	Event_CREATE_WAYPOINT *pTrueEvent = (Event_CREATE_WAYPOINT*)(pEvt);

	PE::Handle hWayPoint("WayPoint", sizeof(WayPoint));
	WayPoint *pWayPoint = new(hWayPoint) WayPoint(*m_pContext, m_arena, hWayPoint, pTrueEvent);
	pWayPoint->addDefaultComponents();

	addComponent(hWayPoint);
}

WayPoint *ClientGameObjectManagerAddon::getWayPoint(const char *name)
{
	PE::Handle *pHC = m_components.getFirstPtr();

	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<WayPoint>())
		{
			WayPoint *pWP = (WayPoint *)(pC);
			if (StringOps::strcmp(pWP->m_name, name) == 0)
			{
				// equal strings, found our waypoint
				return pWP;
			}
		}
	}
	return NULL;
}
Target *ClientGameObjectManagerAddon::getTarget()
{
	PE::Handle *pHC = m_components.getFirstPtr();

	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<Target>())
		{
			Target *pTarget = (Target *)(pC);
			return pTarget;

		}
	}
	return NULL;
}


void ClientGameObjectManagerAddon::createTank(int index, int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    Tank

	//game object manager
	//  TankController
	//    scene node
	
	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	pMeshInstance->initFromFile("kingtiger.x_main_mesh.mesha", "Default", threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	Vector3 spawnPos(-36.0f + 6.0f * index, 0 , 21.0f);
	pSN->m_base.setPos(spawnPos);
	
	pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects
	Matrix4x4 top;
	top.loadIdentity();
	PE::Handle hTankController("TankController", sizeof(TankController));
	TankController *pTankController = new(hTankController) TankController(*m_pContext, m_arena, hTankController, 0.05f, spawnPos,  0.03f, 100, 5, 1.0f, top);
	pTankController->addDefaultComponents();

	addComponent(hTankController);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of square controller just for referencing purposes
	pTankController->addComponent(hSN, &alllowedEventsToPropagate[0]);
}

void ClientGameObjectManagerAddon::createSpaceShip(int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    SpaceShip

	//game object manager
	//  SpaceShipController
	//    scene node

	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	pMeshInstance->initFromFile("space_frigate_6.mesha", "FregateTest", threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	Vector3 spawnPos(0, 0, 0.0f);
	pSN->m_base.setPos(spawnPos);

	pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects

	PE::Handle hSpaceShip("ClientSpaceShip", sizeof(ClientSpaceShip));
	ClientSpaceShip *pSpaceShip = new(hSpaceShip) ClientSpaceShip(*m_pContext, m_arena, hSpaceShip, 0.05f, spawnPos,  0.05f);
	pSpaceShip->addDefaultComponents();

	addComponent(hSpaceShip);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of space ship just for referencing purposes
	pSpaceShip->addComponent(hSN, &alllowedEventsToPropagate[0]);

	pSpaceShip->activate();
}

void ClientGameObjectManagerAddon::createTankGhost(Matrix4x4 &base, Matrix4x4 &top, int &threadOwnershipMask, int ghostId, int hitPoint, int shellLeft, int type)
{
	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	//pMeshInstance->initFromFile("kingtiger.x_main_mesh.mesha", "Default", threadOwnershipMask);
	if (type == 1)
	{
		/*pMeshInstance->initFromFile("kingtiger.x_main_mesh.mesha", "Default", threadOwnershipMask);

		// need to create a scene node for this mesh
		PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		pSN->addDefaultComponents();

		Vector3 spawnPos = base.getPos();

		pSN->m_base.setPos(spawnPos);
		pSN->m_base.setU(base.getU());
		pSN->m_base.setV(base.getV());
		pSN->m_base.setN(base.getN());

		pSN->addComponent(hMeshInstance);

		RootSceneNode::Instance()->addComponent(hSN);

		// now add game objects
		float time_coe = 0.5f * type + 0.5f;
		PE::Handle hTankController("TankController", sizeof(TankController));
		TankController *pTankController = new(hTankController) TankController(*m_pContext, m_arena, hTankController, 0.05f,
			spawnPos, 0.05f, hitPoint, shellLeft, time_coe);
		pTankController->addDefaultComponents();

		addComponent(hTankController);

		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		pNetworkManager->getNetworkContext().getGhostManager()->addToLibrary(hTankController, ghostId);

		// add the same scene node to tank controller
		static int alllowedEventsToPropagate[] = { 0 }; // we will pass empty array as allowed events to propagate so that when we add
														// scene node to the square controller, the square controller doesnt try to handle scene node's events
														// because scene node handles events through scene graph, and is child of square controller just for referencing purposes
		pTankController->addComponent(hSN, &alllowedEventsToPropagate[0]);*/
	}
	else
	{
		pMeshInstance->initFromFile("t90abase.x_t90a_baseshape_mesh.mesha", "TankGame", threadOwnershipMask);

		PE::Handle hMeshInstance1("MeshInstance", sizeof(MeshInstance));
		MeshInstance *pMeshInstance1 = new(hMeshInstance1) MeshInstance(*m_pContext, m_arena, hMeshInstance1);

		pMeshInstance1->addDefaultComponents();

		pMeshInstance1->initFromFile("t90atop.x_t90a_topshape_mesh.mesha", "TankGame", threadOwnershipMask);

		PE::Handle hSN1("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pSN1 = new(hSN1) SceneNode(*m_pContext, m_arena, hSN1);
		pSN1->addDefaultComponents();

		Vector3 spawnPos = base.getPos();

		pSN1->m_base.setPos(spawnPos);
		pSN1->m_base.setU((base).getU());
		pSN1->m_base.setV((base).getV());
		pSN1->m_base.setN((base).getN());

		pSN1->addComponent(hMeshInstance1);

		RootSceneNode::Instance()->addComponent(hSN1);

		PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		pSN->addDefaultComponents();

		pSN->m_base.setPos(spawnPos);
		pSN->m_base.setU(base.getU());
		pSN->m_base.setV(base.getV());
		pSN->m_base.setN(base.getN());

		pSN->addComponent(hMeshInstance);

		PE::Handle hPC("PHYSICS_COMPONENT", sizeof(PhysicsComponent));
		PhysicsComponent *pPC = new (hPC) PhysicsComponent(*m_pContext, m_arena, hPC);
		pPC->addDefaultComponents();

		pPC->m_base.setPos(pSN->m_base.getPos());
		pPC->m_base.setU(pSN->m_base.getU());
		pPC->m_base.setV(pSN->m_base.getV());
		pPC->m_base.setN(pSN->m_base.getN());

		pPC->m_isAnimated = true;

		float d_x = 2.00f;
		float d_y = 1.50f;
		float d_z = 3.80f;

		PE::Handle hS("SHAPE", sizeof(Shape));
		Shape *pS = new (hS) Shape(*m_pContext, m_arena, hS);
		pS->addDefaultComponents();
		pS->initialize("box", pSN->m_base, d_x, d_y, d_z);

		pPC->addShape(hS);

		m_pContext->getPhysicsManager()->addPhys(hPC);

		RootSceneNode::Instance()->addComponent(hSN);

		// now add game objects
		float time_coe = 0.5f * type + 0.5f;
		PE::Handle hTankController("TankController", sizeof(TankController));
		TankController *pTankController = new(hTankController) TankController(*m_pContext, m_arena, hTankController, 0.05f,
			spawnPos, 0.03f, hitPoint, shellLeft, time_coe, top);
		pTankController->addDefaultComponents();

		addComponent(hTankController);

		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		pNetworkManager->getNetworkContext().getGhostManager()->addToLibrary(hTankController, ghostId);

		// add the same scene node to tank controller
		static int alllowedEventsToPropagate[] = { 0 }; // we will pass empty array as allowed events to propagate so that when we add
														// scene node to the square controller, the square controller doesnt try to handle scene node's events
														// because scene node handles events through scene graph, and is child of square controller just for referencing purposes
		pTankController->addComponent(hSN, &alllowedEventsToPropagate[0]);

		static int alllowedEventsToPropagate1[] = { 0 };

		pTankController->addComponent(hSN1, &alllowedEventsToPropagate1[0]);
		pTankController->addComponent(hPC);
		//pMeshInstance->initFromFile("t90a.x_t90a1shape_mesh.mesha", "TankGame", threadOwnershipMask);
	}

}

void ClientGameObjectManagerAddon::createShell(Matrix4x4 &base, bool isActive, int &threadOwnershipMask, int ghostID)
{
	
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	pSN->m_base.setPos(base.getPos());
	pSN->m_base.setU(base.getU());
	pSN->m_base.setV(base.getV());
	pSN->m_base.setN(base.getN());
	
	//m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	/*PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	
	pMeshInstance->initFromFile("bullet.x_bullet_mesh.mesha", "TankGame", threadOwnershipMask);
	pSN->addComponent(hMeshInstance);*/

	PE::Handle hPC("PHYSICS_COMPONENT", sizeof(PhysicsComponent));
	PhysicsComponent *pPC = new (hPC) PhysicsComponent(*m_pContext, m_arena, hPC);

	pPC->addDefaultComponents();
	pPC->m_base.setPos(pSN->m_base.getPos());
	pPC->m_base.setU(pSN->m_base.getU());
	pPC->m_base.setV(pSN->m_base.getV());
	pPC->m_base.setN(pSN->m_base.getN());

	pPC->m_isAnimated = true;

	float d_x = 0.20f;
	float d_y = 0.20f;
	float d_z = 0.20f;

	PE::Handle hS("SHAPE", sizeof(Shape));
	Shape *pS = new (hS) Shape(*m_pContext, m_arena, hS);
	pS->addDefaultComponents();
	pS->initialize("box", pSN->m_base, d_x, d_y, d_z);

	PE::Handle hS1("SHAPE", sizeof(Shape));
	Shape *pS1 = new (hS1) Shape(*m_pContext, m_arena, hS1);
	pS1->addDefaultComponents();
	pS1->initialize("sphere", pSN->m_base, d_x);

	pPC->addShape(hS1);
	pPC->addShape(hS);

	m_pContext->getPhysicsManager()->addPhys(hPC);
	if (isActive)
	{
		m_pContext->getPhysicsManager()->addActivePhys(hPC);
	}
	RootSceneNode::Instance()->addComponent(hSN);

	PE::Handle hClientShell("ClientShell", sizeof(ClientShell));
	ClientShell *pClientShell = new(hClientShell) ClientShell(*m_pContext, m_arena, hClientShell, base.getN(),
		isActive, 100, 0.03f);
	pClientShell->addDefaultComponents();

	addComponent(hClientShell);

	static int alllowedEventsToPropagate[] = { 0 };
	pClientShell->addComponent(hSN, &alllowedEventsToPropagate[0]);
	pClientShell->addComponent(hPC);

	if (isActive && ghostID == -1)
	{
		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		GhostManager* pGhostManager = pNetworkManager->getNetworkContext().getGhostManager();
		
		while (pGhostManager->ghostIdExist(m_nextShellId))
		{
			m_nextShellId++;
		}
		
		pClientShell->m_ghostID = m_nextShellId++;
		
		//m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	}
	else
	{
		pClientShell->m_ghostID = ghostID;
	}

	//m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

}

void ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK(PE::Events::Event *pEvt)
{
	Event_SERVER_CLIENT_CONNECTION_ACK *pRealEvt = (Event_SERVER_CLIENT_CONNECTION_ACK *)(pEvt);
	PE::Handle *pHC = m_components.getFirstPtr();

	m_tankToActivate = pRealEvt->m_clientId;

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pRealEvt->m_clientId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->activate();
				m_tankToActivate = -1;
				break;
			}
			++itc;
		}
	}
}

void ClientGameObjectManagerAddon::do_Create_Ghost(PE::Events::Event *pEvt)
{
	//assert(pEvt->isInstanceOf<Event_Create_Ghost>());

	Event_Create_Ghost *pTrueEvent = (Event_Create_Ghost*)(pEvt);
	
	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

	if (pTrueEvent->m_ghostType == TANK)
	{
		createTankGhost(pTrueEvent->m_transform, pTrueEvent->m_transform2, m_pContext->m_gameThreadThreadOwnershipMask, pTrueEvent->m_ghostId,
			pTrueEvent->m_intVal[0], pTrueEvent->m_intVal[1], 0/*pTrueEvent->m_ghostId % 2*/); //
	}
	else if (pTrueEvent->m_ghostType == SHELL)
	{
		createShell(pTrueEvent->m_transform, false, m_pContext->m_gameThreadThreadOwnershipMask, pTrueEvent->m_ghostId);
	}

	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	if (m_tankToActivate >= 0)
	{
		PE::Handle *pHC = m_components.getFirstPtr();
		int itc = 0;

		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<TankController>())
			{
				if (itc == m_tankToActivate) //activate tank controller for local client based on local clients id
				{
					TankController *pTK = (TankController *)(pC);
					pTK->activate();
					m_tankToActivate = -1;
					break;
				}
				++itc;
			}
		}
	}
	
}

void ClientGameObjectManagerAddon::do_MoveTank(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_MoveTank_S_to_C>());

	Event_MoveTank_S_to_C *pTrueEvent = (Event_MoveTank_S_to_C*)(pEvt);

	PE::Handle *pHC = m_components.getFirstPtr();

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pTrueEvent->m_clientTankId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->overrideTransform(pTrueEvent->m_transform);
				break;
			}
			++itc;
		}
	}
}

void ClientGameObjectManagerAddon::do_Restart_Game(PE::Events::Event *pEvt)
{
	CharacterControl::Events::Event_Restart_Game evt(*m_pContext);

	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
	pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);

}

void ClientGameObjectManagerAddon::do_Create_Shell(PE::Events::Event *pEvt)
{
	Event_Create_Shell *pTrueEvent = (Event_Create_Shell*)(pEvt);

	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	createShell(pTrueEvent->m_transform, pTrueEvent->m_active, m_pContext->m_gameThreadThreadOwnershipMask);
	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
}

void ClientGameObjectManagerAddon::do_Destroy_Ghost(PE::Events::Event *pEvt)
{
	Event_Destroy_Ghost* pTrueEvent = (Event_Destroy_Ghost*)(pEvt);
	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
	PE::Handle h = pNetworkManager->getNetworkContext().getGhostManager()->getGhostHandle(pTrueEvent->m_ghostId);

	removeObject(h);
}

void ClientGameObjectManagerAddon::do_Resolve_Collision(PE::Events::Event *pEvt) 
{
	Event_Resolve_Collision* pTrueEvent = (Event_Resolve_Collision*)(pEvt);
	if (pTrueEvent->moving.isValid())
	{
		Component* pC = pTrueEvent->moving.getObject<Component>();
		PE::Handle hClientShell = pC->getFirstParentByType<ClientShell>();
		//removeObject(hClientShell);
	}
	if (pTrueEvent->collided.isValid())
	{
		Component* pC = pTrueEvent->collided.getObject<Component>();
		PE::Handle hClientShell = pC->getFirstParentByType<ClientShell>();
		PE::Handle hClientTank = pC->getFirstParentByType<TankController>();
		if (hClientShell.isValid())
		{
			removeObject(hClientShell);
		}
		else if (hClientTank.isValid())
		{
			PE::Handle h("Event_Get_Hit", sizeof(Event_Get_Hit));
			Event_Get_Hit *pEvt = new(h) Event_Get_Hit(100);

			hClientTank.getObject<Component>()->handleEvent(pEvt);

			h.release();
		}
		
		
	}

}

void ClientGameObjectManagerAddon::removeObject(PE::Handle h)
{
	if (!h.isValid())
		return;
	Component* pC = h.getObject<Component>();

	ClientShell* pCS = (ClientShell*) pC;

	pCS->toRemovePhys = true;
	/*int pCCount = pC->countComponents<PhysicsComponent>();
	
	for (int i = 0; i < pCCount; i++)
	{
		PE::Handle hPC = pC->getFirstComponentHandle<PhysicsComponent>();
		pC->removeComponent(hPC);
		m_pContext->getPhysicsManager()->removePhys(hPC);
		
		PhysicsComponent* pPC = hPC.getObject<PhysicsComponent>();

		int shapeCount = pPC->countComponents<Shape>();
		for (int j = 0; j < shapeCount;	j++)
		{
			PE::Handle hShape = pPC->getFirstComponentHandle<Shape>();
			hShape.release();
		}
		hPC.release();
	}*/

	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
	int ghostID = pNetworkManager->getNetworkContext().getGhostManager()->findGhostID(h);
	if (ghostID >= 0)
	{		
		pCS->toRemoveMask = true;
	}
	/*m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

	int sNCount = pC->countComponents<SceneNode>();
	for (int i = 0; i < sNCount; i++)
	{
		PE::Handle hSN = pC->getFirstComponentHandle<SceneNode>();
		pC->removeComponent(hSN);
		
		SceneNode* pSN = hSN.getObject<SceneNode>();
		PE::Handle hMeshInstance = pSN->getFirstComponentHandle<MeshInstance>();
		pSN->removeComponent(hMeshInstance);
		hMeshInstance.release();

		RootSceneNode::Instance()->removeComponent(hSN);

		//hSN.release();
	}
	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
	
	//removeComponent(h);
	//h.release();*/
}
}
}
