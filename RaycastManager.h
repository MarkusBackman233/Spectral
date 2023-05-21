#pragma once

#include "Vector3.h"
#include "Triangle.h"
#include "Mesh.h"
class RaycastManager
{

public:	
	struct RaycastHit {
		Math::Vector3 point;
		Math::Vector3 normal;
		float distance;
	};

	bool Raycast(Math::Vector3 rayOrigin,
				 Math::Vector3 rayDirection,
				 RaycastHit& hit,
				 Mesh* physicsMesh);



};

