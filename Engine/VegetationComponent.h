#pragma once
#include "Component.h"
#include "Vector3.h"
#include "Matrix.h"

#include "BillboardRenderer.h"

class Mesh;
class Model;
class DefaultMaterial;



struct Spline
{
	std::vector<Math::Vector3> m_points;

	Math::Vector3 GetPositionAtFraction(float fraction) const;
	Math::Vector3 GetTangentAtFraction(float fraction) const;
private:

	Math::Vector3 CatmullRom(const Math::Vector3& p0, const Math::Vector3& p1, const Math::Vector3& p2, const Math::Vector3& p3, float t) const;

	Math::Vector3 CatmullRomTangent(const Math::Vector3& p0, const Math::Vector3& p1, const Math::Vector3& p2, const Math::Vector3& p3, float t) const;

};

class VegetationComponent :
	public Component
{
public:


	VegetationComponent(GameObject* owner);
	VegetationComponent(GameObject* owner, VegetationComponent* vegetationComponent);
	Component::Type GetComponentType() override { return Component::Type::Vegetation; };
	~VegetationComponent();
	void Start() override;
	//void Reset() override;
	//void Update(float deltaTime) override;

	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;
#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	void Render() override;

private:

	void Generate();

	void AddSplineToMesh(const Spline& spline, const Math::Matrix& localPose, float startWidth, float endWidth, int segments, int columns);

	std::shared_ptr<Model> m_model;


	Math::Vector3 CatmullRom(const Math::Vector3& p0, const Math::Vector3& p1, const Math::Vector3& p2, const Math::Vector3& p3, float t);

	void DrawSpline(const Spline& spline, const Math::Matrix& matrix, float maxWidth, float minWidth);

	void AddBranch(const Spline& parentSpline, float startWidth, std::vector<BillboardVertex>& leafs, bool flipped, float heigthFraction);

	struct TreeParams
	{
		float TrunkBottomWidth = 0.3f;
		float TrunkTopWidth = 0.01f;
		float TrunkShapeRandomness = 0.3f;

		int NbBranches = 30;
		int PolyCount = 12;
		int Segments = 12;

		float BranchStartWidth = 0.05f;
		float BranchWidthFalloff = 0.5f;
		float BranchRandomDirection = 0.5f;

		float branchLength = 0.5f;

		float branchUppWeight = 0.2f;

		float leafSize = 1.0f;
	} m_params;
	InstanceManager::InstanceData m_instData;
};

