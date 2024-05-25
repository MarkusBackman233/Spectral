#pragma once
#include "utility"
#include "pch.h"
class DefaultAssets
{
public:
	static void LoadDefaults();

private: 
	static void LoadBitMap(const std::string& filename, int resourceId);
};

