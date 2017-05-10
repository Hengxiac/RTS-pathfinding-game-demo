// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PhysicsManager.h"
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"


// Sibling/Children includes

#include "PrimeEngine/Lua/LuaEnvironment.h"

namespace PE {
namespace Components {
	using namespace PE::Events;
	PE_IMPLEMENT_CLASS1(PhysicsManager, Component);
	PhysicsManager::PhysicsManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
		: Component(context, arena, hMyself), m_PhysComponents(context, arena, 64), m_ActivePhysComponents(context, arena, 16)
	{
	}

	void PhysicsManager::addDefaultComponents()
	{
		
		addComponent(m_pContext->getLuaEnvironment()->getHandle());
		Component::addDefaultComponents();

		PE_REGISTER_EVENT_HANDLER(Event_PHYSICS_UPDATE, PhysicsManager::do_PHYS_UPDATE);
	}


	void PhysicsManager::addPhys(Handle hPhys)
	{	
		if (hPhys.getObject<Component>()->isInstanceOf<PhysicsComponent>())
		{
			m_PhysComponents.add(hPhys);
		}

	}

	void PhysicsManager::addActivePhys(Handle hPhys)
	{
		if (hPhys.getObject<Component>()->isInstanceOf<PhysicsComponent>())
		{
			m_ActivePhysComponents.add(hPhys);
		}

	}

	void PhysicsManager::removePhys(Handle hPhys)
	{
		int index = m_PhysComponents.indexOf(hPhys);
		if (m_PhysComponents.m_size > index)
		{
			m_PhysComponents.remove(index);
		}
		
		index = m_ActivePhysComponents.indexOf(hPhys);
		
		if (m_ActivePhysComponents.m_size > index)
		{
			m_ActivePhysComponents.remove(index);
		}
	}

	void PhysicsManager::moveComponents() 
	{
		for (int i = 0; i < m_PhysComponents.m_size; i++)
		{
			PhysicsComponent *pPC = m_PhysComponents[i].getObject<PhysicsComponent>();
			pPC->moveComponent();
		}
	}

	void PhysicsManager::drawComponents() {
		for (int i = 0; i < m_PhysComponents.m_size; i++)
		{
			PhysicsComponent *pPC = m_PhysComponents[i].getObject<PhysicsComponent>();
			pPC->drawComponent();
		}
	}

	void PhysicsManager::do_PHYS_UPDATE()
	{
		moveComponents();
	}

	void PhysicsManager::checkRayIntersection(const Ray &r)
	{
		for (int i = 0; i < m_PhysComponents.m_size; i++)
		{
			PhysicsComponent *pPC = m_PhysComponents[i].getObject<PhysicsComponent>();
			//pPC->checkRayIntersection(r);
		}
	}

