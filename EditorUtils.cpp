#include "EditorUtils.h"
#include "Matrix.h"
#include "GameObject.h"
#include "ComponentFactory.h"
#include "ObjectManager.h"
#include "src/IMGUI/ImGuizmo.h"

void EditorUtils::MatrixToFloatMatrix(const Math::Matrix& matrix, float* floatMatrix)
{
    floatMatrix[0] = matrix.m_matrix[0][0];
    floatMatrix[1] = matrix.m_matrix[0][1];
    floatMatrix[2] = matrix.m_matrix[0][2];
    floatMatrix[3] = matrix.m_matrix[0][3];

    floatMatrix[4] = matrix.m_matrix[1][0];
    floatMatrix[5] = matrix.m_matrix[1][1];
    floatMatrix[6] = matrix.m_matrix[1][2];
    floatMatrix[7] = matrix.m_matrix[1][3];

    floatMatrix[8] = matrix.m_matrix[2][0];
    floatMatrix[9] = matrix.m_matrix[2][1];
    floatMatrix[10] = matrix.m_matrix[2][2];
    floatMatrix[11] = matrix.m_matrix[2][3];

    floatMatrix[12] = matrix.m_matrix[3][0];
    floatMatrix[13] = matrix.m_matrix[3][1];
    floatMatrix[14] = matrix.m_matrix[3][2];
    floatMatrix[15] = matrix.m_matrix[3][3];
}

void EditorUtils::FloatMatrixToMatrix(const float* floatMatrix, Math::Matrix& matrix)
{
    matrix.m_matrix[0][0] = floatMatrix[0];
    matrix.m_matrix[0][1] = floatMatrix[1];
    matrix.m_matrix[0][2] = floatMatrix[2];
    matrix.m_matrix[0][3] = floatMatrix[3];

    matrix.m_matrix[1][0] = floatMatrix[4];
    matrix.m_matrix[1][1] = floatMatrix[5];
    matrix.m_matrix[1][2] = floatMatrix[6];
    matrix.m_matrix[1][3] = floatMatrix[7];

    matrix.m_matrix[2][0] = floatMatrix[8];
    matrix.m_matrix[2][1] = floatMatrix[9];
    matrix.m_matrix[2][2] = floatMatrix[10];
    matrix.m_matrix[2][3] = floatMatrix[11];

    matrix.m_matrix[3][0] = floatMatrix[12];
    matrix.m_matrix[3][1] = floatMatrix[13];
    matrix.m_matrix[3][2] = floatMatrix[14];
    matrix.m_matrix[3][3] = floatMatrix[15];
}

void EditorUtils::DuplicateGameObject(GameObject* duplicate, GameObject* source)
{
    for (auto* child : source->GetChildren())
    {
        GameObject* duplicateGameObject = ObjectManager::GetInstance()->CreateObject(source->GetName());
        duplicateGameObject->SetParent(duplicate);
        DuplicateGameObject(duplicateGameObject, child);
    }
    for (const auto& component : source->GetComponents())
    {
        duplicate->AddComponent(ComponentFactory::CreateComponent(duplicate, component->GetComponentType(), component));
    }
    duplicate->GetMatrix() = source->GetMatrix();
    duplicate->GetLocalMatrix() = source->GetLocalMatrix();
}

void EditorUtils::DecomposeMatrix(const Math::Matrix& matrix, Math::Vector3& position, Math::Vector3& rotation, Math::Vector3& scale)
{
    float floatMatrix[16]{};

    EditorUtils::MatrixToFloatMatrix(matrix, floatMatrix);
    ImGuizmo::DecomposeMatrixToComponents(floatMatrix, (float*)&position, (float*)&rotation, (float*)&scale);
}

void EditorUtils::RecomposeMatrix(Math::Matrix& matrix, const Math::Vector3& position, const Math::Vector3& rotation, const Math::Vector3& scale)
{
    float floatMatrix[16]{};
    MatrixToFloatMatrix(matrix, floatMatrix);
    ImGuizmo::RecomposeMatrixFromComponents((float*)&position, (float*)&rotation, (float*)&scale, floatMatrix);
    FloatMatrixToMatrix(floatMatrix, matrix);
}
