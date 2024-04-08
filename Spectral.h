#pragma once
#include "pch.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Matrix.h"
#include "Player.h"
#include <list>
#include <vector>
#include <deque>
#include "LightComponent.h"
#include "unordered_map"

class LevelProperties;
class GameObject;
class ObjectManager;
class Editor;
class Triangle;
class Texture;
class ParticleSimulation;

class Spectral
{
public:
	Spectral();
	~Spectral();

	static Spectral* GetInstance() {
		static Spectral instance;
		return &instance;
	}


	void Render();
	void Update();

	void SetAmbientLight(const ColorFLOAT& color);
	void SetFogColor(const ColorFLOAT& color);
	ColorFLOAT& GetAmbientLight() { return m_ambientLighting; }
	ColorFLOAT& GetFogColor() { return m_fogColor; }

	Math::Matrix& GetProjectionMatrix() { return m_projectionMatrix; }
	const Math::Matrix& GetViewMatrix() { return m_player->GetCameraMatrixInversed(); }
	Math::Vector2i&	GetScreenSize()	{ return m_screenSize; }
	float GetDeltaTime() const { return m_deltaTime; }

	bool m_render;
	bool m_renderPhysX;
	bool m_renderWireframe;

private:
	struct LightBufferLocations
	{
		int enabledLoc;
		int typeLoc;
		int positionLoc;
		int targetLoc;
		int colorLoc;
	};

	void CalculateProjectionMatrix(int width, int height);
	float CalculateDeltaTime();

	void RenderPhysX();

	float m_deltaTime;

	Math::Vector2i m_screenSize;
	Math::Vector2 m_halfScreenSize;


	uint32_t* m_albedoBuffer;
	float* m_depthBuffer;

    Player* m_player;

	Math::Matrix m_projectionMatrix;
	ObjectManager* m_ObjectManager;
	Math::Vector3 m_viewOffset;

	GameObject* m_skyBox;

	LevelProperties* m_levelProperties;

	Math::Vector3 m_directionLighting;


	ColorFLOAT m_ambientLighting;
	ColorFLOAT m_fogColor;

	std::unordered_map<int,LightBufferLocations> m_lightBuffers;
};

