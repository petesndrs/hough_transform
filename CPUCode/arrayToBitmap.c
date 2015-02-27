#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "bitmapUtils.h"

//#define DEBUG_PRINT

int encodebitmap1(char* name, int x, int y, uint8_t* map){
	printf("Opening output %s\n",name);
	
    FILE* fd_out = fopen(name,"wb");
    if (fd_out==NULL){
        printf("Cannot open output file\n");
        return __LINE__;
	}
	
	// calculate size with padding
	uint32_t line_length = x;
	if ( (x % 32) != 0) line_length += (32 - (line_length % 32));
#ifdef DEBUG_PRINT
	printf("x=%d, linelength=%d\n",x,line_length);
#endif
	// read in the file header
	struct bmp_file_header file_header;	
	memset(&file_header,0,sizeof(struct bmp_file_header));
	file_header.type[0] = 'B';
	file_header.type[1] = 'M';
	file_header.size = sizeof(struct bmp_file_header) + sizeof(struct bmp_dib_header) + 8 + ((line_length*y)/8);
	file_header.start = sizeof(struct bmp_file_header) + sizeof(struct bmp_dib_header) + 8;
	size_t bytes_written = fwrite(&file_header,1,sizeof(struct bmp_file_header),fd_out);
    printf("Out write %lu\n", bytes_written);
					
	// read in the dib header
	struct bmp_dib_header dib_header;
	memset(&dib_header,0,sizeof(struct bmp_dib_header));
	dib_header.size = sizeof(struct bmp_dib_header);
	dib_header.x = x;
	dib_header.y = y;
	dib_header.a = 1;
	dib_header.b = 1;
	bytes_written = fwrite(&dib_header,1,sizeof(struct bmp_dib_header),fd_out);
#ifdef DEBUG_PRINT
	printf("Out write %lu\n", bytes_written);
#endif
	uint8_t junk[8] = {0,0,0,0,0xff,0xff,0xff,0};
	bytes_written = fwrite(junk,1,8,fd_out);
#ifdef DEBUG_PRINT
	printf("Out write %lu\n", bytes_written);
#endif
	uint8_t* map_ptr = map;
	for (int iy=0; iy<y; ++iy){
		for (int ix=0; ix<(int)line_length; ix+=8){
			int xleft = x - ix;
			
			uint8_t tmp = 0;
			if (xleft>0) { if(*map_ptr==1) tmp |= 0x80; map_ptr++;}
			if (xleft>1) { if(*map_ptr==1) tmp |= 0x40; map_ptr++;}
			if (xleft>2) { if(*map_ptr==1) tmp |= 0x20; map_ptr++;}
			if (xleft>3) { if(*map_ptr==1) tmp |= 0x10; map_ptr++;}
			if (xleft>4) { if(*map_ptr==1) tmp |= 0x08; map_ptr++;}
			if (xleft>5) { if(*map_ptr==1) tmp |= 0x04; map_ptr++;}
			if (xleft>6) { if(*map_ptr==1) tmp |= 0x02; map_ptr++;}
			if (xleft>7) { if(*map_ptr==1) tmp |= 0x01; map_ptr++;}
			
			bytes_written = fwrite(&tmp,1,1,fd_out);
		}
	}
	
	printf("Closing output %s\n",name);
	fclose(fd_out);

	return 0;
}

int encodebitmap24(char* name, int x, int y, uint8_t* map, int scale){
	printf("Opening output %s\n",name);
	
    FILE* fd_out = fopen(name,"wb");
    if (fd_out==NULL){
        printf("Cannot open output file\n");
        return __LINE__;
	}
	
	// calculate size with padding
	uint16_t line_length = x*3;
	if ( (line_length % 4) != 0) line_length += (4 - (line_length % 4));
	uint16_t padding = (line_length - (x*3));
#ifdef DEBUG_PRINT
	printf("x=%d, linelength=%d, padd=%d bytes\n",x,line_length,padding);
#endif
	// read in the file header
	struct bmp_file_header file_header;	
	memset(&file_header,0,sizeof(struct bmp_file_header));
	file_header.type[0] = 'B';
	file_header.type[1] = 'M';
	file_header.size = sizeof(struct bmp_file_header) + sizeof(struct bmp_dib_header) + ((line_length*y));
	file_header.start = sizeof(struct bmp_file_header) + sizeof(struct bmp_dib_header);
	size_t bytes_written = fwrite(&file_header,1,sizeof(struct bmp_file_header),fd_out);
#ifdef DEBUG_PRINT
	printf("Out write %lu\n", bytes_written);
#endif

	// read in the dib header
	struct bmp_dib_header dib_header;
	memset(&dib_header,0,sizeof(struct bmp_dib_header));
	dib_header.size = sizeof(struct bmp_dib_header);
	dib_header.x = x;
	dib_header.y = y;
	dib_header.a = 1;
	dib_header.b = 24;
	bytes_written = fwrite(&dib_header,1,sizeof(struct bmp_dib_header),fd_out);
#ifdef DEBUG_PRINT
	printf("Out write %lu\n", bytes_written);
#endif
	uint8_t* map_ptr = map;
	for (int iy=0; iy<y; ++iy){
		for (int ix=0; ix<x; ++ix){
			int xleft = x - ix;
			
			uint8_t tmp[3];
			if (xleft>0) { 
				uint8_t value = scale * *map_ptr;
				tmp[0] = value;
				tmp[1] = value;
				tmp[2] = value;
				map_ptr++;
				}
			
			bytes_written = fwrite(&tmp,1,3,fd_out);
			
			// on last write in line add padding
			if (xleft==1 && padding) {
				uint8_t pad[4] = {0};
				bytes_written = fwrite(&pad,1,padding,fd_out);
			}
		}
	}
	
	printf("Closing output %s\n",name);
	fclose(fd_out);

	return 0;
}


