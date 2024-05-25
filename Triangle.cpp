#include "Triangle.h"


Triangle::Triangle(const Math::Vector3& p1, const  Math::Vector3& p2, const  Math::Vector3& p3)
{
	m_vertex[0] = p1;
	m_vertex[1] = p2;
	m_vertex[2] = p3;
}


Math::Vector3 Triangle::CalculateNormal() const
{
    const Math::Vector3 U = m_vertex[1] - m_vertex[0];
    const Math::Vector3 V = m_vertex[2] - m_vertex[0];

    Math::Vector3 normal(
        U.y * V.z - U.z * V.y,
        U.z * V.x - U.x * V.z,
        U.x * V.y - U.y * V.x
    );

    return normal.GetNormal().GetNegative();
}

int Triangle::ClipAgainstPlane(const Math::Vector3& planePosition, const Math::Vector3& planeNormal, Triangle& out_tri1, Triangle& out_tri2) const
{
    Math::Vector3 insidePoints[3];
    Math::Vector3 outsidePoints[3];
    int insidePointCount = 0;
    int outsidePointCount = 0;    

    Math::Vector2 UVInsidePoints[3];
    Math::Vector2 UVOutsidePoints[3];
    int UVInsidePointCount = 0;
    int UVOutsidePointCount = 0;

    for(int i = 0; i < 3; i ++)
    { 
        const float distance = planeNormal.Dot(m_vertex[i] - planePosition);

        if(distance >= 0)
        {
            insidePoints[insidePointCount++] = m_vertex[i];
            UVInsidePoints[UVInsidePointCount++] = m_UV[i];
        }
        else
        {
            outsidePoints[outsidePointCount++] = m_vertex[i];
            UVOutsidePoints[UVOutsidePointCount++] = m_UV[i];
        }
    }

    if (insidePointCount == 0)
    {
        return 0;
    }    
    else if (insidePointCount == 3)
    {
        out_tri1 = *this;
        return 1;
    }

    if (insidePointCount == 1 && outsidePointCount == 2)
    {
        out_tri1.m_vertex[0] = insidePoints[0];
        out_tri1.m_UV[0] = UVInsidePoints[0];

        float t;

        out_tri1.m_vertex[1] = planePosition.IntersectPlane( planeNormal, insidePoints[0], outsidePoints[0], t);
        out_tri1.m_UV[1].x = t * (UVOutsidePoints[0].x - UVInsidePoints[0].x) + UVInsidePoints[0].x;
        out_tri1.m_UV[1].y = t * (UVOutsidePoints[0].y - UVInsidePoints[0].y) + UVInsidePoints[0].y;
        out_tri1.m_UV[1].w = t * (UVOutsidePoints[0].w - UVInsidePoints[0].w) + UVInsidePoints[0].w;

        out_tri1.m_vertex[2] = planePosition.IntersectPlane( planeNormal, insidePoints[0], outsidePoints[1], t);
        out_tri1.m_UV[2].x = t * (UVOutsidePoints[1].x - UVInsidePoints[0].x) + UVInsidePoints[0].x;
        out_tri1.m_UV[2].y = t * (UVOutsidePoints[1].y - UVInsidePoints[0].y) + UVInsidePoints[0].y;
        out_tri1.m_UV[2].w = t * (UVOutsidePoints[1].w - UVInsidePoints[0].w) + UVInsidePoints[0].w;
        out_tri1.m_normalDirection = m_normalDirection;
        return 1;
    }    
    else if (insidePointCount == 2 && outsidePointCount == 1)
    {
        out_tri1.m_vertex[0] = insidePoints[0];
        out_tri1.m_vertex[1] = insidePoints[1];
        out_tri1.m_UV[0] = UVInsidePoints[0];
        out_tri1.m_UV[1] = UVInsidePoints[1];

        float t;

        out_tri1.m_vertex[2] = planePosition.IntersectPlane( planeNormal, insidePoints[0], outsidePoints[0], t);   
        out_tri1.m_UV[2].x = t * (UVOutsidePoints[0].x - UVInsidePoints[0].x) + UVInsidePoints[0].x;
        out_tri1.m_UV[2].y = t * (UVOutsidePoints[0].y - UVInsidePoints[0].y) + UVInsidePoints[0].y;
        out_tri1.m_UV[2].w = t * (UVOutsidePoints[0].w - UVInsidePoints[0].w) + UVInsidePoints[0].w;
        out_tri1.m_normalDirection = m_normalDirection;

        out_tri2.m_vertex[0] = insidePoints[1];
        out_tri2.m_UV[0] = UVInsidePoints[1];

        out_tri2.m_vertex[1] = out_tri1.m_vertex[2];
        out_tri2.m_UV[1] = out_tri1.m_UV[2];
        out_tri2.m_normalDirection = m_normalDirection;

        out_tri2.m_vertex[2] = planePosition.IntersectPlane( planeNormal, insidePoints[1], outsidePoints[0], t);
        out_tri2.m_UV[2].x = t * (UVOutsidePoints[0].x - UVInsidePoints[1].x) + UVInsidePoints[1].x;
        out_tri2.m_UV[2].y = t * (UVOutsidePoints[0].y - UVInsidePoints[1].y) + UVInsidePoints[1].y;
        out_tri2.m_UV[2].w = t * (UVOutsidePoints[0].w - UVInsidePoints[1].w) + UVInsidePoints[1].w;

        return 2;
    }
    return 0;
}
