#include "ParticipatingMediaSystem.hpp"

#include <cstdlib>
#include <iostream>

#include "RayTracer.hpp"
#include "Sphere.h"

#define DIRECTION_VARIETY_SCALE 2.0f

ParticipatingMediaSystem::ParticipatingMediaSystem(ParticipatingMediaSystem::type _type, Vector _position, std::vector<Object*> *_sceneObjects, Vector *_eyePos) :
m_SceneObjects(_sceneObjects),
m_EyePos(_eyePos),
m_Type(_type),
m_hasGeneratedParticlePositions(false)
{
	int numParticles = rand() % 150 + 150; //~ Gives 15 - 30 particles

	m_Particles = std::vector<particle>(numParticles);

	for (unsigned int i = 0; i < m_Particles.size(); i += 1)
	{
		m_Particles.at(i).position = _position;
		m_Particles.at(i).direction = Vector( - 1.0f * DIRECTION_VARIETY_SCALE + (float)(rand() % (int)(200 * DIRECTION_VARIETY_SCALE)) / 100.0f, 
											  - 1.0f * DIRECTION_VARIETY_SCALE + (float)(rand() % (int)(200 * DIRECTION_VARIETY_SCALE)) / 100.0f, 
											  - 1.0f * DIRECTION_VARIETY_SCALE + (float)(rand() % (int)(200 * DIRECTION_VARIETY_SCALE)) / 100.0f);
		m_Particles.at(i).direction.normalise();

		m_Particles.at(i).magnitude = ((float)(rand() % 40)/ 100.0f + 0.08f); //~ 0.08 - 0.12
		m_Particles.at(i).affectRadius = _type == type::SMOKE ? 2.0f : 4.0f;
		
		m_Particles.at(i).initialLife = (rand() % 5 + 5) * (_type == type::SMOKE ? 1 : 2); //~ 8-16 for smoke, 32 - 64 for fog
		m_Particles.at(i).remainingLife = m_Particles.at(i).initialLife;

		m_Particles.at(i).delayRemaining = 0;// rand() % (int)((float)m_Particles.at(i).initialLife * 0.2f); //~ 0 - 20% of initial life
	}

	m_NumSteps = (rand() % 5 + 9) * (_type == type::SMOKE ? 1 : 8);
}

ParticipatingMediaSystem::~ParticipatingMediaSystem()
{
}


void ParticipatingMediaSystem::step(std::vector<Object*> &_sceneObjects, std::vector<Object*> &_endObjects)
{
	m_NumSteps -= 1;

	int numDone = 0;

	std::vector<Color> colors;
	colors.push_back(Color::RED);
	colors.push_back(Color::BLUE);
	colors.push_back(Color::GREEN);
	colors.push_back(Color::CYAN);
	colors.push_back(Color::GRAY);

	for (unsigned int i = 0; i < m_Particles.size(); i += 1)
	{
		if (m_Particles.at(i).delayRemaining > 0)
		{
			m_Particles.at(i).delayRemaining -= 1;
			continue;
		}
		if (m_Particles.at(i).remainingLife <= 0)
		{
			numDone += 1;
			continue;
		}

		m_Particles.at(i).remainingLife -= 1;

		float t = -1.0f;

		PointBundle p = closestPt(m_Particles.at(i).position, m_Particles.at(i).direction, _sceneObjects);

		if (p.index == -1 || p.dist > m_Particles.at(i).magnitude)
		{
			//~ Hasn't collided this step

			m_Particles.at(i).position += m_Particles.at(i).direction * m_Particles.at(i).magnitude;

			break;
		}

		//~ A collision occurs we need to deal with it

		float distanceToCollision = p.dist;
		float distanceAfterCollision = m_Particles.at(i).magnitude - p.dist;

		//~ Move it to the collision point
		m_Particles.at(i).position += m_Particles.at(i).direction * distanceToCollision;

		//~ Change its direction depending on the collision normal
		Vector normal = m_SceneObjects->at(p.index)->normal(p.point);

		Vector oldDir = m_Particles.at(i).direction;
		oldDir *= -1.0f;

		Vector newDir = ((normal * 2.0f) * oldDir.dot(normal)) - oldDir;

		m_Particles.at(i).direction = newDir;

		//~ Move it the remaining distance on its new direction
		
	}
	

	if (m_Particles.size() != (unsigned int)(numDone) && m_NumSteps > 0)
	{
		step(_sceneObjects, _endObjects);
	}
	else
	{
		for (unsigned int i = 0; i < m_Particles.size(); i += 1)
		{
			//Sphere *sphere = new Sphere(0.5f, colors.at(i % 5));
//############################################################################################################################# NEED TO TRANSLATE

			//_endObjects.push_back(sphere);
		}

		m_hasGeneratedParticlePositions = true;
	}
}

bool ParticipatingMediaSystem::hasGeneratedParticlePositions(void)
{
	return m_hasGeneratedParticlePositions;
}
std::vector<particle> *ParticipatingMediaSystem::getParticles(void)
{
	return &m_Particles;
}
float ParticipatingMediaSystem::getC(void)
{
	return m_Type == type::FOG ? 0.2f : 0.9f;
}