#include <unistd.h>
#include <curses.h>
#include <panel.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINELEN 256
#define NUMLINES 256

int line_counter = 0;

bool load(char *, char (*)[LINELEN]);
bool save_file(char *, char (*)[LINELEN]);
int command_input(PANEL *);
void insert_input(PANEL *, char (*)[LINELEN], int, int);
void edit_input(PANEL *, int, int, char **);
void print_main(PANEL *, char (*)[LINELEN]);
void p_refresh(void);
void get_file_name(PANEL *, char *);
int collect_text(WINDOW *, char *, int, int);

int main(int argc, char *argv[]){

	// int gdb = 1; // for debugging
	//  while (gdb) {
	// 	// enter new value in gdb
	// }
	char *file_name;
	bool file_loaded = false;

	char (*store)[LINELEN] = malloc(sizeof(*store) * NUMLINES); // cannot make it a global bc malloc
	
	if(argc > 1){
		file_name = *(++argv);
		file_loaded = load(file_name, store);
	}

	// Setup ncurses
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	if(has_colors()){
		start_color();
		init_pair(1, COLOR_GREEN, COLOR_BLACK);
		init_pair(2, COLOR_CYAN, COLOR_BLACK);
	}
	refresh();
	WINDOW *main = newwin(LINES - 1, COLS, 0, 0);
	WINDOW *cmd = newwin(1 , COLS, LINES - 1, 0);
	wbkgd(main, COLOR_PAIR(1));
	wbkgd(cmd, COLOR_PAIR(2));
	keypad(main, TRUE);
	keypad(cmd, TRUE);

	PANEL *main_p = new_panel(main);
	PANEL *cmd_p = new_panel(cmd);

	hide_panel(cmd_p);

	int cur_y, cur_x;

	if(file_loaded){
		print_main(main_p, store);
	}
	while (1) {
		int ch = wgetch(main);
		if(ch == ':'){
			int res;
			res = command_input(cmd_p);
			if(res < 0){
				break;
			}
			hide_panel(cmd_p);
		}
		if(ch == 'i'){
			// implement insert mode function
			getyx(main, cur_y, cur_x);
			insert_input(main_p, store, cur_y, cur_x);
		}
		getyx(main, cur_y, cur_x);
		switch (ch) {
			case 'j':
				cur_y++;
				break;
			case 'h':
				if(cur_x > 0) cur_x--;
				break;
			case 'k':
				if(cur_y > 0) cur_y--;
				break;
			case 'l':
				cur_x++;
				break;
		}
		wmove(main, cur_y, cur_x);
		p_refresh();
	}

	char save_name[256];
	if(!file_loaded){
		// get a string 
		get_file_name(cmd_p, save_name);
	}

	endwin();
	if(!file_loaded){
		save_file(save_name, store);
	}
	else{
		save_file(file_name, store);
	}
	free(store);
	return 0;
}

void print_main(PANEL *man_p, char (*store)[LINELEN]){
	WINDOW *main = panel_window(man_p);
	wclear(main);
	wmove(main, 0, 0);
	for(int i = 0; i < line_counter; i++){
		wprintw(main, "%s", store[i]);
	}

}

int command_input(PANEL *cmd_p){
	show_panel(cmd_p);
	WINDOW *cmd = panel_window(cmd_p);
	wclear(cmd);
	mvwaddch(cmd, 0, 0, ':');
	p_refresh();

	int ch, prev;
	while (1) {
		ch = wgetch(cmd);
		if(ch == KEY_BACKSPACE || ch == '\b'){
			int cur_y, cur_x;
			getyx(cmd, cur_y, cur_x);
			mvwaddch(cmd, cur_y, cur_x - 1, ' ');
			wmove(cmd, cur_y, cur_x - 1);
			p_refresh();
			continue;
		}
		waddch(cmd, ch);
		p_refresh();
		if(ch == '\n'){
			if(prev == 'q')
				return -1;
			else
				return 0;
		}
		prev = ch;

		
	}
	return 0;
}

void insert_input(PANEL *main_p, char (*store)[LINELEN], int cur_y, int cur_x){
	WINDOW *main = panel_window(main_p);
	int esc;
	char line_buf[LINELEN];
	esc = 0;
	//getyx(main, cur_y, cur_x);
	while (esc < 1) {
		if(cur_y > line_counter){
			while (line_counter < cur_y) {
				strcpy(store[line_counter++], "\n"); // populate blank lines inbetween with new line
			}
			cur_x = 0;
			wmove(main, cur_y, cur_x); // move to start of line so we don't have to prepend spaces, might change later
		}
		if(cur_y < line_counter){
			strcpy(line_buf, store[cur_y]); // preload text into buffer
		}
		esc = collect_text(main, line_buf, cur_y, cur_x);
		if(esc < 2 && cur_y == line_counter){
			strcpy(store[line_counter++], line_buf);
		}
		else if(esc < 2 && cur_y < line_counter){
			strcpy(store[cur_y], line_buf);
		}
		getyx(main, cur_y, cur_x);
	}
}

bool load(char *file_name, char (*store)[LINELEN]){

	FILE *fp = fopen(file_name, "r");
	if(fp == NULL){
		return false;
	}
	while (fgets(store[line_counter++], LINELEN, fp))
		;
	
	fclose(fp);
	return true;
}

