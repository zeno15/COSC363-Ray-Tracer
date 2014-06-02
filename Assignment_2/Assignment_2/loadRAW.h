//======================================================================
// LoadRAW.h
// Image loader for files in RAW format.
// Assumption:  Colour image in RGB format (24 bpp) in interleaved order.
// Note: RAW format stores an image in top to bottom order, and will
//       appear vertically flipped.
// Author:
// R. Mukundan, Department of Computer Science and Software Engineering
// University of Canterbury, Christchurch, New Zealand.
//======================================================================
#if !defined(H_RAW)
#define H_RAW

#include <iostream>
#include <fstream>
using namespace std;

void loadRAW(string filename, int width, int height)
{
    char* imageData;
    int size;
    ifstream file( filename.c_str(), ios::in | ios::binary);
	if(!file)
	{
		cout << "*** Error opening image file: " << filename.c_str() << endl;
		exit(1);
	}

	size = width * height * 3;  //Total number of bytes to be read
	imageData = new char[size];
	file.read(imageData, size);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    delete imageData;	         	         
}


#endif