	void PhysicsManager::do_PHYS_UPDATE(PE::Events::Event *pEvt)
	{
		PE::Events::Event_PHYSICS_UPDATE *pRealEvt = (PE::Events::Event_PHYSICS_UPDATE*)(pEvt);

		for (int i = 0; i < m_ActivePhysComponents.m_size; i++)
		{
			for (int j = 0; j < m_PhysComponents.m_size; j++) {

				if (m_ActivePhysComponents[i] == m_PhysComponents[j])
					continue;

				Vector3 vDir = { 0.0f, 0.0f, 0.0f };
				int CollisionResult = m_ActivePhysComponents[i].getObject<PhysicsComponent>()->checkCollision(m_PhysComponents[j].getObject<PhysicsComponent>(), &vDir);

				if (CollisionResult != C_NULL)
				{
					PE::Handle h("Event_Resolve_Collision", sizeof(Event_Resolve_Collision));
					Event_Resolve_Collision *pEvt = new(h) Event_Resolve_Collision(m_ActivePhysComponents[i], m_PhysComponents[j]);	

					m_pContext->getGameObjectManager()->handleEvent(pEvt);
					h.release();
				}
			}
		}
	}
	void PhysicsManager::do_PHYS_UPDATE(int a)
	{	

		//PE::Events::Event_PHYSICS_UPDATE *pRealEvt = (PE::Events::Event_PHYSICS_UPDATE*)(pEvt);
		//float frameTime = pRealEvt->m_frameTime;

		//moveComponents();
		// Collision
		for (int i = 0; i < m_PhysComponents.m_size; i++) 
		{
			if (!m_PhysComponents[i].getObject<PhysicsComponent>()->m_isAnimated)
				continue;

			bool gravityResult = false;

			for (int j = 0; j < m_PhysComponents.m_size; j++) {
				if (i == j)
					continue;
				
				Vector3 vDir = { 0.0f, 0.0f, 0.0f };
				int CollisionResult = m_PhysComponents[i].getObject<PhysicsComponent>()->checkCollision(m_PhysComponents[j].getObject<PhysicsComponent>(), &vDir);
				
				if (CollisionResult != C_NULL)
				{	
					if (CollisionResult == C_TOP)
						gravityResult = true;
					else  if(CollisionResult != C_COMPLEX)
					{
						Vector3 pos1 = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();
						Vector3 pos2 = m_PhysComponents[j].getObject<PhysicsComponent>()->m_base.getPos();
							
						vDir.m_y = 0.0f;

						if (m_PhysComponents[i].getObject<PhysicsComponent>()->m_isAnimated && m_PhysComponents[j].getObject<PhysicsComponent>()->m_isAnimated)
						{
							Vector3 dir = pos2 - pos1;
							dir.normalize();
							Vector3 slide = Vector3(0.0f, 1.0f, 0.0f).crossProduct(dir);
							float dist =  m_PhysComponents[i].getObject<PhysicsComponent>()->nextDist;
							m_PhysComponents[i].getObject<PhysicsComponent>()->setPos(pos1 + vDir - slide * dist/2);
							//pos1 = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();
						}
							
						else
						{
							Vector3 nDir = vDir;
							nDir.normalize();

							m_PhysComponents[i].getObject<PhysicsComponent>()->setPos(pos1 
								- vDir); //*m_PhysComponents[i].getObject<PhysicsComponent>()->nextDist

							/*float cos_ = vDir.dotProduct(m_PhysComponents[i].getObject<PhysicsComponent>()->nextDir) /
								sqrt(vDir.dotProduct(vDir) * m_PhysComponents[i].getObject<PhysicsComponent>()->
									nextDir.dotProduct(m_PhysComponents[i].getObject<PhysicsComponent>()->nextDir));*/
							

							Vector3 slideDir = nDir.crossProduct(Vector3(0.f, 1.0f, 0.0f));
							//Vector3 slideDir = m_PhysComponents[i].getObject<PhysicsComponent>()->nextDir - nDir * cos_ ;

							slideDir.normalize();

							pos1 = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();
							float dist = m_PhysComponents[i].getObject<PhysicsComponent>()->nextDist;
							m_PhysComponents[i].getObject<PhysicsComponent>()->setPos(pos1 - slideDir * dist);
						}
					}
					else if (CollisionResult == C_COMPLEX)
					{					
						Vector3 nDir = vDir;
						nDir.normalize();
						Vector3 pos1 = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();
						m_PhysComponents[i].getObject<PhysicsComponent>()->setPos(pos1 + vDir);

						Vector3 slideDir = nDir.crossProduct(Vector3(0.f, 1.0f, 0.0f));

						pos1 = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();
						Vector3 pos2 = m_PhysComponents[j].getObject<PhysicsComponent>()->m_base.getPos();

						Vector3 dir = pos1 - pos2;

						dir.normalize();

						float dist = m_PhysComponents[i].getObject<PhysicsComponent>()->nextDist;
						m_PhysComponents[i].getObject<PhysicsComponent>()->setPos(pos1 + dir * dist);
					}
							
				}
			}

			/*if (!gravityResult) 
			{
				m_PhysComponents[i].getObject<PhysicsComponent>()->yVelocity += 
					(frameTime* acceleration);
				Vector3 pos1 = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();

				
				float y_dis = m_PhysComponents[i].getObject<PhysicsComponent>()->yVelocity*frameTime;

				Vector3 temp = pos1 + Vector3(0, -y_dis, 0);
				
				if (pos1.m_y*(pos1.m_y - y_dis) < 0) {
					Vector3 temp__(pos1.m_x, 0.0f, pos1.m_z);
					m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.setPos(temp__);

					//Vector3 whoyou = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();
					
					gravityResult = true;

					for (int j = 0; j < m_PhysComponents.m_size; j++) {
						if (i == j)
							continue;

						Vector3 vDir = { 0.0f, 0.0f, 0.0f };
						int CollisionResult = m_PhysComponents[i].getObject<PhysicsComponent>()->checkCollision(m_PhysComponents[j].getObject<PhysicsComponent>(), &vDir);

						if (CollisionResult == C_TOP)
						{

							gravityResult = true;
						}
					}
					if (!gravityResult)
					{
						m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.setPos(temp);
					}
					else
					{
						m_PhysComponents[i].getObject<PhysicsComponent>()->yVelocity = 0.0f;
						Vector3 poss = m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.getPos();
					}
				}
				else
				{
					m_PhysComponents[i].getObject<PhysicsComponent>()->m_base.setPos(temp);
					//m_PhysComponents[i].getObject<PhysicsComponent>()->yVelocity = 0.0f;
				}

				
			}
			else 
			{
				m_PhysComponents[i].getObject<PhysicsComponent>()->yVelocity = 0.0f;
			}*/
		}

		// Gravity

		/*for (int i = 0; i < m_PhysComponents.m_size; i++)
		{
			PhysicsComponent* temp = m_PhysComponents[i].getObject<PhysicsComponent>();

			if (!m_PhysComponents[i].getObject<PhysicsComponent>()->m_isAnimated)
				continue;
			bool gravityResult = false;

			for (int j = 0; j < m_Planes.m_size; j++) {			
				Vector3 vDir = { 0.0f, 0.0f, 0.0f };
				if (m_PhysComponents[i].getObject<PhysicsComponent>()->checkCollision(m_Planes[j].getObject<PhysicsComponent>(), &vDir) == C_TOP)
				{
					gravityResult = true;
					break;
				}
			}
			if (!gravityResult)
			{
				Vector3 pos1 = m_PhysComponents[i].getObject<PhysicsComponent>()->m_hSceneNode.getObject<SceneNode>()->m_base.getPos();
				
				Vector3 dir = { 0.0f, -1.0f, 0.0f };
				
				m_PhysComponents[i].getObject<PhysicsComponent>()->m_hSceneNode.getObject<SceneNode>()->m_base.setPos(pos1 + (dir*0.2) );

			}
		}*/

	}
}; // namespace Components
}; // namespace PE

