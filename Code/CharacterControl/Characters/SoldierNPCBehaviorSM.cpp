#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/DebugRenderer.h"
#include "../ClientGameObjectManagerAddon.h"
#include "../CharacterControlContext.h"
#include "SoldierNPCMovementSM.h"
#include "SoldierNPCAnimationSM.h"
#include "SoldierNPCBehaviorSM.h"
#include "SoldierNPC.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Render/IRenderer.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"
#include "PrimeEngine/Navigation/NavMeshManager.h"
#include"../Characters/Target.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl {

	namespace Components {

		PE_IMPLEMENT_CLASS1(SoldierNPCBehaviorSM, Component);

		SoldierNPCBehaviorSM::SoldierNPCBehaviorSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, PE::Handle hMovementSM)
			: Component(context, arena, hMyself), m_nextPos(), isSelected(false), isPatrolling(false)
			, m_hMovementSM(hMovementSM)
		{

		}

		void SoldierNPCBehaviorSM::start()
		{
			if (m_havePatrolWayPoint)
			{
				m_state = WAITING_FOR_WAYPOINT; // will update on next do_UPDATE()
			}
			else
			{
				m_state = IDLE; // stand in place

				PE::Handle h("SoldierNPCMovementSM_Event_STOP", sizeof(SoldierNPCMovementSM_Event_STOP));
				SoldierNPCMovementSM_Event_STOP *pEvt = new(h) SoldierNPCMovementSM_Event_STOP();

				m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
				// release memory now that event is processed
				h.release();

			}
		}

		void SoldierNPCBehaviorSM::addDefaultComponents()
		{
			Component::addDefaultComponents();

			PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_TARGET_REACHED, SoldierNPCBehaviorSM::do_SoldierNPCMovementSM_Event_TARGET_REACHED);
			PE_REGISTER_EVENT_HANDLER(Event_UPDATE, SoldierNPCBehaviorSM::do_UPDATE);

			PE_REGISTER_EVENT_HANDLER(Event_UPDATE_TARGET_POS, SoldierNPCBehaviorSM::do_UPDATE_TARGET_POS);
			PE_REGISTER_EVENT_HANDLER(Event_CHECK_SELECTED, SoldierNPCBehaviorSM::do_CHECK_SELECTED);

			PE_REGISTER_EVENT_HANDLER(Event_UPDATE_GROUP, SoldierNPCBehaviorSM::do_UPDATE_GROUP);

			PE_REGISTER_EVENT_HANDLER(Event_PRE_RENDER_needsRC, SoldierNPCBehaviorSM::do_PRE_RENDER_needsRC);
		}

		// sent by movement state machine whenever it reaches current target
		void SoldierNPCBehaviorSM::do_SoldierNPCMovementSM_Event_TARGET_REACHED(PE::Events::Event *pEvt)
		{
			PEINFO("SoldierNPCBehaviorSM::do_SoldierNPCMovementSM_Event_TARGET_REACHED\n");

			if (!isPatrolling)
			{
				if (!m_nextPos.isEmpty())
					m_nextPos.deleteFront();
			}
			else
			{
				if (m_nextPos.getFront()) {
					m_nextPos.add(*m_nextPos.getFront());
					m_nextPos.deleteFront();
				}

			}
			if (m_state == PATROLLING_WAYPOINTS)
			{
				if (!m_nextPos.isEmpty())
				{
					// search for next position
					Vector3* pNP = m_nextPos.getFront();

					if (pNP)
					{
						m_state = PATROLLING_WAYPOINTS;
						PE::Handle h("SoldierNPCMovementSM_Event_MOVE_TO", sizeof(SoldierNPCMovementSM_Event_MOVE_TO));
						Events::SoldierNPCMovementSM_Event_MOVE_TO *pEvt = new(h) SoldierNPCMovementSM_Event_MOVE_TO(*pNP);

						pEvt->m_running = true; //(rand() % 2) > 0;
						m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
						// release memory now that event is processed
						h.release();

						PE::Handle h2("SoldierNPCAnimSM_Event_RUN", sizeof(SoldierNPCAnimSM_Event_RUN));
						Events::SoldierNPCAnimSM_Event_RUN *pOutEvt = new(h2) SoldierNPCAnimSM_Event_RUN();

						SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
						pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

						// release memory now that event is processed
						h2.release();
					}
				}
				else
				{
					m_state = IDLE;
					// no need to send the event. movement state machine will automatically send event to animation state machine to play idle animation
				}
			}

		}

		// this event is executed when thread has RC
		void SoldierNPCBehaviorSM::do_PRE_RENDER_needsRC(PE::Events::Event *pEvt)
		{
			Event_PRE_RENDER_needsRC *pRealEvent = (Event_PRE_RENDER_needsRC *)(pEvt);

			{
				//char buf[80];
				//sprintf(buf, "Patrol Waypoint: %s",m_curPatrolWayPoint);
				SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
				PE::Handle hSoldierSceneNode = pSol->getFirstComponentHandle<PE::Components::SceneNode>();
				Matrix4x4 base = hSoldierSceneNode.getObject<PE::Components::SceneNode>()->m_worldTransform;

				/*Vector4 plane;
				bool out_of_frustum = false;
				for (int j = 0; j < 6; j++) {
				plane = pRealEvent->m_frustumPlane[j];
				out_of_frustum = true;
				for (int i = 0; i < 8;i++) {
				out_of_frustum = out_of_frustum && (base.getPos().m_x * plane.m_x +
				base.getPos().m_y * plane.m_y + base.getPos().m_z * plane.m_z + plane.m_w) < 0;
				}
				if (out_of_frustum) break;
				}*/

				//if (!out_of_frustum)
				/*DebugRenderer::Instance()->createTextMesh(
				buf, false, false, true, false, 0,
				base.getPos(), 0.01f, pRealEvent->m_threadOwnershipMask);*/
				if (isSelected)
				{

					float lLength = 0.5f;
					float sLength = 0.2f;
					float yOffset = 0.002f;
					float coe = 1.412f;

					Array<Vector3> lines(*m_pContext, m_arena, 32);

					Vector3 p1 = base * Vector3{ sLength, yOffset, sLength };
					Vector3 p2 = base * Vector3{ lLength, yOffset, lLength };

					Vector3 p3 = base * Vector3{ -sLength, yOffset, sLength };
					Vector3 p4 = base * Vector3{ -lLength, yOffset, lLength };

					Vector3 p5 = base * Vector3{ sLength, yOffset, -sLength };
					Vector3 p6 = base * Vector3{ lLength, yOffset, -lLength };

					Vector3 p7 = base * Vector3{ -sLength, yOffset, -sLength };
					Vector3 p8 = base * Vector3{ -lLength, yOffset, -lLength };

					Vector3 p1_ = base * Vector3{ coe * sLength, yOffset, 0.0f };
					Vector3 p2_ = base * Vector3{ coe * lLength, yOffset, 0.0f };

					Vector3 p3_ = base * Vector3{ coe * (-sLength), yOffset, 0.0f };
					Vector3 p4_ = base * Vector3{ coe * (-lLength), yOffset, 0.0f };

					Vector3 p5_ = base * Vector3{ 0.0f, yOffset, coe * sLength };
					Vector3 p6_ = base * Vector3{ 0.0f, yOffset, coe * lLength };

					Vector3 p7_ = base * Vector3{ 0.0f, yOffset, coe * (-sLength) };
					Vector3 p8_ = base * Vector3{ 0.0f, yOffset, coe * (-lLength) };

					Vector3 color{ 0.6f, 1.0f, 0.2f };

					lines.add(p1);
					lines.add(color);
					lines.add(p2);
					lines.add(color);

					lines.add(p3);
					lines.add(color);
					lines.add(p4);
					lines.add(color);

					lines.add(p5);
					lines.add(color);
					lines.add(p6);
					lines.add(color);

					lines.add(p7);
					lines.add(color);
					lines.add(p8);
					lines.add(color);

					lines.add(p1_);
					lines.add(color);
					lines.add(p2_);
					lines.add(color);

					lines.add(p3_);
					lines.add(color);
					lines.add(p4_);
					lines.add(color);

					lines.add(p5_);
					lines.add(color);
					lines.add(p6_);
					lines.add(color);

					lines.add(p7_);
					lines.add(color);
					lines.add(p8_);
					lines.add(color);

					DebugRenderer *render = DebugRenderer::Instance();
					render->createLineMesh(true, base, &(*lines.getFirstPtr()).m_x, 16, 0);
					lines.reset(0);


				}

				//we can also construct points ourself
				bool sent = false;
				//ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
				if (!m_nextPos.isEmpty() && (isSelected || DebugRenderer::Instance()->m_renderOpen))
				{
					int size = m_nextPos.m_size * 4;
					if (isPatrolling&&m_nextPos.m_size > 2)
						size += 4;
					Array<Vector3> route(*m_pContext, m_arena, size);
					Vector3 *pNP = m_nextPos.getFront();
					Vector3 *prev = m_nextPos.getFront();
					if (pNP)
					{
						Vector3 pos = base.getPos();
						Vector3 color(0.0f, 1.0f, 0);
						route.add(pos);
						route.add(color);
						route.add(*pNP);
						route.add(color);

						for (int i = 1; i < m_nextPos.m_size; i++) {
							pNP = m_nextPos.get(i);
							route.add(*prev);
							route.add(color);
							route.add(*pNP);
							route.add(color);
							prev = pNP;
						}

						if (isPatrolling&&m_nextPos.m_size > 2)
						{
							route.add(*m_nextPos.getFront());
							route.add(color);
							route.add(*pNP);
							route.add(color);
						}

						DebugRenderer::Instance()->createLineMesh(true, base, &(*route.getFirstPtr()).m_x, size / 2, 0);// send event while the array is on the stack
						route.reset(0);
						sent = true;
					}
				}
				if (!sent) // if for whatever reason we didnt retrieve waypoint info, send the event with transform only
					DebugRenderer::Instance()->createLineMesh(true, base, NULL, 0, 0);// send event while the array is on the stack
			}
		}

		void SoldierNPCBehaviorSM::do_UPDATE(PE::Events::Event *pEvt)
		{
			if (m_state == WAITING_FOR_WAYPOINT)
			{
				if (m_nextPos.m_size > 0)
				{
					//ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());

					// search for waypoint object
					Vector3 *pNP = m_nextPos.getFront();
					if (pNP)
					{
						m_state = PATROLLING_WAYPOINTS;
						PE::Handle h("SoldierNPCMovementSM_Event_MOVE_TO", sizeof(SoldierNPCMovementSM_Event_MOVE_TO));
						Events::SoldierNPCMovementSM_Event_MOVE_TO *pEvt = new(h) SoldierNPCMovementSM_Event_MOVE_TO(*pNP);
						pEvt->m_running = true;
						m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
						// release memory now that event is processed
						h.release();

						PE::Handle h2("SoldierNPCAnimSM_Event_RUN", sizeof(SoldierNPCAnimSM_Event_RUN));
						Events::SoldierNPCAnimSM_Event_RUN *pOutEvt = new(h2) SoldierNPCAnimSM_Event_RUN();

						SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
						pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

						// release memory now that event is processed
						h2.release();
					}

				}
				else
				{
					// should not happen, but in any case, set state to idle
					m_state = IDLE;

					PE::Handle h("SoldierNPCMovementSM_Event_STOP", sizeof(SoldierNPCMovementSM_Event_STOP));
					SoldierNPCMovementSM_Event_STOP *pEvt = new(h) SoldierNPCMovementSM_Event_STOP();

					m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
					// release memory now that event is processed
					h.release();

					//m_state = SHOOTING;
				}
			}
			if (m_state == IDLE)
			{
				ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
				if (pGameObjectManagerAddon)
				{
					// search for waypoint object
					Target *pTarget = pGameObjectManagerAddon->getTarget();

					if (pTarget)
					{

						PE::Handle h("SoldierNPCMovementSM_Event_SHOOT_TO", sizeof(SoldierNPCMovementSM_Event_SHOOT_TO));
						Events::SoldierNPCMovementSM_Event_SHOOT_TO *pEvt = new(h) SoldierNPCMovementSM_Event_SHOOT_TO(pTarget->m_base.getPos());

						m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
						// release memory now that event is processed
						h.release();
					}
				}
			}
		}



		void SoldierNPCBehaviorSM::do_UPDATE_TARGET_POS(PE::Events::Event *pEvt)
		{
			if (!isSelected)
				return;
			Event_UPDATE_TARGET_POS *pRealEvt = (Event_UPDATE_TARGET_POS*)pEvt;

			m_havePatrolWayPoint = true;

			m_state = WAITING_FOR_WAYPOINT;

			if (!pRealEvt->patrolling)
				isPatrolling = false;
			else if (pRealEvt->patrolling && isPatrolling)
				pRealEvt->deleteOld = false;
			else
				isPatrolling = true;

			if (pRealEvt->deleteOld)
			{
				while (!m_nextPos.isEmpty())
					m_nextPos.deleteFront();
			}

			Array<Vector3, 1> path(*m_pContext, m_arena, 1);
			if (m_nextPos.isEmpty()) {
				SceneNode* pSN = m_hMovementSM.getObject<SoldierNPCMovementSM>()->getParentsSceneNode();
				if (pSN)
				{
					m_pContext->getNavMeshManager()->GetPath(&path, pSN->m_base.getPos(), pRealEvt->m_targetPos);
				}

			}

			else
			{
				Vector3 start = *(m_nextPos.get(m_nextPos.m_size - 1));
				m_pContext->getNavMeshManager()->GetPath(&path, Vector3{ start.m_x, start.m_y, start.m_z }, pRealEvt->m_targetPos);
			}

			for (int i = 1; i<path.m_size; i++)
				m_nextPos.add(path[i]);

			path.reset(0);
		}

		void SoldierNPCBehaviorSM::do_CHECK_SELECTED(PE::Events::Event *pEvt)
		{
			Event_CHECK_SELECTED *pRealEvt = (Event_CHECK_SELECTED*)pEvt;

			SoldierNPCMovementSM* pMSM = m_hMovementSM.getObject<SoldierNPCMovementSM>();
			if (pMSM)
			{
				SceneNode* pSN = pMSM->getParentsSceneNode();

				if (pSN)
				{
					PhysicsComponent* pPC = pSN->getFirstComponent<PE::Components::PhysicsComponent>();

					if (pPC)
					{
						if (!pRealEvt->multiSelect)
							isSelected = pPC->checkRayIntersection(pRealEvt->m_origin, pRealEvt->m_target);
						else
							isSelected = isSelected || pPC->checkRayIntersection(pRealEvt->m_origin, pRealEvt->m_target);
						//Vector3 temp = r.getPosForY0();
						//float a = 1;
					}
				}


			}
		}

		void SoldierNPCBehaviorSM::do_UPDATE_GROUP(PE::Events::Event *pEvt)
		{
			Event_UPDATE_GROUP *pRealEvent = (Event_UPDATE_GROUP *)(pEvt);

			if (pRealEvent->isStore)
			{
				if (isSelected)
				{
					m_pContext->getGameObjectManager()->m_groupControl[pRealEvent->num].add(m_hMyself);
				}

			}
			else if (!pRealEvent->isStore)
			{
				isSelected = false;
				Array<PE::Handle> aH = m_pContext->getGameObjectManager()->m_groupControl[pRealEvent->num];
				for (int i = 0; i < aH.m_size; i++)
				{
					if (aH[i] == m_hMyself)
						isSelected = true;
				}
			}

		}

	}
}




