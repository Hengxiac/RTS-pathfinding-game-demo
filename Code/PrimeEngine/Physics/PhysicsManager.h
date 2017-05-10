#ifndef __PYENGINE_2_0_PHYSICS_MANAGER__
#define __PYENGINE_2_0_PHYSICS_MANAGER__

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




// Sibling/Children includes


namespace PE {
namespace Components {
struct PhysicsManager : public Component
{
	PE_DECLARE_CLASS(PhysicsManager);

	PhysicsManager(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);
	virtual ~PhysicsManager() {}

	virtual void addDefaultComponents();

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PHYS_UPDATE)
		virtual void do_PHYS_UPDATE(PE::Events::Event *pEvt);


	void moveComponents();

	void do_PHYS_UPDATE();

	void do_PHYS_UPDATE(int a);

	void drawComponents();

	void addPhys(Handle hPhys);

	void addActivePhys(Handle hPhys);

	void removePhys(Handle hPhys);
	//void PhysicsManager::run_simulation(PrimitiveTypes::Float32 frameTime);

	Array<Handle> m_PhysComponents;

	Array<Handle> m_ActivePhysComponents;

	const float acceleration = 9.8f;
	
	void checkRayIntersection(const Ray &r);
};
}; // namespace Components
}; // namespace PE
#endif#
