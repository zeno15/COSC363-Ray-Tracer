/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Sphere.h"
#include <math.h>
#include <iostream>
#include "loadTGA.h"

/**
* Sphere's intersection method.  The input is a ray (pos, dir). 
*/
float Sphere::intersect(Vector pos, Vector dir)
{
	Vector oldDir = dir;

	transformRay(pos, dir);

    Vector vdif = pos - center;
    float b = dir.dot(vdif);
    float len = vdif.length();
    float c = len*len - radius*radius;
    float delta = b*b - c;
   
	if(fabs(delta) < 0.001) return -1.0; 
    if(delta < 0.0) return -1.0;

    float t1 = -b - sqrt(delta);
    float t2 = -b + sqrt(delta);
    if(fabs(t1) < 0.001 )
    {
		if (t2 > 0)
		{
			//~ modify t
			return t2;
		}
		else
		{
			t1 = -1.0;
		}
    }
	if (fabs(t2) < 0.001)
	{
		t2 = -1.0;
	}

	float t = (t1 < t2)? t1: t2;

	//~ modify t

	return t;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
Vector Sphere::normal(Vector p)
{
	transformRay(p, Vector());

    Vector n = p - center;
    n.normalise();
	transformNormal(n);
    return n;
}

Color Sphere::getColorTex(Vector _vec)
{
	//return color;

	transformRay(_vec, Vector());

	Vector vn = Vector(0.0, 1.0, 0.0);
	Vector ve = Vector(1.0, 0.0, 0.0);

	Vector vp = _vec;

	vp.normalise();

	float phi = acosf(-vn.dot(vp));

	float v = phi / 3.14159f;
	float u;

	float sinphi = sin(phi);

	if (sinphi == 0.0f)
	{
		sinphi = 0.1f;
	}

	float theta = (acosf(vp.dot(ve) / sinphi)) / (2.0f * 3.14159f);

	if (vn.cross(ve).dot(vp) > 0)
	{
		u = theta;
	}
	else
	{
		u = 1.0f - theta;
	}

	u = u > 1.0f ? 1.0f : (u < 0.0f ? 0.0f : u);
	v = v > 1.0f ? 1.0f : (v < 0.0f ? 0.0f : v);

	u *= (texture->width - 1);
	v *= (texture->height - 1);

	//std::cout << "U: " << static_cast<int>(u) << ", V: " << static_cast<int>(v) << std::endl;

	if (static_cast<int>(u) < 0)
	{
		u = 0;
	}

	Vector col = texture->imgData.at((int)(v) * texture->width + (int)(u));

	return Color(col.x, col.y, col.z);
}