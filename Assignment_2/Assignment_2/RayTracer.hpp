#ifndef INCLUDED_RAY_TRACER_HPP
#define INCLUDED_RAY_TRACER_HPP

#include "Color.h"
#include "Vector.h"

#define BACKGROUND_COL Color::GRAY

//A useful struct
struct PointBundle   
{
	Vector point;
	int index;
	float dist;
};

Color trace(Vector pos, Vector dir, int step, float &_t, std::vector<Object*> &_sceneObjects);

PointBundle closestPt(Vector pos, Vector dir, std::vector<Object*> &_sceneObjects);

#endif //~ INCLUDED_RAY_TRACER_HPP