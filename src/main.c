/**
 * SCT Sciences Creamy Text Editor(c) 2016 Jeron Lau RAHS Computer Science Club.
 * LICENSED UNDER THE UNLICENSE
**/

#include <stdio.h> // For using printf()
#include <string.h> // For using strlen()
#include <stdlib.h> // For exit()

#include "sct.h"

#define UNTITLED "UN-TITLED"

#define KEYWORD_COUNT 55

typedef struct {
	WINDOW *w;
	int8_t cursorx, cursory;
	int8_t selectx, selecty;
	uint8_t lh;
	chunk_t kw[KEYWORD_COUNT];
} sct_context_t;

static textlist_t* init_textlist(void) {
	textlist_t* rtn = malloc(sizeof(textlist_t));
	rtn->prev = NULL;
	memset(rtn->text, '\0', 81);
	rtn->next = NULL;
	rtn->tabs = 0;
	return rtn;
}

void textlist_insert(textlist_t* afterwhat) {
	textlist_t* new_next = afterwhat->next;
	textlist_t* new_item = malloc(sizeof(textlist_t));

	// Sandwich
	afterwhat->next = new_item;
	if(new_next) new_next->prev = new_item;

	// Set item
	new_item->prev = afterwhat;
	new_item->next = new_next;

	memset(new_item->text, '\0', 81);
	new_item->tabs = 0;
}

static textlist_t* textlist_delete(textlist_t* deletewhat) {
	textlist_t* next_item = deletewhat->next;

	next_item->prev = NULL;

	free(deletewhat);

	return next_item;
}

static textlist_t* sct_nextline(sct_context_t* context, textlist_t* line,
	textlist_t** text, int8_t* y, uint32_t* sln)
{
	if(*y == context->lh - 1) {
		*sln = *sln + 1;
		*text = (*text)->next;
		return line->next;
	}
	*y = *y + 1;
	return line->next;
}

static textlist_t* sct_prevline(textlist_t* line, textlist_t** text, int8_t* y,
	uint32_t* sln)
{
	if(*y == 0) {
		if(*sln) {
			*sln = *sln - 1;
			*text = (*text)->prev;
			return line->prev;
		}
		return line;
	}
	*y = *y - 1;
	return line->prev;
}

static void sct_exit(uint8_t* running, const char* message) {
	*running = 0;
	endwin();
}

static void sct_draw(sct_context_t* context, textlist_t* text, textlist_t* line,
	uint32_t sln)
{
	int i;
	uint8_t infile = 1;

	for(i = 0; i < context->lh; i++) {
		if(infile) {
			for(int j = 0; j < text->tabs; j++) {
				mvprintw(i + 1, (j+1) * 8, "        ");
			}
			attron(A_DIM);
			mvprintw(i + 1, 0, "%5d | ", sln + i + 1);
			attroff(A_DIM);
			mvprintw(i + 1, 8 + text->tabs * 8, "%s\n", text->text);
			sct_chunk(context->w, text->text, context->kw,
				KEYWORD_COUNT, i + 1, text->tabs);
			if(text->next) {
				text = text->next;
			}else{
				infile = 0;
			}
		}else{
			attron(A_DIM);
			mvprintw(i + 1, 0, "      |\n");
			attroff(A_DIM);
		}
	}
	wmove(context->w, 1 + context->cursory, 8 + context->cursorx + (line->tabs * 8));
	if(context->selecty)
		chgat(79, A_REVERSE, 0, NULL);
	else
		chgat(context->selectx ? context->selectx : 1, A_REVERSE, 0, NULL);
	refresh();
}

/*static void sct_rendermultiselect(WINDOW* w, textlist_t text, int8_t* x, int8_t* y,
	int8_t* lx, int8_t* ly)
{
	wmove(w, 1 + *y, *x + (line->tabs * 8));
	chgat(1, A_REVERSE, 0, NULL);
	refresh();
}*/

