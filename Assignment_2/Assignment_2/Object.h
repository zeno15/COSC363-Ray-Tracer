/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The object class
*  This is a generic type for storing objects in the scene.
*  Being an abstract class, this class cannot be instantiated.
*  Sphere, Plane etc, must be defined as subclasses of Object
*      and provide implementations for the virtual functions
*      intersect()  and normal().
-------------------------------------------------------------*/

#ifndef H_OBJECT
#define H_OBJECT

#include "Vector.h"
#include "Color.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform2.hpp>

struct tex;

class Object 
{
protected:
	Color color;

	tex * texture;

	glm::mat4x4 scaling;
	glm::mat4x4 translation;
	glm::mat4x4 rotation;

	bool reflective;
	bool refractive;
	float refractiveIndex;

	void transformRay(Vector &_RayOrigin, Vector &_RayDirection);
	void transformNormal(Vector &_Normal);

public:
	Object() :
	texture(nullptr){
		translation = glm::mat4x4(1.0f);
		rotation = glm::mat4x4(1.0f);
		reflective = false;
		refractive = false;

	}
    virtual float intersect(Vector pos, Vector dir, float *_tmax = nullptr) = 0;
	virtual Vector normal(Vector pos) = 0;
	virtual ~Object() {}
	Color getColor();

	virtual Color getColorTex(Vector _vec) = 0;

	void setColor(Color col);

	void translate(Vector _vec);

	void rotate(Vector _angles);

	virtual void scale(Vector _scaleFactors) = 0;

	void setTexture(tex *_tex);

	bool hasTexture(void);

	bool getReflective();
	bool getRefractive();
	float getRefractiveIndex();
	void setReflective(bool _value);
	void setRefractive(bool _value, float _index);
};

#endif