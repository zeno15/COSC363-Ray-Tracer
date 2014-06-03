#ifndef INCLUDED_RAY_TRACER_HPP
#define INCLUDED_RAY_TRACER_HPP

#include "Color.h"
#include "Vector.h"
#include <glm/glm.hpp>

#define BACKGROUND_COL Color::GRAY

//A useful struct
struct PointBundle   
{
	Vector point;
	int index;
	float dist;
};

struct pixelDraw
{
	glm::vec2 _vertex1;
	glm::vec2 _vertex2;
	glm::vec2 _vertex3;
	glm::vec2 _vertex4;

	glm::vec3 _colour;
};

Color trace(Vector pos, Vector dir, int step, float &_t, std::vector<Object*> &_sceneObjects, float _currentRefractiveIndex);

PointBundle closestPt(Vector pos, Vector dir, std::vector<Object*> &_sceneObjects);

void rayTrace(std::vector<pixelDraw> &_outputVector, std::vector<Object *> &_sceneObjects, int _x_start, int _y_start, int _x_end, int _y_end, float _pixel_size, int _samplingType, int _numberSamples, bool _run = true);

#endif //~ INCLUDED_RAY_TRACER_HPP