//=====================================================================
// LoadTGA.h
// Image loader for files in TGA format.
// Assumption:  Uncompressed data.
//
// Author:
// R. Mukundan, Department of Computer Science and Software Engineering
// University of Canterbury, Christchurch, New Zealand.
//=====================================================================
#if !defined(H_TGA)
#define H_TGA

#include <vector>
#include <iostream>
#include <fstream>
#include <GL/freeglut.h>
#include "Vector.h"

using namespace std;

struct tex {
	int width;
	int height;
	std::vector<Vector> imgData;
};

void loadTGA(string filename, tex &_textureObject);

#endif

