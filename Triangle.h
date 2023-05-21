#pragma once
#include "Vector3.h"
#include "Vector2.h"
#include <cmath>
#include <stdint.h>

class Triangle
{
public:
	Triangle(void) {}
	Triangle(Math::Vector3& p1, Math::Vector3& p2, Math::Vector3& p3);


	Math::Vector3 CalculateNormal();

	int ClipAgainstPlane(Math::Vector3 planePosition, Math::Vector3 planeNormal, Triangle& out_tri1, Triangle& out_tri2);

	Math::Vector3 m_normalDirection;

	Math::Vector3 m_vertex[3];
	Math::Vector2 m_UV[3];



	void SetColor(uint32_t color){ m_color = color; }

	uint32_t GetColor(){ return m_color; }	


	float m_lightDot = 0.0f;

	Math::Vector3 m_diffuse;
	Math::Vector3 m_specular;


private:
	uint32_t m_color;
	

};
