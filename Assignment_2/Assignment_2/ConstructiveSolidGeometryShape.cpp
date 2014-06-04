#include "ConstructiveSolidGeometryShape.h"


ConstructiveSolidGeometryShape::ConstructiveSolidGeometryShape(Object *_objectA, Object *_objectB, OPERATOR _op)
{
	m_objectA = _objectA;
	m_objectB = _objectB;
	m_op = _op;
}

ConstructiveSolidGeometryShape::~ConstructiveSolidGeometryShape()
{
}


float ConstructiveSolidGeometryShape::intersect(Vector pos, Vector dir, float *_tmax = nullptr)
{
	transformRay(pos, dir);

	//~ Set color to the right objects color here? else may need to change the way Object::getColor works

	return -1.0f;
}

Vector ConstructiveSolidGeometryShape::normal(Vector p)
{
	transformRay(p, Vector());

	Vector n;



	transformNormal(n);

	return n;
}

Color ConstructiveSolidGeometryShape::getColorTex(Vector _vec)
{
	return color;
}
	
void ConstructiveSolidGeometryShape::scale(Vector _scaleFactors)
{
	m_objectA->scale(_scaleFactors);
	m_objectB->scale(_scaleFactors);
}