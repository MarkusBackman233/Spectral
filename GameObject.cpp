#include "GameObject.h"
#include "PhysXManager.h"
GameObject::GameObject(Math::Vector3 position, Math::Vector3 rotation) 
	: m_position(Math::Vector3(0.0f,0.0f,0.0f))
	, m_rotation(Math::Vector3(0.0f, 0.0f, 0.0f))
	, m_isKinematic(false)
{
	m_position = position;
	m_rotation = rotation;

}

GameObject::GameObject(Math::Vector3 position) 
	: m_isKinematic(false)
{
	m_position = position;
}

GameObject::GameObject()
	: m_isKinematic(false)
{
}

GameObject::~GameObject()
{
}

void GameObject::SetTexture(std::string fileName)
{
	m_texture = TextureManager::GetInstance().GetTexture(fileName);
}

void GameObject::SetNormal(std::string fileName)
{
	m_normalMap = TextureManager::GetInstance().GetTexture(fileName);
}

void GameObject::SetMesh(std::string fileName)
{
	m_mesh = ModelManager::GetInstance().GetMesh(fileName);
	CalculateBoundingBox();
	Math::Matrix matRotZ, matRotX;
	matRotZ = matRotX.MakeRotationX(0.0f);
	matRotX = matRotX.MakeRotationY(0.0f);

	Math::Matrix translationMatrix;
	translationMatrix = translationMatrix.MakeTranslation(m_position);


	m_matrix = m_matrix.MakeIdentity();
	m_matrix = matRotZ * matRotX;
	m_matrix = m_matrix * translationMatrix;

	for (auto& triangle : m_mesh.triangles)
	{
		Triangle newTriangle = triangle;
		for (int j = 0; j < 3; j++)
		{
			newTriangle.m_vertex[j] = m_matrix * triangle.m_vertex[j];
		}

		m_triangles.push_back(newTriangle);
	}

}

void GameObject::SetName(std::string name)
{
	m_name = name;
}

void GameObject::Translate(Math::Vector3 position)
{
	m_position = position;

	auto pose = m_physicsDynamicObject->getGlobalPose();
	Math::Vector3 quat(-pose.q.x, -pose.q.y, -pose.q.z, 1.0f);

	quat.w = pose.q.w;

	float angle = 2 * acos(quat.w);
	Math::Vector3 axis = quat / sqrt(1 - quat.w * quat.w);

	Math::Matrix matrixRotation;
	matrixRotation = matrixRotation.MakeRotationAA(axis, angle);


	m_matrix = m_matrix.MakeIdentity();
	if(quat.x != 0 || quat.y != 0 || quat.z != 0)
	m_matrix = m_matrix * matrixRotation;
	m_matrix = m_matrix * m_matrix.MakeTranslation(m_position);

	int it = 0;
	for (auto& triangle : m_mesh.triangles)
	{
		for (int j = 0; j < 3; j++)
		{
			m_triangles[it].m_vertex[j] = m_matrix * triangle.m_vertex[j];
		}
		m_triangles[it].m_normalDirection = m_triangles[it].CalculateNormal();
		++it;
	}
}

void GameObject::Rotate(Math::Vector3 rotation)
{
	Math::Matrix matRotZ, matRotX;
	matRotZ = matRotX.MakeRotationX(rotation.x);
	matRotX = matRotX.MakeRotationY(rotation.y);

	m_matrix = m_matrix.MakeIdentity();
	m_matrix = matRotZ * matRotX;

	for (auto& triangle : m_mesh.triangles)
		for (int j = 0; j < 3; j++)
		{
			triangle.m_vertex[j] = m_matrix * triangle.m_vertex[j];
		}
}

void GameObject::UpdatePhysicsPosition()
{
	Math::Vector3 pos(Math::Vector3(m_physicsDynamicObject->getGlobalPose().p.x, m_physicsDynamicObject->getGlobalPose().p.y, m_physicsDynamicObject->getGlobalPose().p.z));

	Translate(pos);
}

void GameObject::CreateActor(physicsType type, bool isKinematic)
{
	PhysXManager& physXManager = PhysXManager::GetInstance();
	physx::PxTransform objectTransform(physx::PxVec3(m_position.x, m_position.y, m_position.z));

	m_isKinematic = isKinematic;



	if (type == physicsType::dynamicActor)
	{
		m_physicsDynamicObject = physXManager.GetPhysics()->createRigidDynamic(objectTransform);
		if (m_physicsDynamicObject)
		{
			m_physicsDynamicObject->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, isKinematic);


			physXManager.GetScene()->addActor(*m_physicsDynamicObject);
		}
	}
}

