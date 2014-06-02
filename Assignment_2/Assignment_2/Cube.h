#ifndef H_CUBE
#define H_CUBE

#include "Object.h"

//~ Axis aligned cube object

class Cube : public Object
{

private:
	Vector corner1;
	Vector corner2;

public:	
    Cube(Color _col)
	{
		color = _col;

		corner1 = Vector(-1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f);
		corner2 = Vector(+1.0f / 2.0f, +1.0f / 2.0f, +1.0f / 2.0f);
	};

	float intersect(Vector pos, Vector dir);

	Vector normal(Vector p);

	virtual Color getColorTex(Vector _vec);

};

#endif //!H_CUBE
