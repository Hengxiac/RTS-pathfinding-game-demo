#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/Skeleton.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/DefaultAnimationSM.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Scene/DebugRenderer.h"
#include "PhysicsComponent.h"

namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(PhysicsComponent, Component);

		PhysicsComponent::PhysicsComponent(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) 
			: Component(context, arena, hMyself), m_isAnimated(false), m_shapes(context, arena, 4), yVelocity(0)
		{			
			//m_hSceneNode = hSceneNode;

		}

		void PhysicsComponent::addDefaultComponents()
		{
			Component::addDefaultComponents();
			// Data components

			// event handlers

		}
		void PhysicsComponent::addToSceneNode(const PE::Handle & hSceneNode) {

			m_hSceneNode = hSceneNode;
			m_hSceneNode.getObject<Component>()->addComponent(m_hMyself);
		}


		void PhysicsComponent::addShape(Handle m_shape) {
			if (m_shape.getObject<Component>()->isInstanceOf<Shape>())
				m_shapes.add(m_shape);
		}

		void PhysicsComponent::moveComponent() {
			
			if (!m_isAnimated){
				return;
			}
			if (nextDir.m_x != 0.0f || nextDir.m_y != 0.0f || nextDir.m_z != 0.0f)
			{
				turnInDirection(nextDir, 3.1415f);
				setPos(m_base.getPos() + nextDir*nextDist);

				/*SceneNode* pSN = m_hSceneNode.getObject<PE::Components::SceneNode>();
				pSN->m_base.setPos(m_base.getPos() + nextDir*nextDist);
				pSN->m_base.turnInDirection(nextDir, 3.1415f);*/
			}
		
		}

		void PhysicsComponent::drawComponent() {

			for (int i = 0; i < m_shapes.m_size;i++) {
				Shape* current = m_shapes[i].getObject<Shape>();
				if (!strcmp(current->m_collisionType, "box"))
				{
					Vector3 boundingCoord[8];
					
					Matrix4x4 base = current->m_base;
					// find the bounding volume in world space
					boundingCoord[0] = base.getPos() - current->m_dimensions.m_x * base.getU() - current->m_dimensions.m_y * base.getV() - current->m_dimensions.m_z * base.getN();
					boundingCoord[1] = base.getPos() + current->m_dimensions.m_x * base.getU() - current->m_dimensions.m_y * base.getV() - current->m_dimensions.m_z * base.getN();
					boundingCoord[2] = base.getPos() - current->m_dimensions.m_x * base.getU() + current->m_dimensions.m_y * base.getV() - current->m_dimensions.m_z * base.getN();
					boundingCoord[3] = base.getPos() + current->m_dimensions.m_x * base.getU() + current->m_dimensions.m_y * base.getV() - current->m_dimensions.m_z * base.getN();
					boundingCoord[4] = base.getPos() - current->m_dimensions.m_x * base.getU() - current->m_dimensions.m_y * base.getV() + current->m_dimensions.m_z * base.getN();
					boundingCoord[5] = base.getPos() + current->m_dimensions.m_x * base.getU() - current->m_dimensions.m_y * base.getV() + current->m_dimensions.m_z * base.getN();
					boundingCoord[6] = base.getPos() - current->m_dimensions.m_x * base.getU() + current->m_dimensions.m_y * base.getV() + current->m_dimensions.m_z * base.getN();
					boundingCoord[7] = base.getPos() + current->m_dimensions.m_x * base.getU() + current->m_dimensions.m_y * base.getV() + current->m_dimensions.m_z * base.getN();

					Vector3 color(1.0f, 1.0f, 0.0f);

					if (m_isAnimated)
					{
						color.m_x = 0.3f;
						color.m_y = 1.0f;
						color.m_z = 1.0f;

						
						/*{
							color.m_x = 0.6f;
							color.m_y = 1.0f;
							color.m_z = 0.2f;
						}*/
							

						DebugRenderer::Instance()->createBoundingBoxMesh(false, base, boundingCoord, color, 0);
					}
					}

					
			}

		}

		void PhysicsComponent::turnLeft(float angle) 
		{
			m_base.turnLeft(angle);
			for (int i = 0; i < m_shapes.m_size;i++) 
			{
				Shape* current = m_shapes[i].getObject<Shape>();
				current->m_base.turnLeft(angle);
			}
		}

		void PhysicsComponent::move(Vector3 distances)
		{
			m_base.moveForward(distances.getZ());
			m_base.moveRight(distances.getX());
			m_base.moveUp(distances.getY());

			for (int i = 0; i < m_shapes.m_size;i++)
			{
				Shape* current = m_shapes[i].getObject<Shape>();
				current->m_base.moveForward(distances.getZ());
				current->m_base.moveRight(distances.getX());
				current->m_base.moveUp(distances.getY());
			}
		}

		void PhysicsComponent::setTransform(const Matrix4x4 &t)
		{
			m_base.setPos(t.getPos());
			m_base.setU(t.getU());
			m_base.setV(t.getV());
			m_base.setN(t.getN());


			for (int i = 0; i < m_shapes.m_size;i++)
			{
				Shape* current = m_shapes[i].getObject<Shape>();
				current->initialize(current->m_collisionType, t, current->m_dimensions.getX(), 
					current->m_dimensions.getY(), current->m_dimensions.getZ());
			}
		}

		bool PhysicsComponent::checkRayIntersection(Vector3 origin, Vector3 target) {
			if (!m_isAnimated)
				return false;
			bool isSelected;

			for (int i = 0; i < m_shapes.m_size;i++) {
				Shape* current = m_shapes[i].getObject<Shape>();
				if (!strcmp(current->m_collisionType, "box"))
				{
					isSelected = current->intersect(origin, target);
					if (isSelected)
						return true;
				}
			}
			return false;
		}

		void PhysicsComponent::setPos(Vector3 pos) {

			Vector3 RelativePos = pos - m_base.getPos();
			m_base.setPos(pos);

			for (int i = 0; i < m_shapes.m_size;i++) {
				m_shapes[i].getObject<Shape>()->m_base.setPos(m_shapes[i].getObject<Shape>()->m_base.getPos() + RelativePos);
			}
		}

		void PhysicsComponent::turnInDirection(Vector3 targetDirection, PrimitiveTypes::Float32 maxAngle) {
			
			m_base.turnInDirection(targetDirection, maxAngle);

			for (int i = 0; i < m_shapes.m_size;i++) {
				m_shapes[i].getObject<Shape>()->m_base.turnInDirection(targetDirection, maxAngle);
			}
		}

		int PhysicsComponent::checkCollision(PhysicsComponent* collider, Vector3* Vdir) {

			if (!m_isAnimated) return C_NULL;

			if (m_base.getPos() == collider->m_base.getPos())
				return C_NULL;

			for (int i = 0; i < m_shapes.m_size; i++)
			{
				for (int j = 0; j < collider->m_shapes.m_size; j++)
				{
					char* temp = m_shapes[i].getObject<Shape>()->m_collisionType;
					Vector3 pos1 = m_shapes[i].getObject<Shape>()->m_base.getPos();
					Vector3 pos2 = collider->m_shapes[j].getObject<Shape>()->m_base.getPos();

					
					if (!strcmp(m_shapes[i].getObject<Shape>()->m_collisionType, "sphere") && !strcmp(collider->m_shapes[j].getObject<Shape>()->m_collisionType, "sphere"))
					{
						*Vdir = pos2 - pos1;

						float distanceSquare = (*Vdir).dotProduct(*Vdir);

						if (distanceSquare <= pow((m_shapes[i].getObject<Shape>()->m_dimensions.m_x + collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_x), 2))
						{
							*Vdir = *Vdir*(sqrt(distanceSquare) - m_shapes[i].getObject<Shape>()->m_dimensions.m_x -
								collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_x);
							return C_SPHERE;
						}
						else
							return C_NULL;
					}
					else if (!strcmp(m_shapes[i].getObject<Shape>()->m_collisionType, "sphere") && !strcmp(collider->m_shapes[j].getObject<Shape>()->m_collisionType, "box"))
					{
						Matrix4x4 Mwm = collider->m_shapes[j].getObject<Shape>()->m_base;
						Matrix4x4 Mm = Mwm.inverse()*(m_shapes[i].getObject<Shape>()->m_base);


						pos1 = Mm.getPos();

						Vector3 dir = pos1;
						Vector3 dirAbs;
						dirAbs.m_x = abs(dir.m_x);
						dirAbs.m_y = abs(dir.m_y);
						dirAbs.m_z = abs(dir.m_z);

						Vector3 vu = dirAbs - collider->m_shapes[j].getObject<Shape>()->m_dimensions;
						vu.m_x = max(vu.m_x, 0.0f);
						vu.m_y = max(vu.m_y, 0.0f);
						vu.m_z = max(vu.m_z, 0.0f);


						if (vu.dotProduct(vu) <= pow(m_shapes[i].getObject<Shape>()->m_dimensions.m_x, 2))
						{
							if (vu.m_x > 0 && vu.m_y <= 0 && vu.m_z <= 0
								&& collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_y!=0
								&& collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_z != 0)
							{
								if (dir.m_x >= 0)
								{
									Vector3 U = collider->m_shapes[j].getObject<Shape>()->m_base.getU();
									*Vdir = -(m_shapes[j].getObject<Shape>()->m_dimensions.m_x - vu.m_x) * collider->m_shapes[j].getObject<Shape>()->m_base.getU();
									return C_RIGHT;
								}

								else
								{
									*Vdir = +(m_shapes[j].getObject<Shape>()->m_dimensions.m_x - vu.m_x) * collider->m_shapes[j].getObject<Shape>()->m_base.getU();
									return C_LEFT;
								}

							}

							else if (vu.m_x <= 0 && vu.m_y > 0 && vu.m_z <= 0
								&& collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_z != 0
								&& collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_x != 0)
							{
								if (dir.m_y >= 0)
								{
									*Vdir = -(m_shapes[j].getObject<Shape>()->m_dimensions.m_x - vu.m_y) * collider->m_shapes[j].getObject<Shape>()->m_base.getV();
									return C_TOP;
								}
								else
								{
									*Vdir = +(m_shapes[j].getObject<Shape>()->m_dimensions.m_x - vu.m_y) * collider->m_shapes[j].getObject<Shape>()->m_base.getV();
									return C_BOTTOM;
								}

							}

							else if (vu.m_x <= 0 && vu.m_y <= 0 && vu.m_z > 0 
								&& collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_y != 0
								&& collider->m_shapes[j].getObject<Shape>()->m_dimensions.m_x != 0)
							{
								if (dir.m_z >= 0)
								{
									*Vdir = -(m_shapes[j].getObject<Shape>()->m_dimensions.m_x - vu.m_z) * collider->m_shapes[j].getObject<Shape>()->m_base.getN();
									return C_FAR;
								}
								else
								{
									*Vdir = +(m_shapes[j].getObject<Shape>()->m_dimensions.m_x - vu.m_z) * collider->m_shapes[j].getObject<Shape>()->m_base.getN();
									return C_NEAR;
								}

							}
							else if (vu.m_x > 0 && vu.m_z > 0)
							{	
								float vuLength = sqrt(vu.dotProduct(vu));
								vu = vu * ((m_shapes[j].getObject<Shape>()->m_dimensions.m_x - vuLength) / vuLength);
								
								//if (vu.m_z > vu.m_x)
								{
									if (dir.m_z >= 0)
										*Vdir = (*Vdir) + vu.m_z * collider->m_shapes[j].getObject<Shape>()->m_base.getN();
 									else
										*Vdir = (*Vdir) - vu.m_z * collider->m_shapes[j].getObject<Shape>()->m_base.getN();
								}
								//else
								{
									if (dir.m_x >= 0)
										*Vdir = (*Vdir) + vu.m_x * collider->m_shapes[j].getObject<Shape>()->m_base.getU();
									else
										*Vdir = (*Vdir) - vu.m_x * collider->m_shapes[j].getObject<Shape>()->m_base.getU();

								}

								Vector3 nDir = (*Vdir);
								nDir.normalize();
								Vector3 slideDir = nDir.crossProduct(Vector3(0.f, 1.0f, 0.0f));

								//Mm.setPos ()
													
								return C_COMPLEX;

							}
								

						}
						else
							return C_NULL;
					}
					else
						return C_NULL;
			}
		
			
				/*Vector3 boundingCoord[8];

				boundingCoord[0] = base.getPos() - collider.m_boxDimensions[0] * base.getU() - collider.m_boxDimensions[1] * base.getV() - collider.m_boxDimensions[2] * base.getN();
				boundingCoord[1] = base.getPos() - collider.m_boxDimensions[0] * base.getU() + collider.m_boxDimensions[1] * base.getV() - collider.m_boxDimensions[2] * base.getN();
				boundingCoord[2] = base.getPos() + collider.m_boxDimensions[0] * base.getU() - collider.m_boxDimensions[1] * base.getV() - collider.m_boxDimensions[2] * base.getN();
				boundingCoord[3] = base.getPos() + collider.m_boxDimensions[0] * base.getU() + collider.m_boxDimensions[1] * base.getV() - collider.m_boxDimensions[2] * base.getN();

				boundingCoord[4] = base.getPos() - collider.m_boxDimensions[0] * base.getU() - collider.m_boxDimensions[1] * base.getV() + collider.m_boxDimensions[2] * base.getN();
				boundingCoord[5] = base.getPos() - collider.m_boxDimensions[0] * base.getU() + collider.m_boxDimensions[1] * base.getV() + collider.m_boxDimensions[2] * base.getN();
				boundingCoord[6] = base.getPos() + collider.m_boxDimensions[0] * base.getU() - collider.m_boxDimensions[1] * base.getV() + collider.m_boxDimensions[2] * base.getN();
				boundingCoord[7] = base.getPos() + collider.m_boxDimensions[0] * base.getU() + collider.m_boxDimensions[1] * base.getV() + collider.m_boxDimensions[2] * base.getN();
			
				Vector4 planeEquations[6];
				planeEquations[0] = GetPlaneEquation(boundingCoord[0], boundingCoord[1], boundingCoord[2]); //front
				planeEquations[1] = GetPlaneEquation(boundingCoord[4], boundingCoord[6], boundingCoord[5]); //back
				planeEquations[2] = GetPlaneEquation(boundingCoord[0], boundingCoord[4], boundingCoord[1]); //left
				planeEquations[3] = GetPlaneEquation(boundingCoord[7], boundingCoord[6], boundingCoord[3]); //right
				planeEquations[4] = GetPlaneEquation(boundingCoord[0], boundingCoord[2], boundingCoord[4]);	//bottom
				planeEquations[5] = GetPlaneEquation(boundingCoord[5], boundingCoord[7], boundingCoord[1]);	//up*/
			}
			return C_NULL;
		}

		
		/*Vector4 GetPlaneEquation(Vector3 P1, Vector3 P2, Vector3 P3) {
		
			Vector3 dir12 = P2 - P1;
			Vector3 dir13 = P3 - P1;

			Vector3 normal = dir12.crossProduct(dir13);
			normal.normalize();

			float d = -(normal.m_x * P1.m_x + normal.m_y * normal.m_y + normal.m_z * normal.m_z);
			Vector4 result = { normal.m_x,normal.m_y,normal.m_z,d };
			return result;
		}*/

	}; // namespace Components
}; // namespace PE
