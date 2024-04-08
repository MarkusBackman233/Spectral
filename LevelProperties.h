#pragma once
#include "pch.h"
class LevelProperties
{
public:
	float fogThreshold;
	float decayRate;
	float ambientLighting;
	float fogDistance;

	LevelProperties() {
		fogThreshold = 1.0f;
		decayRate = 0.5f;
		ambientLighting = 0.155f;
		fogDistance = 30.0;
	};
};