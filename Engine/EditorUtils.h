#pragma once
#include "pch.h"
namespace Math
{
	class Matrix;
	class Vector3;
};

class GameObject;

class EditorUtils
{
public:
	static void DuplicateGameObject(GameObject* duplicate, GameObject* source);

	static void DecomposeMatrix(const Math::Matrix& matrix, Math::Vector3& position, Math::Vector3& rotation, Math::Vector3& scale);
	static void RecomposeMatrix(Math::Matrix& matrix, const Math::Vector3& position, const Math::Vector3& rotation, const Math::Vector3& scale);

	static bool CursorToWorldDirection(Math::Vector3& origin, Math::Vector3& direction);

	static bool IsCursorInViewport();
};