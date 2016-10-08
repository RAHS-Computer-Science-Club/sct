/**
 * SCT Sciences Creamy Text Editor (c) Jeron Lau RAHS Computer Science Club.
 * The Unlicense
**/

#include <stdio.h> // For using printf()
#include <string.h> // For using strlen()
#include <stdint.h> // For using int8_t, uint8_t etc variable types.
#include <ncurses.h> // For getting input, etc.
#include <stdlib.h> // For exit()
#include <unistd.h> // For sleep()

typedef struct {
	void* prev;
	char text[81];
	void* next;
}textlist_t;

textlist_t* init_textlist(void) {
	textlist_t* rtn = malloc(sizeof(textlist_t));
	rtn->prev = NULL;
	memset(rtn->text, '\0', 81);
	rtn->text[0] = '\0';
	rtn->next = NULL;
	return rtn;
}

static void sct_exit(uint8_t* running, const char* message) {
	*running = 0;
	endwin();
}

static void sct_draw(WINDOW* w, textlist_t* text, int8_t* x, int8_t* y) {
	int i;
	uint8_t infile = 1;

	curs_set(FALSE);
	for(i = 0; i < 10; i++) {
		if(infile) {
			mvprintw(i + 1, 0, "%s\n", text->text);
			if(text->next) {
				text = text->next;
			}else{
				infile = 0;
			}
		}else{
			mvprintw(i + 1, 0, " ~ ~ ~ \n");
		}
	}
	wmove(w, 1 + *y, *x);
	curs_set(TRUE);
}

// Main function.
int main(int argc, char *argv[]) {
	// Variable to store character name in.
	textlist_t* text = init_textlist();
	uint8_t running = 1;
	WINDOW *w = initscr();
	int8_t cursorx = 0, cursory = 0;

	cbreak();
	noecho();
	nodelay(w, TRUE);
	keypad(w, TRUE);
	meta(w, TRUE);
	nodelay(w, TRUE);
	curs_set(FALSE);
	raw();

	// Print some text
	mvprintw(0, 0, "Science's Creamy Text Editor - SCT");
	sct_draw(w, text, &cursorx, &cursory);
	while(running) {
		int32_t chr = getch();
		if(chr != ERR) {
			const char* name = keyname(chr);

			// Close
			if(strcmp(name, "^Q") == 0) {
				sct_exit(&running, "EXITING ON QUIT");
			}
			else if(strcmp(name, "^W") == 0) {
				sct_exit(&running, "EXITING ON FILE CLOSE");
			}
			else if(strcmp(name, "^C") == 0) {
				sct_exit(&running, "EXITING ON FORCE CLOSE");
			}
			// Save
			else if(strcmp(name, "^S") == 0) {
				sct_exit(&running, "EXITING ON SAVE");
			}
			// Functions
			else if(strcmp(name, "KEY_BACKSPACE") == 0) {
				cursorx--;
				if(cursorx < 0) cursorx = 0;
				text->text[cursorx] = '\0';
				sct_draw(w, text, &cursorx, &cursory);
			}
			else if(strcmp(name, "KEY_DC") == 0) {
				sct_exit(&running, "Delete");
			}
			else if(strcmp(name, "^D") == 0) {
				sct_exit(&running, "DELETE LINE");
			}
			else if(strcmp(name, "^I") == 0) {
				sct_exit(&running, "TAB");
			}
			else if(strcmp(name, "KEY_BTAB") == 0) {
				// Shift - Tab
			}
			else if(strcmp(name, "^J") == 0) {
				// Newline
			}
			else if(strcmp(name, "^[") == 0) {
				// Newline
			}
			else if(strcmp(name, "KEY_UP") == 0) {
				
			}
			else if(strcmp(name, "KEY_DOWN") == 0) {
				
			}
			else if(strcmp(name, "KEY_PPAGE") == 0) {
				// Page UP
			}
			else if(strcmp(name, "KEY_NPAGE") == 0) {
				// Page DN
			}
			else {
				if(cursorx > 79) cursorx = 79;
				text->text[cursorx] = name[0];
				cursorx++;
				sct_draw(w, text, &cursorx, &cursory);
				// Character Insert
//			printf("NAME: %s\n", name);
			}
		}
	}
	endwin();
	// Return 0 on success.
	return 0;
}
