#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/Skeleton.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/DefaultAnimationSM.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "Shape.h"


namespace PE {
namespace Components {

		PE_IMPLEMENT_CLASS1(Shape, Component);

		Shape::Shape(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
			: Component(context, arena, hMyself)
		{
		}

		void Shape::addDefaultComponents()
		{

			Component::addDefaultComponents();
		}

	
		void Shape::initialize(const char* collisionType, const Matrix4x4 &base,float dx, float dy, float dz) {
			
			m_base.setPos(base.getPos());
			m_base.setU(base.getU());
			m_base.setV(base.getV());
			m_base.setN(base.getN());

			if (!strcmp(collisionType, "sphere")) {

				strcpy(m_collisionType, collisionType);
				m_dimensions.m_x = dx;

				m_base.moveUp(m_dimensions.m_x);
			}
			else if (!strcmp(collisionType, "box"))
			{
				strcpy(m_collisionType, collisionType);
				m_dimensions.m_x = dx;
				m_dimensions.m_y = dy;
				m_dimensions.m_z = dz;

				m_base.moveUp(m_dimensions.m_y);

			}
		}

		void Shape::initialize(const char* collisionType, Vector3 pos, Vector3 u, Vector3 v, Vector3 n, float dx, float dy, float dz) {

			m_base.setPos(pos);
			m_base.setU(u);
			m_base.setV(v);
			m_base.setN(n);

			if (!strcmp(collisionType, "sphere")) {

				strcpy(m_collisionType, collisionType);
				m_dimensions.m_x = dx;

				m_base.moveUp(m_dimensions.m_x);
			}
			else if (!strcmp(collisionType, "box"))
			{
				strcpy(m_collisionType, collisionType);
				m_dimensions.m_x = dx;
				m_dimensions.m_y = dy;
				m_dimensions.m_z = dz;

				m_base.moveUp(m_dimensions.m_y);

			}
		}
		bool Shape::intersect(Vector3 origin, Vector3 target)
		{
			Vector3 tOrig = m_base.inverse() * origin;
			Vector3 tTarg = m_base.inverse() * target;
			
			Ray r = Ray(tOrig, tTarg);

			Vector3 parameters[2];
			parameters[0].m_x = -m_dimensions.m_x;
			parameters[0].m_y = -m_dimensions.m_y;
			parameters[0].m_z = -m_dimensions.m_z;

			parameters[1].m_x = m_dimensions.m_x;
			parameters[1].m_y = m_dimensions.m_y;
			parameters[1].m_z = m_dimensions.m_z;

			float tmin, tmax, tymin, tymax, tzmin, tzmax;

			tmin = (parameters[r.sign[0]].m_x - r.origin.m_x) * r.inv_direction.m_x;
			tmax = (parameters[1 - r.sign[0]].m_x - r.origin.m_x) * r.inv_direction.m_x;
			tymin = (parameters[r.sign[1]].m_y - r.origin.m_y) * r.inv_direction.m_y;
			tymax = (parameters[1 - r.sign[1]].m_y - r.origin.m_y) * r.inv_direction.m_y;
			if ((tmin > tymax) || (tymin > tmax))
				return false;
			if (tymin > tmin)
				tmin = tymin;
			if (tymax < tmax)
				tmax = tymax;
			tzmin = (parameters[r.sign[2]].m_z - r.origin.m_z) * r.inv_direction.m_z;
			tzmax = (parameters[1 - r.sign[2]].m_z - r.origin.m_z) * r.inv_direction.m_z;
			if ((tmin > tzmax) || (tzmin > tmax))
				return false;
			if (tzmin > tmin)
				tmin = tzmin;
			if (tzmax < tmax)
				tmax = tzmax;

			return true;
		}
		
	}; // namespace Components
}; // namespace PE
