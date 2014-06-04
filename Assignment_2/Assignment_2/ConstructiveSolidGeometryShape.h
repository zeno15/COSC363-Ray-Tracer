#ifndef INCLUDED_CONSTRUCTIVE_SOLID_GEOMETRY_SHAPE_HPP
#define INCLUDED_CONSTRUCTIVE_SOLID_GEOMETRY_SHAPE_HPP

#include "Object.h"

class ConstructiveSolidGeometryShape : public Object
{
public:
	enum OPERATOR {
		UNION,
		INTERSECTION,
		DIFFERENCE
	};

	ConstructiveSolidGeometryShape(Object *_objectA, Object *_objectB, OPERATOR _op);
	~ConstructiveSolidGeometryShape();
	
	float intersect(Vector pos, Vector dir, float *_tmax /*= nullptr*/);

	Vector normal(Vector p);

	virtual Color getColorTex(Vector _vec);
	
	virtual void scale(Vector _scaleFactors);

private:
	Object *m_objectA;
	Object *m_objectB;
	OPERATOR m_op;
};

#endif //~ INCLUDED_CONSTRUCTIVE_SOLID_GEOMETRY_SHAPE_HPP