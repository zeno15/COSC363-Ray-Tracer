// ========================================================================
// COSC 363  Computer Graphics  Lab07
// A simple ray tracer
// ========================================================================

#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <time.h>
#include "Vector.h"
#include "Sphere.h"
#include "Plane.h"
#include "Cube.h"
#include "Color.h"
#include "Object.h"
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ParticipatingMediaSystem.hpp"
#include "loadTGA.h"

#include "RayTracer.hpp"

using namespace std;

const float WIDTH = 40.0f;  
const float HEIGHT = 40.0f;
const float EDIST = 40.0f; 
int PPU = 4;     
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5f;
const float XMAX =  WIDTH * 0.5f;
const float YMIN = -HEIGHT * 0.5f;
const float YMAX =  HEIGHT * 0.5f;

#define REFLECTIVE_INDEX	-5			//~ Change to the index in the sceneobject vector of the reflective object
#define TO_RADIANS			(3.14159265f / 180.0f)
#define ANGLE_CHANGE		5.0f

#define NORMAL_SAMPLING		0
#define SUPER_SAMPLING		1
#define ADAPTIVE_SAMPLING	2
#define STOCHASTIC_SAMPLING 3

vector<Object*> sceneObjects;
vector<Object*> extraObjects;

vector<Vector> lights;

Vector eyePosition = Vector(0.0f, 0.0f, 80.0f);

tex texture1;
tex texture2;


bool showFog = false;

unsigned int numsamples = 1;

unsigned int samplingType = 0;

float colorCompareThreshold = 0.05f;

int width, height;

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

Color trace(Vector pos, Vector dir, int step, float &_t, std::vector<Object*> &_sceneObjects)
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

		Color reflectionColor = trace(q.point, reflectionVector, step + 1, t, _sceneObjects);

		colorSum.combineColor(reflectionColor, 0.8f);
	}

	

	return colorSum;
}