void GameObject::CreateShape(physicsShape shape)
{
	PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
	PhysXManager& physXManager = PhysXManager::GetInstance();
	physx::PxMaterial* defaultMaterial = physXManager.GetPhysics()->createMaterial(0.5f, 0.5f, 0.1f);

	if (shape == physicsShape::triangleMesh)
	{
		PxTriangleMeshGeometry triGeom;
		triGeom.triangleMesh = m_mesh.CreatePhysxTriangleMesh();
		PxShape* meshShape = physXManager.GetPhysics()->createShape(triGeom, &defaultMaterial, 1, true, shapeFlags);
		m_physicsDynamicObject->attachShape(*meshShape);
	}
	else if (shape == physicsShape::box)
	{
		//PxBoxGeometry boxGeometry(physx::PxVec3(1.0f, 1.0f, 1.0f));
		PxBoxGeometry boxGeometry(physx::PxVec3((m_maxBounds.x - m_minBounds.x) * 0.5f, (m_maxBounds.y - m_minBounds.y) * 0.5f, (m_maxBounds.z - m_minBounds.z) * 0.5f));

		PxShape* boxShape = physXManager.GetPhysics()->createShape(boxGeometry, *defaultMaterial, true, shapeFlags);
		m_physicsDynamicObject->attachShape(*boxShape);
	}	
	else if (shape == physicsShape::sphere)
	{
		PxSphereGeometry geometry(1.0f);
		PxShape* shape = physXManager.GetPhysics()->createShape(geometry, *defaultMaterial, true, shapeFlags);
		m_physicsDynamicObject->attachShape(*shape);
	}
}

void GameObject::CalculateBoundingBox()
{
	m_minBounds = Math::Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	m_maxBounds = Math::Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

	for (const auto& vertex : m_mesh.vertexes)
	{
		if (vertex.x < m_minBounds.x) m_minBounds.x = vertex.x;
		if (vertex.y < m_minBounds.y) m_minBounds.y = vertex.y;
		if (vertex.z < m_minBounds.z) m_minBounds.z = vertex.z;

		if (vertex.x > m_maxBounds.x) m_maxBounds.x = vertex.x;
		if (vertex.y > m_maxBounds.y) m_maxBounds.y = vertex.y;
		if (vertex.z > m_maxBounds.z) m_maxBounds.z = vertex.z;
	}
}


void GameObject::CreatePhysXMesh()
{

	PhysXManager& physXManager = PhysXManager::GetInstance();
	//
	//
	physx::PxMaterial* defaultMaterial = physXManager.GetPhysics()->createMaterial(0.5f, 0.5f, 0.1f);
	////plane rigid static
	PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
	//
	//PxTriangleMeshGeometry geom(m_mesh.m_physXMeshCollider);
	//
	physx::PxTransform boxTransform(physx::PxVec3(m_position.x, m_position.y, m_position.z));
	//
	//m_physicsRigidObject = physXManager.GetPhysics()->createRigidStatic(boxTransform);
	//{
	//	PxShape* shape = physXManager.GetPhysics()->createShape(geom, &defaultMaterial, 1, true, shapeFlags);
	//	m_physicsRigidObject->attachShape(*shape);
	//	shape->release(); // this way shape gets automatically released with actor
	//}
	//
	//physXManager.GetScene()->addActor(*m_physicsRigidObject);





	






	//PxTriangleMeshGeometry geom;
	//geom.triangleMesh = m_mesh.m_physXMeshCollider;
	//geom.scale = PxVec3(0.1f, 0.1f, 0.1f);
	//
	//
	//PxShape* shape = physXManager.GetPhysics()->createShape(geom, &defaultMaterial, 1, true, shapeFlags);
	////PxShape* shape = PxRigidActorExt::createExclusiveShape(*dyn, geom, *defaultMaterial);
	//shape->setContactOffset(0.1f);
	//shape->setRestOffset(0.02f);
	//m_physicsDynamicObject = physXManager.GetPhysics()->createRigidDynamic(boxTransform);
	//{
	//	PxShape* shape = physXManager.GetPhysics()->createShape(geom, &defaultMaterial, 1, true, shapeFlags);
	//	m_physicsDynamicObject->attachShape(*shape);
	//	shape->release(); // this way shape gets automatically released with actor
	//}
	//m_physicsDynamicObject->setLinearDamping(0.2f);
	//m_physicsDynamicObject->setAngularDamping(0.1f);
	//m_physicsDynamicObject->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	//PxReal density = 100.f;
	//PxRigidBodyExt::updateMassAndInertia(*m_physicsDynamicObject, density);
	//
	//physXManager.GetScene()->addActor(*m_physicsDynamicObject);
	//
	//m_physicsDynamicObject->setSolverIterationCounts(50, 1);
	//m_physicsDynamicObject->setMaxDepenetrationVelocity(5.f);






	m_physicsDynamicObject = physXManager.GetPhysics()->createRigidDynamic(boxTransform);
	if (m_physicsDynamicObject)
	{
		m_physicsDynamicObject->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

		PxTriangleMeshGeometry triGeom;
		triGeom.triangleMesh = m_mesh.m_physXMeshCollider;
		PxShape* meshShape = physXManager.GetPhysics()->createShape(triGeom, &defaultMaterial, 1, true, shapeFlags);
		m_physicsDynamicObject->attachShape(*meshShape);
		physXManager.GetScene()->addActor(*m_physicsDynamicObject);
	}

}
