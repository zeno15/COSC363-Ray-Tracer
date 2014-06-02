#include "loadTGA.h"

void loadTGA(string filename, tex &_textureObject)
{
    char id, cmap, imgtype, bpp, c_garb;
    char* imageData, temp;
    short int s_garb, wid, hgt;
    int nbytes, size, indx;
    ifstream file( filename.c_str(), ios::in | ios::binary);
	if(!file)
	{
		cout << "*** Error opening image file: " << filename.c_str() << endl;
		exit(1);
	}
	file.read (&id, 1);
	file.read (&cmap, 1);
	file.read (&imgtype, 1);	
	if(imgtype != 2 && imgtype != 3 )   //2= colour (uncompressed),  3 = greyscale (uncompressed)
	{
		cout << "*** Incompatible image type: " << (int)imgtype << endl;
		exit(1);
	}
	//Color map specification
	file.read ((char*)&s_garb, 2);
	file.read ((char*)&s_garb, 2);
	file.read (&c_garb, 1);
	//Image specification
	file.read ((char*)&s_garb, 2);  //x origin
	file.read ((char*)&s_garb, 2);  //y origin
	file.read ((char*)&wid, 2);     //image width								    
	file.read ((char*)&hgt, 2);     //image height
	file.read (&bpp, 1);     //bits per pixel
	file.read (&c_garb, 1);  //img descriptor
	nbytes = bpp / 8;           //No. of bytes per pixels
	size = wid * hgt * nbytes;  //Total number of bytes to be read
	imageData = new char[size];
	file.read(imageData, size);
	if(nbytes > 2)   //swap R and B
	{
	    for(int i = 0; i < wid*hgt;  i++)
	    {
	        indx = i*nbytes;
	        temp = imageData[indx];
	        imageData[indx] = imageData[indx+2];
	        imageData[indx+2] = temp;
        }
    }

	_textureObject.width = wid;
	_textureObject.height = hgt;
	
	for (unsigned int i = 0; i < (unsigned int)(size / 3); i += 1)
	{
		int r = static_cast<int>(imageData[i * 3 + 0]);
		int g = static_cast<int>(imageData[i * 3 + 1]);
		int b = static_cast<int>(imageData[i * 3 + 2]);

		r = r < 0 ? 255 : r;
		g = g < 0 ? 255 : g;
		b = b < 0 ? 255 : b;

		_textureObject.imgData.push_back(Vector((float)(r) / 255.0f, (float)(g) / 255.0f, (float)(b) / 255.0f));
	}
	
	delete imageData;	         	         
}