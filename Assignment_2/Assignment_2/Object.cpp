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
	transformation = glm::translate(transformation, glm::vec3(_vec.x, _vec.y, _vec.z));

	translatemat = glm::translate(translatemat, glm::vec3(_vec.x, _vec.y, _vec.z));
}

void Object::transformRay(Vector &_RayOrigin, Vector &_RayDirection)
{
	glm::mat4x4 transformationInv = glm::inverse(transformation);

	glm::vec4 newOri = transformationInv * glm::vec4(_RayOrigin.x, _RayOrigin.y, _RayOrigin.z, 1.0f);

	//glm::mat4x4 invScaling = glm::inverse(scaling);

	

	glm::vec4 newDir = scaling * transformationInv * glm::vec4(_RayDirection.x, _RayDirection.y, _RayDirection.z, 0.0f);

	_RayOrigin = Vector(newOri.x, newOri.y, newOri.z);
	_RayDirection = Vector(newDir.x, newDir.y, newDir.z);

	_RayDirection.normalise();
}
void Object::transformt(float &_t, Vector _originalDirection, Vector _newDirection)
{

}

void Object::transformNormal(Vector &_Normal)
{
	glm::mat4x4 transformationInvTranspose = glm::inverseTranspose(transformation);

	glm::vec4 newNormal = transformation * glm::vec4(_Normal.x, _Normal.y, _Normal.z, 0.0f);

	_Normal = Vector(newNormal.x, newNormal.y, newNormal.z);

	_Normal.normalise();
}

void Object::rotate(Vector _angles)
{
	transformation = glm::rotate(transformation, _angles.x, glm::vec3(1, 0, 0));
	transformation = glm::rotate(transformation, _angles.y, glm::vec3(0, 1, 0));
	transformation = glm::rotate(transformation, _angles.z, glm::vec3(0, 0, 1));

	rotation = glm::rotate(rotation, _angles.x, glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, _angles.y, glm::vec3(0, 1, 0));
	rotation = glm::rotate(rotation, _angles.z, glm::vec3(0, 0, 1));


}

void Object::scale(Vector _scaleFactors)
{
	transformation = glm::scale(transformation, glm::vec3(_scaleFactors.x, _scaleFactors.y, _scaleFactors.z));

	scaling = glm::scale(scaling, glm::vec3(_scaleFactors.x, _scaleFactors.y, _scaleFactors.z));
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