/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Plane class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Plane.h"
#include "Vector.h"
#include <math.h>
#include "loadTGA.h"

//Function to test if an input point is within the quad.
bool Plane::isInside(Vector q)
{
	Vector n = normal(q);
	Vector ua = b-a,  ub = c-b, uc = d-c, ud = a-d;
	Vector va = q-a,  vb = q-b, vc = q-c, vd = q-d;
	//Complete this function

	return ((ua.cross(va).dot(n) > 0.0f) &&
		    (ub.cross(vb).dot(n) > 0.0f) &&
			(uc.cross(vc).dot(n) > 0.0f) &&
			(ud.cross(vd).dot(n) > 0.0f));

}

//Function to compute the paramter t at the point of intersection.
float Plane::intersect(Vector pos, Vector dir)
{
	Vector oldDir = dir;

	transformRay(pos, dir);

	Vector n = normal(pos);
	Vector vdif = a-pos;
	float vdotn = dir.dot(n);
	if(fabs(vdotn) < 1.e-4) return -1;
    float t = vdif.dot(n)/vdotn;
	if(fabs(t) < 0.0001) return -1;
	Vector q = pos + dir*t;
	if (isInside(q))
	{
		return t;
	}
    else return -1;
}

// Function to compute the unit normal vector
// Remember to output a normalised vector!
Vector Plane::normal(Vector pos)
{
	transformRay(pos, Vector());

	Vector normal = (b - a).cross(c - a);

	normal.normalise();

	transformNormal(normal);

	return normal;
}

Color Plane::getColorTex(Vector _vec)
{
	float x = (_vec.x + scaleFact.x / 2.0f) / (scaleFact.x);
	float z = (_vec.z + scaleFact.z / 2.0f) / (scaleFact.z);


	Vector col = texture->imgData.at((int)(z * (texture->height - 1)) * texture->width + (int)(x * (texture->width - 1)));

	return Color(col.x, col.y, col.z);
}

void Plane::scale(Vector _scaleFactors)
{
	scaleFact.x *= _scaleFactors.x;
	scaleFact.y *= _scaleFactors.y;
	scaleFact.z *= _scaleFactors.z;

	float xSize = 1.0f * scaleFact.x;
	float ySize = 1.0f * scaleFact.y;
	float zSize = 1.0f * scaleFact.z;

	a = Vector(-xSize / 2.0f, 0.0f, +zSize / 2.0f);
	b = Vector(+xSize / 2.0f, 0.0f, +zSize / 2.0f);
	c = Vector(+xSize / 2.0f, 0.0f, -zSize / 2.0f);
	d = Vector(-xSize / 2.0f, 0.0f, -zSize / 2.0f);
}