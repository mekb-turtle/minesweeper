#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <signal.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <poll.h>
#define SIZE_I 20
#define SIZE_J 20
#define MINES 40
#define MINE_TEXT "\x1b[38;5;9m@"
#define FLAG1_TEXT "\x1b[38;5;11m!"
#define FLAG2_TEXT "\x1b[38;5;11m?"
#define NOSTEP_TEXT "\x1b[38;5;7m#"
#define BLANK_SPACE "\x1b[38;5;7m."
#define BORDER_COLOR "\x1b[1;38;5;15m"
#define BORDER_TOP "┌─┐"
#define BORDER_LEFT "│"
#define BORDER_RIGHT "│"
#define BORDER_BOTTOM "└─┘"
#define NUMBER_COLOR "\x1b[1m"
#define _1_COLOR "\x1b[38;5;12m"
#define _2_COLOR "\x1b[38;5;10m"
#define _3_COLOR "\x1b[38;5;9m"
#define _4_COLOR "\x1b[38;5;4m"
#define _5_COLOR "\x1b[38;5;1m"
#define _6_COLOR "\x1b[38;5;6m"
#define _7_COLOR "\x1b[38;5;0m"
#define _8_COLOR "\x1b[38;5;7m"
#define RESET "\x1b[0m"
#define puts(a) fputs(a, stdout)
#define putc(a) fputc(a, stdout)
#define new_line() { ++new_lines; puts("\x1b[0m\r\n"); }
int new_lines;
struct pos {
	uint8_t i, j;
} sel = { .i = SIZE_I/2, .j = SIZE_J/2 };
struct tile {
	bool mine;
	bool step;
	uint8_t flag;
} board[SIZE_I][SIZE_J];
uint8_t get_neighbours(uint8_t i_, uint8_t j_, struct pos** out, bool invert) {
	if (out) *out = malloc(8 * sizeof(struct pos));
	uint8_t c = 0;
	for (uint8_t i = i_ == 0 ? 0 : i_ - 1; i_ >= SIZE_I - 1 ? i < SIZE_I : i <= i_ + 1; ++i)
	for (uint8_t j = j_ == 0 ? 0 : j_ - 1; j_ >= SIZE_J - 1 ? j < SIZE_J : j <= j_ + 1; ++j) {
		if (i>=SIZE_I||j>=SIZE_J)
		printf("%i %i\n",i,j);
		if (board[i][j].mine != invert) {
			if (out) (*out)[c] = (struct pos) { .i = i, .j = j };
			++c;
		}
	}
	return c;
}
bool game_over = 0;
bool won = 0;
void render_tile(uint8_t i, uint8_t j, uint8_t y) {
	uint8_t n = get_neighbours(i, j, NULL, 0);
	bool p = sel.i == i && sel.j == j;
	putc(' ');
	if (y == 0) {
		puts(p ? RESET BORDER_COLOR BORDER_TOP    RESET : RESET "\x1b[3C" RESET);
		return;
	}
	if (y == 2) {
		puts(p ? RESET BORDER_COLOR BORDER_BOTTOM RESET : RESET "   " RESET);
		return;
	}
	puts(RESET);
	if (p)
		puts(BORDER_COLOR BORDER_LEFT);
	else
		puts(" ");
	puts(RESET);
	if (game_over && board[i][j].mine) board[i][j].step = 1;
	if (board[i][j].step) {
		if (board[i][j].mine) {
			puts(MINE_TEXT);
		} else if (n == 0) {
			puts(BLANK_SPACE);
		} else {
			puts(NUMBER_COLOR);
			switch (n) {
				case 1: puts(_1_COLOR); break;
				case 2: puts(_2_COLOR); break;
				case 3: puts(_3_COLOR); break;
				case 4: puts(_4_COLOR); break;
				case 5: puts(_5_COLOR); break;
				case 6: puts(_6_COLOR); break;
				case 7: puts(_7_COLOR); break;
				case 8: puts(_8_COLOR); break;
			}
			putc(n + '0');
		}
	} else if (board[i][j].flag == 1) {
		puts(FLAG1_TEXT);
	} else if (board[i][j].flag == 2) {
		puts(FLAG2_TEXT);
	} else {
		puts(NOSTEP_TEXT);
	}
	puts(RESET);
	if (p)
		puts(BORDER_COLOR BORDER_RIGHT);
	else
		putc(' ');
	puts(RESET);
	fflush(stdout);
}
void render_row(uint8_t i, uint8_t y) {
	for (uint8_t j = 0; j < SIZE_J; ++j) {
		render_tile(i, j, y);
	}
}
bool turn = 0;
void render_board() {
	if (turn) {
		for (int j = 0; j < new_lines; ++j) printf("\x1bM");
	}
	turn = 1;
	new_lines = 0;
	new_line();
	new_line();
	puts("\x1b[2K");
	for (uint8_t i = 0; i < SIZE_I; ++i) {
		render_row(i, 0);
		new_line();
		render_row(i, 1);
		new_line();
		render_row(i, 2);
		puts("\n\x1bM");
	}
	new_line();
	new_line();
}
void clear_board() {
	for (uint8_t i = 0; i < SIZE_I; ++i)
	for (uint8_t j = 0; j < SIZE_J; ++j) {
		board[i][j] = (struct tile) { .mine = 0, .step = 0, .flag = 0 };
	}
}
unsigned int seedp;
int rand_() {
	return rand_r(&seedp);
}
void set_random(uint8_t i_, uint8_t j_) {
	uint8_t i, j;
	while (1) {
		i = rand_() % SIZE_I;
		j = rand_() % SIZE_J;
		if (i == i_ && j == j_) continue;
		if (board[i][j].mine) continue;
		board[i][j].mine = 1;
		break;
	}
}
void set_all_random(uint8_t i_, uint8_t j_) {
	assert(MINES < SIZE_I * SIZE_J);
	for (uint8_t i = 0; i < MINES; ++i) {
		set_random(i_, j_);
	}
}
#define TILE board[s.i][s.j]
bool flag(struct pos s) {
	if (TILE.step) return 0;
	TILE.flag = (TILE.flag + 1) % 3;
	return 1;
}
void shuffle(struct pos *a, size_t n) {
	if (n > 1) {
		for (size_t i = 0; i < n - 1; ++i) {
			size_t j = (rand_() / (RAND_MAX / (n - i) + 1)) + i;
			struct pos t = a[j];
			a[j] = a[i];
			a[i] = t;
		}
	}
}
bool step(struct pos s) {
	if (TILE.step || TILE.flag) return 0;
	TILE.flag = TILE.step = 1;
	if (TILE.mine) game_over = 1;
	if (get_neighbours(s.i, s.j, NULL, 0) > 0) return 0;
	struct pos *a;
	uint8_t n = get_neighbours(s.i, s.j, &a, 1);
	shuffle(a, n);
	for (uint8_t i = 0; i < n; ++i) {
		if (step(a[i])) render_board();
	}
	return 1;
}
bool check_win() {
	won = 1;
	for (uint8_t i = 0; i < SIZE_I; ++i)
	for (uint8_t j = 0; j < SIZE_J; ++j) {
		if (board[i][j].mine == board[i][j].step) { won = 0; break; }
	}
	return won;
}
struct termios term_old;
bool is_tty;
void end(int code) {
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term_old) != 0) code = code || 1;
	exit(code);
}
void end1() { end(1); }
bool started = 0;
int poll_() {
	struct pollfd fds;
	fds.fd = STDIN_FILENO;
	fds.events = POLLIN;
	return poll(&fds, 1, 0);
}
int main() {
	struct timeval tv;
	if (gettimeofday(&tv, NULL) != 0) return 1;
	seedp = tv.tv_usec;
	is_tty = isatty(STDIN_FILENO);
	if (is_tty) {
		struct termios term_new;
		if (tcgetattr(STDIN_FILENO, &term_old) != 0) return 1;
		term_new = term_old;
		term_new.c_lflag &= ~(ICANON | ISIG | ECHOK | ECHONL | ECHO | ECHOKE | IEXTEN);
		if (tcsetattr(STDIN_FILENO, TCSANOW, &term_new) != 0) return 1;
	}
	signal(SIGTSTP, end1);
	signal(SIGINT,  end1);
	signal(SIGSTOP, end1);
	signal(SIGTERM, end1);
	signal(SIGHUP,  end1);
	clear_board();
	while (1) {
		render_board();
		if (game_over) { printf("Game over!\n"); break; }
		if (won) { printf("You won!\n"); break; }
		while (poll_() > 0) {
			fgetc(stdin);
		}
		while (1) {
			int b = fgetc(stdin);
			if (b < 0 || b == 4 || b == 26 || b == 17) end(1); else
			if (b == 3) end(0); else
			if (b == 'w' || b == 'A') {
				if (sel.i > 0)          { --sel.i; break; }} else
			if (b == 's' || b == 'B') {
				if (sel.i < SIZE_I - 1) { ++sel.i; break; }} else
			if (b == 'd' || b == 'C') {
				if (sel.j < SIZE_J - 1) { ++sel.j; break; }} else
			if (b == 'a' || b == 'D') {
				if (sel.j > 0)          { --sel.j; break; }} else
			if (b == 'h' || b == 'H') { 
				if (!started) {
					set_all_random(sel.i, sel.j);
					started = 1;
				}
				step(sel);
				check_win();
				break;
			}
			if (b == 'f' || b == 'F') { flag(sel); break; }
		}
	}
	end(0);
	return 0;
}
