/**
 * SCT Sciences Creamy Text Editor(c) 2016 Jeron Lau RAHS Computer Science Club.
 * LICENSED UNDER THE UNLICENSE
**/

#include <stdint.h> // For using int8_t, uint8_t etc variable types.
#include <ncurses.h> // For getting input, etc.

typedef struct {
	int8_t tabs;
	void* prev;
	char text[81];
	void* next;
}textlist_t;

typedef struct {
	char chunk[32];
	uint8_t color;
	uint8_t style;
	uint8_t start;
}chunk_t;

void sct_chunk(WINDOW *w, const char* data, chunk_t* keywords, uint8_t numkeywords, uint8_t ln, uint8_t tabs);
void sct_file_save(const char* filename, textlist_t* text);
uint8_t sct_file_load(const char* filename, textlist_t* text);
void textlist_insert(textlist_t* afterwhat);
