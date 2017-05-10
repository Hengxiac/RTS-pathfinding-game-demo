#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/DebugRenderer.h"
#include "../ClientGameObjectManagerAddon.h"
#include "../CharacterControlContext.h"
#include "TargetMovementSM.h"
#include "TargetBehaviorSM.h"
#include "Target.h"
#include "SoldierNPC.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Render/IRenderer.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

namespace Components{

PE_IMPLEMENT_CLASS1(TargetBehaviorSM, Component);

TargetBehaviorSM::TargetBehaviorSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, PE::Handle hMovementSM) 
: Component(context, arena, hMyself)
, m_hMovementSM(hMovementSM)
{

}

void TargetBehaviorSM::start()
{
	if (m_havePatrolWayPoint)
	{
		m_state = WAITING_FOR_WAYPOINT; // will update on next do_UPDATE()
	}
	else
	{
		m_state = IDLE; // stand in place

		PE::Handle h("SoldierNPCMovementSM_Event_STOP", sizeof(TargetMovementSM_Event_STOP));
		TargetMovementSM_Event_STOP *pEvt = new(h) TargetMovementSM_Event_STOP();

		m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
		// release memory now that event is processed
		h.release();
		
	}	
}

void TargetBehaviorSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(TargetMovementSM_Event_TARGET_REACHED, TargetBehaviorSM::do_TargetMovementSM_Event_TARGET_REACHED);
	PE_REGISTER_EVENT_HANDLER(TargetMovementSM_Event_UPDATE_POSITION, TargetBehaviorSM::do_TargetMovementSM_Event_UPDATE_POSITION);
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, TargetBehaviorSM::do_UPDATE);

	PE_REGISTER_EVENT_HANDLER(Event_PRE_RENDER_needsRC, TargetBehaviorSM::do_PRE_RENDER_needsRC);
}

// sent by movement state machine whenever it reaches current target
void TargetBehaviorSM::do_TargetMovementSM_Event_TARGET_REACHED(PE::Events::Event *pEvt)
{
	PEINFO("SoldierNPCBehaviorSM::do_SoldierNPCMovementSM_Event_TARGET_REACHED\n");

	if (m_state == PATROLLING_WAYPOINTS)
	{
		ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
		if (pGameObjectManagerAddon)
		{
			// search for waypoint object
			WayPoint *pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
			if (pWP && (StringOps::length(pWP->m_nextWayPointName[0]) > 0 || StringOps::length(pWP->m_nextWayPointName[1]) > 0 || StringOps::length(pWP->m_nextWayPointName[2]) > 0))
			{
				// have next waypoint to go to
				int index = 0;
				while (true) {
					index = rand() % 3;
					if (StringOps::length(pWP->m_nextWayPointName[index]) > 0)
					{
						pWP = pGameObjectManagerAddon->getWayPoint(pWP->m_nextWayPointName[index]);
						break;
					}
				}
				
				if (pWP)
				{
					StringOps::writeToString(pWP->m_name, m_curPatrolWayPoint, 32);

					m_state = PATROLLING_WAYPOINTS;
					PE::Handle h("TargetMovementSM_Event_MOVE_TO", sizeof(TargetMovementSM_Event_MOVE_TO));
					Events::TargetMovementSM_Event_MOVE_TO *pEvt = new(h) TargetMovementSM_Event_MOVE_TO(pWP->m_base.getPos());

					m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
					// release memory now that event is processed
					h.release();
				}
			}
			else
			{
				m_state = IDLE;
				// no need to send the event. movement state machine will automatically send event to animation state machine to play idle animation
			}
		}
	}
}

void TargetBehaviorSM::do_TargetMovementSM_Event_UPDATE_POSITION(PE::Events::Event *pEvt) {
	ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
	if (pGameObjectManagerAddon)
	{
		// search for waypoint object
		TargetMovementSM_Event_UPDATE_POSITION *pRealEvt = (TargetMovementSM_Event_UPDATE_POSITION *)(pEvt);
		Target *pTarget = pGameObjectManagerAddon->getTarget();
		if (pTarget)
		{
			pTarget->m_base.setPos(pRealEvt->m_curr_base);
			
		}
	}
}

// this event is executed when thread has RC
void TargetBehaviorSM::do_PRE_RENDER_needsRC(PE::Events::Event *pEvt)
{
	Event_PRE_RENDER_needsRC *pRealEvent = (Event_PRE_RENDER_needsRC *)(pEvt);
	if (m_havePatrolWayPoint)
	{
		char buf[80];
		sprintf(buf, "Patrol Waypoint: %s",m_curPatrolWayPoint);
		Target *pSol = getFirstParentByTypePtr<Target>();
		PE::Handle hTargetSceneNode = pSol->getFirstComponentHandle<PE::Components::SceneNode>();
		Matrix4x4 base = hTargetSceneNode.getObject<PE::Components::SceneNode>()->m_worldTransform;
		
		Vector4 plane;
		bool out_of_frustum = false;
		for (int j = 0; j < 6; j++) {
			plane = pRealEvent->m_frustumPlane[j];
			out_of_frustum = true;
			for (int i = 0; i < 8;i++) {
				out_of_frustum = out_of_frustum && (base.getPos().m_x * plane.m_x +
					base.getPos().m_y * plane.m_y + base.getPos().m_z * plane.m_z + plane.m_w) < 0;
			}
			if (out_of_frustum) break;
		}
		
		if(!out_of_frustum)
		DebugRenderer::Instance()->createTextMesh(
			buf, false, false, true, false, 0,
			base.getPos(), 0.01f, pRealEvent->m_threadOwnershipMask);
		
		{
			//we can also construct points ourself
			bool sent = false;
			ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
			if (pGameObjectManagerAddon)
			{
				WayPoint *pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
				if (pWP)
				{
					Vector3 target = pWP->m_base.getPos();
					Vector3 pos = base.getPos();
					Vector3 color(1.0f, 1.0f, 0);
					Vector3 linepts[] = {pos, color, target, color};
					
					DebugRenderer::Instance()->createLineMesh(true, base,  &linepts[0].m_x, 2, 0);// send event while the array is on the stack
					sent = true;
				}
			}
			if (!sent) // if for whatever reason we didnt retrieve waypoint info, send the event with transform only
				DebugRenderer::Instance()->createLineMesh(true, base, NULL, 0, 0);// send event while the array is on the stack
		}
	}
}

void TargetBehaviorSM::do_UPDATE(PE::Events::Event *pEvt)
{
	if (m_state == WAITING_FOR_WAYPOINT)
	{
		if (m_havePatrolWayPoint)
		{
			ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
			if (pGameObjectManagerAddon)
			{
				// search for waypoint object
				WayPoint *pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
				if (pWP)
				{
					m_state = PATROLLING_WAYPOINTS;
					PE::Handle h("TargetNPCMovementSM_Event_MOVE_TO", sizeof(TargetMovementSM_Event_MOVE_TO));
					Events::TargetMovementSM_Event_MOVE_TO *pEvt = new(h) TargetMovementSM_Event_MOVE_TO(pWP->m_base.getPos());

					m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
					// release memory now that event is processed
					h.release();
				}
			}
		}
		else
		{
			// should not happen, but in any case, set state to idle
			m_state = IDLE;

			PE::Handle h("TargetMovementSM_Event_STOP", sizeof(TargetMovementSM_Event_STOP));
			TargetMovementSM_Event_STOP *pEvt = new(h) TargetMovementSM_Event_STOP();

			m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
			// release memory now that event is processed
			h.release();

		}
	}
}


}}




