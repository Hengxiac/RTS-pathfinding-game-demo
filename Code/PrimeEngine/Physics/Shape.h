#ifndef __PYENGINE_2_0_SHAPE_
#define __PYENGINE_2_0_SHAPE_

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"

#include "../Physics/PhysicsComponent.h"
#include "../Physics/ray.h"


// Sibling/Children includes


namespace PE {
namespace Components {
struct Shape : public Component
{
	PE_DECLARE_CLASS(Shape);

	Shape(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);
	virtual ~Shape() {}

	virtual void addDefaultComponents();

	void Shape::initialize(const char* collisionType, const Matrix4x4 &base, float dx, float dy = 0.0f, float dz = 0.0f);
	void Shape::initialize(const char* collisionType, Vector3 pos, Vector3 u, Vector3 v, Vector3 n, float dx, float dy, float dz);
	Matrix4x4 m_base;

	char m_collisionType[16];

	Vector3 m_dimensions;

	bool intersect(Vector3 origin, Vector3 Target);

};
}; // namespace Components
}; // namespace PE
#endif#
