#ifndef __PYENGINE_2_0_PHYSICS_COMPONENT_H__ 
#define __PYENGINE_2_0_PHYSICS_COMPONENT_H__ 

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4x4.h"
#include "Shape.h"
#include "PrimeEngine\Physics\Ray.h"

//External Includes
#include <assert.h>

#define		C_NULL		0
#define 	C_NEAR		1
#define		C_FAR		2
#define		C_LEFT		3
#define		C_RIGHT		4
#define		C_TOP		5
#define		C_BOTTOM	6
#define		C_SPHERE    7
#define		C_COMPLEX	8
namespace PE {
namespace Components {
struct PhysicsComponent : public Component
{
	PE_DECLARE_CLASS(PhysicsComponent);

	 //Constructor -------------------------------------------------------------
	PhysicsComponent::PhysicsComponent(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

	virtual ~PhysicsComponent() {}



	
	// Component ------------------------------------------------------------

	virtual void addDefaultComponents();

	void addToSceneNode(const PE::Handle & hSceneNode);

	int checkCollision(PhysicsComponent *collider, Vector3* Vdir);
	
	void addShape(Handle m_shape);

	void moveComponent();

	void setPos(Vector3 pos);

	void turnInDirection(Vector3 targetDirection, PrimitiveTypes::Float32 maxAngle);

	void turnLeft(float angle);

	void move(Vector3 distances);

	void setTransform(const Matrix4x4 &t);
	
	bool m_isAnimated;

	void drawComponent();

	bool checkRayIntersection(Vector3 origin, Vector3 target);

	Array<Handle, 1> m_shapes;

	Handle m_hSceneNode;

	Matrix4x4 m_base;
 
	Vector3 nextDir;

	float nextDist;

	float yVelocity;

	

}; // class SceneNode

}; // namespace Components
}; // namespace PE

#endif 