static void sct_renderlineselect(sct_context_t* context, textlist_t* line, int8_t x,
	int8_t y, int8_t lx)
{
	wmove(context->w, 1 + y, x + (line->tabs * 8));
	chgat(lx - x, A_REVERSE, 0, NULL);
	refresh();
}

void sct_insert(sct_context_t* context, textlist_t* text, textlist_t* line,
	int8_t* x, int8_t* y, char c, uint32_t sln)
{
	uint8_t max = 79 - (line->tabs * 8);
	if(*x > max) *x = max;
	if(strlen(line->text) >= max) return;
	memmove(line->text + *x + 1, line->text + *x, strlen(line->text + *x) + 1);
	line->text[*x] = c;
	*x = *x + 1;
	sct_draw(context, text, line, sln);
}

static int8_t sct_mouseinputx(textlist_t* line, int8_t x) {
	int rx = x - ((line->tabs + 1)* 8);

	if(rx < 0) rx = 0;
	if(rx > strlen(line->text)) rx = strlen(line->text);
	if(x > 79) x = 79;
	return rx;
}

static int8_t sct_mouseinputy(int8_t y) {
	int ry = y - 1;

	if(ry < 0) ry = 0;
	return ry;
}

static textlist_t* sct_cursorup(sct_context_t* context, textlist_t** text,
	 textlist_t* line, uint32_t* sln)
{
	if(line->prev) {
		line = sct_prevline(line, text, &context->cursory, sln);
		context->cursorx = strlen(line->text);
		sct_draw(context, *text, line, *sln);
	}
	return line;
}

static textlist_t* sct_cursordn(sct_context_t* context, textlist_t** text,
	textlist_t* line, uint32_t* sln, uint8_t end)
{
	if(line->next) {
		line = sct_nextline(context, line, text, &context->cursory, sln);
		context->cursorx = end ? strlen(line->text) : 0;
		sct_draw(context, *text, line, *sln);
	}
	return line;
}

static textlist_t* sct_backspace(sct_context_t* context, textlist_t** text,
	textlist_t* line, uint32_t* sln)
{
	textlist_t* prev_item = line->prev;
	textlist_t* next_item = line->next;

	if(next_item) next_item->prev = prev_item;
	prev_item->next = next_item;

	free(line);

	// Scroll Up if Needed
	if(*sln) {
		int end;
		textlist_t* temp = *text;
		for(int i = 0; i < context->lh; i++) {
			temp = temp->next;
			if(temp == NULL && (*text)->prev) {
				*sln = *sln - 1;
				context->cursory++;
				*text = (*text)->prev;
				break;
			}
		}
	}

	if(next_item) {
		return next_item;
	}else{
		sct_cursorup(context, text, line, sln);
		return prev_item;
	}
}

static void sct_title(int32_t w, int32_t h, const char* f, uint8_t s) {
	attron(A_BOLD | COLOR_PAIR(1));
	mvprintw(0,0,"Science's Creamy Text Editor - SCT %03dx%03d %45s",w,h,f);
	if(!s) mvprintw(0,86 - strlen(f), "*");
	attroff(A_BOLD | COLOR_PAIR(1));
}

static void sct_notify(sct_context_t* context, const char* message) {
	attron(A_DIM);
	mvprintw(1 + context->lh, 0, "|-----|");
	attroff(A_DIM);
	attron(A_BOLD | COLOR_PAIR(1));
	mvprintw(1 + context->lh, 8, "%80s", message);
	attroff(A_BOLD | COLOR_PAIR(1));
}

