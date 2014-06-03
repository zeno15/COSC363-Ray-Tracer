/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The object class
*  This is a generic type for storing objects in the scene
*  Sphere, Plane etc. must be defined as subclasses of Object.
*  Being an abstract class, this class cannot be instantiated.
-------------------------------------------------------------*/

#include "Object.h"

#include "loadTGA.h"

Color Object::getColor()
{
	return color;
}

void Object::setColor(Color col)
{
	color = col;
}

void Object::translate(Vector _vec)
{
	translation = glm::translate(translation, glm::vec3(_vec.x, _vec.y, _vec.z));
}

void Object::transformRay(Vector &_RayOrigin, Vector &_RayDirection)
{
	glm::mat4x4 transform = glm::mat4x4(1.0f) * translation * rotation;

	glm::mat4x4 transformationInv = glm::inverse(transform);

	glm::vec4 newOri = transformationInv * glm::vec4(_RayOrigin.x, _RayOrigin.y, _RayOrigin.z, 1.0f);

	glm::vec4 newDir = transformationInv * glm::vec4(_RayDirection.x, _RayDirection.y, _RayDirection.z, 0.0f);

	_RayOrigin = Vector(newOri.x, newOri.y, newOri.z);
	_RayDirection = Vector(newDir.x, newDir.y, newDir.z);
}

void Object::transformNormal(Vector &_Normal)
{
	glm::mat4x4 transform = glm::mat4x4(1.0f) * translation * rotation;

	glm::mat4x4 transformationInvTranspose = glm::inverseTranspose(transform);

	glm::vec4 newNormal = transformationInvTranspose * glm::vec4(_Normal.x, _Normal.y, _Normal.z, 0.0f);

	_Normal = Vector(newNormal.x, newNormal.y, newNormal.z);

	_Normal.normalise();
}

void Object::rotate(Vector _angles)
{

	rotation = glm::rotate(rotation, _angles.x, glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, _angles.y, glm::vec3(0, 1, 0));
	rotation = glm::rotate(rotation, _angles.z, glm::vec3(0, 0, 1));


}


void Object::setTexture(tex *_tex)
{
	texture = _tex;
}

bool Object::hasTexture(void)
{
	return !(texture == nullptr);
}

bool Object::getReflective()
{
	return reflective;
}
bool Object::getRefractive()
{
	return refractive;
}
float Object::getRefractiveIndex()
{
	return refractiveIndex;
}
void Object::setReflective(bool _value)
{
	reflective = _value;
}
void Object::setRefractive(bool _value, float _index)
{
	refractive = _value;
	refractiveIndex = _index;
}