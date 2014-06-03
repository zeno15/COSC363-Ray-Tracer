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

#define MAX_THREADS			4

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

Vector eyePosition = Vector(0.0f, 0.0f, 80.0f);

tex texture1;
tex texture2;
tex texture3;


bool showFog = false;

unsigned int numsamples = 1;

unsigned int samplingType = 0;

float colorCompareThreshold = 0.05f;

int width, height;

int numThreads = 1;

char *texdata;


/*
* This function compares the given ray with all objects in the scene
* and computes the closest point  of intersection.
*/
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

/*
* Computes the colour value obtained by tracing a ray.
* If reflections and refractions are to be included, then secondary rays will 
* have to be traced from the point, by converting this method to a recursive
* procedure.
*/

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

			if (shadow.dist > q.point.dist(lights.at(i))) continue;

			if (_sceneObjects.at(shadow.index)->getRefractive()) continue; //~ Assuming refractive objects dont leave shadows

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
	else if (_sceneObjects.at(q.index)->getRefractive() && step < MAX_STEPS)
	{
		float coeffOfTransmission = 0.8f;

		float refractionRatio = _currentRefractiveIndex / _currentRefractiveIndex;

		float cosRefractionAngle = sqrtf(1.0f - std::pow(refractionRatio, 2.0f) * (1.0f - std::pow(dir.dot(n), 2.0f)));

		Vector refractionVector = dir - n * (refractionRatio * dir.dot(n) + cosRefractionAngle) * refractionRatio;

		float t = 0.0f;

		float refractiveIndexToUse = 0.0f;

		if (n.dot(dir) > 0.0f)
		{
			refractiveIndexToUse = 1.0f;
		}
		else
		{
			refractiveIndexToUse = _currentRefractiveIndex;
		}

		Color refractionColor = trace(q.point, refractionVector, step + 1, t, _sceneObjects, refractiveIndexToUse);

		colorSum.combineColor(refractionColor, coeffOfTransmission);
	}

	

	return colorSum;
}

void adaptiveSample(float _cx, float _cy, float _sizeX, float _sizeY, int step, std::vector<pixelDraw> &_outputVector, std::vector<Object *> _sceneObjects)
{
	float t = 0.0f;
	bool valid = true;

	Vector dir = Vector(_cx, _cy, -EDIST);
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

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each pixel as quads.
//---------------------------------------------------------------------------------------
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
		eyePosition += Vector(0.0f, 0.0f, -1.0f);
		glutPostRedisplay();
	}
	else if (key == 'a')
	{
		//~ Move camera left
		eyePosition += Vector(-1.0f, 0.0f, 0.0f);
		glutPostRedisplay();
	}
	else if (key == 's')
	{
		//~ Move camera backward
		eyePosition += Vector(0.0f, 0.0f, 1.0f);
		glutPostRedisplay();
	}
	else if (key == 'd')
	{
		//~ Move camera right
		eyePosition += Vector(1.0f, 0.0f, 0.0f);
		glutPostRedisplay();
	}
	else if (key == 'b')
	{
		std::cout << "Toggling fog/smoke" << std::endl;
		showFog = !showFog;
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
	else if (key == '0')
	{
		samplingType = 0;
		std::cout << "Normal sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == '1')
	{
		samplingType = 1;
		std::cout << "Super sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == '2')
	{
		samplingType = 2;
		std::cout << "adaptive sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == '3')
	{
		samplingType = 3;
		std::cout << "stochastic sampling" << std::endl;
		glutPostRedisplay(); 
	}
	else if (key == 'r')
	{
		colorCompareThreshold *= 10.0f;
		std::cout << "Increasing color compare threshold: " << colorCompareThreshold << std::endl;
		glutPostRedisplay();
	}
	else if (key == 't')
	{
		colorCompareThreshold /= 10.0f;
		std::cout << "Decreasing color compare threshold: " << colorCompareThreshold << std::endl;
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
	loadTGA("bumpmap.tga", texture3);

	lights.push_back(Vector(0.0f, 100.0f, 100.0f));

	Sphere *sphere1 = new Sphere(Color::BLUE);
	sphere1->translate(Vector(15.0f, 8.0f, -10.0f));
	sphere1->scale(Vector(8.0f, 8.0f, 8.0f));
	sphere1->setReflective(true);

	Sphere *sphere2= new Sphere(Color::WHITE);
	sphere2->translate(Vector(0.0f, 15.0f, 10.0f));
	sphere2->scale(Vector(10.0f, 10.0f, 10.0f));
	sphere2->setRefractive(true, 1.5f);


	Plane *plane1 = new Plane(Color::RED);
	plane1->scale(Vector(100.0f, 1.0f, 100.0f));
	plane1->setTexture(&texture1);

	


	sceneObjectsThread1.push_back(plane1);
	sceneObjectsThread1.push_back(sphere1);
	sceneObjectsThread1.push_back(sphere2);

	for (unsigned int i = 0; i < sceneObjectsThread1.size(); i += 1)
	{
		sceneObjectsThread2.push_back(sceneObjectsThread1.at(i));
		sceneObjectsThread3.push_back(sceneObjectsThread1.at(i));
		sceneObjectsThread4.push_back(sceneObjectsThread1.at(i));
	}

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
    initialize();

    glutMainLoop();

    return 0;
}
