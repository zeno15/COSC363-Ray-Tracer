#ifndef H_CUBE
#define H_CUBE

#include "Object.h"

//~ Axis aligned cube object

class Cube : public Object
{

private:
	Vector corner1;
	Vector corner2;

	Vector scaleFact;

public:	
    Cube(Color _col)
	{
		color = _col;
		scaleFact = Vector(1.0f, 1.0f, 1.0f);

		scale(Vector(1.0f, 1.0f, 1.0f));
	};

	float intersect(Vector pos, Vector dir);

	Vector normal(Vector p);

	virtual Color getColorTex(Vector _vec);

	virtual void scale(Vector _scaleFactors);

};

#endif //!H_CUBE
