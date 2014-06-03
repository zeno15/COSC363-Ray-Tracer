#ifndef INCLUDED_CYLINDAR_H
#define INCLUDED_CYLINDAR_H

#include "Object.h"

class Cylindar : public Object
{
public:
	Cylindar(Color _color);
	~Cylindar();

	float intersect(Vector pos, Vector dir);

	Vector normal(Vector p);

	virtual Color getColorTex(Vector _vec);
	
	virtual void scale(Vector _scaleFactors);

private:
	Vector m_Origin;
	Vector scaleFact;
	float m_RadiusX;
	float m_RadiusZ;
	float m_Height;

};

#endif //~ INCLUDED_CYLINDAR_H