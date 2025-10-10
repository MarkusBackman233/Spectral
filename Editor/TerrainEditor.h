#pragma once
#include "Vector3.h"
#include <optional>
#include <random>

class TerrainComponent;
class Mesh;
class GrassPatch;

class TerrainEditor
{
public:
	TerrainEditor(TerrainComponent* terrainComponent);


	bool Update(); // returns false if user wants to close terrain editor
	void RaycastPaint();

private:

	std::optional<Math::Vector3> RaycastTerrain(Mesh* terrainMesh);

	void EditSplat(Mesh* terrainMesh, const Math::Vector3& localHit, float direction);
	void EditHeight(Mesh* terrainMesh, const Math::Vector3& localHit, float direction);
	void EditGrass(Mesh* terrainMesh, const Math::Vector3& localHit, float direction);


	void AddGrass(const Math::Vector3& localHit);
	void RemoveGrass(const Math::Vector3& localHit);
	GrassPatch* GetGrassPatchFromWorldSpace(const Math::Vector3& pos);
	GrassPatch* GetGrassPatchFromCoords(int x, int z);
	std::vector<GrassPatch*> GetGrassPatchesInRadius(const Math::Vector3& center, float radius);

	TerrainComponent* m_terrainComponent;

	bool m_terrainDirty = false;

	enum BrushMode
	{
		Height,
		Color,
		Grass,

		Num,
	};



	struct TerrainEditorSettings
	{
		float BrushSize = 20.0f;
		float BrushStrength = 10.0f;
		float BrushGrassRadius = 2.0f;
		BrushMode BrushMode = BrushMode::Height;
		int SelectedBrushMaterial = 0;
	} m_settings;
	int m_grassPatchStride = -1;

	std::random_device m_randomDevice;
	std::mt19937 m_gen;
	std::uniform_real_distribution<float> m_randomAngleDist;


};