void adaptiveSample(float _cx, float _cy, float _sizeX, float _sizeY, int step)
{
	float t = 0.0f;
	bool valid = true;

	Vector dir = Vector(_cx, _cy, -EDIST);
	dir.normalise();

	Color col = trace(eyePosition, dir, 1, t, sceneObjects);


	dir = Vector(_cx - _sizeX / 2.0f, _cy - _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color tl = trace(eyePosition, dir, 1, t, sceneObjects);

	dir = Vector(_cx + _sizeX / 2.0f, _cy - _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color tr = trace(eyePosition, dir, 1, t, sceneObjects);

	dir = Vector(_cx + _sizeX / 2.0f, _cy + _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color br = trace(eyePosition, dir, 1, t, sceneObjects);

	dir = Vector(_cx - _sizeX / 2.0f, _cy + _sizeY / 2.0f, -EDIST);
	dir.normalise();
	Color bl = trace(eyePosition, dir, 1, t, sceneObjects);

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


	if (valid || step > (int)numsamples)
	{
		glColor3f(col.r, col.g, col.b);
		glVertex2f(_cx - _sizeX / 2.0f, _cy - _sizeY / 2.0f);
		glVertex2f(_cx + _sizeX / 2.0f, _cy - _sizeY / 2.0f);
		glVertex2f(_cx + _sizeX / 2.0f, _cy + _sizeY / 2.0f);
		glVertex2f(_cx - _sizeX / 2.0f, _cy + _sizeY / 2.0f);
	}
	else
	{
		adaptiveSample(_cx - _sizeX / 4.0f, _cy - _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1);
		adaptiveSample(_cx + _sizeX / 4.0f, _cy - _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1);
		adaptiveSample(_cx + _sizeX / 4.0f, _cy + _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1);
		adaptiveSample(_cx - _sizeX / 4.0f, _cy + _sizeY / 4.0f, _sizeX / 2.0f, _sizeY / 2.0f, step + 1);
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
	float halfPixelSize = pixelSize/2.0f;
	float x1, y1, xc, yc;
	
	if (samplingType == NORMAL_SAMPLING)
	{
		numsamples = 1;
		samplingType = SUPER_SAMPLING;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);  //Each pixel is a quad.

	float t = 0.0f;

	for(int i = 0; i < widthInPixels; i++)	//Scan every "pixel"
	{
		x1 = XMIN + i*pixelSize;
		xc = x1 + halfPixelSize;
		for(int j = 0; j < heightInPixels; j++)
		{
			y1 = YMIN + j*pixelSize;
			yc = y1 + halfPixelSize;

			if (samplingType == SUPER_SAMPLING)
			{
				Color col = Color::BLACK;

				for (unsigned int k = 0; k < numsamples; k += 1)
				{
					float xUsed = x1 + ((float)(k)+0.5f) / (float)(numsamples) * pixelSize;

					for (unsigned int l = 0; l < numsamples; l += 1)
					{
						float yUsed = y1 + ((float)(l)+0.5f) / (float)(numsamples) * pixelSize;

						Vector dir = Vector(xUsed, yUsed, -EDIST);

						dir.normalise();

						col.combineColor(trace(eyePosition, dir, 1, t, sceneObjects), 1.0f / ((float)(numsamples * numsamples)));
					}
				}

				glColor3f(col.r, col.g, col.b);
				glVertex2f(x1, y1);				//Draw each pixel with its color value
				glVertex2f(x1 + pixelSize, y1);
				glVertex2f(x1 + pixelSize, y1 + pixelSize);
				glVertex2f(x1, y1 + pixelSize);
			}
			else if (samplingType == ADAPTIVE_SAMPLING)
			{
				adaptiveSample(xc, yc, pixelSize, pixelSize, 1);
				if (j == 0)
				{
					if (i % (widthInPixels / 20) == 0)
					{
						std::cout << 100.0f * (float)(i) / (float)(widthInPixels) << "%" << std::endl;
					}
				}
			}
			else if (samplingType == STOCHASTIC_SAMPLING)
			{
				Color col = Color::BLACK;

				for (unsigned int k = 0; k < numsamples; k += 1)
				{
					float x = x1 + (float)(rand() % 1000) / 1000.0f * pixelSize;
					float y = y1 + (float)(rand() % 1000) / 1000.0f * pixelSize;

					Vector dir = Vector(x, y, -EDIST);

					dir.normalise();

					col.combineColor(trace(eyePosition, dir, 1, t, sceneObjects), 1.0f / ((float)(numsamples)));
					
				}

				glColor3f(col.r, col.g, col.b);
				glVertex2f(x1, y1);				//Draw each pixel with its color value
				glVertex2f(x1 + pixelSize, y1);
				glVertex2f(x1 + pixelSize, y1 + pixelSize);
				glVertex2f(x1, y1 + pixelSize);
			}

        }

		
		std::cout << (float)(i) / (float)(widthInPixels)* 100.0f << "%" << std::endl;
		
    }

    glEnd();
    glFlush();

	std::cout << "Redrawn" << std::endl;
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

	lights.push_back(Vector(0.0f, 10.0f, 100.0f));

	Sphere *sphere1 = new Sphere(Color::BLUE);
	sphere1->translate(Vector(0.0f, 10.0f, 0.0f));
	sphere1->rotate(Vector(0.0, 0.0, 0.0));
	sphere1->scale(Vector(1.0f, 1.0f, 1.0f));
	sphere1->setTexture(&texture2);
	Cube *cube = new Cube(Color::BLUE);
	//cube->translate(Vector(20.0f, 10.0f, 0.0f));
	cube->rotate(Vector(45.0f, 45.0f, 45.0f));
	Plane *plane1 = new Plane(Color(0, 1, 1));
	plane1->setTexture(&texture1);
	//plane1->translate(Vector(0.0, -10.0, 0.0));
	//plane1->scale(Vector(80.0f, 1.0f, 80.0f));


	sceneObjects.push_back(sphere1);

	sceneObjects.push_back(cube);	

	sceneObjects.push_back(plane1);

}


int main(int argc, char *argv[]) 
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Raytracing");

	glutKeyboardFunc(keyBoard);
    glutDisplayFunc(display);
    initialize();

    glutMainLoop();

    return 0;
}
