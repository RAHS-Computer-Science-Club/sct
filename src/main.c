/**
 * SCT Sciences Creamy Text Editor (c) Jeron Lau RAHS Computer Science Club.
 * The Unlicense
**/

#include <stdio.h> // For using printf()
#include <string.h> // For using strlen()
#include <stdint.h> // For using int8_t, uint8_t etc variable types.

typedef struct {
	void* prev;
	char text[80];
	void* next;
}textlist_t;

textlist_t* init_textlist(void) {
	textlist_t* rtn = malloc(sizeof(textlist_t));
	rtn->prev = NULL;
	rtn->text[0] = 0;
	rtn->next = NULL;
	return rtn;
}

// Main function.
int main(int argc, char *argv[]) {
	// Variable to store character name in.
	textlist_t* text = init_textlist();

	// Print some text
	fputs("Science's Creamy Text Editor - SCT\n", stdout);
	// Return 0 on success.
	return 0;
}
