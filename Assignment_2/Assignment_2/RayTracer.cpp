// ========================================================================
// COSC 363  Computer Graphics  Lab07
// A simple ray tracer
// ========================================================================

#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <thread>
#include "Vector.h"
#include "Sphere.h"
#include "Plane.h"
#include "Cube.h"
#include "Torus.h"
#include "Cylindar.h"
#include "Color.h"
#include "Object.h"
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h"

#include "RayTracer.hpp"


const float WIDTH = 40.0f;  
const float HEIGHT = 40.0f;
const float EDIST = 40.0f; 
int PPU = 4;     
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5f;
const float XMAX =  WIDTH * 0.5f;
const float YMIN = -HEIGHT * 0.5f;
const float YMAX =  HEIGHT * 0.5f;

#define TO_RADIANS			(3.14159265f / 180.0f)
#define ANGLE_CHANGE		5.0f

#define NORMAL_SAMPLING		0
#define SUPER_SAMPLING		1
#define ADAPTIVE_SAMPLING	2
#define STOCHASTIC_SAMPLING 3

#define PRIMITIVE_SCENE		0
#define TRANSFORM_SCENE		1
#define TEXTURED_SCENE		2
#define MULTI_LIGHT_SCENE	3
#define REFLECTION_SCENE	4

#define MAX_THREADS			4
#define MAX_OBJECTS			20

#define SCALE				0
#define ROTATE				1
#define TRANSLATE			2

std::vector<Object*> sceneObjectsThread1;
std::vector<Object*> sceneObjectsThread2;
std::vector<Object*> sceneObjectsThread3;
std::vector<Object*> sceneObjectsThread4;
std::vector<Object*> extraObjects;

std::vector<Vector> lights;

std::vector<pixelDraw>	pixelsToDrawThread1;
std::vector<pixelDraw>	pixelsToDrawThread2;
std::vector<pixelDraw>	pixelsToDrawThread3;
std::vector<pixelDraw>	pixelsToDrawThread4;

Vector eyePosition = Vector(0.0f, 50.0f, 150.0f);

tex texture1;
tex texture2;


unsigned int numsamples = 1;

unsigned int samplingType = 0;
unsigned int sceneNumber = 0;

float colorCompareThreshold = 0.05f;


int numThreads = 1;

float angle = 0.0f;

Object *transformable = nullptr;

unsigned int transformModify = SCALE;


PointBundle closestPt(Vector pos, Vector dir, std::vector<Object*> &_sceneObjects)
{
    Vector  point(0, 0, 0);
	float min = 10000.0;

	PointBundle out = {point, -1, 0.0};

	
    for(unsigned int i = 0;  i < _sceneObjects.size();  i++)
	{
        float t = _sceneObjects[i]->intersect(pos, dir);
		if(t > 0)        //Intersects the object
		{
			point = pos + dir*t;
			if(t < min)
			{
				out.point = point;
				out.index = i;
				out.dist = t;
				min = t;
			}
		}
	}

	return out;
}

