/**
 * SCT Sciences Creamy Text Editor(c) 2016 Jeron Lau RAHS Computer Science Club.
 * LICENSED UNDER THE UNLICENSE
**/

#include <string.h> // For strcmp
#include <stdlib.h>

//void sct_
#include "sct.h"

void sct_chunk(WINDOW *w, const char* data, chunk_t* keywords, uint8_t numkeywords, uint8_t ln, uint8_t tabs) {
	chunk_t chunks[256];
	uint8_t chunkc = 0;
	uint8_t cursor = 0;

	chunks[0].start = 0;
	for(int i = 0; i < strlen(data); i++) {
		if(data[i] != ' ' && data[i] != ',' && data[i] != '*' && 
			data[i] != ')' && data[i] != '(' && data[i] != ']' &&
			data[i] != '[' && data[i] != '}' && data[i] != '{' &&
			data[i] != ';' && data[i] != '\n')
		{
			chunks[chunkc].chunk[cursor] = data[i];
			cursor++;
		}else{
			chunks[chunkc].chunk[cursor] = '\0';
			chunks[chunkc + 1].start = i + 1;
			chunkc++;
			cursor = 0;
		}
	}
	chunks[chunkc].chunk[cursor] = '\0';
	chunkc++;
	for(int i = 0; i < chunkc; i++) {
		for(int j = 0; j < numkeywords; j++) {
			if(strcmp(keywords[j].chunk, chunks[i].chunk) == 0) {
				wmove(w, ln, 8 + (tabs * 8) + chunks[i].start);
				chgat(strlen(keywords[j].chunk), A_BOLD,
					keywords[j].color, NULL);
			}
		}
	}
}
