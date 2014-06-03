/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Plane class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#ifndef H_PLANE
#define H_PLANE

#include "Vector.h"
#include "Object.h"

class Plane : public Object
{
private:
    Vector a, b, c, d;      //The 4 vertices of a quad

	Vector scaleFact;

public:	
	Plane(void);
	
    Plane(Color col)
	{
		scaleFact = Vector(1.0f, 1.0f, 1.0f);

		scale(scaleFact);

		color = col;
	};

	
	virtual Color getColorTex(Vector _vec);

	bool isInside(Vector pos);
	
	float intersect(Vector pos, Vector dir);
	
	Vector normal(Vector pos);

	
	virtual void scale(Vector _scaleFactors);

};

#endif //!H_PLANE
