#pragma once
#include "pch.h"
#include "Component.h"

class ParticleComponent : public Component
{
public:
	ParticleComponent(GameObject* owner);

	void Update(float deltaTime) override;

	void Start() override;
	void Reset() override;

	struct Particle
	{
		void SetAwake()
		{
			sleeping = false;
			sleepFrames = 0;
		};

		Math::Vector3 position;
		Math::Vector3 velocity;
		float moisture;
		float mass = 0.8f;
		Particle* bonds[6];
		int sleepFrames = 0;

		bool sleeping = false;


		Math::Vector3 oldPositions[5];
	};

	struct Octree
	{
		float size = 10;
		float position[2];

		int depth = 0;
		Octree* parent;
		Octree* children[4];

		void CreateChildren()
		{
			float childSize = size * 0.25f;
			float halfSize = size * 0.5f;
			for (int i = 0; i < 4; i++)
			{
				children[i] = new Octree();
				children[i]->parent = this;
				children[i]->depth = depth + 1;

				switch (i)
				{
				case 0:
					children[i]->position[0] = position[0] - halfSize;
					break;
				}

				if (childSize > 0.1f)
				{
					children[i]->CreateChildren();
				}
			}
		}
	};

	void Render() override;

#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	Math::Vector3 m_gravity;
	float m_particleSize;
	float m_restitution;
	float m_attraction;

private:
	void SolveParticles(float timeStep);
	void KeepParticeInBounds(std::shared_ptr<Particle> particle) const;

	std::vector<std::shared_ptr<Particle>> m_particles;
	float m_particleSizeSquared;
	float m_restDistance;
};

