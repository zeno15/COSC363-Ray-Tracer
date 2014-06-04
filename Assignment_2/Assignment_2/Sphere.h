/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#ifndef H_SPHERE
#define H_SPHERE

#include "Object.h"

/**
 * Defines a simple Sphere located at 'center' 
 * with the specified radius
 */
class Sphere : public Object
{

private:
    Vector center;
	Vector radii;

	Vector scaleFact;

public:		
    Sphere(Color col)
		: center(Vector()), radii(Vector(1.0f, 1.0f, 1.0f))
	{
		color = col;
		scaleFact = Vector(1.0f, 1.0f, 1.0f);

		scale(scaleFact);
	};

	float intersect(Vector pos, Vector dir, float *_tmax = nullptr);

	Vector normal(Vector p);

	virtual Color getColorTex(Vector _vec);

	virtual void scale(Vector _scaleFactors);

};

#endif //!H_SPHERE
