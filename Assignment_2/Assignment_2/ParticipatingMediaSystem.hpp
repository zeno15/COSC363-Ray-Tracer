#ifndef INCLUDED_PARTICIPATING_MEDIA_SYSTEM_HPP
#define INCLUDED_PARTICIPATING_MEDIA_SYSTEM_HPP

#include <vector>

#include "Vector.h"
#include "Object.h"

struct particle
{
	Vector position;
	Vector direction;

	float magnitude;
	float affectRadius;

	int delayRemaining;

	int initialLife;
	int remainingLife;
};

class ParticipatingMediaSystem
{
public:
	enum type {
		SMOKE,
		FOG
	};

	ParticipatingMediaSystem(ParticipatingMediaSystem::type _type, Vector _position, std::vector<Object*> *_sceneObjects, Vector *_eyePos);
	~ParticipatingMediaSystem();

	void step(std::vector<Object*> &_sceneObjects, std::vector<Object*> &_endObjects);

	bool hasGeneratedParticlePositions(void);

	std::vector<particle> *getParticles(void);

	float getC(void);

private:
	std::vector<particle>			m_Particles;

	int								m_NumSteps;	//~ Relates to what type it is (fog travels further than smoke)

	std::vector<Object*> *			m_SceneObjects;

	Vector *						m_EyePos;

	bool							m_hasGeneratedParticlePositions;

	type							m_Type;
};

#endif //~ INCLUDED_PARTICIPATING_MEDIA_SYSTEM_HPP