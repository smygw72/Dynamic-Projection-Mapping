#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "head.h"
#include <windows.h>
#include <gl/gl.h>

typedef struct _BitmapHeader {
	char    distinct1;
	char    distinct2;
	int             filesize;
	short   reserve1;
	short   reserve2;
	int             offset;
}BitmapHeader;

typedef struct _BitmapInfoHeader {
	int             header;
	int             width;
	int             height;
	short   plane;
	short   bits;
	int             compression;
	int             comp_image_size;
	int             x_resolution;
	int             y_resolution;
	int             pallet_num;
	int             important_pallet_num;
}BitmapInfoHeader;

int WriteBitmap(const char*, int width, int height);