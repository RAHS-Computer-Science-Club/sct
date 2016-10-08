#include <stdint.h> // For using int8_t, uint8_t etc variable types.

typedef struct {
	int8_t tabs;
	void* prev;
	char text[81];
	void* next;
}textlist_t;

void sct_file_save(const char* filename, textlist_t* text);
void sct_file_load(const char* filename, textlist_t* text);
void textlist_insert(textlist_t* afterwhat);
