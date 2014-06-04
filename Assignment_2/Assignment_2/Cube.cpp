
#include "Cube.h"
#include <math.h>
#include <algorithm>
#include <iostream>

#define EPS 0.01f

/**
* Cube's intersection method.  The input is a ray (pos, dir). 
*/
float Cube::intersect(Vector pos, Vector dir, float *_tmax /*= nullptr*/)
{
	Vector oldDir = dir;

	transformRay(pos, dir);

	float t1,t2,tnear = -1000.0f,tfar = 1000.0f,temp,tCube;

	float b1[3];
	b1[0] = corner1.x;
	b1[1] = corner1.y;
	b1[2] = corner1.z;

	float b2[3];
	b2[0] = corner2.x;
	b2[1] = corner2.y;
	b2[2] = corner2.z;

	float rayDirection[3];
	rayDirection[0] = dir.x;
	rayDirection[1] = dir.y;
	rayDirection[2] = dir.z;

	float rayStart[3];
	rayStart[0] = pos.x;
	rayStart[1] = pos.y;
	rayStart[2] = pos.z;

	bool intersectFlag = true;

	for(int i =0; i < 3; i++)
	{
		if(rayDirection[i] == 0)
		{
			if (rayStart[i] < b1[i] || rayStart[i] > b2[i])
			{
				intersectFlag = false;
			}
		}
		else
		{
			t1 = (b1[i] - rayStart[i])/rayDirection[i];
			t2 = (b2[i] - rayStart[i])/rayDirection[i];

			if(t1 > t2)
			{
				temp = t1;
				t1 = t2;
				t2 = temp;
			}
			if(t1 > tnear)
				tnear = t1;

			if(t2 < tfar)
				tfar = t2;

			if(tnear > tfar)
				intersectFlag = false;

			if(tfar < 0)
				intersectFlag = false;

		}
	}
	if (intersectFlag == false)
	{
		tCube = -1;
	}
	else
	{
		tCube = tnear;
		if (_tmax != nullptr)
		{
			*_tmax = tfar;
		}
	}
 

	return tCube;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the Cube.
*/
Vector Cube::normal(Vector p)
{
	transformRay(p, Vector());

	Vector Min;
	Vector Max;

	Vector n = Vector();

    Min.x = std::min(corner1.x, corner2.x);
    Min.y = std::min(corner1.y, corner2.y);
    Min.z = std::min(corner1.z, corner2.z);
    Max.x = std::max(corner1.x, corner2.x);
    Max.y = std::max(corner1.y, corner2.y);
    Max.z = std::max(corner1.z, corner2.z);

	if (fabsf(p.x - Min.x) < EPS)
	{
		n = Vector(-1.0f, 0.0f, 0.0f);
	}
	if (fabsf(p.x - Max.x) < EPS)
	{
		n =  Vector(+1.0f, 0.0f, 0.0f);
	}

	if (fabsf(p.y - Min.y) < EPS)
	{
		n =  Vector(0.0f, -1.0f, 0.0f);
	}
	if (fabsf(p.y - Max.y) < EPS)
	{
		n =  Vector(0.0f, +1.0f, 0.0f);
	}

	if (fabsf(p.z - Min.z) < EPS)
	{
		n =  Vector(0.0f, 0.0f, -1.0f);
	}
	if (fabsf(p.z - Max.z) < EPS)
	{
		n =  Vector(0.0f, 0.0f, +1.0f);
	}

	transformNormal(n);
	
	return n;
}

Color Cube::getColorTex(Vector _vec)
{
	return color;
}

void Cube::scale(Vector _scaleFactors)
{
	scaleFact.x *= _scaleFactors.x;
	scaleFact.y *= _scaleFactors.y;
	scaleFact.z *= _scaleFactors.z;

	float xSize = 1.0f * scaleFact.x;
	float ySize = 1.0f * scaleFact.y;;
	float zSize = 1.0f * scaleFact.z;;

	corner1 = Vector(-xSize / 2.0f, 
					 -ySize / 2.0f, 
					 -zSize / 2.0f);
	corner2 = Vector(+xSize / 2.0f, 
					 +ySize / 2.0f, 
					 +zSize / 2.0f);
}