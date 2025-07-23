#pragma once
#include <cstdint>
class TerrainComponent;

class TerrainEditor
{
public:
	TerrainEditor(TerrainComponent* terrainComponent);


	bool Update(); // returns false if user wants to close terrain editor
	void RaycastPaint();

private:
	TerrainComponent* m_terrainComponent;

	bool m_terrainDirty = false;

	enum BrushMode
	{
		Height,
		Color,

		Num,
	};



	struct TerrainEditorSettings
	{
		float BrushSize = 20.0f;
		float BrushStrength = 10.0f;
		BrushMode BrushMode = BrushMode::Height;
		int SelectedBrushMaterial = 0;
	} m_settings;

};

