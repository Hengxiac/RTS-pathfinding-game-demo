#ifndef __PrimeEnginePacket_H__
#define __PrimeEnginePacket_H__

#define PE_PACKET_HEADER 4
#define PE_PACKET_TOTAL_SIZE (4 * 1024)

// max payload of an event sent over network
#define PE_MAX_EVENT_PAYLOAD 512
#define PE_MAX_GHOST_PAYLOAD 128

// max number of events that can be backed up (in case network is budy)
// if hit this, need to throttle network
#define PE_MAX_EVENT_JAM 16 

#define PE_GHOST_MASK_SIZE 4
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>
#include <vector>

// Inter-Engine includes


// Sibling/Children includes
#include "EventTransmissionData.h"
#include "GhostTransmissionData.h"

namespace PE {

// struct used to maintain list of transmitted (sent) data
// is used to store what was sent. by looking at this record, stream amanagers need to be able to figure out
// what needs to be resent
struct TransmissionRecord
{
	PrimitiveTypes::UInt32 m_id;

	std::vector<EventTransmissionData> m_sentEvents;
	std::vector<GhostTransmissionData> m_sentGhosts;

	TransmissionRecord *m_pNextTransmission;
};

struct Packet : public PEAllocatableAndDefragmentable
{
	union
	{
		PrimitiveTypes::UInt32 m_packetDataSizeInInet;
		char m_data[PE_PACKET_TOTAL_SIZE];
	};
};

}; // namespace PE
#endif
