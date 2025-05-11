#pragma once
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include "DetourCrowd.h"

class NavigationManager : public rcContext
{
public:
	static NavigationManager* GetInstance() {
		static NavigationManager instance;
		return &instance;
	}
	NavigationManager() {};


	bool GenerateNavMesh();
	void InitCrowd();
	rcPolyMesh* m_pmesh;
	rcPolyMeshDetail* m_dmesh;

	dtNavMesh* m_navMesh;
	dtNavMeshQuery* m_navQuery;

	dtCrowd* m_crowd;

protected:
	virtual void doLog(const rcLogCategory category, const char* msg, const int len) override;

private:	
	//class InputGeom* m_geom;


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

