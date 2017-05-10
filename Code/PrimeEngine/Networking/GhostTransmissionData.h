#ifndef __PrimeEngineGhostTransmissionData_H__
#define __PrimeEngineGhostTransmissionData_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>
#include <vector>
#include <deque>

// Inter-Engine includes

#include "../Events/Component.h"

extern "C"
{
#include "../../luasocket_dist/src/socket.h"
};

#include "PrimeEngine/Networking/NetworkContext.h"
#include "PrimeEngine/Utils/Networkable.h"

// Sibling/Children includes


namespace PE {

	namespace Events
	{
		struct Event;
	};

struct GhostTransmissionData
{
	bool m_isGuaranteed;
	int m_size;
	int m_orderId;
	int m_ghostId;
	bool m_statusFlag;
	int m_ghostType;
	bool m_stateMask[PE_GHOST_MASK_SIZE]; //PE::Components::GhostManager::MASK_SIZE
	char m_payload[PE_MAX_GHOST_PAYLOAD];
};

}; // namespace PE
#endif
