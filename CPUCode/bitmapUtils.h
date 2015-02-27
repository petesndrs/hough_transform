#ifndef BITMAPUTILS_H_
#define BITMAPUTILS_H_

extern int encodebitmap1(char* name, int x, int y, uint8_t* map);
extern int encodebitmap24(char* name, int x, int y, uint8_t* map, int scale);
extern int decodebitmap(char* name, int* x_ret, int* y_ret, uint8_t** map);

struct bmp_file_header {
	char type[2];	//2
	uint32_t size;	//6
	uint32_t something;	//10
	uint32_t start;	//14
} __attribute__((__packed__));

struct bmp_dib_header {
	uint32_t size; // 4
	int32_t x; // 8
	int32_t y; // 12
	uint16_t a; // 14
	uint16_t b; // 16
	uint8_t pad[24];
} __attribute__((__packed__));

#endif /* BITMAPUTILS_H_ */
