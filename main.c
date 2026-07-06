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
void edit_input(PANEL *, int, int, char **);
void print_main(PANEL *);
void p_refresh(void);
void get_file_name(PANEL *, char *);
int collect_text(WINDOW *, char *);

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
	int cur_y, cur_x, esc;
	//int , ch, i;
	char line_buf[LINELEN];
	//i =
	esc = 0;
	getyx(main, cur_y, cur_x);
	cur_x = 0;
	wmove(main, cur_y, cur_x);
	while (esc < 1) {
		esc = collect_text(main, line_buf);
		if(esc < 2){
		strcpy(*store_cur, line_buf);
		filebuf[line_counter++] = *store_cur;
		(*store_cur)+= LINELEN;
		}
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

void get_file_name(PANEL *cmd_p, char *buff){
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
}

int collect_text(WINDOW *win, char *lin_buf){
	// Idea to create a cursor for last entered text index
	// if arrows are used to move ahead or behind the current index that
	// original spot from before the arrows needs to be recorded
	int ch, cur_y, cur_x, i, esc;
	getyx(win, cur_y, cur_x);
	i = esc = 0;
	int last_cur = 0; // should always be the index after the furthest most right char in line buf
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
				while (lin_buf[i++] != '\0')
					;
				lin_buf[i - 1] = '\n';
				lin_buf[i] = '\0';
				break;
			}
			else{
				esc = 2;
			}
			break;
		}
		if(ch == KEY_BACKSPACE || ch == '\b'){
			if(i > 0) --i;
			getyx(win, cur_y, cur_x);
			mvwaddch(win, cur_y, cur_x - 1, ' ');
			wmove(win, cur_y, cur_x - 1);
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
		if(last_cur == 0 || i == last_cur){
			lin_buf[i++] = ch;
			waddch(win, ch);
			last_cur = i; // now last_cur is at one ahead of last char entered
		}
		else if(i < last_cur){
			// grab char between 
			int ch_1, ch_2, offset;
			ch_1 = lin_buf[i]; // previously entered ch at insertion point
			ch_2 = lin_buf[i + 1]; // char after that one that ch_1 will replace
			lin_buf[i++] = ch; // insert collected char and vance index, i is at ch_2
			mvwinsch(win, cur_y, i - 1, ch);
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
		if(ch == '\n'){
			lin_buf[i] = '\0';
			break;
		}
	}
	return esc;
}
