#pragma once
#include "pch.h"
class DefaultAssets
{
public:
	static void Load();



private: 
	static void LoadBitMap(const std::string& filename, int resourceId);
	static void LoadPNG(const std::string& filename, int resourceId);
	static void LoadCubeMesh();
	static void LoadPlaneMesh();
	static void LoadSphereMesh();
};

