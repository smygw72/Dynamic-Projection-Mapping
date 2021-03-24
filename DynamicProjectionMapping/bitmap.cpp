#include <iostream>
#include "Bitmap.h"
#pragma warning(disable:4996)

using namespace std;
//-------------BMP•Û‘¶--------------------------------------------------------------
void InitHeaders(BitmapHeader *header, BitmapInfoHeader *info) {
	header->distinct1 = 'B';
	header->distinct2 = 'M';
	header->filesize = 0;
	header->reserve1 = 0;
	header->reserve2 = 0;
	header->offset = 54;

	info->header = 40;
	info->width = 0;
	info->height = 0;
	info->plane = 1;
	info->bits = 24;
	info->compression = 0;
	info->comp_image_size = 0;
	info->x_resolution = 0;
	info->y_resolution = 0;
	info->pallet_num = 0;
	info->important_pallet_num = 0;
}

void WriteHeader(BitmapHeader *header, FILE *fp) {
	fwrite(&(header->distinct1), sizeof(char), 1, fp);
	fwrite(&(header->distinct2), sizeof(char), 1, fp);
	fwrite(&(header->filesize), sizeof(int), 1, fp);
	fwrite(&(header->reserve1), sizeof(short), 1, fp);
	fwrite(&(header->reserve2), sizeof(short), 1, fp);
	fwrite(&(header->offset), sizeof(int), 1, fp);
}

void WriteInfoHeader(BitmapInfoHeader *info, FILE *fp) {
	fwrite(&(info->header), sizeof(int), 1, fp);
	fwrite(&(info->width), sizeof(int), 1, fp);
	fwrite(&(info->height), sizeof(int), 1, fp);
	fwrite(&(info->plane), sizeof(short), 1, fp);
	fwrite(&(info->bits), sizeof(short), 1, fp);
	fwrite(&(info->compression), sizeof(int), 1, fp);
	fwrite(&(info->comp_image_size), sizeof(int), 1, fp);
	fwrite(&(info->x_resolution), sizeof(int), 1, fp);
	fwrite(&(info->y_resolution), sizeof(int), 1, fp);
	fwrite(&(info->pallet_num), sizeof(int), 1, fp);
	fwrite(&(info->important_pallet_num), sizeof(int), 1, fp);
}

int WriteBitmap(const char* filename, int width, int height) {
	GLubyte *pixel_data;
	FILE *fp;
	BitmapHeader header;
	BitmapInfoHeader info;
	int alignmentParam;
	int i = 0;
	int j = 0;
	int x;
	int y;
	unsigned char zero = 0;


	glGetIntegerv(GL_PACK_ALIGNMENT, &alignmentParam);


	int glByteWidth;

	if (width * 3 % alignmentParam == 0)
		glByteWidth = width * 3;
	else

		glByteWidth = width * 3 + alignmentParam - (width * 3) % alignmentParam;


	pixel_data = (GLubyte*)malloc((glByteWidth)*(height)*(sizeof(GLubyte)));

	glReadPixels(
		0, 0,
		width, height,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		pixel_data);

	if ((fp = fopen(filename, "wb")) == NULL) {
		return -1;
	}

	InitHeaders(&header, &info);

	info.width = width;
	info.height = height;
	int writeWidth;

	if (width * 3 % 4 == 0)
		writeWidth = width * 3;
	else
		writeWidth = width * 3 + 4 - (width * 3) % 4;

	header.filesize =
		writeWidth*height
		+ 14
		+ 40;
	WriteHeader(&header, fp);
	WriteInfoHeader(&info, fp);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			j = fwrite((pixel_data + x * 3 + glByteWidth*y + 2), sizeof(GLubyte), 1, fp);
			j = fwrite((pixel_data + x * 3 + glByteWidth*y + 1), sizeof(GLubyte), 1, fp);
			j = fwrite((pixel_data + x * 3 + glByteWidth*y), sizeof(GLubyte), 1, fp);
		}
		if (width * 3 % 4 != 0)
			for (int j = 0; j<4 - (width * 3) % 4; j++)
				fwrite(&zero, sizeof(GLubyte), 1, fp);
	}

	free(pixel_data);
	fclose(fp);
	return 0;
}
