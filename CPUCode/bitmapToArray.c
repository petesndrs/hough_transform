#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "bitmapUtils.h"

//#define DEBUG_PRINT

int decodebitmap(char* name, int* x_ret, int* y_ret, uint8_t** map){
	
	printf("Opening input %s\n",name);
	
    FILE* fd_in = fopen(name,"r");
    if (fd_in==NULL){
        printf("Cannot open input file\n");
        return __LINE__;
	}
		
	size_t total_bytes = 0;
	// read in the file header
	struct bmp_file_header file_header;	
	size_t bytes_read = fread(&file_header,1,sizeof(struct bmp_file_header),fd_in);
#ifdef DEBUG_PRINT
	printf("In read %lu\n", bytes_read);
#endif
	total_bytes += bytes_read;

#ifdef DEBUG_PRINT
    printf("%c %c\n",file_header.type[0],file_header.type[1]);	
    printf("%d %d\n",file_header.size,file_header.start);
#endif

	// read in the dib header
	struct bmp_dib_header dib_header;
	bytes_read = fread(&dib_header,1,sizeof(struct bmp_dib_header),fd_in);
#ifdef DEBUG_PRINT
    printf("In read %lu\n", bytes_read);
#endif
	total_bytes += bytes_read;
	
#ifdef DEBUG_PRINT
    printf("dib header size %d\n",dib_header.size);
	printf("x %d y %d\n",dib_header.x, dib_header.y);
	printf("a %d b %d\n",dib_header.a, dib_header.b);
#endif

	*x_ret = dib_header.x;
	*y_ret = dib_header.y;
	
	size_t headers_left = file_header.start - total_bytes;
#ifdef DEBUG_PRINT
	printf("headers left %ld\n",headers_left);
#endif
	uint8_t* junk = malloc(headers_left);
	if (junk == NULL) {
		printf ("Error cannot alloc\n");
		return __LINE__;
	}
	bytes_read = fread(junk,1,headers_left,fd_in);
#ifdef DEBUG_PRINT
	printf("In read %lu\n", bytes_read);
#endif
	free(junk);
		
	*map = calloc(dib_header.x*dib_header.y,1);	
	if (*map == NULL) {
		printf ("Error cannot alloc\n");
		return __LINE__;
	}
	uint8_t* map_ptr = *map;

	// Read the pixel data
	switch(dib_header.b){
	case 1:
		{
		// Align to two bytes (16 bits)
		uint16_t line_length = dib_header.x;
		if ( (dib_header.x % 32) != 0) line_length += (32 - (line_length % 32));
		for (int iy=0; iy<dib_header.y; ++iy){
			for (int ix=0; ix<line_length; ix+=8){
				int xleft = dib_header.x - ix;
				uint8_t tmp = 0;
				bytes_read = fread(&tmp,1,1,fd_in);
				if (ferror(fd_in)){
					printf ("Error reading from file\n");
					break;
					}
				if (feof(fd_in)){
					printf ("Error end of file\n");
					break;
					}
				if (xleft>0) { if (tmp & 0x80) *map_ptr = 1; map_ptr++;}
				if (xleft>1) { if (tmp & 0x40) *map_ptr = 1; map_ptr++;}
				if (xleft>2) { if (tmp & 0x20) *map_ptr = 1; map_ptr++;}
				if (xleft>3) { if (tmp & 0x10) *map_ptr = 1; map_ptr++;}
				if (xleft>4) { if (tmp & 0x08) *map_ptr = 1; map_ptr++;}
				if (xleft>5) { if (tmp & 0x04) *map_ptr = 1; map_ptr++;}
				if (xleft>6) { if (tmp & 0x02) *map_ptr = 1; map_ptr++;}
				if (xleft>7) { if (tmp & 0x01) *map_ptr = 1; map_ptr++;}
				}
			}
		break;
		}
	case 4:
		{
		// Align to two bytes (4 nibbles)
		uint16_t line_length = dib_header.x;
		if ( (dib_header.x % 4) != 0) line_length += (4 - (line_length % 4));
		for (int iy=0; iy<dib_header.y; ++iy){
			for (int ix=0; ix<line_length; ix+=2){
				int xleft = dib_header.x - ix;
				uint8_t tmp = 0;
				bytes_read = fread(&tmp,1,1,fd_in);
				if (ferror(fd_in)){
					printf ("Error reading from file\n");
					break;
				}
				if (feof(fd_in)){
					printf ("Error end of file\n");
					break;
				}
				if (xleft>0) { if ((tmp&0xf0)==0xf0) *map_ptr = 1; map_ptr++;}
				if (xleft>1) { if ((tmp&0x0f)==0x0f) *map_ptr = 1; map_ptr++;}
				}
			}
		break;
		}
	case 8:
		{
		// Align to four bytes
		uint16_t line_length = dib_header.x;
		if ( (dib_header.x % 4) != 0) line_length += (4 - (line_length % 4));
		for (int iy=0; iy<dib_header.y; ++iy){
			for (int ix=0; ix<line_length; ix+=1){
				int xleft = dib_header.x - ix;
				uint8_t tmp = 0;
				bytes_read = fread(&tmp,1,1,fd_in);
				if (ferror(fd_in)){
					printf ("Error reading from file\n");
					break;
					}
				if (feof(fd_in)){
					printf ("Error end of file\n");
					break;
					}
				if (xleft>0) {if (tmp==0xff) *map_ptr = 1; map_ptr++;}
				}
			}
		break;
		}
	case 24:
		{
		// Align
		uint16_t line_length = dib_header.x*3;
		if ( (line_length % 4) != 0) line_length += (4 - (line_length % 4));
		for (int iy=0; iy<dib_header.y; ++iy){
			for (int ix=0; ix<line_length; ix+=3){
				int xleft = (dib_header.x*3) - ix;
				uint8_t tmp[3];
				if (xleft>0) {
					bytes_read = fread(&tmp,1,3,fd_in);
				} else {
					// absorb the sub-pixel alignment
					bytes_read = fread(&tmp,1,(line_length-dib_header.x*3),fd_in);
				}
				if (ferror(fd_in)){
					printf ("Error reading from file\n");
					break;
					}
				if (feof(fd_in)){
					printf ("Error end of file\n");
					break;
					}
				if (xleft>0) {if (tmp[0]==0xff && tmp[1]==0xff && tmp[2]==0xff) *map_ptr = 1; map_ptr++;}
				}
			}
		break;
		}
	}
	
	printf("Closing input %s\n",name);
	fclose(fd_in);

	return 0;
}
