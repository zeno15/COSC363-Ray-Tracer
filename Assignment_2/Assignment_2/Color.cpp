/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The color class
*  A simple colour class with a set of operations including
*    phong lighting.
-------------------------------------------------------------*/
#include "Color.h"

#include <cmath>
#include <iostream>
#include <algorithm>

//Multiplies the current colour by a scalar factor
void Color::scaleColor(float scaleFactor) 
{
    r = r * scaleFactor;
    g = g * scaleFactor;
    b = b * scaleFactor;
}

//Modulates the current colour by a given colour
void Color::combineColor(Color col)
{
    r *= col.r;
    g *= col.g;
    b *= col.b;
}

//Adds a scaled version of a colour to the current colour
void Color::combineColor(Color col, float scaleFactor)
{
    r +=  scaleFactor * col.r;
    g +=  scaleFactor * col.g;
    b +=  scaleFactor * col.b;
}

//~ Returns true if the two colours differ, used in adaptive anti aliasing
bool Color::compare(Color col, float threshold)
{
	float minR = std::min(r, col.r);
	float maxR = std::max(r, col.r);
	float minG = std::min(g, col.g);
	float maxG = std::max(g, col.g);
	float minB = std::min(b, col.b);
	float maxB = std::max(b, col.b);

	float Cr = (maxR - minR) / (maxR + minR);
	float Cg = (maxG - minG) / (maxG + minG);
	float Cb = (maxB - minB) / (maxB + minB);

	

	if (std::max(std::max(Cr, Cg), Cb) > threshold)
	{
		return true;
	}

	return false;
}

//Phong lighting equations:
// Input:  Light's ambient color, l.n,  (r.v)^f
// Assumptions:
//   Material ambient = Material diffuse = current color
//   Material specular = white
//   Light diffuse = Light specular = white

Color Color::phongLight(Color ambientCol, float diffTerm, float specTerm)
{
	Color col;
    col.r = (ambientCol.r) * r + diffTerm * r + specTerm;
    col.g = (ambientCol.g) * g + diffTerm * g + specTerm;
    col.b = (ambientCol.b) * b + diffTerm * b + specTerm;
	return col;
}

const Color Color::WHITE = Color(1, 1, 1);
const Color Color::BLACK = Color(0, 0, 0);
const Color Color::RED = Color(1, 0, 0);
const Color Color::GREEN = Color(0, 1, 0);
const Color Color::CYAN = Color(0, 1, 1);
const Color Color::BLUE = Color(0, 0, 1);
const Color Color::GRAY = Color(0.2f, 0.2f, 0.2f);
const Color Color::YELLOW = Color(1.0f, 1.0f, 0.2f);