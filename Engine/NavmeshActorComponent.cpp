#include "NavmeshActorComponent.h"
#include "GameObject.h"
#include "Json.h"
#include "NavigationManager.h"
#include "src/IMGUI/imgui.h"

NavmeshActorComponent::NavmeshActorComponent(GameObject* owner)
	: Component(owner)
    , m_agentId(-1)
    , m_hasTarget(false)
    , m_agentParams{ 1.0f, 2.0f, 60.0f, 4.0f, 10.0f, 1.0f, 1.0f, DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OBSTACLE_AVOIDANCE | DT_CROWD_SEPARATION ,0 ,0 ,0 }
{

}

NavmeshActorComponent::NavmeshActorComponent(GameObject* owner, NavmeshActorComponent* characterControllerComponent)
	: Component(owner)
    , m_agentId(-1)
    , m_hasTarget(false)
    , m_agentParams(characterControllerComponent->GetAgentParams())
{
}

NavmeshActorComponent::~NavmeshActorComponent()
{
    if (m_agentId != -1)
    {
        NavigationManager::GetInstance()->RemoveAgent(m_agentId);
    }
}

void NavmeshActorComponent::Start()
{
    if (m_agentId == -1)
    {
        CreateAgent();
    }
}

void NavmeshActorComponent::Reset()
{
    m_hasTarget = false;
    if (m_agentId != -1)
    {
        NavigationManager::GetInstance()->RemoveAgent(m_agentId);
    }
    m_agentId = -1;
}

void NavmeshActorComponent::Update(float deltaTime)
{
    const dtCrowdAgent* agent = NavigationManager::GetInstance()->GetAgentData(m_agentId);
    Math::Vector3 velocity(agent->vel[0], agent->vel[1], agent->vel[2]);
    Math::Vector3 position(agent->npos[0], agent->npos[1], agent->npos[2]);
    
    if ((position - m_target).LengthSquared() < agent->params.radius * agent->params.radius)
    {
        m_hasTarget = false;
    }


    if (velocity.Length() > 0.01f)
    {
        Math::Vector3 velNorm = velocity;
        velNorm.y = 0.0f;
        velNorm.Normalize();

        m_yrot = m_yrot + (atan2(velNorm.z, velNorm.x)-m_yrot) * deltaTime * 5.0f;

        auto mat = Math::Matrix::MakeRotationY(m_yrot);
        auto scale = m_owner->GetWorldMatrix().GetScale();

        mat.SetFront(mat.GetFront() * scale.z);
        mat.SetRight(mat.GetRight() * scale.x);
        mat.SetUp(mat.GetUp() * scale.y);

        position.y += m_agentParams.height;
        mat.SetPosition(position);
        m_owner->SetWorldMatrix(mat);
    }
}

Json::Object NavmeshActorComponent::SaveComponent()
{
    Json::Object object;

    object.emplace("Radius", m_agentParams.radius);
    object.emplace("Height", m_agentParams.height);
    object.emplace("MaxAcceleration", m_agentParams.maxAcceleration);
    object.emplace("MaxSpeed", m_agentParams.maxSpeed);
    object.emplace("CollisionQueryRange", m_agentParams.collisionQueryRange);
    object.emplace("PathOptimizationRange", m_agentParams.pathOptimizationRange);
    object.emplace("SeparationWeight", m_agentParams.separationWeight);
    object.emplace("UpdateFlags", static_cast<int>(m_agentParams.updateFlags));
    object.emplace("ObstacleAvoidanceType", static_cast<int>(m_agentParams.obstacleAvoidanceType));
    object.emplace("QueryFilterType", static_cast<int>(m_agentParams.queryFilterType));

    return std::move(object);
}

void NavmeshActorComponent::LoadComponent(const rapidjson::Value& object)
{
    m_agentParams.radius = object["Radius"].GetFloat();
    m_agentParams.height = object["Height"].GetFloat();
    m_agentParams.maxAcceleration = object["MaxAcceleration"].GetFloat();
    m_agentParams.maxSpeed = object["MaxSpeed"].GetFloat();
    m_agentParams.collisionQueryRange = object["CollisionQueryRange"].GetFloat();
    m_agentParams.pathOptimizationRange = object["PathOptimizationRange"].GetFloat();
    m_agentParams.separationWeight = object["SeparationWeight"].GetFloat();
    m_agentParams.updateFlags = static_cast<unsigned char>(object["UpdateFlags"].GetInt());
    m_agentParams.obstacleAvoidanceType = static_cast<unsigned char>(object["ObstacleAvoidanceType"].GetInt());
    m_agentParams.queryFilterType = static_cast<unsigned char>(object["QueryFilterType"].GetInt());

}

#ifdef EDITOR
void NavmeshActorComponent::ComponentEditor()
{
    ImGui::PushItemWidth(100.0f);
    ImGui::InputFloat("Radius", &m_agentParams.radius);
    ImGui::InputFloat("Height", &m_agentParams.height);
    ImGui::InputFloat("Max Acceleration", &m_agentParams.maxAcceleration);
    ImGui::InputFloat("Max Speed", &m_agentParams.maxSpeed);
    ImGui::InputFloat("Collision Query Range", &m_agentParams.collisionQueryRange);
    ImGui::InputFloat("Path Optimization Range", &m_agentParams.pathOptimizationRange);
    ImGui::InputFloat("Separation Weight", &m_agentParams.separationWeight);

    bool anticipateTurns = (m_agentParams.updateFlags & DT_CROWD_ANTICIPATE_TURNS) != 0;
    bool obstacleAvoidance = (m_agentParams.updateFlags & DT_CROWD_OBSTACLE_AVOIDANCE) != 0;
    bool crowdSeparation = (m_agentParams.updateFlags & DT_CROWD_SEPARATION) != 0;


    if (ImGui::Checkbox("Anticipate Turns", &anticipateTurns))
    {
        if (anticipateTurns)
            m_agentParams.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
        else
            m_agentParams.updateFlags &= ~DT_CROWD_ANTICIPATE_TURNS;
    }

    if (ImGui::Checkbox("Obstacle Avoidance", &obstacleAvoidance))
    {
        if (obstacleAvoidance)
            m_agentParams.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
        else
            m_agentParams.updateFlags &= ~DT_CROWD_OBSTACLE_AVOIDANCE;
    }

    if (ImGui::Checkbox("Crowd Separation", &crowdSeparation))
    {
        if (crowdSeparation)
            m_agentParams.updateFlags |= DT_CROWD_SEPARATION;
        else
            m_agentParams.updateFlags &= ~DT_CROWD_SEPARATION;
    }
    //ImGui::InputFloat("Obstacle Avoidance Type", m_agentParams.obstacleAvoidanceType);
    //ImGui::InputFloat("Query Filter Type", m_agentParams.queryFilterType);

    ImGui::PopItemWidth();
}
#endif // EDITOR

void NavmeshActorComponent::SetTarget(const Math::Vector3& target)
{
    if (m_agentId == -1)
    {
        CreateAgent();
    }

    if (NavigationManager::GetInstance()->RequestTarget(target, m_agentId))
    {
        m_hasTarget = true;
        m_target = target;
    }
}
bool NavmeshActorComponent::HasTarget() const
{
    return m_hasTarget;
}

dtCrowdAgentParams NavmeshActorComponent::GetAgentParams() const
{
    return m_agentParams;
}

void NavmeshActorComponent::CreateAgent()
{
    m_agentId = NavigationManager::GetInstance()->CreateAgent(m_owner, m_agentParams);
}

