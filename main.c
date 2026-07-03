#include <curses.h>
#include <panel.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINELEN 256
#define NUMLINES 256

char *filebuf[NUMLINES];
int line_counter = 0;

bool load(char *, char **);
int command_input(PANEL *);
void print_main(PANEL *);

int main(int argc, char *argv[]){

	char *file_name;
	bool file_loaded = false;

	char *store = malloc(sizeof(char[LINELEN]) * NUMLINES);
	char *store_cur = store;
	
	if(argc > 1){
		file_name = *(++argv);
		file_loaded = load(file_name, &store_cur);
	}

	// Setup ncurses
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	if(has_colors()){
		start_color();
		init_pair(1, COLOR_GREEN, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
	}
	refresh();
	WINDOW *main = newwin(LINES - 1, COLS, 0, 0);
	WINDOW *cmd = newwin(1 , COLS, LINES - 1, 1);
	wbkgd(main, COLOR_PAIR(1));
	wbkgd(cmd, COLOR_PAIR(2));

	PANEL *main_p = new_panel(main);
	PANEL *cmd_p = new_panel(cmd);

	hide_panel(cmd_p);

	int cur_y, cur_x;

	if(file_loaded){
		print_main(main_p);
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
		move(cur_y, cur_x);
		update_panels();
		doupdate();
	}


	free(store);
	endwin();
	return 0;
}

void print_main(PANEL *man_p){
	WINDOW *main = panel_window(man_p);
	wmove(main, 0, 0);
	for(int i = 0; i < line_counter; i++){
		wprintw(main, "%s", filebuf[i]);
	}

}

int command_input(PANEL *cmd_p){
	show_panel(cmd_p);
	WINDOW *cmd = panel_window(cmd_p);
	mvwaddch(cmd, 0, 1, ':');
	update_panels();
	doupdate();
	int ch, prev;
	while (1) {
		ch = wgetch(cmd);
		waddch(cmd, ch);
		update_panels();
		doupdate();
		if(ch == 'q'){
			return -1;
		}
		
	}
	return 0;
}

bool load(char *file_name, char **store_cur){

	FILE *fp = fopen(file_name, "r");
	if(fp == NULL){
		return false;
	}
	char line_buff[LINELEN];
	while (fgets(line_buff, LINELEN, fp)) {
		strcpy(*store_cur, line_buff);
		filebuf[line_counter] = *store_cur;
		(*store_cur)+= LINELEN;
		line_counter++;
	}
	fclose(fp);
	return true;
}
