#pragma once
#include <SDL.h>
#include <iostream>
#include <algorithm>
#include "Math.h"
#include <vector>
#include "Vector3.h"
#include "Mesh.h"
#include "Triangle.h"
#include "Matrix.h"
#include "Player.h"
#include "Vector2.h"
#include <string>
#include "GameObject.h"
#include "ObjectHandler.h"
#include "RaycastManager.h"
#include "PhysXManager.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
//#define SCREEN_WIDTH 1920
//#define SCREEN_HEIGHT 1080
//#define SCREEN_WIDTH 1280
//#define SCREEN_HEIGHT 720

class Spectral
{
public:
	struct Light
	{
		Math::Vector3 position;
		Math::Vector3 color;
	};

	Spectral(SDL_Renderer* renderer, SDL_Window* window, SDL_Event& event);
	~Spectral();
	void Render();
	void Update();
	Uint32 GetPixel(GameObject* gameObject, Triangle* triangle, const Triangle& originalTriangle, float u, float v, uint16_t width, uint16_t height);

	Player* GetPlayer() { return m_player; }

	void DrawLine(Math::Vector2 p0, Math::Vector2 p1, uint32_t color);
	void DrawCube( GameObject* gameObject);
	void DrawTriangle(const Triangle& triangle);


	void ProjectLine(const Math::Vector3& p0, const Math::Vector3& p1);


	void TexturedTriangle(GameObject* gameObject, Triangle* triangle, const Triangle& originalTriangle);
	int ComputeOutCode(Math::Vector2 point);
	int ComputeOutCode(Math::Vector3 point);

	bool CohenSutherlandLineClip(Math::Vector2& p0, Math::Vector2& p1);

    uint32_t DarkenColor(uint32_t color, float darknessLevel);

	uint32_t m_albedoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	uint32_t m_normalBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

	uint32_t m_positionBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	uint32_t m_compositeImage[SCREEN_WIDTH * SCREEN_HEIGHT];

	SDL_Renderer* m_renderer;
	SDL_Window* m_window;

	Math::Matrix m_worldMatrix;

	int currentPixel;

	void SetWindowAndRenderer(SDL_Window* window, SDL_Renderer* renderer)
	{
		m_window = window;
		m_renderer = renderer;
	}

	float* m_depthBuffer = nullptr;

private:

	std::vector<Light> m_ligts;

	bool m_hasClicked;

	float fTheta;

    Player* m_player;

	Math::Matrix m_projectionMatrix;
	Math::Matrix m_invProjectionMatrix;

	ObjectHandler* m_ObjectHandler;

	Math::Vector3 m_directionLighting;
	const Math::Vector3 m_viewOffset;
	std::vector<uint32_t> m_colorsInLine;

	bool m_render = true;
	bool m_renderPhysX = false;
	bool m_renderWireframe = false;



	const int xmin = 0;  
	const int ymin = 0;
	const int xmax = SCREEN_WIDTH;
	const int ymax = SCREEN_HEIGHT;


	const int INSIDE = 0; // 0000
	const int LEFT = 1;   // 0001
	const int RIGHT = 2;  // 0010
	const int BOTTOM = 4; // 0100
	const int TOP = 8;    // 1000
	int clip3(const Math::Vector3 n, Math::Vector3& v0, Math::Vector3& v1, Math::Vector3& v2, Math::Vector3& v3);

	float lerp(float a, float b, float f);
};

