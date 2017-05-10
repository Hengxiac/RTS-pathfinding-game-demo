#ifndef __PYENGINE_2_0_RAY_H__
#define __PYENGINE_2_0_RAY_H__


// APIAbstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
// Windows
#if APIABSTRACTION_D3D9 | APIABSTRACTION_D3D11
#define _USE_MATH_DEFINES
#include <math.h>
#endif

// Inter-Engine includes

#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/Math/Vector3.h"

// Sibling/Children includes

class Ray {
public:
	Ray() { }
	Ray(Vector3 o, Vector3 d) {
		origin = o;
		direction = d;
		inv_direction = Vector3(1 / d.m_x, 1 / d.m_y, 1 / d.m_z);
		sign[0] = (inv_direction.m_x < 0);
		sign[1] = (inv_direction.m_y < 0);
		sign[2] = (inv_direction.m_z < 0);
	}
	Ray(const Ray &r) {
		origin = r.origin;
		direction = r.direction;
		inv_direction = r.inv_direction;
		sign[0] = r.sign[0]; sign[1] = r.sign[1]; sign[2] = r.sign[2];
	}

	Vector3 getPosForY0()
	{
		float length = origin.m_y / direction.m_y;
		Vector3 result = { origin.m_x - direction.m_x * length, origin.m_y - direction.m_y * length,
			origin.m_z - direction.m_z * length };

		return result;
	}

	Vector3 origin;
	Vector3 direction;
	Vector3 inv_direction;
	int sign[3];
};


#endif
