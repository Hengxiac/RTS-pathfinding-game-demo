#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "SoldierNPCMovementSM.h"
#include "SoldierNPCAnimationSM.h"
#include "SoldierNPC.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl {

	// Events sent by behavior state machine (or other high level state machines)
	// these are events that specify where a soldier should move
	namespace Events {

		PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_MOVE_TO, Event);

		SoldierNPCMovementSM_Event_MOVE_TO::SoldierNPCMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
			: m_targetPosition(targetPos)
			, m_running(false)
		{ }

		PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_STOP, Event);

		PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_TARGET_REACHED, Event);

		PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_SHOOT_TO, Event);

		SoldierNPCMovementSM_Event_SHOOT_TO::SoldierNPCMovementSM_Event_SHOOT_TO(Vector3 targetPos /* = Vector3 */)
			: m_targetPosition(targetPos)
		{ }
	}

	namespace Components {

		PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM, Component);


		SoldierNPCMovementSM::SoldierNPCMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
			: Component(context, arena, hMyself)
			, m_state(STANDING), nextDist(0.0f)
		{}

		SceneNode *SoldierNPCMovementSM::getParentsSceneNode()
		{
			PE::Handle hParent = getFirstParentByType<Component>();
			if (hParent.isValid())
			{
				// see if parent has scene node component
				return hParent.getObject<Component>()->getFirstComponent<SceneNode>();

			}
			return NULL;
		}

		void SoldierNPCMovementSM::addDefaultComponents()
		{
			Component::addDefaultComponents();

			PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_MOVE_TO, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO);
			PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_SHOOT_TO, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_SHOOT_TO);
			PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_STOP, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STOP);

			PE_REGISTER_EVENT_HANDLER(Event_UPDATE, SoldierNPCMovementSM::do_UPDATE);
			PE_REGISTER_EVENT_HANDLER(Event_PRE_PHYSICS, SoldierNPCMovementSM::do_PRE_PHYSICS);
			PE_REGISTER_EVENT_HANDLER(Event_POST_PHYSICS, SoldierNPCMovementSM::do_POST_PHYSICS);
		}

		void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
		{
			SoldierNPCMovementSM_Event_MOVE_TO *pRealEvt = (SoldierNPCMovementSM_Event_MOVE_TO *)(pEvt);

			// change state of this state machine


			m_targetPostion = pRealEvt->m_targetPosition;

			OutputDebugStringA("PE: PROGRESS: SoldierNPCMovement::do_SoldierNPCMovementSM_Event_MOVE_TO() :received event, running: ");
			OutputDebugStringA(pRealEvt->m_running ? "true" : "false");


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
			}

		}

		void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_SHOOT_TO(PE::Events::Event *pEvt)
		{
			SoldierNPCMovementSM_Event_SHOOT_TO *pRealEvt = (SoldierNPCMovementSM_Event_SHOOT_TO *)(pEvt);

			// change state of this state machine


			m_targetPostion = pRealEvt->m_targetPosition;


			m_state = SHOOTING_TO_TARGET;

			// make sure the animations are playing
			PE::Handle h("SoldierNPCAnimSM_Event_SHOOT", sizeof(SoldierNPCAnimSM_Event_SHOOT));
			Events::SoldierNPCAnimSM_Event_SHOOT *pOutEvt = new(h) SoldierNPCAnimSM_Event_SHOOT();

			SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
			pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

			// release memory now that event is processed
			h.release();


		}

		void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STOP(PE::Events::Event *pEvt)
		{
			Events::SoldierNPCAnimSM_Event_STOP Evt;

			SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
			pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);
		}

		void SoldierNPCMovementSM::do_UPDATE(PE::Events::Event *pEvt)
		{

			nextDist = 0.0f;
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

						float speed = (m_state == WALKING_TO_TARGET) ? 1.4f : 4.0f;
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
						//pSN->m_base.turnInDirection(dir, 3.1415f);
						//pSN->m_base.setPos(curPos + dir * dist);
						nextDir = dir;
						nextDist = dist;
					}

					if (reached)
					{
						m_state = STANDING;

						// target has been reached. need to notify all same level state machines (components of parent)
						{
							PE::Handle h("SoldierNPCMovementSM_Event_TARGET_REACHED", sizeof(SoldierNPCMovementSM_Event_TARGET_REACHED));
							Events::SoldierNPCMovementSM_Event_TARGET_REACHED *pOutEvt = new(h) SoldierNPCMovementSM_Event_TARGET_REACHED();

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
								//m_state = SHOOTING_TO_TARGET;
								//Events::SoldierNPCAnimSM_Event_STOP evt;
								Events::SoldierNPCAnimSM_Event_STOP evt;

								SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
								pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&evt);
							}
						}
					}
				}
			}
			if (m_state == SHOOTING_TO_TARGET) {


				SceneNode *pSN = getParentsSceneNode();
				if (pSN)
				{
					Vector3 curPos = pSN->m_base.getPos();
					Vector3 dir = (m_targetPostion - curPos);
					dir.m_y = 0.0f;
					dir.normalize();
					//pSN->m_base.turnInDirection(dir, 3.1415f);
					nextDir = dir;
					nextDist = 0.0f;
				}

			}
		}

		void SoldierNPCMovementSM::do_PRE_PHYSICS(PE::Events::Event *pEvt)
		{
			SceneNode *pSN = getParentsSceneNode();

			if (pSN)
			{
				PhysicsComponent *pPC = pSN->getFirstComponent<PE::Components::PhysicsComponent>();
				if (pPC)
				{
					pPC->nextDist = nextDist;
					pPC->nextDir = nextDir;
				}

			}

		}

		void SoldierNPCMovementSM::do_POST_PHYSICS(PE::Events::Event *pEvt)
		{
			SceneNode *pSN = getParentsSceneNode();

			if (pSN)
			{
				PhysicsComponent *pPC = pSN->getFirstComponent<PE::Components::PhysicsComponent>();

				if (pPC)
				{
					pSN->m_base.setPos(pPC->m_base.getPos());
					pSN->m_base.setU(pPC->m_base.getU());
					pSN->m_base.setV(pPC->m_base.getV());
					pSN->m_base.setN(pPC->m_base.getN());
				}

			}


		}

	}
}




