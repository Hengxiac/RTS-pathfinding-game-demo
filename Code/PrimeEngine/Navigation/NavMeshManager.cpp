// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "NavMeshManager.h"
#include "../Scene/DebugRenderer.h"
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


// Sibling/Children includes

#include "PrimeEngine/Lua/LuaEnvironment.h"

#define USE_CURVED_SMOOTHING 1

namespace PE {
	namespace Components {
		using namespace PE::Events;
		PE_IMPLEMENT_CLASS1(NavMeshManager, Component);
		NavMeshManager::NavMeshManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
			: Component(context, arena, hMyself), /*m_mapDimensions{0.0f, 0.0f, 0.0f, 0.0f},*/ triangleList(context, arena)
		{
		}

		void NavMeshManager::addDefaultComponents()
		{

			addComponent(m_pContext->getLuaEnvironment()->getHandle());
			Component::addDefaultComponents();

		}

		void NavMeshManager::buildTriangles() {
			if (m_hPositionBufferCPU.isValid())
			{
				m_numTri = m_hIndexBufferCPU.getObject<PositionBufferCPU>()->m_values.m_size / 3;

				PositionBufferCPU* pPB = m_hPositionBufferCPU.getObject<PositionBufferCPU>();
				Array<float> vtxPos = pPB->m_values;

				triangleList.reset(m_numTri);

				for (int i = 0; i < m_numTri; i++)
				{
					Triangle temp;

					temp.vtx[0].m_x = vtxPos[9 * i];
					temp.vtx[0].m_y = vtxPos[9 * i + 1];
					temp.vtx[0].m_z = vtxPos[9 * i + 2];

					temp.vtx[1].m_x = vtxPos[9 * i + 3];
					temp.vtx[1].m_y = vtxPos[9 * i + 4];
					temp.vtx[1].m_z = vtxPos[9 * i + 5];

					temp.vtx[2].m_x = vtxPos[9 * i + 6];
					temp.vtx[2].m_y = vtxPos[9 * i + 7];
					temp.vtx[2].m_z = vtxPos[9 * i + 8];

					temp.pos.m_x = (temp.vtx[0].m_x + temp.vtx[1].m_x + temp.vtx[2].m_x) / 3;
					temp.pos.m_y = (temp.vtx[0].m_y + temp.vtx[1].m_y + temp.vtx[2].m_y) / 3;
					temp.pos.m_z = (temp.vtx[0].m_z + temp.vtx[1].m_z + temp.vtx[2].m_z) / 3;

					temp.id = i + 1;

					triangleList.add(temp);
				}
				for (int i = 0; i < m_numTri; i++)
				{
					triangleList[i].neighbor = 0;
					for (int j = 0; j < m_numTri; j++)
					{
						if (i != j)
						{
							if (isAdjacent(triangleList[i], triangleList[j]))
								triangleList[i].adjacent[triangleList[i].neighbor++] = triangleList[j].id;
						}
					}
				}

			}

		}

		bool NavMeshManager::isAdjacent(Triangle tri1, Triangle tri2)
		{
			int count = 0;
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					if(tri1.vtx[i] == tri2.vtx[j])
					   count++;
				}
			}

