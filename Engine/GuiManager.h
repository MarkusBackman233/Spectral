#pragma once
#include "pch.h"
#include "Vector2.h"
#include "Windows.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

class GuiManager
{
public:
	GuiManager();

	void CreateResources(ID3D11Device* device, HWND hwnd, ID3D11DeviceContext* context);


	void Text(const std::string& text, const Math::Vector2& position);
	void Render();

private:
	std::vector<std::pair<std::string, Math::Vector2>> m_textToDraw;
};