static uint8_t sct_popup(sct_context_t* context, const char* message, char input[]) {
	uint8_t cancel = 0;

	memset(input, '\0', strlen(input));
	mvprintw(2,14,"                                                      ");
	mvprintw(3,14," ____________________________________________________ ");
	mvprintw(4,14," |%50s| ", message);
	mvprintw(5,14," |__________________________________________________| ");
	mvprintw(6,14," | >:                                               | ");
	mvprintw(7,14," |__________________________________________________| ");
	mvprintw(8,14,"                                                      ");
	mvprintw(6,20,"%s", input);
	while(1) {
		int in = getch();
		const char* key = keyname(in);

		if(in == ERR) continue;
		if(strcmp(key, "KEY_BACKSPACE") == 0) {
			input[strlen(input) - 1] = '\0';
		}else if(strcmp(key, "^J") == 0) {
			cancel = 0;
			break;
		}else if(strlen(key) == 1) {
			if(strlen(input) > 44) continue;
			input[strlen(input)] = key[0];
			input[strlen(input) + 1] = '\0';
		}else{
			sct_notify(context, "Canceled.");
			cancel = 1;
			memcpy(input, UNTITLED, strlen(UNTITLED) + 1);
			break;
		}
		mvprintw(6, 20, "%s ", input);
	}
	return cancel;
}

static void
addkeyword(sct_context_t* context, uint8_t i, const char* name, uint8_t color) {
	memcpy(context->kw[i].chunk, name, strlen(name) + 1);
	context->kw[i].color = color;
}

