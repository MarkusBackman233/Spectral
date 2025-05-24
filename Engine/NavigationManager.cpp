#include "NavigationManager.h"
#include "Mesh.h"
#include <DetourNavMeshBuilder.h>

#include "ObjectManager.h"
#include "MeshComponent.h"
#include "GameObject.h"
#include "PhysicsShapeComponent.h"
#include "RigidbodyComponent.h"
#include "iRender.h"
#include <random>

void NavigationManager::Update(float deltaTime)
{
	m_crowd->update(deltaTime, nullptr);
}

void NavigationManager::DebugRender()
{
	{
		const auto navMesh = NavigationManager::GetInstance()->m_navMesh;
		for (int i = 0; i < navMesh->getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = navMesh->getTileG(i);

			const dtPoly* polys = tile->polys;
			const float* verts = tile->verts;
			const dtPolyDetail* detailMeshes = tile->detailMeshes;
			const float* detailVerts = tile->detailVerts;

			for (int j = 0; j < tile->header->polyCount; ++j)
			{
				const dtPoly& p = polys[j];
				if (p.getType() != DT_POLYTYPE_GROUND) continue; // skip off-mesh connections

				const dtPolyDetail& pd = detailMeshes[j];

				for (int k = 0; k < pd.triCount; ++k)
				{
					const unsigned char* t = &tile->detailTris[(pd.triBase + k) * 4];

					Math::Vector3 tri[3];
					for (int m = 0; m < 3; ++m)
					{
						const int vertIndex = t[m];
						const float* v;

						if (vertIndex < pd.vertCount)
							v = &detailVerts[(pd.vertBase + vertIndex) * 3];
						else
							v = &verts[p.verts[vertIndex - pd.vertCount] * 3];

						tri[m] = Math::Vector3(v[0], v[1], v[2]);
					}

					// Draw triangle as lines
					Render::DrawLine(tri[0], tri[1]);
					Render::DrawLine(tri[1], tri[2]);
					Render::DrawLine(tri[2], tri[0]);
				}
			}
		}

	}
}

