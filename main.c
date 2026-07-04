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
bool save_file(char *);
int command_input(PANEL *);
void insert_input(PANEL *, char **);
void print_main(PANEL *);
void p_refresh(void);
char *get_file_name(PANEL *, char *);

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
			insert_input(main_p, &store_cur);
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
		save_file(save_name);
	}
	else{
		save_file(file_name);
	}
	free(store);
	return 0;
}

void print_main(PANEL *man_p){
	WINDOW *main = panel_window(man_p);
	wclear(main);
	wmove(main, 0, 0);
	for(int i = 0; i < line_counter; i++){
		wprintw(main, "%s", filebuf[i]);
	}

}

int command_input(PANEL *cmd_p){
	show_panel(cmd_p);
	WINDOW *cmd = panel_window(cmd_p);
	wclear(cmd);
	mvwaddch(cmd, 0, 1, ':');
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

void insert_input(PANEL *main_p, char **store_cur){
	WINDOW *main = panel_window(main_p);
	int cur_y, cur_x, ch, i, esc;
	char line_buf[LINELEN];
	i = esc = 0;
	while (1) {
		ch = wgetch(main);
		if(ch == 27){
			esc = 1;		
			break;
		}
		if(ch == KEY_BACKSPACE || ch == '\b'){
			if(i > 0){
				i--;
				getyx(main, cur_y, cur_x);
				mvwaddch(main, cur_y, cur_x - 1, ' ');
				wmove(main, cur_y, cur_x - 1);
				p_refresh();
			}
			continue;
		}
		if(ch > 255){
			continue;
		}
		else {
			line_buf[i++] = ch;
			waddch(main, ch);
			p_refresh();
		}
		if(ch == '\n'){
			strcpy(*store_cur, line_buf);
			filebuf[line_counter++] = *store_cur;
			(*store_cur)+= LINELEN;
			i = 0;
			continue;
		}
	}
	if(esc && i > 0){
		line_buf[i++] = '\n';
		line_buf[i] = '\0';
		strcpy(*store_cur, line_buf);
		filebuf[line_counter++] = *store_cur;
		(*store_cur)+= LINELEN;
	}
	
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

bool save_file(char *file_name){
	FILE *fp;
	fp = fopen(file_name, "w");
	if(fp == NULL){
		printf("ERROR in saving %s\n", file_name);
		return false;
	}
	int i = 0;
	while (i < line_counter) {
		fputs(filebuf[i++], fp);
	}
	fclose(fp);
	return true;
}

void p_refresh(void){
	update_panels();
	doupdate();
}

char *get_file_name(PANEL *cmd_p, char *buff){
	WINDOW *cmd = panel_window(cmd_p);

	wclear(cmd);
	waddstr(cmd, ": add file name");
	p_refresh();
	wmove(cmd, 0, 2);
	int ch, i;
	i = 0;
	while (i < 256) {
		ch = wgetch(cmd);
		if(ch == KEY_BACKSPACE || ch == '\b'){
			int cur_y, cur_x;
			getyx(cmd, cur_y, cur_x);
			mvwaddch(cmd, cur_y, cur_x - 1, ' ');
			wmove(cmd, cur_y, cur_x - 1);
			p_refresh();
			continue;
		}
		if(ch > 255){
			continue;
		}
		if(ch == '\n'){
			buff[i] = '\0';
			break;
		}
		waddch(cmd, ch);
		buff[i++] =  ch;
		p_refresh();
		
	}
	return buff;

}