// Main function.
int main(int argc, char *argv[]) {
	sct_context_t context;
	context.w = initscr();
	context.cursorx = 0;
	context.cursory = 0;
	context.selectx = 0;
	context.selecty = 0;
	context.lh = 10;
	// Variable to store character name in.
	textlist_t* text = init_textlist();
	textlist_t* line = text;
	uint8_t running = 1;
	uint8_t mouseHeldDown = 0;
	char filename[1024];
	uint32_t sln = 0; // Starting Line Number
	int16_t width = getmaxx(context.w);
	int16_t height = getmaxy(context.w);
	uint8_t saved = 1;

	context.lh = height - 2;
	// Variable Types Syntax Highlighting
	addkeyword(&context, 0, "void", 3);
	addkeyword(&context, 1, "uint8_t", 3);
	addkeyword(&context, 2, "uint16_t", 3);
	addkeyword(&context, 3, "uint32_t", 3);
	addkeyword(&context, 4, "uint64_t", 3);
	addkeyword(&context, 5, "int8_t", 3);
	addkeyword(&context, 6, "int16_t", 3);
	addkeyword(&context, 7, "int32_t", 3);
	addkeyword(&context, 8, "int64_t", 3);
	addkeyword(&context, 9, "int", 3);
	addkeyword(&context, 10, "short", 3);
	addkeyword(&context, 11, "char", 3);
	addkeyword(&context, 12, "long", 3);
	addkeyword(&context, 13, "static", 3);
	addkeyword(&context, 14, "inline", 3);
	addkeyword(&context, 15, "const", 3);

	// Functionality
	addkeyword(&context, 16, "break", 1);
	addkeyword(&context, 17, "return", 1);
	addkeyword(&context, 18, "typedef", 1);
	addkeyword(&context, 19, "struct", 1);
	addkeyword(&context, 20, "if", 1);
	addkeyword(&context, 21, "else", 1);
	addkeyword(&context, 22, "switch", 1);
	addkeyword(&context, 23, "while", 1);
	addkeyword(&context, 24, "for", 1);
	addkeyword(&context, 25, "case", 1);
	addkeyword(&context, 26, "default", 1);
	addkeyword(&context, 27, "sizeof", 1);

	// Macro
	addkeyword(&context, 28, "#include", 7);
	addkeyword(&context, 29, "#define", 7);
	addkeyword(&context, 30, "#undef", 7);
	addkeyword(&context, 31, "#ifdef", 7);
	addkeyword(&context, 32, "#endif", 7);

	// Constants
	addkeyword(&context, 33, "NULL", 2);
	addkeyword(&context, 34, "true", 2);
	addkeyword(&context, 35, "false", 2);
	addkeyword(&context, 36, "TRUE", 2);
	addkeyword(&context, 37, "FALSE", 2);
	addkeyword(&context, 38, "INFINITY", 2);

	// Comments
	addkeyword(&context, 39, "//", 5);

	// Standard Functions:
	addkeyword(&context, 40, "realloc", 4 + 7);
	addkeyword(&context, 41, "malloc", 4 + 7);
	addkeyword(&context, 42, "free", 4 + 7);
	addkeyword(&context, 43, "memcpy", 4 + 7);
	addkeyword(&context, 44, "memmove", 4 + 7);
	addkeyword(&context, 45, "memset", 4 + 7);
	addkeyword(&context, 46, "strcmp", 4 + 7);
	addkeyword(&context, 47, "strncmp", 4 + 7);
	addkeyword(&context, 48, "printf", 4 + 7);
	addkeyword(&context, 49, "scanf", 4 + 7);
	addkeyword(&context, 50, "exit", 4 + 7);
	addkeyword(&context, 51, "nanosleep", 4 + 7);
	addkeyword(&context, 52, "sleep", 4 + 7);
	addkeyword(&context, 53, "strlen", 4 + 7);
	addkeyword(&context, 54, "strcat", 4 + 7);

	memset(filename, 0, 1024);
	if(argc == 2) {
		memcpy(filename, argv[1], strlen(argv[1]));
		sct_file_load(filename, text);
	}else{
		memcpy(filename, UNTITLED, strlen(UNTITLED));
	}

	cbreak();
	noecho();
	nodelay(context.w, TRUE);
	keypad(context.w, TRUE);
	meta(context.w, TRUE);
	nodelay(context.w, TRUE);
	curs_set(FALSE);
	raw();
	mousemask(ALL_MOUSE_EVENTS, NULL);

	// Colors
	start_color();
	init_color(COLOR_BLACK, 0, 0, 0);
/*	init_color(COLOR_WHITE, 1000, 1000, 1000);
	init_color(COLOR_RED, 1000, 0, 0);
	init_color(COLOR_GREEN, 0, 1000, 0);
	init_color(COLOR_BLUE, 0, 0, 1000);
	init_color(COLOR_YELLOW, 0, 1000, 0);
	init_color(COLOR_CYAN, 0, 1000, 1000);
	init_color(COLOR_MAGENTA, 1000, 1000, 0);*/
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_BLUE, COLOR_BLACK);
	init_pair(6, COLOR_CYAN, COLOR_BLACK);
	init_pair(7, COLOR_MAGENTA, COLOR_BLACK);

	sct_title(width, height, filename, saved);
	sct_draw(&context, text, line, sln);
	sct_notify(&context, "Started Science's Creamy Text Editor!");
	while(running) {
		int32_t chr = getch();
		if(chr != ERR) {
			const char* name = keyname(chr);

		SCT_DETECT:;
			// Close
			if(strcmp(name, "^Q") == 0) {
				sct_exit(&running, "EXITING ON QUIT");
			} else if(strcmp(name, "^W") == 0) {
				sct_exit(&running, "EXITING ON FILE CLOSE");
			// Clipboard
			} else if(strcmp(name, "^C") == 0) {
				sct_notify(&context, "Copy.");
			} else if(strcmp(name, "^V") == 0) {
				sct_notify(&context, "Paste.");
			} else if(strcmp(name, "^X") == 0) {
				sct_notify(&context, "Cut.");
			// Save
			}else if(strcmp(name,"^S")==0 || strcmp(name,"^E")==0) {
				if(strcmp(filename, UNTITLED) == 0) {
					if(sct_popup(&context, "Save As...", filename))
					{
						sct_draw(&context, text, line, sln);
						continue;
					}
					// Redraw screen
					sct_draw(&context, text, line, sln);
				}
				sct_file_save(filename, text);
				saved = 1;
				sct_title(width,height,filename,saved);
				sct_notify(&context, "Saved file.");
				if(strcmp(name, "^E") == 0)
					sct_exit(&running, "SAVE & QUIT");
			} else if(strcmp(name, "^O") == 0) {
				if(sct_popup(&context, "Open What File?", filename)) {
					sct_draw(&context, text, line, sln);
					continue;
				}
				while(text->prev != NULL)
					text = text->prev;
				if(sct_file_load(filename, text)) {
					sct_notify(&context, "Failed to open file.");
				}else{
					sct_notify(&context, "Successfully loaded file.");
				}
				saved = 1, sln = 0, context.cursorx = 0,
					context.cursory = 0,
					context.selectx = 0,
					context.selecty = 0, line = text;
				sct_title(width, height, filename, saved);
				// Redraw screen
				sct_draw(&context, text, line, sln);
			// Functions
			} else if(strcmp(name, "KEY_BACKSPACE") == 0) {
				context.cursorx--;
				if(context.cursorx < 0) {
					if(line->tabs > 0) {
						line->tabs--;
						context.cursorx = 0;
					}else if(line->prev != NULL) {
						line = sct_backspace(&context,
							&text, line, &sln);
						context.cursorx = strlen(line->text);
					}else{
						context.cursorx = 0;
					}
				}else{
					line->text[context.cursorx] = '\0';
					if(line->text[context.cursorx + 1])
						memmove(line->text +
							context.cursorx,
							line->text +
							context.cursorx +1,
							strlen(line->text +
							 context.cursorx + 1)+1);
				}
				sct_draw(&context, text, line, sln);
				// Changed
				if(saved) {
					saved = 0;
					sct_title(width,height,filename,saved);
				}
				sct_notify(&context, "Edit.");
			} else if(strcmp(name, "KEY_DC") == 0) {
				sct_exit(&running, "Delete");
//				line = sct_backspace(line->next);
				if(saved) {
					saved = 0;
					sct_title(width,height,filename,saved);
				}
				sct_notify(&context, "Edit.");
			} else if(strcmp(name, "^D") == 0) {
				if(line->prev != NULL) {
					// Delete non-first line
					line = sct_backspace(&context, &text,
						line, &sln);
//					if(line->next)
//						line = line->next;
//					else
//						sct_cursorup();
					context.cursorx = strlen(line->text);
				}else if(line->next != NULL) {
					// Delete 1st line
					text = textlist_delete(line);
					line = text;
					context.cursorx = 0;
				}else{
					// Delete last line left
					memset(text->text, '\0', 81);
					context.cursorx = 0;
				}
				sct_draw(&context, text, line, sln);
				// Changed
				if(saved) {
					saved = 0;
					sct_title(width,height,filename,saved);
				}
				sct_notify(&context, "Edit.");
			} else if(strcmp(name, "^I") == 0) {
				// Tab
				line->tabs++;
				if(line->tabs > 8) line->tabs = 8;
				sct_draw(&context, text, line, sln);
				// Input
				if(saved) {
					saved = 0;
					sct_title(width,height,filename,saved);
				}
				sct_notify(&context, "Edit.");
			} else if(strcmp(name, "KEY_BTAB") == 0) {
				// Shift - Tab
				line->tabs--;
				if(line->tabs < 0) line->tabs = 0;
				sct_draw(&context, text, line, sln);
				// Input
				if(saved) {
					saved = 0;
					sct_title(width,height,filename,saved);
				}
				sct_notify(&context, "Edit.");
			} else if(strcmp(name, "^J") == 0) {
				// Newline
				textlist_insert(line);
				line = sct_nextline(&context, line, &text, &context.cursory, &sln);
				context.cursorx = 0;
				sct_draw(&context, text, line, sln);
				// Input
				if(saved) {
					saved = 0;
					sct_title(width,height,filename,saved);
				}
				sct_notify(&context, "Edit.");
			} else if(strcmp(name, "^[") == 0) {
				int32_t chrtr;
				const char* which = NULL;

				sct_notify(&context, "w or s - save, q - quit, e - save &"
					" quit, o - open, d - delete line");
				while(1) {
					chrtr = getch();
					which = keyname(chrtr);
					if(strlen(which) == 1 && chrtr != ERR)
						break;
				}
				if(which[0] == 'w' || which[0] == 's') {
					name = "^S";
					goto SCT_DETECT;
				}else if(which[0] == 'q') {
					name = "^Q";
					goto SCT_DETECT;
				}else if(which[0] == 'e') {
					name = "^E";
					goto SCT_DETECT;
				}else if(which[0] == 'o') {
					name = "^O";
					goto SCT_DETECT;
				}else if(which[0] == '^') {
					continue;
				}else if(which[0] == 'd') {
					name = "^D";
					goto SCT_DETECT;
				}else{
					sct_notify(&context, "unknown function.");
				}
				// Alt + other
			} else if(strcmp(name, "KEY_UP") == 0) {
				line = sct_cursorup(&context, &text, line, &sln);
				context.selectx = 0;
			} else if(strcmp(name, "KEY_DOWN") == 0) {
				line = sct_cursordn(&context, &text, line, &sln, 1);
				context.selectx = 0;
			} else if(strcmp(name, "KEY_RIGHT") == 0) {
				if(context.cursorx >= strlen(line->text)) {
					line = sct_cursordn(&context, &text, line, &sln, 0);
				}else{
					context.cursorx++;
					sct_draw(&context, text, line, sln);
				}
				context.selectx = 0;
				context.selecty = 0;
			} else if(strcmp(name, "KEY_LEFT") == 0) {
				if(context.cursorx <= 0) {
					line = sct_cursorup(&context, &text, line, &sln);
				}else{
					context.cursorx--;
					sct_draw(&context, text, line, sln);
				}
				context.selectx = 0;
				context.selecty = 0;
			} else if(strcmp(name, "KEY_PPAGE") == 0) {
				// Page UP
			} else if(strcmp(name, "KEY_NPAGE") == 0) {
				// Page DN
			} else if(strcmp(name, "KEY_MOUSE") == 0) {
				MEVENT event;

				getmouse(&event);
				if(event.bstate & BUTTON4_PRESSED) {
					name = "KEY_UP";
					goto SCT_DETECT;
				}else if(event.bstate & BUTTON2_PRESSED ||
					event.bstate == 0x8000000 ||
					event.bstate ==	0x0200000)
				{
					name = "KEY_DOWN";
					goto SCT_DETECT;
				}
				context.cursory = sct_mouseinputy(event.y);
				line = text;
				for(int i = 0; i < context.cursory; i++) {
					if(line->next) {
						line = line->next;
					}else{
						context.cursory = i;
					}
				}
				context.cursorx = sct_mouseinputx(line,event.x);
				if(event.bstate==BUTTON1_DOUBLE_CLICKED) {
					uint8_t erase = 1;
					context.selecty = 0;
					// Select Word
					while(context.cursorx > 0) {
						char c = line->text[context.cursorx];
						if(c == ' ' || c == ',') {
							erase = 0;
							break;
						}
						context.cursorx--;
					}
					context.cursorx++;
					for(int i = context.cursorx;;i++) {
						char c = line->text[i];
						if(c == ' ' || c == '\0' ||
						 c == ',')
						{
							context.selectx =
							    i - context.cursorx;
							break;
						}
					}
					context.cursorx -= erase;
					context.selectx += erase;
				}else if(event.bstate==BUTTON1_TRIPLE_CLICKED) {
					// Select Line
					context.selecty = 1;
					context.cursorx = 0;
					context.selectx = 0;
				}else{
					context.selectx = 0;
					context.selecty = 0;
				}
				sct_draw(&context, text, line, sln);
			} else if(strcmp(name, "KEY_RESIZE") == 0) {
				width = getmaxx(context.w);
				height = getmaxy(context.w);
				context.lh = height - 2;
				sct_title(width, height, filename, saved);
				sct_draw(&context, text, line, sln);
				sct_notify(&context, "Resized.");
			} else {
				sct_insert(&context, text, line, &context.cursorx, &context.cursory,
					name[0], sln);
				if(saved) {
					saved = 0;
					sct_title(width,height,filename,saved);
				}
				sct_notify(&context, "Edit.");
				// Character Insert
//				printf("NAME: %s", name);
			}
		}
	}
	endwin();
	// Return 0 on success.
	return 0;
}
