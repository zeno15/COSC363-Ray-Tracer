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
float Sphere::intersect(Vector pos, Vector dir, float *_tmax /*= nullptr*/)
{
	transformRay(pos, dir);

	float a = std::pow(dir.x, 2.0f) / std::pow(radii.x, 2.0f) + std::pow(dir.y, 2.0f) / std::pow(radii.y, 2.0f) + std::pow(dir.z, 2.0f) / std::pow(radii.z, 2.0f);
	float b = 2.0f * (pos.x * dir.x / std::pow(radii.x, 2.0f) + pos.y * dir.y / std::pow(radii.y, 2.0f) + pos.z * dir.z / std::pow(radii.z, 2.0f));
	float c = std::pow(pos.x, 2.0f) / std::pow(radii.x, 2.0f) + std::pow(pos.y, 2.0f) / std::pow(radii.y, 2.0f) + std::pow(pos.z, 2.0f) / std::pow(radii.z, 2.0f) - 1.0f;


	if (b * b  - 4.0f * a * c < 0.0f)
	{
		return -1.0f;
	}

	float t1 = (-b + sqrtf(b * b  - 4.0f * a * c)) / (2.0f * a);
	float t2 = (-b - sqrtf(b * b  - 4.0f * a * c)) / (2.0f * a);

	float t = t1 > 0.0f ? (t2 > 0.0f ? (t1 < t2 ? t1 : t2) : -1.0f) : -1.0f;

	if (_tmax != nullptr)
	{
		if (t == -1.0f)
		{
			*_tmax = -1.0f;
		}
		else if (t == t1)
		{
			*_tmax = t2;
		}
		else if (t == t2)
		{
			*_tmax = t1;
		}
	}

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

void Sphere::scale(Vector _scaleFactors)
{
	scaleFact.x *= _scaleFactors.x;
	scaleFact.y *= _scaleFactors.y;
	scaleFact.z *= _scaleFactors.z;

	float xSize = 1.0f * scaleFact.x;
	float ySize = 1.0f * scaleFact.y;
	float zSize = 1.0f * scaleFact.z;

	radii.x = xSize;
	radii.y = ySize;
	radii.z = zSize;
}