			return (count == 2);

		}

		void NavMeshManager::drawNavMesh()
		{
			if (triangleList.m_size>0)
			{
				Array<Vector3> lines(*m_pContext, m_arena, triangleList.m_size * 12);

				Vector3 color{ 1.0f, 0.5f, 0.0f };
				for (int i = 0; i < triangleList.m_size; i++)
				{
					Vector3 vtx0(triangleList[i].vtx[0]);
					Vector3 vtx1(triangleList[i].vtx[1]);
					Vector3 vtx2(triangleList[i].vtx[2]);

					vtx0 = m_base * vtx0;
					vtx1 = m_base * vtx1;
					vtx2 = m_base * vtx2;

					lines.add(vtx0);
					lines.add(color);
					lines.add(vtx1);
					lines.add(color);

					lines.add(vtx1);
					lines.add(color);
					lines.add(vtx2);
					lines.add(color);

					lines.add(vtx2);
					lines.add(color);
					lines.add(vtx0);
					lines.add(color);
				}
				DebugRenderer *render = DebugRenderer::Instance();
				render->createLineMesh(true, m_base, &(*lines.getFirstPtr()).m_x, lines.m_size / 12 * 6, 0);
				lines.reset(0);
			}
		}


		void NavMeshManager::Astar(int startid, int destination, int* route)
		{
			if (startid == destination)
			{
				route[0] = 1;
				route[1] = startid;
			}
			else
			{
				PriorityQueue<int> Pqueue;
				int *come_from = new int[m_numTri + 1];
				float *cost_so_far = new float[m_numTri + 1];


				float newcost, heuristiccost, priority;
				int currentID, next, nodenow = destination, nodenums = 0;

				for (int i = 0; i < m_numTri; i++)
				{
					come_from[i] = -1;
					cost_so_far[i] = -1;
				}
				come_from[startid] = 0;
				cost_so_far[startid] = 0;
				Pqueue.EnterQueue(startid, 0);
				while (!Pqueue.isEmpty())
				{
					currentID = Pqueue.DeQueue();

					if (currentID == destination)
						break;
					else
					{
						int neighbor = 1;
						Triangle tri(triangleList[currentID - 1]);
						while (neighbor <= triangleList[currentID - 1].neighbor)
						{
							next = triangleList[currentID - 1].adjacent[neighbor - 1];
							neighbor++;
							//newcost = cost_so_far[currentID] + EULERDISTANCE(triangleList[currentID - 1].pos.m_x - triangleList[next - 1].pos.m_x,  triangleList[currentID - 1].pos.m_z -  triangleList[next - 1].pos.m_z);
							newcost = cost_so_far[currentID] + DIAGNOSEDISTANCE(triangleList[currentID - 1].pos.m_x , triangleList[next - 1].pos.m_x, triangleList[currentID - 1].pos.m_z , triangleList[next - 1].pos.m_z);
							if (cost_so_far[next] == -1 || newcost < cost_so_far[next])
							{
								cost_so_far[next] = newcost;
								come_from[next] = currentID;
								//heuristiccost = EULERDISTANCE(triangleList[next - 1].pos.m_x - triangleList[destination - 1].pos.m_x, triangleList[next - 1].pos.m_z -  triangleList[destination - 1].pos.m_z);
								heuristiccost = DIAGNOSEDISTANCE(triangleList[next - 1].pos.m_x , triangleList[destination - 1].pos.m_x, triangleList[next - 1].pos.m_z , triangleList[destination - 1].pos.m_z);
								priority = newcost + heuristiccost;
								Pqueue.EnterQueue(next, priority);
							}
						}

					}
				}
				///////////////////////////////////////////////
				//get route array
				int *stack = new int[m_numTri], stacktop = 0;
				while (nodenow != startid)
				{
					stack[stacktop++] = nodenow;
					nodenow = come_from[nodenow];//get the inverse of the route
				}
				stack[stacktop] = startid;
				nodenums = 1;
				while (stacktop >= 0)
				{
					route[nodenums++] = stack[stacktop--];
				}
				nodenums--;
				route[0] = nodenums;   //route[0] store the number of nodes in the A* path

				delete come_from;
				delete cost_so_far;
			}
		}

		/*int NavMeshManager::FindNavmeshTri(Vector3 sourcePosition)
		{
			int mapsize = m_numTri;
			double A12, B12, C12, A31, B31, C31, A23, B23, C23;

			double E12, E31, E23;
			bool found = false;
			int id = 0;
			for (int num = 0; found == false && num <= mapsize - 1; num++)
			{
				A12 = triangleList[num].vtx[0].m_z - triangleList[num].vtx[1].m_z;       // A=dY
				B12 = triangleList[num].vtx[1].m_x - triangleList[num].vtx[0].m_x;       // B=-dX
				C12 = (triangleList[num].vtx[0].m_x - triangleList[num].vtx[1].m_x)*triangleList[num].vtx[1].m_z - (triangleList[num].vtx[0].m_z - triangleList[num].vtx[1].m_z)*triangleList[num].vtx[1].m_x;                      // C=dXY-dYX 

				A31 = triangleList[num].vtx[2].m_z - triangleList[num].vtx[0].m_z;   // A=dY
				B31 = triangleList[num].vtx[0].m_x - triangleList[num].vtx[2].m_x;      // B=-dX
				C31 = (triangleList[num].vtx[2].m_x - triangleList[num].vtx[0].m_x)*triangleList[num].vtx[0].m_z - (triangleList[num].vtx[2].m_z - triangleList[num].vtx[0].m_z)*triangleList[num].vtx[0].m_x;                              // C=dXY-dYX 

				A23 = triangleList[num].vtx[1].m_z - triangleList[num].vtx[2].m_z;     // A=dY
				B23 = triangleList[num].vtx[2].m_x - triangleList[num].vtx[1].m_x;       // B=-dX
				C23 = (triangleList[num].vtx[1].m_x - triangleList[num].vtx[2].m_x)*triangleList[num].vtx[2].m_z - (triangleList[num].vtx[1].m_z - triangleList[num].vtx[2].m_z)*triangleList[num].vtx[2].m_x;



				E12 = A12*sourcePosition.m_x + B12*sourcePosition.m_z + C12;
				E31 = A31*sourcePosition.m_x + B31*sourcePosition.m_z + C31;
				E23 = A23*sourcePosition.m_x + B23*sourcePosition.m_z + C23;


				if ((E12 <= 0.0f && E31 <= 0.0f && E23 <= 0.0f) || (E12 >= 0.0f&& E31 >= 0.0f && E23 >= 0.0f))
				{
					found = true;
					id = triangleList[num].id;
				}
			}
			return id;
		}
		*/

		bool sameside(Vector3 A, Vector3 B, Vector3 C, Vector3 P)
		{
			Vector3 AB = B - A;
			Vector3 AC = C - A;
			Vector3 AP = P - A;
            
			Vector3 ABC = AB.crossProduct(AC);
			Vector3 ABP = AB.crossProduct(AP);

			return ABC.dotProduct(ABP) >= 0;
		}// a step to check if P is inside tri ABC
		int NavMeshManager::FindNavmeshTri(Vector3 sourcePosition)
		{
			int mapsize = m_numTri;
			bool found = false;
			int id = 0;
			for (int num = 0; found == false && num <= mapsize - 1; num++)
			{
				found = sameside(triangleList[num].vtx[0], triangleList[num].vtx[1], triangleList[num].vtx[2], sourcePosition) &&
					    sameside(triangleList[num].vtx[1], triangleList[num].vtx[2], triangleList[num].vtx[0], sourcePosition) &&
					    sameside(triangleList[num].vtx[2], triangleList[num].vtx[0], triangleList[num].vtx[1], sourcePosition);

				if (found == true)
					id = triangleList[num].id;
			}

			return id;
		}



		bool NavMeshManager::findportallist(int *route, int *portalleft, int*portalright)
		{
			int num = route[0];
			portalleft[0] = portalright[0] = num-1;
			int count = 1;
			int now, next;
			int left, right;
			int segment[2];
			for (; count <= num-1; count++)
			{
				now = route[count];
				next = route[count + 1];
				checkAdjacentBound(now, next, segment);
				left = segment[0];
				right = segment[1];

				if (triarea2(triangleList[now - 1].pos, triangleList[now - 1].vtx[right], triangleList[now - 1].vtx[left]) < 0.0f)
				{
					portalleft[count] = right;
					portalright[count] = left;
				}
				else
				{
					portalleft[count] = left;
					portalright[count] = right;
				}
			}
			return true;
		}


		bool NavMeshManager::checkAdjacentBound(int now, int next, int* edge)
		{

			int vert1 = 0, vert2 = 0;
			bool found1 = false, found2 = false;
			for (vert1 = 0; !(found1 == true && found2 == true) && vert1 <= 3/*the number of verts*/; vert1++)
				for (vert2 = 0; vert2 <= 3; vert2++)
				{
					if (triangleList[now - 1].vtx[vert1].m_x == triangleList[next - 1].vtx[vert2].m_x && triangleList[now - 1].vtx[vert1].m_z == triangleList[next - 1].vtx[vert2].m_z)
					{
						if (found1 == false)
						{
							found1 = true;
							edge[0] = vert1;
							vert2 = 3;//to end the loop
						}
						else if (found2 == false)
						{
							found2 = true;
							edge[1] = vert1;
							vert2 = 3;//to end the loop
						}

					}
				}
			return true;
		}

		int NavMeshManager::funnelAlgorithm(Vector3 start, Vector3 dest, int * route, int* portalleft, 
			int*portalright,Array<Vector3, 1> *path)
		{

			Vector3 leftportal[60], rightportal[60];
			int nportals = 0;
			// Start portal
			rightportal[nportals] = leftportal[nportals] = start;
			nportals++;
			// Portal between navmesh polygons
			for (int i = 1; i < route[0]; ++i)
			{
				rightportal[i] = triangleList[route[i] - 1].vtx[portalright[i]];
				leftportal[i] = triangleList[route[i] - 1].vtx[portalleft[i]];
				nportals++;
			}
			// End portal
			rightportal[nportals] = leftportal[nportals] = dest;
			nportals++;
			
			Vector3 apex, left, right, nextleft, nextright;
			int apexindex = 0,rightindex = 0,leftindex =0 ;
			int npts = 0;
			left  = apex = leftportal[npts];
			right = rightportal[npts];

			(*path).add(start);
			npts++;
		
			for (int i = 1; i < nportals; i++)
			{
				nextleft  = leftportal[i];
				nextright = rightportal[i];

				if (triarea2(apex, nextleft, left) >= 0.0f)
				{
					if (isequal(left, apex) || triarea2(apex, right, nextleft) >= 0.0f)
					{

						left = nextleft;
						leftindex = i;
					
					}
					else
					{
						(*path).add(right);
						npts++;
						apex = right;
						apexindex = rightindex;
						left = apex;
						leftindex = apexindex;
						i = apexindex;
						continue;
					}

				}

				if (triarea2(apex, nextright, right) <= 0.0f)
				{
					if (isequal(right, apex) || triarea2(apex, nextright, left) >= 0.0f)
					{
						right = nextright;
						rightindex = i;						
					}
					else
					{
						(*path).add(left);
						npts++;
						apex = left;
						apexindex = leftindex;
						right = apex;
						rightindex = apexindex;
						i = apexindex;						
						continue;
					}

				}
	

			}

			(*path).add(leftportal[nportals - 1]);
			npts++;
			return npts;
		}

		void smoothpath(Array<Vector3, 1>*path)
		{
			int waypoints = path->m_size;
			float turningradius = 0.05f, Step = 0.3f, playerhalfsize = 0.2f;
			Vector3 Normal1,Normal2, p1to2, p2to3, Insert;
			Vector3 up(0, 1, 0);              //world up vector, in order to compute normal
			Vector3 originpath[60];
			originpath[0] = (*path)[0];
			int waypointnow = 1;
			for (int i = 1; i <= waypoints - 2; i++)
			{
				p1to2 = (*path)[i] - (*path)[i - 1];
				p2to3 = (*path)[i + 1] - (*path)[i];
				p1to2.normalize();
				p2to3.normalize();
				Normal1 = up.crossProduct(p1to2);
				Normal2 = up.crossProduct(p2to3);
				if (Normal1.dotProduct(p2to3) < 0)
				{
					Normal1 = -Normal1;
					Normal2 = -Normal2;
				}
				Normal1.normalize();
				Normal2.normalize();


				Insert = (*path)[i] - 3 * Step * p1to2 + (turningradius - playerhalfsize) * Normal1;
				originpath[waypointnow]= Insert;
				waypointnow++;
				Insert = (*path)[i] - 2 * Step * p1to2 + (turningradius * 2 - playerhalfsize) * Normal1;
				originpath[waypointnow] = Insert;
				waypointnow++;
				Insert =(*path)[i] - Step * p1to2 + (turningradius * 3 - playerhalfsize)* Normal1;
				originpath[waypointnow] = Insert;
				waypointnow++;


                originpath[waypointnow++] = (*path)[i] + (4 * turningradius -playerhalfsize) * Normal1;
				
				Insert = (*path)[i] + Step * p2to3 + (turningradius * 3 - playerhalfsize) * Normal2;
				originpath[waypointnow] = Insert;
				waypointnow++;
				Insert = (*path)[i] + 2 * Step * p2to3 + (turningradius * 2 - playerhalfsize)* Normal2;
				originpath[waypointnow] = Insert;
				waypointnow++;
				Insert = (*path)[i] + 3 * Step * p2to3 + (turningradius - playerhalfsize)* Normal2;
				originpath[waypointnow] = Insert;
				waypointnow++;

			}
			originpath[waypointnow++] = (*path)[waypoints - 1];
			path->reset(waypointnow);
			for (int i = 0; i <= waypointnow - 1; i++)
				(*path).add(originpath[i]);
		}

		int NavMeshManager::GetPath(Array<Vector3,1> *path, Vector3 start, Vector3 end)
		{

			int *route = new int[m_numTri + 1];
			int count = 1, startid, destinationid,nodenum;
			Vector3 localstart, localend;
			localstart = /*m_base **/ start;
			localend   = m_base * end;
			startid = FindNavmeshTri(localstart);
			destinationid = FindNavmeshTri(localend);

			int *portleft = new int[m_numTri + 1], *portright = new int[m_numTri + 1];

			if (!(startid == 0 || destinationid == 0))
			{
				Astar(startid, destinationid, route);
				findportallist(route, portleft, portright);
				nodenum = funnelAlgorithm(localstart, localend, route, portleft, portright, path);
#if USE_CURVED_SMOOTHING
				smoothpath(path);
				nodenum = path->m_size;
#endif
			}
			else
			{
				nodenum = 0;
			}
			Vector3 Pos;
			int num = 0;
			//path[num++] = start;
			for (; num </*(*path).m_size*/nodenum; num++)
			{
				//Pos = m_base.inverse()*(triangleList[route[num] - 1].pos);
				Pos = /*m_base.inverse()**/(*path)[num];
				(*path)[num] = Pos;

			}
			delete route;
			delete portleft;
			delete portright;
			return nodenum;
		}
	}; // namespace Components
}; // namespace PE

