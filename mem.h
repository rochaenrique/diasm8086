#ifndef _H_MEM_
#define _H_MEM_

#include <stdio.h>
#include "diasm8086.h"

static FILE* fp = 0;
static u8 memory[1024 * 1024];

int get_byte();
bool load_file_mem(const char* filename);

int get_byte() {
	int resul = fgetc(fp);
	IF_EXIT(resul == EOF, "Error: EOF reached\n");
	return resul;
}

bool load_file_mem(const char* filename) {
	fp = fopen(filename, "rb");
	return fp != 0;
}

#endif // _H_MEM_
