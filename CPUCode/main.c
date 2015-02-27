/*
 * main.c
 *
 *  Created on: Dec 20, 2014
 *      Author:
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <MaxSLiCInterface.h>
#include "Maxfiles.h"

#include "bitmapUtils.h"

//#define DEBUG_PRINT

int main() {

	char* default_file = "../Data/test1.bmp";
	int xx, yy, err;

	uint8_t* image; // image if allocated in decodebitmap
	err = decodebitmap(default_file,&xx,&yy,&image);
	if (err) {
		printf("Error %d unpacking bitmap into array\n",err);
		return 1;
	}
	printf("File = \'%s\' x = %d, y = %d\n",default_file,xx,yy);

	const int size = hough_transform_maxSize;
	if (xx*yy > size){
		printf("Image is too large to perform transformation\n");
		return 2;
	}

	uint64_t *imageContents = calloc(sizeof(uint64_t), size);
	if (imageContents==NULL){
		printf("Allocation error\n");
		return 3;
	}

	// invert image pixels
	for (int i = 0; i < (xx*yy); i++) {
		imageContents[i] = 1-image[i];
	}
	free(image);

	const int tsize = hough_transform_numAngles * hough_transform_maxAxis * 2;
	uint64_t *transformInitContents = calloc(sizeof(uint64_t), tsize);
	if (transformInitContents==NULL){
		printf("Allocation error\n");
		return 4;
	}
	uint64_t *transformContents = malloc(sizeof(uint64_t)*tsize);
	if (transformContents==NULL){
		printf("Allocation error\n");
		return 5;
	}
	uint32_t *outDataA = calloc(sizeof(uint32_t), tsize);
	if (outDataA==NULL){
		printf("Allocation error\n");
		return 6;
	}

	printf("Running DFE.\n");
	hough_transform(
		xx*yy, xx, yy,
		outDataA,
		imageContents, transformInitContents, transformContents);

	free(outDataA);
	free(imageContents);
	free(transformInitContents);

	uint64_t max = 0;
	for (int i = 0; i < tsize; i++) {
		if (transformContents[i]>max) max=transformContents[i];
#ifdef DEBUG_PRINT
		if (transformContents[i]>0)
			printf("%d: %d %ld\n",i,outDataA[i], transformContents[i]);
#endif
	}

	printf("Maximum = %ld\n",max);

	uint8_t *transformContentsNorm = malloc(sizeof(uint8_t)*tsize);
	if (outDataA==NULL){
		printf("Allocation error\n");
		return 7;
	}

	for (int i = 0; i < tsize; i++) {
		// Normalise
		uint64_t norm = 255*transformContents[i];
		norm /= max;
		// Invert
		transformContentsNorm[i] = 255-norm;
	}
	free(transformContents);

	err = encodebitmap24("../Data/out.bmp",hough_transform_numAngles,hough_transform_maxAxis * 2,transformContentsNorm,1);
	free(transformContentsNorm);
	if (err){
		printf("Error encoding output bitmap\n");
		return 8;
	}

	return 0;

}
