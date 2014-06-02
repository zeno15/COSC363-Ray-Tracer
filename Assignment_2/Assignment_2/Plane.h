/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Plane class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#ifndef H_PLANE
#define H_PLANE

#define RATIO 100.0f

#include "Vector.h"
#include "Object.h"

class Plane : public Object
{
private:
    Vector a, b, c, d;      //The 4 vertices of a quad

public:	
	Plane(void);
	
    Plane(Color col)
	{
		a = Vector(-RATIO / 2.0f, 0.0f, +RATIO / 2.0f);
		b = Vector(+RATIO / 2.0f, 0.0f, +RATIO / 2.0f);
		c = Vector(+RATIO / 2.0f, 0.0f, -RATIO / 2.0f);
		d = Vector(-RATIO / 2.0f, 0.0f, -RATIO / 2.0f);

		color = col;
	};

	
	virtual Color getColorTex(Vector _vec);

	bool isInside(Vector pos);
	
	float intersect(Vector pos, Vector dir);
	
	Vector normal(Vector pos);

};

#endif //!H_PLANE