bool save_file(char *file_name, char (*store)[LINELEN]){
	FILE *fp;
	fp = fopen(file_name, "w");
	if(fp == NULL){
		printf("ERROR in saving %s\n", file_name);
		return false;
	}
	int i = 0;
	while (i < line_counter) {
		fputs(store[i++], fp);
	}
	fclose(fp);
	return true;
}

void p_refresh(void){
	update_panels();
	doupdate();
}

void get_file_name(PANEL *cmd_p, char *buff){
	WINDOW *cmd = panel_window(cmd_p);

	wclear(cmd);
	waddstr(cmd, ": add file name");
	p_refresh();
	wmove(cmd, 0, 1);
	int i, ch;
	i = 1;
	//char line_buff[256];
	while (i) {
		ch = wgetch(cmd);
		waddch(cmd, ch);
		wclrtoeol(cmd);
		buff[0] = ch;
		buff[1] = '\0';
		i = collect_text(cmd, buff, -1, 1);
	}
	int j = 0;
	while(buff[j] != '\n'){
		if(buff[j] == ' '){
			buff[j] = '_';
		}
		j++;
	}
	buff[j] = '\0';
}

int collect_text(WINDOW *win, char *lin_buf, int call_y, int call_x){
	int ch, cur_y, cur_x, i, esc;
	getyx(win, cur_y, cur_x);
	i = esc = 0;
	int last_cur = 0; // should always be the index after the furthest most right char in line buf
	if(call_y < line_counter){
		i = call_x;
		last_cur = strlen(lin_buf) ; //add back + 1 if causes issues strlen does not include null char
		if(strcmp(lin_buf, "\n") == 0)
			last_cur = 0;
	}
	while (i < LINELEN && last_cur < LINELEN) {
		ch = wgetch(win);
		// Characters not selected to store
		if(ch == 27){
			esc = 1;
			// maybe edit linbuf here
			if(i > 0 && i == last_cur){
				lin_buf[i++] = '\n';
				lin_buf[i] = '\0';
			}
			else if(i < last_cur){
				while (lin_buf[i++] != '\0') // when we match null char we still advance i one more space
					;
				if(lin_buf[i - 2] != '\n'){// so we need to check 2 spaces behind if i already equal newline
					lin_buf[i - 1] = '\n'; // if true replace null with new line then add null
					lin_buf[i] = '\0';
				}
				break;
			}
			else{
				esc = 2;
			}
			break;
		}
		if(ch == KEY_BACKSPACE || ch == '\b'){
			if(i == last_cur && i > 0){
				--last_cur;
				--i;
				getyx(win, cur_y, cur_x);
				wmove(win, cur_y, cur_x - 1); // this makes the display side make sense
				wdelch(win);
				lin_buf[i] = '\0';
			}
			else if(i < last_cur){
				wdelch(win);
				getyx(win, cur_y, cur_x);
				wmove(win, cur_y, cur_x - 1); // this makes the display side make sense
				int ch_1, offset;
				offset = i;
				ch_1 = lin_buf[offset + 1];
				while (ch_1 != '\0') {
					lin_buf[offset++] = ch_1;
					ch_1 = lin_buf[offset + 1];
				}
				lin_buf[offset] = '\0';
				--i;
			}
			continue;
		}
		if(ch > 255){
			switch (ch) {
				case KEY_UP:
				case KEY_LEFT:
					lin_buf[last_cur] = '\0';
					if(i > 0){
						--i;
					} 
					getyx(win, cur_y, cur_x);
					wmove(win, cur_y, cur_x - 1);
					break;
				case KEY_DOWN:
				case KEY_RIGHT:
					if(i < LINELEN){
						++i;
					} 
					getyx(win, cur_y, cur_x);
					wmove(win, cur_y, cur_x + 1);
					break;
			}
			continue;
		}
		// Add chars to lin_buf
		if(i < last_cur){
			// handle display
			mvwinsch(win, cur_y, i, ch);
			move(cur_y, i + 1);
			// handle buffer
			int ch_1, ch_2, offset;
			ch_1 = lin_buf[i]; // previously entered ch at insertion point
			ch_2 = lin_buf[i + 1]; // char after that one that ch_1 will replace
			lin_buf[i++] = ch; // insert collected char and advance index, i is at ch_2
			offset = i;// offset equals insertion point of next char which should equal ch_2
			while(ch_1 != '\0' && offset < LINELEN){
				lin_buf[offset] = ch_1;// insert ch_1 to next spot and advance offset, which means offset points
				// to 1 past ch_2?
				ch_1 = ch_2;// put ch_2 into ch_1 for next insert
				ch_2 = lin_buf[++offset];// assign ch_2 
			}
			lin_buf[offset] = ch_1;
			last_cur = offset;
		}
		else if(i > last_cur){
			// fill line_buf at indexes between last_cur and i with ' '
			for(int j = last_cur; j < i; j++){
				lin_buf[j] = ' ';
			}
			lin_buf[i++] = ch;
			waddch(win, ch);
			last_cur = i;
		}
		else if(i == last_cur){
			lin_buf[i++] = ch;
			waddch(win, ch);
			last_cur = i; // now last_cur is at one ahead of last char entered
		}
		if(ch == '\n'){
			lin_buf[i] = '\0';
			break;
		}
	}
	return esc;
}