Color trace(Vector pos, Vector dir, int step, float &_t, std::vector<Object*> &_sceneObjects, float _currentRefractiveIndex)
{
    PointBundle q = closestPt(pos, dir, _sceneObjects);

	_t = q.dist;

    if(q.index == -1) return BACKGROUND_COL;        //no intersection

	Color col;

	if (_sceneObjects[q.index]->hasTexture())
	{
		col = _sceneObjects[q.index]->getColorTex(q.point);
	}
	else
	{
		col = _sceneObjects[q.index]->getColor(); //Object's colour.
	}

	Vector n = _sceneObjects.at(q.index)->normal(q.point);

	std::vector<bool> inSight(lights.size(), true);

	int outOfSightCount = 0;

	for (unsigned int i = 0; i < lights.size(); i += 1)
	{
		Vector l = lights.at(i) - q.point;
		l.normalise();

		float lDotn = l.dot(n);

		if (lDotn <= 0.0f)
		{
			//~ Point is not in direct view of light source i.
			inSight.at(i) = false;
			outOfSightCount += 1;
		}
	}

	if (outOfSightCount == lights.size())
	{
		//~ Point is not in sight of ANY light sources, use only ambient light

		col = col.phongLight(BACKGROUND_COL, 0.0f, 0.0f);
	}
	else
	{
		float maxlDotn = 0.0f;
		float avelDotn = 0.0f;
		float maxspec = 0.0f;

		for (unsigned int i = 0; i < lights.size(); i += 1)
		{
			if (!inSight.at(i)) continue;

			Vector l = lights.at(i) - q.point;
			l.normalise();

			float lDotn = l.dot(n);

			Vector r = ((n * 2.0f) * lDotn) - l;
			r.normalise();

			Vector v(-dir.x, -dir.y, -dir.z);

			float rDotv = r.dot(v);

			float spec = 0.0f;

			if (rDotv >= 0.0f)
			{
				spec = pow(rDotv, 10);
			}

			avelDotn += lDotn;

			if (lDotn > maxlDotn)
			{
				maxlDotn = lDotn;
			}
			if (spec > maxspec)
			{
				maxspec = spec;
			}
		}

		col = col.phongLight(BACKGROUND_COL, maxlDotn, maxspec);

		for (unsigned int i = 0; i < lights.size(); i += 1)
		{
			Vector l = lights.at(i) - q.point;
			l.normalise();

			PointBundle shadow = closestPt(q.point, l, _sceneObjects);

			if (shadow.index == -1) continue;

			if (shadow.index == q.index) continue;				//~ Dont want self shadowing

			if (shadow.dist > q.point.dist(lights.at(i))) continue;

			if (_sceneObjects.at(shadow.index)->getRefractive()) continue; //~ Assuming refractive objects dont leave shadows
			if (_sceneObjects.at(q.index)->getRefractive()) continue; //~ Assuming refractive objects dont leave shadows

			col = col.phongLight(BACKGROUND_COL, 0.0f, 0.0f);
		}
	}

	Color colorSum = col;

	if (_sceneObjects.at(q.index)->getReflective() && step < MAX_STEPS)
	{
		//~ Reflective Object
		Vector v(-dir.x, -dir.y, -dir.z);

		Vector reflectionVector = n * n.dot(v) * 2.0f - v;

		float t = 0.0f;

		Color reflectionColor = trace(q.point, reflectionVector, step + 1, t, _sceneObjects, _currentRefractiveIndex);

		colorSum.combineColor(reflectionColor, 0.8f);
	}

	return colorSum;
}

