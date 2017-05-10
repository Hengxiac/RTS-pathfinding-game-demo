#ifndef __PYENGINE_2_0_NAVMESH_MANAGER__
#define __PYENGINE_2_0_NAVMESH_MANAGER__

#define NOMINMAX
#define EULERDISTANCE(x, y) sqrt((x)*(x)+(y)*(y))
#define MAHATTANDISTANCE(x1,y1,x2,y2) fabs(x1-x2) + fabs(y1-y2)
#define DIAGNOSEDISTANCE(x1,x2,y1,y2) fabs(x1-x2) + fabs(y1-y2) + (0.414 - 2) * (fabs(x1 - x2) < fabs(y1 - y2)?(x1- x2) : (y1 - y2))
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"
#include "PrimeEngine/Math/Matrix4x4.h"
#include "PrimeEngine/Pathfinding/PriorityQueue.h"

// Sibling/Children includes


namespace PE {
	namespace Components {
		struct NavMeshManager : public Component
		{
			PE_DECLARE_CLASS(NavMeshManager);

			struct Triangle
			{
				Vector3 vtx[3];
				Vector3 pos;
				int id;
				int adjacent[8];
				int neighbor;
			};
			NavMeshManager(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);
			virtual ~NavMeshManager() {}

			virtual void addDefaultComponents();

			void buildTriangles();

			void drawNavMesh();

			bool NavMeshManager::isAdjacent(Triangle tri1, Triangle tri2);

			Handle m_hIndexBufferCPU;

			Handle m_hPositionBufferCPU;

			Matrix4x4 m_base; // local transform

							  //PrimitiveTypes::Float32 m_mapDimensions[4];

			int m_numTri;

			Array<Triangle, 1> triangleList;

			void Astar(int startid, int destination, int* route);

			int FindNavmeshTri(Vector3 sourcePosition);

			bool findportallist(int *route, int *portalleft, int *portalright);

			bool checkAdjacentBound(int now, int next, int*segment);

			int funnelAlgorithm(Vector3 start, Vector3 dest, int * route, int* portalleft, int*portalright,Array<Vector3, 1> *path);

			int GetPath(Array<Vector3, 1> *path, Vector3 start, Vector3 end);

			inline double triarea2(const Vector3 a, const Vector3 b, const Vector3 c)
			{
				const float ax = b.m_x - a.m_x;
				const float ay = b.m_z - a.m_z;
				const float bx = c.m_x - a.m_x;
				const float by = c.m_z - a.m_z;
				return bx*ay - ax*by;
			}

			inline bool isequal(const Vector3 a, const Vector3 b)
			{
				static const float eq = 0.001f*0.001f;
				return ((a.m_x - b.m_x)*(a.m_x - b.m_x) + (a.m_z - b.m_z)*(a.m_z - b.m_z) < eq);
			}
		};
	}; // namespace Components
}; // namespace PE
#endif#
