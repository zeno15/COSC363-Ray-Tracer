#ifndef INCLUDED_TORUS_H
#define INCLUDED_TORUS_H

#include "Object.h"

class Torus : public Object
{
public:
	Torus(float _A, float _B, Color _color);
	~Torus();

	float intersect(Vector pos, Vector dir, float *_tmax = nullptr);

	Vector normal(Vector p);

	virtual Color getColorTex(Vector _vec);
	
	virtual void scale(Vector _scaleFactors);

private:
	float m_A;
	float m_B;
};

#endif //~ INCLUDED_TORUS_H