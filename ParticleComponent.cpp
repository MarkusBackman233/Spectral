#include "ParticleComponent.h"

ParticleComponent::ParticleComponent(GameObject* owner)
	: Component(owner)
	, m_particleSize(0.5f)
	, m_particleSizeSquared(m_particleSize* m_particleSize)
	, m_restitution(0.01f)
	, m_restDistance(0.000001f)
	, m_attraction(1.0f)
{
	//m_gravity = Math::Vector3(0.0f, -9.81f, 0.0f);
	m_componentName = "ParticleComponent";
	m_componentType = ComponentType_LightComponent;
}


void ParticleComponent::Start()
{
	m_particleSizeSquared = m_particleSize * m_particleSize;
	for (int x = -25; x < 25; x++)
	{
		for (int y = -25; y < 25; y++)
		{
			if ((std::rand() % 101) * 0.01f > 0.5f)
				continue;

			std::shared_ptr<Particle> newParticle = std::make_shared<Particle>();
			newParticle->position = Math::Vector3((float)x * 0.05f, (float)y * 0.1f, 0.0f);
			newParticle->velocity.x = ((std::rand() % 101) * 0.01f - 0.5f) * 300.0f;
			for (int i = 0; i < 5; i++)
			{
				newParticle->oldPositions[i] = newParticle->position;
			}

			m_particles.push_back(newParticle);
		}
	}


	//std::shared_ptr<Particle> newParticle = std::make_shared<Particle>();
	//newParticle->position = Math::Vector3(-0.5f,1.0f, 0.0f);
	//newParticle->velocity.x = 10.0f;
	//m_particles.push_back(newParticle);
	//
	//std::shared_ptr<Particle> newParticle2 = std::make_shared<Particle>();
	//newParticle2->position = Math::Vector3(0.5f, 1.0f, 0.0f);
	//newParticle2->velocity.x = -10.0f;
	//m_particles.push_back(newParticle2);
}

void ParticleComponent::Reset()
{
	m_particles.clear();
}

void ParticleComponent::Update(float deltaTime)
{
	static float accumulatedTime = 0.0f;
	const float fixedTimeStep = 1.0f / 120.0f;

	accumulatedTime += deltaTime;

	while (accumulatedTime >= fixedTimeStep)
	{
		SolveParticles(fixedTimeStep);
		accumulatedTime -= fixedTimeStep;
	}
}


void ParticleComponent::Render()
{
	for (const auto& particle : m_particles)
	{
		//if(!particle->sleeping)
		//	raylib::DrawCircleV(raylib::Vector2{ -particle->position.x,particle->position.y }, m_particleSize*0.3f, raylib::PURPLE);
		//else
		//	raylib::DrawCircleV(raylib::Vector2{ -particle->position.x,particle->position.y }, m_particleSize*0.3f, raylib::DARKPURPLE);
		//int thick = m_particleSize;
		//for (int i = 0; i < 4; i++)
		//{
		//	//thick *= 0.9f;
		//}
	}
}
#ifdef EDITOR
void ParticleComponent::ComponentEditor()
{
	ImGui::InputFloat("Particle Size##01", &m_particleSize);
	ImGui::InputFloat("Restitution##01", &m_restitution);
	ImGui::InputFloat("Attraction##01", &m_attraction);
	ImGui::InputFloat3("Gravity##01", &m_gravity.x);
}
#endif
void ParticleComponent::SolveParticles(float timeStep)
{
	for (const auto& particle : m_particles)
	{
		if (particle->sleeping)
			continue;

		auto oldPosition = particle->position;

		Math::Vector3 gravity;


		int influances = 1;

		for (const auto& otherParticle : m_particles)
		{
			if (particle == otherParticle)
				continue;

			auto relativePosition = otherParticle->position - particle->position;
			auto distanceBetweenParticles = relativePosition.Length();


			auto normal = relativePosition.GetNormal();
			influances++;
			gravity += normal;


			if (distanceBetweenParticles <= m_particleSizeSquared)
			{
				auto penetrationDepth = m_particleSizeSquared - distanceBetweenParticles;

				if (penetrationDepth > 0.0)
				{
					auto correction = normal * penetrationDepth * 0.5f;
					particle->position -= correction;
					otherParticle->position += correction;
	
					KeepParticeInBounds(otherParticle);
				}

				auto relativeVelocity = particle->velocity - otherParticle->velocity;
				auto relativeVelocityAlongNormal = relativeVelocity.Dot(normal);

				if (relativeVelocityAlongNormal > 0.0)
				{
					float j = -(1 + m_restitution) * relativeVelocityAlongNormal;
					j /= 1 / particle->mass + 1 / otherParticle->mass;

					Math::Vector3 impulse = normal * j;

					particle->velocity += impulse / particle->mass;
					otherParticle->velocity -= impulse / otherParticle->mass;
				}
			}
		}

		particle->velocity += gravity / static_cast<float>(influances) * m_attraction;

		particle->velocity += m_gravity * timeStep;

		particle->position += particle->velocity * timeStep;
		KeepParticeInBounds(particle);
		if ((particle->position - particle->oldPositions[0]).Length() > 0.2f)
		{
			particle->oldPositions[4] = particle->oldPositions[3];
			particle->oldPositions[3] = particle->oldPositions[2];
			particle->oldPositions[2] = particle->oldPositions[1];
			particle->oldPositions[1] = particle->oldPositions[0];
			particle->oldPositions[0] = particle->position;
		}


		//if ((oldPosition - particle->position).Length() < m_restDistance)
		//{
		//	particle->sleepFrames++;
		//	if (particle->sleepFrames > 5)
		//	{
		//		particle->sleeping = true;
		//	}
		//}
		//else
		//{
		//	particle->sleepFrames = 0;
		//}
	}
}

void ParticleComponent::KeepParticeInBounds(std::shared_ptr<Particle> particle) const
{
	if (particle->position.y <= -10.0f)
	{
		particle->position.y = -10.0f;
		particle->velocity = particle->velocity.reflect(Math::Vector3(0, 1, 0)) * m_restitution;
	}
	//if (particle->position.x >= 10.0f)
	//{
	//	particle->position.x = 10.0f;
	//	particle->velocity = particle->velocity.reflect(Math::Vector3(-1, 0, 0)) * m_restitution;
	//}
	//if (particle->position.x <= -10.0f)
	//{
	//	particle->position.x = -10.0f;
	//	particle->velocity = particle->velocity.reflect(Math::Vector3(1, 0, 0)) * m_restitution;
	//}
}