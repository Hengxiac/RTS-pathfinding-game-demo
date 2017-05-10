#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "TargetMovementSM.h"
#include "Target.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

// Events sent by behavior state machine (or other high level state machines)
// these are events that specify where a soldier should move
namespace Events{

PE_IMPLEMENT_CLASS1(TargetMovementSM_Event_MOVE_TO, Event);

TargetMovementSM_Event_MOVE_TO::TargetMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
: m_targetPosition(targetPos)
, m_running(false)
{ }

PE_IMPLEMENT_CLASS1(TargetMovementSM_Event_STOP, Event);

PE_IMPLEMENT_CLASS1(TargetMovementSM_Event_TARGET_REACHED, Event);

PE_IMPLEMENT_CLASS1(TargetMovementSM_Event_UPDATE_POSITION, Event);
}

namespace Components{

PE_IMPLEMENT_CLASS1(TargetMovementSM, Component);


TargetMovementSM::TargetMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
: Component(context, arena, hMyself)
, m_state(STANDING)
{}

SceneNode *TargetMovementSM::getParentsSceneNode()
{
	PE::Handle hParent = getFirstParentByType<Component>();
	if (hParent.isValid())
	{
		// see if parent has scene node component
		return hParent.getObject<Component>()->getFirstComponent<SceneNode>();
		
	}
	return NULL;
}

void TargetMovementSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(TargetMovementSM_Event_MOVE_TO, TargetMovementSM::do_TargetMovementSM_Event_MOVE_TO);
	PE_REGISTER_EVENT_HANDLER(TargetMovementSM_Event_STOP, TargetMovementSM::do_TargetMovementSM_Event_STOP);
	
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, TargetMovementSM::do_UPDATE);
}

void TargetMovementSM::do_TargetMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
{
	TargetMovementSM_Event_MOVE_TO *pRealEvt = (TargetMovementSM_Event_MOVE_TO *)(pEvt);
	
	// change state of this state machine
	

	m_targetPostion = pRealEvt->m_targetPosition;
	m_state = WALKING_TO_TARGET;


	//OutputDebugStringA("PE: PROGRESS: TargetMovement::do_TargetMovementSM_Event_MOVE_TO() :received event, running: ");
	//OutputDebugStringA(pRealEvt->m_running ? "true" : "false");
	
	/*
	if (pRealEvt->m_running)
	{	
		m_state = RUNNING_TO_TARGET;

		// make sure the animations are playing
		PE::Handle h("SoldierNPCAnimSM_Event_RUN", sizeof(SoldierNPCAnimSM_Event_RUN));
		Events::SoldierNPCAnimSM_Event_RUN *pOutEvt = new(h) SoldierNPCAnimSM_Event_RUN();

		SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
	}
	else
	{
		m_state = WALKING_TO_TARGET;
		
		// make sure the animations are playing
		PE::Handle h("SoldierNPCAnimSM_Event_WALK", sizeof(SoldierNPCAnimSM_Event_WALK));
		Events::SoldierNPCAnimSM_Event_WALK *pOutEvt = new(h) SoldierNPCAnimSM_Event_WALK();

		SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
	}*/
	
}

void TargetMovementSM::do_TargetMovementSM_Event_STOP(PE::Events::Event *pEvt)
{
	
}

void TargetMovementSM::do_UPDATE(PE::Events::Event *pEvt)
{
	if (m_state == WALKING_TO_TARGET || m_state == RUNNING_TO_TARGET)
	{
		// see if parent has scene node component
		SceneNode *pSN = getParentsSceneNode();
		if (pSN)
		{
			Vector3 curPos = pSN->m_base.getPos();
			float dsqr = (m_targetPostion - curPos).lengthSqr();

			bool reached = true;
			if (dsqr > 0.01f)
			{
				// not at the spot yet
				Event_UPDATE *pRealEvt = (Event_UPDATE *)(pEvt);
				
				float speed =  1.4f;
				float allowedDisp = speed * pRealEvt->m_frameTime;

				Vector3 dir = (m_targetPostion - curPos);
				dir.normalize();
				float dist = sqrt(dsqr);
				if (dist > allowedDisp)
				{
					dist = allowedDisp; // can move up to allowedDisp
					reached = false; // not reaching destination yet
				}

				// instantaneous turn
				pSN->m_base.turnInDirection(dir, 3.1415f);
				pSN->m_base.setPos(curPos + dir * dist);

				PE::Handle h("TargetMovementSM_Event_UPDATE_POSITION", sizeof(TargetMovementSM_Event_UPDATE_POSITION));
				Events::TargetMovementSM_Event_UPDATE_POSITION *pOutEvt = new(h) TargetMovementSM_Event_UPDATE_POSITION(pSN->m_base.getPos());

				PE::Handle hParent = getFirstParentByType<Component>();
				if (hParent.isValid())
				{
					hParent.getObject<Component>()->handleEvent(pOutEvt);
				}

				// release memory now that event is processed
				h.release();
			}

			if (reached)
			{
				m_state = STANDING;
				
				// target has been reached. need to notify all same level state machines (components of parent)
				{
					PE::Handle h("TargetMovementSM_Event_TARGET_REACHED", sizeof(TargetMovementSM_Event_TARGET_REACHED));
					Events::TargetMovementSM_Event_TARGET_REACHED *pOutEvt = new(h) TargetMovementSM_Event_TARGET_REACHED();

					PE::Handle hParent = getFirstParentByType<Component>();
					if (hParent.isValid())
					{
						hParent.getObject<Component>()->handleEvent(pOutEvt);
					}
					
					// release memory now that event is processed
					h.release();
				}

				if (m_state == STANDING)
				{
					// no one has modified our state based on TARGET_REACHED callback
					// this means we are not going anywhere right now
					// so can send event to animation state machine to stop
					{
						//Events::SoldierNPCAnimSM_Event_STOP evt;
						
					}
				}
			}
		}
	}
}

}}




