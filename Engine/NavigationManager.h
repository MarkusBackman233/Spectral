#pragma once
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include "DetourCrowd.h"
#include "Vector3.h"

class GameObject;

class NavigationManager : public rcContext
{
public:
	static NavigationManager* GetInstance() {
		static NavigationManager instance;
		return &instance;
	}
	NavigationManager() {};

	void Update(float deltaTime);
	void DebugRender();
	bool GenerateNavMesh();

	int CreateAgent(GameObject* gameObject, const dtCrowdAgentParams& params);
	void RemoveAgent(int agentId);
	const dtCrowdAgent* GetAgentData(int agentId);
	bool RequestTarget(const Math::Vector3& target, int agentId);

	Math::Vector3 GetRandomPointInRadius(const Math::Vector3& center, float radius);

protected:
	virtual void doLog(const rcLogCategory category, const char* msg, const int len) override;

private:	

	void InitCrowd();


	rcPolyMesh* m_pmesh;
	rcPolyMeshDetail* m_dmesh;
	dtNavMesh* m_navMesh;
	dtNavMeshQuery* m_navQuery;
	dtCrowd* m_crowd;

	unsigned char m_navMeshDrawFlags;

	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	int m_partitionType;

	bool m_filterLowHangingObstacles;
	bool m_filterLedgeSpans;
	bool m_filterWalkableLowHeightSpans;

	unsigned char* m_triareas;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcConfig m_cfg;
};