void adaptiveSample(float _cx, float _cy, float _sizeX, float _sizeY, int step, std::vector<pixelDraw> &_outputVector, std::vector<Object *> _sceneObjects)
{
	float t = 0.0f;
	bool valid = true;

	Vector dir = Vector(_cx, _cy, -EDIST);
	dir.normalise();

	glm::vec4 rotation = glm::vec4(dir.x, dir.y, dir.z, 1.0f);

	glm::mat4x4 rotationMat = glm::rotate(glm::mat4x4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

	rotation = rotationMat * rotation;

	dir = Vector(rotation.x, rotation.y, rotation.z);

	dir.normalise();

	Color col = trace(eyePosition, dir, 1, t, _sceneObjects, 1.0f);


	dir = Vector(_cx - _sizeX / 2.0f, _cy - _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color tl = trace(eyePosition, dir, 1, t, _sceneObjects, 1.0f);

	dir = Vector(_cx + _sizeX / 2.0f, _cy - _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color tr = trace(eyePosition, dir, 1, t, _sceneObjects, 1.0f);

	dir = Vector(_cx + _sizeX / 2.0f, _cy + _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color br = trace(eyePosition, dir, 1, t, _sceneObjects, 1.0f);

	dir = Vector(_cx - _sizeX / 2.0f, _cy + _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color bl = trace(eyePosition, dir, 1, t, _sceneObjects, 1.0f);

	if (tl.compare(tr, colorCompareThreshold))
	{
		valid = false;
	}
	if (tl.compare(br, colorCompareThreshold))
	{
		valid = false;
	}
	if (tl.compare(bl, colorCompareThreshold))
	{
		valid = false;
	}


	if (valid || step >= (int)numsamples)
	{
		_outputVector.push_back(pixelDraw());
		_outputVector.back()._colour = glm::vec3(col.r, col.g, col.b);
		_outputVector.back()._vertex1 = glm::vec2(_cx - _sizeX / 2.0f, _cy - _sizeY / 2.0f);
		_outputVector.back()._vertex2 = glm::vec2(_cx + _sizeX / 2.0f, _cy - _sizeY / 2.0f);
		_outputVector.back()._vertex3 = glm::vec2(_cx + _sizeX / 2.0f, _cy + _sizeY / 2.0f);
		_outputVector.back()._vertex4 = glm::vec2(_cx - _sizeX / 2.0f, _cy + _sizeY / 2.0f);
	}
	else
	{
		adaptiveSample(_cx - _sizeX / 4.0f, _cy - _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1, _outputVector, _sceneObjects);
		adaptiveSample(_cx + _sizeX / 4.0f, _cy - _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1, _outputVector, _sceneObjects);
		adaptiveSample(_cx + _sizeX / 4.0f, _cy + _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1, _outputVector, _sceneObjects);
		adaptiveSample(_cx - _sizeX / 4.0f, _cy + _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1, _outputVector, _sceneObjects);
	}
}

void display()
{
	int widthInPixels = (int)(WIDTH * PPU);
	int heightInPixels = (int)(HEIGHT * PPU);
	float pixelSize = 1.0f/PPU;
	
	if (samplingType == NORMAL_SAMPLING)
	{
		numsamples = 1;
		samplingType = SUPER_SAMPLING;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	pixelsToDrawThread1.clear();
	pixelsToDrawThread2.clear();
	pixelsToDrawThread3.clear();
	pixelsToDrawThread4.clear();
	
	if (numThreads <= 0)
	{
		numThreads = 1;
	}
	if (numThreads > MAX_THREADS)
	{
		numThreads = MAX_THREADS;
	}

	int threadWidth = static_cast<int>(static_cast<float>(widthInPixels) /  static_cast<float>(numThreads));

	clock_t startTime = clock();

	std::thread t2(rayTrace, std::ref(pixelsToDrawThread2), std::ref(sceneObjectsThread2), threadWidth * 1, 0, threadWidth * 2, heightInPixels, pixelSize, samplingType, numsamples, (numThreads > 1));

	std::thread t3(rayTrace, std::ref(pixelsToDrawThread3), std::ref(sceneObjectsThread3), threadWidth * 2, 0, threadWidth * 3 + (numThreads == 3 ? PPU : 0), heightInPixels, pixelSize, samplingType, numsamples, (numThreads > 2));

	std::thread t4(rayTrace, std::ref(pixelsToDrawThread4), std::ref(sceneObjectsThread4), threadWidth * 3, 0, threadWidth * 4, heightInPixels, pixelSize, samplingType, numsamples, (numThreads > 3));
	
	rayTrace(pixelsToDrawThread1, sceneObjectsThread1, 0, 0, threadWidth + widthInPixels - numThreads * threadWidth, heightInPixels, pixelSize, samplingType, numsamples, (numThreads > 0));
	
	t2.join();
	t3.join();
	t4.join();

	glBegin(GL_QUADS);  //Each pixel is a quad.

	for (unsigned int i = 0; i < pixelsToDrawThread1.size(); i += 1)
	{


		glColor3f(pixelsToDrawThread1.at(i)._colour.r, pixelsToDrawThread1.at(i)._colour.g, pixelsToDrawThread1.at(i)._colour.b);
		glVertex2f(pixelsToDrawThread1.at(i)._vertex1.x, pixelsToDrawThread1.at(i)._vertex1.y);
		glVertex2f(pixelsToDrawThread1.at(i)._vertex2.x, pixelsToDrawThread1.at(i)._vertex2.y);
		glVertex2f(pixelsToDrawThread1.at(i)._vertex3.x, pixelsToDrawThread1.at(i)._vertex3.y);
		glVertex2f(pixelsToDrawThread1.at(i)._vertex4.x, pixelsToDrawThread1.at(i)._vertex4.y);
	}
	for (unsigned int i = 0; i < pixelsToDrawThread2.size(); i += 1)
	{


		glColor3f(pixelsToDrawThread2.at(i)._colour.r, pixelsToDrawThread2.at(i)._colour.g, pixelsToDrawThread2.at(i)._colour.b);
		glVertex2f(pixelsToDrawThread2.at(i)._vertex1.x, pixelsToDrawThread2.at(i)._vertex1.y);
		glVertex2f(pixelsToDrawThread2.at(i)._vertex2.x, pixelsToDrawThread2.at(i)._vertex2.y);
		glVertex2f(pixelsToDrawThread2.at(i)._vertex3.x, pixelsToDrawThread2.at(i)._vertex3.y);
		glVertex2f(pixelsToDrawThread2.at(i)._vertex4.x, pixelsToDrawThread2.at(i)._vertex4.y);
	}
	for (unsigned int i = 0; i < pixelsToDrawThread3.size(); i += 1)
	{


		glColor3f(pixelsToDrawThread3.at(i)._colour.r, pixelsToDrawThread3.at(i)._colour.g, pixelsToDrawThread3.at(i)._colour.b);
		glVertex2f(pixelsToDrawThread3.at(i)._vertex1.x, pixelsToDrawThread3.at(i)._vertex1.y);
		glVertex2f(pixelsToDrawThread3.at(i)._vertex2.x, pixelsToDrawThread3.at(i)._vertex2.y);
		glVertex2f(pixelsToDrawThread3.at(i)._vertex3.x, pixelsToDrawThread3.at(i)._vertex3.y);
		glVertex2f(pixelsToDrawThread3.at(i)._vertex4.x, pixelsToDrawThread3.at(i)._vertex4.y);
	}
	for (unsigned int i = 0; i < pixelsToDrawThread4.size(); i += 1)
	{


		glColor3f(pixelsToDrawThread4.at(i)._colour.r, pixelsToDrawThread4.at(i)._colour.g, pixelsToDrawThread4.at(i)._colour.b);
		glVertex2f(pixelsToDrawThread4.at(i)._vertex1.x, pixelsToDrawThread4.at(i)._vertex1.y);
		glVertex2f(pixelsToDrawThread4.at(i)._vertex2.x, pixelsToDrawThread4.at(i)._vertex2.y);
		glVertex2f(pixelsToDrawThread4.at(i)._vertex3.x, pixelsToDrawThread4.at(i)._vertex3.y);
		glVertex2f(pixelsToDrawThread4.at(i)._vertex4.x, pixelsToDrawThread4.at(i)._vertex4.y);
	}

    glEnd();
    glFlush();

	clock_t endTime = clock();
	clock_t clockTicksTaken = endTime - startTime;
	double timeInSeconds = clockTicksTaken / (double) CLOCKS_PER_SEC;

	std::cout << "Redrawn in " << timeInSeconds << " seconds" << std::endl;
}

void rayTrace(std::vector<pixelDraw> &_outputVector, std::vector<Object *> &_sceneObjects, int _x_start, int _y_start, int _x_end, int _y_end, float _pixel_size, int _samplingType, int _numberSamples, bool _run /*= true*/)
{
	if (!_run) return;

	float t = 0.0f;

	float x1, xc, y1, yc;

	for(int i = _x_start; i < _x_end; i++)	//Scan every "pixel"
	{
		x1 = XMIN + i*_pixel_size;
		xc = x1 + _pixel_size * 0.5f;
		for(int j = _y_start; j < _y_end; j++)
		{
			y1 = YMIN + j*_pixel_size;
			yc = y1 + _pixel_size * 0.5f;

			if (samplingType == SUPER_SAMPLING)
			{
				Color col = Color::BLACK;

				for (int k = 0; k < _numberSamples; k += 1)
				{
					float xUsed = x1 + ((float)(k)+0.5f) / (float)(_numberSamples) * _pixel_size;

					for (int l = 0; l < _numberSamples; l += 1)
					{
						float yUsed = y1 + ((float)(l)+0.5f) / (float)(_numberSamples) * _pixel_size;

						Vector dir = Vector(xUsed, yUsed, -EDIST);

						dir.normalise();

						glm::vec4 rotation = glm::vec4(dir.x, dir.y, dir.z, 1.0f);

						glm::mat4x4 rotationMat = glm::rotate(glm::mat4x4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

						rotation = rotationMat * rotation;

						dir = Vector(rotation.x, rotation.y, rotation.z);

						dir.normalise();

						col.combineColor(trace(eyePosition, dir, 1, t, _sceneObjects, 1.0f), 1.0f / ((float)(_numberSamples * _numberSamples)));
					}
				}
				_outputVector.push_back(pixelDraw());
				_outputVector.back()._colour = glm::vec3(col.r, col.g, col.b);
				_outputVector.back()._vertex1 = glm::vec2(x1, y1);
				_outputVector.back()._vertex2 = glm::vec2(x1 + _pixel_size, y1);
				_outputVector.back()._vertex3 = glm::vec2(x1 + _pixel_size, y1 + _pixel_size);
				_outputVector.back()._vertex4 = glm::vec2(x1, y1 + _pixel_size);
			}
			else if (samplingType == ADAPTIVE_SAMPLING)
			{
				adaptiveSample(xc, yc, _pixel_size, _pixel_size, 1, _outputVector, _sceneObjects);
			}
			else if (samplingType == STOCHASTIC_SAMPLING)
			{
				Color col = Color::BLACK;

				for (int k = 0; k < _numberSamples; k += 1)
				{
					float x = x1 + (float)(rand() % 1000) / 1000.0f * _pixel_size;
					float y = y1 + (float)(rand() % 1000) / 1000.0f * _pixel_size;

					Vector dir = Vector(x, y, -EDIST);

					dir.normalise();

					glm::vec4 rotation = glm::vec4(dir.x, dir.y, dir.z, 1.0f);

					glm::mat4x4 rotationMat = glm::rotate(glm::mat4x4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

					rotation = rotationMat * rotation;

					dir = Vector(rotation.x, rotation.y, rotation.z);

					dir.normalise();

					col.combineColor(trace(eyePosition, dir, 1, t, _sceneObjects, 1.0f), 1.0f / ((float)(_numberSamples)));
					
				}

				_outputVector.push_back(pixelDraw());
				_outputVector.back()._colour = glm::vec3(col.r, col.g, col.b);
				_outputVector.back()._vertex1 = glm::vec2(x1, y1);
				_outputVector.back()._vertex2 = glm::vec2(x1 + _pixel_size, y1);
				_outputVector.back()._vertex3 = glm::vec2(x1 + _pixel_size, y1 + _pixel_size);
				_outputVector.back()._vertex4 = glm::vec2(x1, y1 + _pixel_size);
			}

        }	
    }
}

void initScene(unsigned int _sceneNumber)
{
	for (unsigned int i = 0; i < sceneObjectsThread1.size(); i += 1)
	{
		delete sceneObjectsThread1.at(i);
	}

	sceneObjectsThread1.clear();
	sceneObjectsThread2.clear();
	sceneObjectsThread3.clear();
	sceneObjectsThread4.clear();
	lights.clear();
	transformable = nullptr;

	switch (_sceneNumber)
	{
	case (4) :
		sceneObjectsThread1.push_back(new Plane(Color::WHITE));
		sceneObjectsThread1.back()->scale(Vector(100.0f, 1.0f, 100.0f));
		sceneObjectsThread1.back()->setTexture(&texture1);
		sceneObjectsThread1.back()->setReflective(true);
		
		sceneObjectsThread1.push_back(new Sphere(Color::BLUE));
		sceneObjectsThread1.back()->scale(Vector(20.0f, 20.0f, 20.0f));
		sceneObjectsThread1.back()->translate(Vector(0.0f, 25.0f, 0.0f));
		sceneObjectsThread1.back()->setReflective(true);

		sceneObjectsThread1.push_back(new Torus(6.0f, 2.0f, Color::GREEN));
		sceneObjectsThread1.back()->translate(Vector(20.0f, 35.0f, 20.0f));

		sceneObjectsThread1.push_back(new Sphere(Color::GRAY));
		sceneObjectsThread1.back()->scale(Vector(8.0f, 8.0f, 8.0f));
		sceneObjectsThread1.back()->translate(Vector(-20.0f, 15.0f, 18.0f));

		lights.push_back(Vector(100.0f, 100.0f, 100.0f));
		break;
	case (3) :
		sceneObjectsThread1.push_back(new Plane(Color::WHITE));
		sceneObjectsThread1.back()->scale(Vector(100.0f, 1.0f, 100.0f));
		sceneObjectsThread1.back()->setTexture(&texture1);

		sceneObjectsThread1.push_back(new Sphere(Color::WHITE));
		sceneObjectsThread1.back()->scale(Vector(20.0f, 20.0f, 20.0f));
		sceneObjectsThread1.back()->setTexture(&texture2);
		sceneObjectsThread1.back()->translate(Vector(0.0f, 25.0f, 0.0f));

		lights.push_back(Vector(100.0f, 100.0f, 100.0f));
		break;
	case (2) :
		sceneObjectsThread1.push_back(new Plane(Color::WHITE));
		sceneObjectsThread1.back()->scale(Vector(100.0f, 1.0f, 100.0f));
		sceneObjectsThread1.back()->setTexture(&texture1);
		
		sceneObjectsThread1.push_back(new Sphere(Color::BLUE));
		sceneObjectsThread1.back()->scale(Vector(10.0f, 10.0f, 10.0f));
		sceneObjectsThread1.back()->translate(Vector(0.0f, 30.0f, 30.0f));

		sceneObjectsThread1.push_back(new Cube(Color::CYAN));
		sceneObjectsThread1.back()->scale(Vector(15.0f, 15.0f, 15.0f));
		sceneObjectsThread1.back()->translate(Vector(-25.0f, 15.0f, 25.0f));
		sceneObjectsThread1.back()->rotate(Vector(45.0f, 45.0f, 45.0f));

		lights.push_back(Vector(100.0f, 100.0f, 100.0f));
		lights.push_back(Vector(0.0f, 100.0f, 100.0f));
		lights.push_back(Vector(-100.0f, 100.0f, 100.0f));

		break;
	case (1) :
		sceneObjectsThread1.push_back(new Sphere(Color::RED));
		sceneObjectsThread1.back()->scale(Vector(20.0f, 20.0f, 20.0f));
		transformable = sceneObjectsThread1.back();

		lights.push_back(Vector(100.0f, 100.0f, 100.0f));
		break;
	default:
		Plane *plane = new Plane(Color::RED);
		plane->scale(Vector(100.0f, 1.0f, 100.0f));

		Sphere *sphere = new Sphere(Color::YELLOW);
		sphere->scale(Vector(10.0f, 10.0f, 10.0f));
		sphere->translate(Vector(25.0f, 15.0f, -10.0f));

		Cube *cube = new Cube(Color::CYAN);
		cube->scale(Vector(20.0f, 16.0f, 30.0f));
		cube->translate(Vector(-25.0f, 10.0f, 0.0f));

		Torus *torus = new Torus(10.0f, 4.0f, Color::GREEN);
		torus->translate(Vector(0.0f, 25.0f, 2.0f));

		Cylindar *cylindar = new Cylindar(Color::BLUE);
		cylindar->translate(Vector(0.0f, 10.0f, 25.0f));
		cylindar->scale(Vector(5.0f, 10.0f, 5.0f));

		sceneObjectsThread1.push_back(plane);
		sceneObjectsThread1.push_back(sphere);
		sceneObjectsThread1.push_back(cube);
		sceneObjectsThread1.push_back(torus);
		sceneObjectsThread1.push_back(cylindar);
		lights.push_back(Vector(100.0f, 100.0f, 100.0f));
		break;
	}

	for (unsigned int i = 0; i < sceneObjectsThread1.size(); i += 1)
	{
		sceneObjectsThread2.push_back(sceneObjectsThread1.at(i));
		sceneObjectsThread3.push_back(sceneObjectsThread1.at(i));
		sceneObjectsThread4.push_back(sceneObjectsThread1.at(i));
	}

	angle = 0.0f;
	eyePosition = Vector(0.0f, 50.0f, 150.0f);
}

void special(int key, int x, int y)
{
	if (transformable != nullptr)
	{
		if (key == GLUT_KEY_F1)
		{
			transformModify = SCALE;
		}
		else if (key == GLUT_KEY_F2)
		{
			transformModify = ROTATE;
		}
		else if (key == GLUT_KEY_F3)
		{
			transformModify = TRANSLATE;
		}
		else if (key == GLUT_KEY_F4)
		{
			delete transformable;
			transformable = new Torus(20.0f, 8.0f, Color::RED);

			sceneObjectsThread1.clear();
			sceneObjectsThread2.clear();
			sceneObjectsThread3.clear();
			sceneObjectsThread4.clear();

			sceneObjectsThread1.push_back(transformable);
			sceneObjectsThread2.push_back(transformable);
			sceneObjectsThread3.push_back(transformable);
			sceneObjectsThread4.push_back(transformable);

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_F5)
		{
			delete transformable;
			transformable = new Sphere(Color::RED);
			transformable->scale(Vector(20.0f, 20.0f, 20.0f));

			sceneObjectsThread1.clear();
			sceneObjectsThread2.clear();
			sceneObjectsThread3.clear();
			sceneObjectsThread4.clear();

			sceneObjectsThread1.push_back(transformable);
			sceneObjectsThread2.push_back(transformable);
			sceneObjectsThread3.push_back(transformable);
			sceneObjectsThread4.push_back(transformable);

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_F6)
		{
			delete transformable;
			transformable = new Cube(Color::RED);
			transformable->scale(Vector(20.0f, 20.0f, 20.0f));

			sceneObjectsThread1.clear();
			sceneObjectsThread2.clear();
			sceneObjectsThread3.clear();
			sceneObjectsThread4.clear();

			sceneObjectsThread1.push_back(transformable);
			sceneObjectsThread2.push_back(transformable);
			sceneObjectsThread3.push_back(transformable);
			sceneObjectsThread4.push_back(transformable);

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_F7)
		{
			delete transformable;
			transformable = new Cylindar(Color::RED);
			transformable->scale(Vector(5.0f, 5.0f, 20.0f));

			sceneObjectsThread1.clear();
			sceneObjectsThread2.clear();
			sceneObjectsThread3.clear();
			sceneObjectsThread4.clear();

			sceneObjectsThread1.push_back(transformable);
			sceneObjectsThread2.push_back(transformable);
			sceneObjectsThread3.push_back(transformable);
			sceneObjectsThread4.push_back(transformable);

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_LEFT)
		{
			if (transformModify == SCALE)
			{
				transformable->scale(Vector(0.9f, 1.0f, 1.0f));
			}
			else if (transformModify == ROTATE)
			{
				transformable->rotate(Vector(0.0f, 5.0f, 0.0f));
			}
			else if (transformModify == TRANSLATE)
			{
				transformable->translate(Vector(-1.0f, 0.0f, 0.0f));
			}

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_RIGHT)
		{
			if (transformModify == SCALE)
			{
				transformable->scale(Vector(1.1f, 1.0f, 1.0f));
			}
			else if (transformModify == ROTATE)
			{
				transformable->rotate(Vector(0.0f, -5.0f, 0.0f));
			}
			else if (transformModify == TRANSLATE)
			{
				transformable->translate(Vector(1.0f, 0.0f, 0.0f));
			}

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_UP)
		{
			if (transformModify == SCALE)
			{
				transformable->scale(Vector(1.0f, 1.0f, 0.9f));
			}
			else if (transformModify == ROTATE)
			{
				transformable->rotate(Vector(5.0f, 0.0f, 0.0f));
			}
			else if (transformModify == TRANSLATE)
			{
				transformable->translate(Vector(0.0f, 0.0f, -1.0f));
			}

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_DOWN)
		{
			if (transformModify == SCALE)
			{
				transformable->scale(Vector(1.0f, 1.0f, 1.1f));
			}
			else if (transformModify == ROTATE)
			{
				transformable->rotate(Vector(-5.0f, 0.0f, 0.0f));
			}
			else if (transformModify == TRANSLATE)
			{
				transformable->translate(Vector(0.0f, 0.0f, 1.0f));
			}

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_PAGE_UP)
		{
			if (transformModify == SCALE)
			{
				transformable->scale(Vector(1.0f, 1.1f, 1.0f));
			}
			else if (transformModify == ROTATE)
			{
				transformable->rotate(Vector(0.0f, 0.0f, -5.0f));
			}
			else if (transformModify == TRANSLATE)
			{
				transformable->translate(Vector(0.0f, 1.0f, 0.0f));
			}

			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_PAGE_DOWN)
		{
			if (transformModify == SCALE)
			{
				transformable->scale(Vector(1.0f, 0.9f, 1.0f));
			}
			else if (transformModify == ROTATE)
			{
				transformable->rotate(Vector(0.0f, 0.0f, 5.0f));
			}
			else if (transformModify == TRANSLATE)
			{
				transformable->translate(Vector(0.0f, -1.0f, 0.0f));
			}

			glutPostRedisplay();
		}
	}
}

void keyBoard(unsigned char key, int x, int y)
{
	if (key == 'q')
	{
		//~ Move camera up
		eyePosition += Vector(0.0f, 1.0f, 0.0f);
		glutPostRedisplay();
	}
	else if (key == 'e')
	{
		//~ Move camera down
		eyePosition += Vector(0.0f, -1.0f, 0.0f);
		glutPostRedisplay();
	}
	else if (key == 'w')
	{
		//~ Move camera forward
		eyePosition += Vector(-sinf(angle * TO_RADIANS), 0.0f, -cosf(angle * TO_RADIANS));
		glutPostRedisplay();
	}
	else if (key == 'a')
	{
		//~ Move camera left
		eyePosition -= Vector(-sinf((angle - 90.0f) * TO_RADIANS), 0.0f, -cosf((angle - 90.0f) * TO_RADIANS));
		glutPostRedisplay();
	}
	else if (key == 's')
	{
		//~ Move camera backward
		eyePosition -= Vector(-sinf(angle * TO_RADIANS), 0.0f, -cosf(angle * TO_RADIANS));
		glutPostRedisplay();
	}
	else if (key == 'd')
	{
		//~ Move camera right
		eyePosition += Vector(-sinf((angle - 90.0f) * TO_RADIANS), 0.0f, -cosf((angle - 90.0f) * TO_RADIANS));
		glutPostRedisplay();
	}
	else if (key == 'z')
	{
		numsamples = numsamples > 1 ? numsamples - 1 : numsamples;
		glutPostRedisplay(); 
		std::cout << "Supersampling (" << numsamples * numsamples << " per pixel" << std::endl;
	}
	else if (key == 'x')
	{
		numsamples += 1;
		glutPostRedisplay(); 
		std::cout << "Supersampling (" << numsamples * numsamples << " per pixel" << std::endl;
	}
	else if (key == 'c')
	{
		PPU = PPU > 1 ? PPU - 1 : PPU;
		glutPostRedisplay(); 
		std::cout << "ppu: " << PPU << std::endl;
	}
	else if (key == 'v')
	{
		PPU += 1;
		glutPostRedisplay(); 
		std::cout << "ppu: " << PPU << std::endl;
	}
	else if (key == 'r')
	{
		samplingType = 0;
		std::cout << "Normal sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == 't')
	{
		samplingType = 1;
		std::cout << "Super sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == 'y')
	{
		samplingType = 2;
		std::cout << "adaptive sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == 'u')
	{
		samplingType = 3;
		std::cout << "stochastic sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == 'o')
	{
		numThreads += 1;
		glutPostRedisplay();
	}
	else if (key == 'l')
	{
		numThreads -= 1;
		glutPostRedisplay();
	}
	else if (key == 'j')
	{
		//~ Rotate left
		angle += 5.0f;
		if (angle > 360.0f)
		{
			angle -= 360.0f;
		}
		glutPostRedisplay();
	}
	else if (key == 'k')
	{
		//~ Rotate right
		angle -= 5.0f;
		if (angle < 360.0f)
		{
			angle += 360.0f;
		}
		glutPostRedisplay();
	}
	else if (key == '1')
	{
		//~ Primitive scene
		sceneNumber = 0;
		initScene(sceneNumber);
		glutPostRedisplay();
	}
	else if (key == '2')
	{
		//~ Transform scene
		sceneNumber = 1;
		initScene(sceneNumber);
		glutPostRedisplay();
	}
	else if (key == '3')
	{
		//~ Textured scene
		sceneNumber = 2;
		initScene(sceneNumber);
		glutPostRedisplay();
	}
	else if (key == '4')
	{
		//~ Multi light scene
		sceneNumber = 3;
		initScene(sceneNumber);
		glutPostRedisplay();
	}
	else if (key == '5')
	{
		//~ reflection scene
		sceneNumber = 4;
		initScene(sceneNumber);
		glutPostRedisplay();
	}
	
}

void initialize()
{
	srand((unsigned int)(time(nullptr)));

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1, 0, 1, 1);

	loadTGA("tex.tga", texture1);
	loadTGA("map.tga", texture2);

	initScene(sceneNumber);
}


int main(int argc, char *argv[]) 
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("mrd66 COSC363 Assignment 2");

	glutKeyboardFunc(keyBoard);
    glutDisplayFunc(display);
	glutSpecialFunc(special);
    initialize();

    glutMainLoop();

    return 0;
}