bool NavigationManager::GenerateNavMesh()
{

	Math::Vector3 min(-100.0f, -100.0f, -100.0f);
	Math::Vector3 max(100.0f, 100.0f, 100.0f);

	std::vector<Math::Vector3> vertexes;
	std::vector<int> triangles;
	

	auto shapes = ObjectManager::GetInstance()->GetComponentsOfType(Component::Type::PhysicsShape);
	for (auto& object : shapes)
	{
		if (auto meshComp = std::dynamic_pointer_cast<PhysicsShapeComponent>(object.lock()))
		{
			auto gameObject = meshComp->GetOwner();

			GameObject* actorGameObject = meshComp->GetClosestActorInHierarchy(gameObject);
			if (actorGameObject)
			{
				if (!actorGameObject->GetComponentOfType<RigidbodyComponent>()->GetPhysicsType() == PhysXManager::PhysicsType::StaticActor)
				{
					continue;
				}
			}

			switch (meshComp->GetShapeType())
			{
			case PhysXManager::PhysicsShape::Box:
			{
				Math::Vector3 boundingBox(1.0f,1.0f,1.0f);
				if (auto meshComp = gameObject->GetComponentOfType<MeshComponent>())
				{
					boundingBox = meshComp->GetMesh()->GetBoundingBoxMax() * 0.5f - meshComp->GetMesh()->GetBoundingBoxMin() * 0.5f;
				}

				Math::Vector3 center = gameObject->GetWorldMatrix().GetPosition();


				std::vector<Math::Vector3> boxVerts = {
					Math::Vector3(-boundingBox.x, -boundingBox.y, -boundingBox.z),
					Math::Vector3(boundingBox.x, -boundingBox.y, -boundingBox.z),
					Math::Vector3(boundingBox.x, -boundingBox.y,  boundingBox.z),
					Math::Vector3(-boundingBox.x, -boundingBox.y,  boundingBox.z),
					Math::Vector3(-boundingBox.x,  boundingBox.y, -boundingBox.z),
					Math::Vector3(boundingBox.x,  boundingBox.y, -boundingBox.z),
					Math::Vector3(boundingBox.x,  boundingBox.y,  boundingBox.z),
					Math::Vector3(-boundingBox.x,  boundingBox.y,  boundingBox.z),
				};

				for (auto& vert : boxVerts)
				{
					vert = vert.Transform(gameObject->GetWorldMatrix());
				}

				int startIndex = static_cast<int>(vertexes.size());
				vertexes.insert(vertexes.end(), boxVerts.begin(), boxVerts.end());

				std::vector<int> boxTris = {
					0, 2, 1, 0, 3, 2,
					4, 5, 6, 4, 6, 7,
					0, 1, 5, 0, 5, 4,
					2, 3, 7, 2, 7, 6,
					1, 2, 6, 1, 6, 5,
					3, 0, 4, 3, 4, 7
				};

				for (int idx : boxTris)
				{
					triangles.push_back(startIndex + idx);
				}
				break;
			}
			default:
				break;
			}


		}
	}
	/*
	auto meshes = ObjectManager::GetInstance()->GetComponentsOfType(Component::Type::Mesh);

	for (auto& object : meshes)
	{
		if (auto meshComp = std::dynamic_pointer_cast<MeshComponent>(object.lock()))
		{
			if (!meshComp->GetMesh())
			{
				continue;
			}

			auto mesh = meshComp->GetMesh();
			int triOffset = vertexes.size();

			for (auto& vertex : mesh->vertexes)
			{
				vertexes.push_back(vertex.position.Transform(meshComp->GetOwner()->GetWorldMatrix()));
			}
			for (auto& index : mesh->indices32)
			{
				triangles.push_back(index + triOffset);
			}
		}
	}
	*/

	const float* bmin = min.Data();
	const float* bmax = max.Data();
	const float* verts = vertexes.data()->Data();
	const int nverts = static_cast<int>(vertexes.size());
	const int* tris = triangles.data();
	const int ntris = static_cast<int>(triangles.size()) / 3;

	//
	// Step 1. Initialize build config.
	//

	// Init build configuration from GUI

	m_cellSize = 0.3f;
	m_cellHeight = 0.2f;
	m_agentHeight = 2.0f;
	m_agentRadius = 1.0f;
	m_agentMaxClimb = 0.9f;
	m_agentMaxSlope = 45.0f;
	m_regionMinSize = 8;
	m_regionMergeSize = 20;
	m_edgeMaxLen = 12.0f;
	m_edgeMaxError = 1.3f;
	m_vertsPerPoly = 6.0f;
	m_detailSampleDist = 6.0f;
	m_detailSampleMaxError = 1.0f;
	//m_partitionType = SAMPLE_PARTITION_WATERSHED;

	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = m_cellSize;
	m_cfg.ch = m_cellHeight;
	m_cfg.walkableSlopeAngle = m_agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(m_agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(m_agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(m_agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
	m_cfg.maxSimplificationError = m_edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
	m_cfg.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	m_cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;

	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	// Reset build times gathering.
	resetTimers();

	// Start the build process.	
	startTimer(RC_TIMER_TOTAL);

	log(RC_LOG_PROGRESS, "Building navigation:");
	log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
	log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

	//
	// Step 2. Rasterize input polygon soup.
	//

	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return false;
	}
	if (!rcCreateHeightfield(this, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return false;
	}

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triareas = new unsigned char[ntris];
	if (!m_triareas)
	{
		log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", ntris);
		return false;
	}

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, 0, ntris * sizeof(unsigned char));

	for (size_t i = 0; i < ntris; i++)
	{
		m_triareas[i] = 1;
	}

	rcMarkWalkableTriangles(this, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	if (!rcRasterizeTriangles(this, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
	{
		log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
		return false;
	}

	//if (!m_keepInterResults)
	{
		delete[] m_triareas;
		m_triareas = 0;
	}

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	m_filterLowHangingObstacles = true;
	m_filterLedgeSpans = true;
	m_filterWalkableLowHeightSpans = true;
	if (m_filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(this, m_cfg.walkableClimb, *m_solid);
	if (m_filterLedgeSpans)
		rcFilterLedgeSpans(this, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	if (m_filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(this, m_cfg.walkableHeight, *m_solid);


	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return false;
	}
	if (!rcBuildCompactHeightfield(this, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return false;
	}

	//if (!m_keepInterResults)
	{
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(this, m_cfg.walkableRadius, *m_chf))
	{
		log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return false;
	}

	// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
	//	rcMarkConvexPolyArea(this, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);


	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles

	//if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(this, *m_chf))
		{
			log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return false;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(this, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
			return false;
		}
	}
	//else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
	//{
	//	// Partition the walkable surface into simple regions without holes.
	//	// Monotone partitioning does not need distancefield.
	//	if (!rcBuildRegionsMonotone(this, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
	//	{
	//		log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
	//		return false;
	//	}
	//}
	//else // SAMPLE_PARTITION_LAYERS
	//{
	//	// Partition the walkable surface into simple regions without holes.
	//	if (!rcBuildLayerRegions(this, *m_chf, 0, m_cfg.minRegionArea))
	//	{
	//		log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
	//		return false;
	//	}
	//}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return false;
	}
	if (!rcBuildContours(this, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return false;
	}

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return false;
	}
	if (!rcBuildPolyMesh(this, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return false;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
		return false;
	}

	if (!rcBuildPolyMeshDetail(this, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return false;
	}

	//if (!m_keepInterResults)
	{
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
	}

	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//
	
	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		for (int i = 0; i < m_pmesh->npolys; ++i)
		{
			m_pmesh->areas[i] = 1;
			m_pmesh->flags[i] = 1;
			//if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
			//	m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;
			//
			//if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
			//	m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
			//	m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
			//{
			//	m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
			//}
			//else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
			//{
			//	m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
			//}
			//else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
			//{
			//	m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			//}
		}


		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		//params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		//params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		//params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		//params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		//params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		//params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		//params.offMeshConCount = m_geom->getOffMeshConnectionCount();
		params.walkableHeight = m_agentHeight;
		params.walkableRadius = m_agentRadius;
		params.walkableClimb = m_agentMaxClimb;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return false;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			dtFree(navData);
			log(RC_LOG_ERROR, "Could not create Detour navmesh");
			return false;
		}

		dtStatus status;

		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			log(RC_LOG_ERROR, "Could not init Detour navmesh");
			return false;
		}
		m_navQuery = dtAllocNavMeshQuery();
		if (!m_navQuery)
		{
			dtFree(navData);
			log(RC_LOG_ERROR, "Could not create Detour m_navQuery");
			return false;
		}

		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			log(RC_LOG_ERROR, "Could not init Detour navmesh query");
			return false;
		}
	}
	
	stopTimer(RC_TIMER_TOTAL);
	log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);
	InitCrowd();
	return true;
}

int NavigationManager::CreateAgent(GameObject* gameObject, const dtCrowdAgentParams& params)
{
	Math::Vector3 currentPosition = gameObject->GetWorldMatrix().GetPosition();
	return m_crowd->addAgent(currentPosition.Data(), &params);
}

void NavigationManager::RemoveAgent(int agentId)
{
	NavigationManager::GetInstance()->m_crowd->removeAgent(agentId);
}

const dtCrowdAgent* NavigationManager::GetAgentData(int agentId)
{
	return m_crowd->getAgent(agentId);
}

bool NavigationManager::RequestTarget(const Math::Vector3& target, int agentId)
{
	dtPolyRef startRef;
	dtQueryFilter filter;
	filter.setIncludeFlags(0xFFFF);  // Adjust according to your walkability flags
	filter.setExcludeFlags(0);       // Exclude nothing

	float halfExtents[3] = { 4.0f, 4.0f, 4.0f };

	m_navQuery->findNearestPoly(target.Data(), halfExtents, &filter, &startRef, nullptr);
	return m_crowd->requestMoveTarget(agentId, startRef, target.Data());
}

float generateRandomFloat() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	return dis(gen);
}

Math::Vector3 NavigationManager::GetRandomPointInRadius(const Math::Vector3& center, float radius)
{

	static std::random_device rd;                      // Uses hardware entropy source if available
	static std::mt19937 gen(rd());                     // Mersenne Twister engine seeded with rd
	static std::uniform_real_distribution<float> dis(0.0, 1.0); // Uniform distribution from 0 to 1


	dtPolyRef startRef;
	dtQueryFilter filter;
	filter.setIncludeFlags(0xFFFF);  // Adjust according to your walkability flags
	filter.setExcludeFlags(0);       // Exclude nothing

	float halfExtents[3] = { radius, radius, radius };

	m_navQuery->findNearestPoly(center.Data(), halfExtents, &filter, &startRef, nullptr);
	float n = dis(gen);
	dtPolyRef randomRef;

	float t[3]{};

	m_navQuery->findRandomPointAroundCircle(startRef,center.Data(),radius, &filter, generateRandomFloat, &randomRef,t);

	return { t[0], t[1], t[2] };
}

void NavigationManager::InitCrowd()
{
	m_crowd = dtAllocCrowd();
	m_crowd->init(128,5.0f, m_navMesh);
}

void NavigationManager::doLog(const rcLogCategory category, const char* msg, const int len)
{
	switch (category)
	{
	case RC_LOG_PROGRESS:
		Logger::Info(msg);
		break;	
	case RC_LOG_WARNING:
		Logger::Info(msg);
		break;
	case RC_LOG_ERROR:
		Logger::Error(msg);
		break;
	default:
		break;
	}
}
