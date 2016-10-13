/**
 * SCT Sciences Creamy Text Editor(c) 2016 Jeron Lau RAHS Computer Science Club.
 * LICENSED UNDER THE UNLICENSE
**/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "sct.h"

void sct_file_save(const char* filename, textlist_t* text) {
	FILE* file = fopen(filename, "w+");

	while(text->prev != NULL)
		text = text->prev;

	while(text) {
		for(int i = 0; i < text->tabs; i++) {
			fputc('\t', file);
		}
		fputs(text->text, file);
		fputc('\n', file);
		text = text->next;
	}

	fclose(file);
}

uint8_t sct_file_load(const char* filename, textlist_t* text) {
	FILE* file = fopen(filename, "r");

	if(file == NULL) return 1;

	while(text->prev != NULL)
		text = text->prev;

	while(1) {
		int i = 0;

		text->tabs = 0;
		// Read at most 80 characters per line.
		while(1) {
			int chr = fgetc(file);

			if(chr == EOF) {
				goto SCT_FILE_LOAD_END;
			}else if(chr == '\t') {
				text->tabs ++;
			}else if(chr == '\n') {
				text->text[i] = '\0';
				break;
			}else{
				// Insert character
				text->text[i] = chr;
				i++;
			}
			// Don't go over 80 characters per line.
			if(i > 80 - (text->tabs * 8))
				break;
		}
		// Go on
		if(text->next == NULL) {
			// Make new line
			textlist_insert(text);
		}
		text = text->next;
	}
SCT_FILE_LOAD_END:;
	textlist_t* prev = text->prev;
	prev->next = NULL;
	while(text) {
		textlist_t* tofree = text;
		text = text->next;
		free(tofree);
	}

	fclose(file);
	return 0;
}
